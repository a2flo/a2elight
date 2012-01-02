/*
 *  Albion 2 Engine "light"
 *  Copyright (C) 2004 - 2012 Florian Ziesche
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License only.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "opencl.h"

#define __ERROR_CODE_INFO(F) \
F(CL_SUCCESS) \
F(CL_DEVICE_NOT_FOUND) \
F(CL_DEVICE_NOT_AVAILABLE) \
F(CL_COMPILER_NOT_AVAILABLE) \
F(CL_MEM_OBJECT_ALLOCATION_FAILURE) \
F(CL_OUT_OF_RESOURCES) \
F(CL_OUT_OF_HOST_MEMORY) \
F(CL_PROFILING_INFO_NOT_AVAILABLE) \
F(CL_MEM_COPY_OVERLAP) \
F(CL_IMAGE_FORMAT_MISMATCH) \
F(CL_IMAGE_FORMAT_NOT_SUPPORTED) \
F(CL_BUILD_PROGRAM_FAILURE) \
F(CL_MAP_FAILURE) \
F(CL_INVALID_VALUE) \
F(CL_INVALID_DEVICE_TYPE) \
F(CL_INVALID_PLATFORM) \
F(CL_INVALID_DEVICE) \
F(CL_INVALID_CONTEXT) \
F(CL_INVALID_QUEUE_PROPERTIES) \
F(CL_INVALID_COMMAND_QUEUE) \
F(CL_INVALID_HOST_PTR) \
F(CL_INVALID_MEM_OBJECT) \
F(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR) \
F(CL_INVALID_IMAGE_SIZE) \
F(CL_INVALID_SAMPLER) \
F(CL_INVALID_BINARY) \
F(CL_INVALID_BUILD_OPTIONS) \
F(CL_INVALID_PROGRAM) \
F(CL_INVALID_PROGRAM_EXECUTABLE) \
F(CL_INVALID_KERNEL_NAME) \
F(CL_INVALID_KERNEL_DEFINITION) \
F(CL_INVALID_KERNEL) \
F(CL_INVALID_ARG_INDEX) \
F(CL_INVALID_ARG_VALUE) \
F(CL_INVALID_ARG_SIZE) \
F(CL_INVALID_KERNEL_ARGS) \
F(CL_INVALID_WORK_DIMENSION) \
F(CL_INVALID_WORK_GROUP_SIZE) \
F(CL_INVALID_WORK_ITEM_SIZE) \
F(CL_INVALID_GLOBAL_OFFSET) \
F(CL_INVALID_EVENT_WAIT_LIST) \
F(CL_INVALID_EVENT) \
F(CL_INVALID_OPERATION) \
F(CL_INVALID_GL_OBJECT) \
F(CL_INVALID_BUFFER_SIZE) \
F(CL_INVALID_MIP_LEVEL)

#define __DECLARE_ERROR_CODE_TO_STRING(code) case code: return #code;

const char* opencl::error_code_to_string(cl_int error_code) {
	switch(error_code) {
		__ERROR_CODE_INFO(__DECLARE_ERROR_CODE_TO_STRING);
		default:
			return "UNKNOWN CL ERROR";
	}
}

#define __HANDLE_CL_EXCEPTION_START(func_str) __HANDLE_CL_EXCEPTION_START_EXT(func_str, "")
#define __HANDLE_CL_EXCEPTION_START_EXT(func_str, additional_info)									\
catch(cl::Error err) {																				\
	a2e_error("line #%s, " func_str "(): %s (%d: %s)%s!", \
			  size_t2string(__LINE__).c_str(), err.what(), err.err(), error_code_to_string(err.err()), additional_info);
#define __HANDLE_CL_EXCEPTION_END }
#define __HANDLE_CL_EXCEPTION(func_str) __HANDLE_CL_EXCEPTION_START(func_str) __HANDLE_CL_EXCEPTION_END
#define __HANDLE_CL_EXCEPTION_EXT(func_str, additional_info) __HANDLE_CL_EXCEPTION_START_EXT(func_str, additional_info) __HANDLE_CL_EXCEPTION_END

/*! creates a opencl object
 */
opencl::opencl(const char* kernel_path, file_io* f_, SDL_Window* wnd, const bool clear_cache) {
	opencl::f = f_;
	opencl::sdl_wnd = wnd;
	opencl::kernel_path_str = kernel_path;
	
	buffer = new stringstream(stringstream::in | stringstream::out);
	
	context = NULL;
	cur_kernel = NULL;
	active_device = NULL;
	
	fastest_cpu = NULL;
	fastest_gpu = NULL;
	
	// TODO: this currently doesn't work if there are spaces inside the path and surrounding
	// the path by "" doesn't work either, probably a bug in the apple implementation -- or clang?
	build_options = "-I" + kernel_path_str.substr(0, kernel_path_str.length()-1);
	build_options += " -cl-strict-aliasing";
	build_options += " -cl-mad-enable";
	build_options += " -cl-no-signed-zeros";
	build_options += " -cl-fast-relaxed-math";
	build_options += " -cl-single-precision-constant";
	build_options += " -cl-denorms-are-zero";
	build_options += " -w";
	
#ifndef __APPLE__
	//nv_build_options = " -cl-nv-verbose";
	//nv_build_options = " -check-kernel-functions";
	//nv_build_options += " -nvptx-mad-enable -inline-all";
#else
	build_options += " -cl-auto-vectorize-enable";
#endif
	
	// clear opencl cache
	if(clear_cache) {
#ifdef __APPLE__
		system("rm -R ~/Library/Caches/com.apple.opencl > /dev/null 2>&1");
		// TODO: delete app specific cache (~/Library/Caches/$identifier/com.apple.opencl)
#elif __WINDOWS__
		// TODO: find it (/Users/$user/AppData/Roaming/NVIDIA/ComputeCache)
#else
		system("rm -R ~/.nv/ComputeCache > /dev/null 2>&1");
#endif
	}
}

/*! opencl destructor
 */
opencl::~opencl() {
	a2e_debug("deleting opencl object");
	
	for(vector<opencl::buffer_object*>::iterator biter = buffers.begin(); biter != buffers.end(); biter++) {
		delete (*biter)->buffer;
	}
	buffers.clear();
	
	destroy_kernels();
	
	if(context != NULL) delete context;
	
	internal_devices.clear();
	devices.clear();
	
	delete buffer;
	
	a2e_debug("opencl object deleted");
}

void opencl::destroy_kernels() {
	for(auto& k : kernels) {
		delete k.second;
	}
	kernels.clear();
	cur_kernel = NULL;
}

void opencl::init(bool use_platform_devices, const size_t platform_index) {
	try {
		platform = new cl::Platform();
		platform->get(&platforms);
		
		if(platforms.size() > platform_index) {
			platforms[platform_index].getDevices(CL_DEVICE_TYPE_ALL, &internal_devices);
		}
		else {
			a2e_error("no opencl platform available!");
			return;
		}
		a2e_debug("%u opencl platform%s found!", platforms.size(), (platforms.size() > 1 ? "s" : ""));
		if(use_platform_devices) {
			a2e_debug("%u opencl device%s found!", internal_devices.size(), (internal_devices.size() > 1 ? "s" : ""));
		}
		
#ifdef __APPLE__
		cl_context_properties cl_properties[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[platform_index](),
			CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)CGLGetShareGroup(CGLGetCurrentContext()),
			0 };
		
		// create a context with all platform devices (this works fine since 10.7)
		context = new cl::Context(internal_devices, cl_properties, clLogMessagesToStdoutAPPLE, NULL, &ierr);
		
#else
		SDL_SysWMinfo wm_info;
		SDL_VERSION(&wm_info.version);
		if(SDL_GetWindowWMInfo(sdl_wnd, &wm_info) != 1) {
			a2e_error("couldn't get window manger info!");
			return;
		}
		
		// context with gl share group (cl/gl interop)
#if __WINDOWS__
		cl_context_properties cl_properties[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[platform_index](),
			CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
			CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
			0
		};
#else // Linux, hopefully *BSD too
		cl_context_properties cl_properties[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[platform_index](),
			CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
			CL_GLX_DISPLAY_KHR, (cl_context_properties)wm_info.info.x11.display,
			0
		};
#endif
		
		if(use_platform_devices) {
			context = new cl::Context(internal_devices, cl_properties, NULL, NULL, &ierr);
		}
		else {
			context = new cl::Context(CL_DEVICE_TYPE_ALL, cl_properties, NULL, NULL, &ierr);
		}
#endif
		
		internal_devices.clear();
		internal_devices = context->getInfo<CL_CONTEXT_DEVICES>();
		a2e_debug("%u opencl device%s found!", internal_devices.size(), (internal_devices.size() > 1 ? "s" : ""));
		
		a2e_debug("opencl context successfully created!");
		
		string dev_type_str;
		unsigned int gpu_counter = opencl::GPU0;
		unsigned int cpu_counter = opencl::CPU0;
		unsigned int fastest_cpu_score = 0;
		unsigned int fastest_gpu_score = 0;
		unsigned int cpu_score = 0;
		unsigned int gpu_score = 0;
		for(vector<cl::Device>::iterator diter = internal_devices.begin(); diter != internal_devices.end(); diter++) {
			dev_type_str = "";
			
			devices.push_back(new opencl::device_object());
			devices.back()->device = &*diter;
			devices.back()->internal_type = diter->getInfo<CL_DEVICE_TYPE>();
			devices.back()->units = diter->getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
			devices.back()->clock = diter->getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>();
			devices.back()->mem_size = diter->getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
			devices.back()->name = diter->getInfo<CL_DEVICE_NAME>();
			devices.back()->vendor = diter->getInfo<CL_DEVICE_VENDOR>();
			devices.back()->version = diter->getInfo<CL_DEVICE_VERSION>();
			devices.back()->driver_version = diter->getInfo<CL_DRIVER_VERSION>();
			devices.back()->extensions = diter->getInfo<CL_DEVICE_EXTENSIONS>();
			
			devices.back()->max_alloc = diter->getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
			devices.back()->max_wg_size = diter->getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
			devices.back()->img_support = diter->getInfo<CL_DEVICE_IMAGE_SUPPORT>() == 1;
			devices.back()->max_img_2d.set(diter->getInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>(), diter->getInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>());
			devices.back()->max_img_3d.set(diter->getInfo<CL_DEVICE_IMAGE3D_MAX_WIDTH>(), diter->getInfo<CL_DEVICE_IMAGE3D_MAX_HEIGHT>(), diter->getInfo<CL_DEVICE_IMAGE3D_MAX_DEPTH>());

			devices.back()->vendor_type = CLV_UNKNOWN;
			string vendor_str = core::str_to_lower(devices.back()->vendor);
			if(strstr(vendor_str.c_str(), "nvidia") != NULL) {
				devices.back()->vendor_type = CLV_NVIDIA;
			}
			else if(strstr(vendor_str.c_str(), "ati") != NULL) {
				devices.back()->vendor_type = CLV_ATI;
			}
			else if(strstr(vendor_str.c_str(), "amd") != NULL) {
				devices.back()->vendor_type = CLV_AMD;
			}
			else if(strstr(vendor_str.c_str(), "intel") != NULL) {
				devices.back()->vendor_type = CLV_INTEL;
			}
			
			if(devices.back()->internal_type & CL_DEVICE_TYPE_CPU) {
				devices.back()->type = (opencl::OPENCL_DEVICE)cpu_counter;
				cpu_counter++;
				dev_type_str += "CPU ";
				
				if(fastest_cpu == NULL) {
					fastest_cpu = devices.back();
					fastest_cpu_score = devices.back()->units * devices.back()->clock;
				}
				else {
					cpu_score = devices.back()->units * devices.back()->clock;
					if(cpu_score > fastest_cpu_score) {
						fastest_cpu = devices.back();
					}
				}
			}
			if(devices.back()->internal_type & CL_DEVICE_TYPE_GPU) {
				devices.back()->type = (opencl::OPENCL_DEVICE)gpu_counter;
				gpu_counter++;
				dev_type_str += "GPU ";
				
				if(fastest_gpu == NULL) {
					fastest_gpu = devices.back();
					fastest_gpu_score = devices.back()->units * devices.back()->clock;
				}
				else {
					gpu_score = devices.back()->units * devices.back()->clock;
					if(gpu_score > fastest_gpu_score) {
						fastest_gpu = devices.back();
					}
				}
			}
			if(devices.back()->internal_type & CL_DEVICE_TYPE_ACCELERATOR) {
				dev_type_str += "Accelerator ";
			}
			if(devices.back()->internal_type & CL_DEVICE_TYPE_DEFAULT) {
				dev_type_str += "Default ";
			}
			
			// TYPE (Units: %, Clock: %): Name, Vendor, Version, Driver Version
			a2e_debug("%s(Units: %u, Clock: %u MHz, Memory: %u MB): %s %s, %s/%s",
					 dev_type_str.c_str(),
					 devices.back()->units,
					 devices.back()->clock,
					 (unsigned int)(devices.back()->mem_size / 1024ul / 1024ul),
					 devices.back()->vendor.c_str(),
					 devices.back()->name.c_str(),
					 devices.back()->version.c_str(),
					 devices.back()->driver_version.c_str());
		}
		
		// create a (single) command queue for each device
		for(device_object* device : devices) {
			queues[device->device] = new cl::CommandQueue(*context, *device->device, 0, &ierr);
		}
		
		// make the first recognized device the active device
		//active_device = devices[0];
		set_active_device(opencl::FASTEST_GPU);
		
		if(fastest_cpu != NULL) a2e_debug("fastest CPU device: %s %s (score: %u)", fastest_cpu->vendor.c_str(), fastest_cpu->name.c_str(), fastest_cpu_score);
		if(fastest_gpu != NULL) a2e_debug("fastest GPU device: %s %s (score: %u)", fastest_gpu->vendor.c_str(), fastest_gpu->name.c_str(), fastest_gpu_score);
		
		load_internal_kernels();
	}
	__HANDLE_CL_EXCEPTION_START("init")
		// try another time w/o using the platform devices
		if(platform_index+1 < platforms.size()) {
			a2e_debug("trying next platform ...");
			init(use_platform_devices, platform_index+1);
		}
	__HANDLE_CL_EXCEPTION_END
	
	if(ro_formats.empty() && wo_formats.empty() && rw_formats.empty()) {
#if 0
		// context has been created, query image format information
		context->getSupportedImageFormats(CL_MEM_READ_ONLY, CL_MEM_OBJECT_IMAGE2D, &ro_formats);
		context->getSupportedImageFormats(CL_MEM_WRITE_ONLY, CL_MEM_OBJECT_IMAGE2D, &wo_formats);
		context->getSupportedImageFormats(CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE2D, &rw_formats);
		
		//
		cout << "## write only formats:" << endl;
		for(auto& format : wo_formats) {
			cout << "\t";
			switch(format.image_channel_order) {
				case CL_R: cout << "CL_R"; break;
				case CL_A: cout << "CL_A"; break;
				case CL_RG: cout << "CL_RG"; break;
				case CL_RA: cout << "CL_RA"; break;
				case CL_RGB: cout << "CL_RGB"; break;
				case CL_RGBA: cout << "CL_RGBA"; break;
				case CL_BGRA: cout << "CL_BGRA"; break;
				case CL_ARGB: cout << "CL_ARGB"; break;
				case CL_INTENSITY: cout << "CL_INTENSITY"; break;
				case CL_LUMINANCE: cout << "CL_LUMINANCE"; break;
				case CL_Rx: cout << "CL_Rx"; break;
				case CL_RGx: cout << "CL_RGx"; break;
				case CL_RGBx: cout << "CL_RGBx"; break;
				default:
					cout << format.image_channel_order;
					break;
			}
			cout << " ";
			switch(format.image_channel_data_type) {
				case CL_SNORM_INT8: cout << "CL_SNORM_INT8"; break;
				case CL_SNORM_INT16: cout << "CL_SNORM_INT16"; break;
				case CL_UNORM_INT8: cout << "CL_UNORM_INT8"; break;
				case CL_UNORM_INT16: cout << "CL_UNORM_INT16"; break;
				case CL_UNORM_SHORT_565: cout << "CL_UNORM_SHORT_565"; break;
				case CL_UNORM_SHORT_555: cout << "CL_UNORM_SHORT_555"; break;
				case CL_UNORM_INT_101010: cout << "CL_UNORM_INT_101010"; break;
				case CL_SIGNED_INT8: cout << "CL_SIGNED_INT8"; break;
				case CL_SIGNED_INT16: cout << "CL_SIGNED_INT16"; break;
				case CL_SIGNED_INT32: cout << "CL_SIGNED_INT32"; break;
				case CL_UNSIGNED_INT8: cout << "CL_UNSIGNED_INT8"; break;
				case CL_UNSIGNED_INT16: cout << "CL_UNSIGNED_INT16"; break;
				case CL_UNSIGNED_INT32: cout << "CL_UNSIGNED_INT32"; break;
				case CL_HALF_FLOAT: cout << "CL_HALF_FLOAT"; break;
				case CL_FLOAT: cout << "CL_FLOAT"; break;
				default:
					cout << format.image_channel_data_type;
					break;
			}
			cout << endl;
		}
#endif
	}
}

bool opencl::is_cpu_support() {
	// if a fastest cpu exists, we do have cpu support
	return (fastest_cpu != NULL);
}

bool opencl::is_gpu_support() {
	// if a fastest gpu exists, we do have gpu support
	return (fastest_gpu != NULL);
}

opencl::kernel_object* opencl::add_kernel_file(const string& identifier, const char* file_name, const string& func_name, const char* additional_options) {
	if(!f->open_file(file_name, file_io::OT_READ)) {
		return NULL;
	}
	
	core::reset(buffer);
	f->read_file(buffer);
	
	string kernel_data;
	kernel_data.reserve((size_t)f->get_filesize());
	kernel_data = buffer->str();
	
	f->close_file();
	
//#ifdef __APPLE__
	// work around caching bug and modify source on each load, TODO: check if this still exists (still present in 10.6.2)
	kernel_data = "#define __" + core::str_to_upper(func_name) +  "_BUILD_TIME__ " + uint2string((unsigned int)time(NULL)) + "\n" + kernel_data;
//#endif
	
	return add_kernel_src(identifier, kernel_data, func_name, additional_options);
}

opencl::kernel_object* opencl::add_kernel_src(const string& identifier, const string& src, const string& func_name, const char* additional_options) {
	a2e_debug("compiling \"%s\" kernel!", identifier.c_str());
	string options = build_options;
	try {
		if(kernels.count(identifier) != 0) {
			a2e_error("kernel \"%s\" already exists!", identifier.c_str());
			return kernels[identifier];
		}
		
		if(additional_options != NULL && strlen(additional_options) > 0) {
			options += (additional_options[0] != ' ' ? " " : "") + string(additional_options);
		}
		
#ifdef __APPLE__
		//options += " -D__APPLE__"; // not necessary any more
#else
		// workaround for the nvidia compiler which apparently defines __APPLE__
		options += " -DUNDEF__APPLE__";
#endif
#ifdef __WINDOWS__
		options += " -D__WINDOWS__"; // TODO: still necessary?
#endif
		
		// add kernel
		kernels[identifier] = new opencl::kernel_object();
		kernels[identifier]->kernel_name = identifier;
		cl::Program::Sources source(1, make_pair(src.c_str(), src.length()));
		kernels[identifier]->program = new cl::Program(*context, source);
		
		// compile for each device independently to add device-specific defines
		for(vector<device_object*>::iterator dev_iter = devices.begin(); dev_iter != devices.end(); dev_iter++) {
			vector<cl::Device> cur_device;
			cur_device.push_back(*(*dev_iter)->device);
			
			string device_options = "";
			switch((*dev_iter)->vendor_type) {
				case CLV_NVIDIA:
					device_options += nv_build_options;
					device_options += " -DNVIDIA";
					break;
				case CLV_ATI:
					device_options += " -DATI";
					break;
				case CLV_INTEL:
					device_options += " -DINTEL";
					break;
				case CLV_AMD:
					device_options += " -DAMD";
					break;
				default:
					device_options += " -DUNKNOWN_VENDOR";
					break;
			}
			if(((*dev_iter)->internal_type & CL_DEVICE_TYPE_CPU) != 0) device_options += " -DCPU";
			if(((*dev_iter)->internal_type & CL_DEVICE_TYPE_GPU) != 0) device_options += " -DGPU";
			if(((*dev_iter)->internal_type & CL_DEVICE_TYPE_ACCELERATOR) != 0) device_options += " -DACCELERATOR";
			
			kernels[identifier]->program->build(cur_device, (options+device_options).c_str());
		}
		
		kernels[identifier]->kernel = new cl::Kernel(*kernels[identifier]->program, func_name.c_str(), &ierr);
		kernels[identifier]->global = new cl::NDRange(1);
		kernels[identifier]->local = new cl::NDRange(1);
		
		kernels[identifier]->arg_count = kernels[identifier]->kernel->getInfo<CL_KERNEL_NUM_ARGS>();
		kernels[identifier]->args_passed.insert(kernels[identifier]->args_passed.begin(), kernels[identifier]->arg_count, false);

		// print out build log
		/*for(vector<cl::Device>::iterator diter = internal_devices.begin(); diter != internal_devices.end(); diter++) {
			char build_log[CLINFO_STR_SIZE];
			memset(build_log, 0, CLINFO_STR_SIZE);
			kernels[identifier]->program.getBuildInfo(*diter, CL_PROGRAM_BUILD_LOG, &build_log);
			a2e_debug("build log: %s", build_log);
		}*/
		
		size_t device_num = 0;
		for(vector<device_object*>::iterator diter = devices.begin(); diter != devices.end(); diter++) {
			//cout << "DEBUG: kernel local memory device #" << device_num << ": " << kernels[identifier]->kernel.getWorkGroupInfo<CL_KERNEL_LOCAL_MEM_SIZE>(*(*diter)->device) << endl;
			device_num++;
		}
	}
	__HANDLE_CL_EXCEPTION_START("add_kernel")
		// print out build log
		for(vector<cl::Device>::iterator diter = internal_devices.begin(); diter != internal_devices.end(); diter++) {
			char build_log[CLINFO_STR_SIZE];
			memset(build_log, 0, CLINFO_STR_SIZE);
			kernels[identifier]->program->getBuildInfo(*diter, CL_PROGRAM_BUILD_LOG, &build_log);
			a2e_error("build log (%s): %s", identifier, build_log);
		}
		
		//log_program_binary(kernels[identifier], options);
		
		// print out current build options
		char buildoptions[CLINFO_STR_SIZE];
		memset(buildoptions, 0, CLINFO_STR_SIZE);
		kernels[identifier]->program->getBuildInfo(internal_devices.back(), CL_PROGRAM_BUILD_OPTIONS, &buildoptions);
		a2e_debug("build options: %s", buildoptions);
	__HANDLE_CL_EXCEPTION_END
	//log_program_binary(kernels[identifier], options);
	return kernels[identifier];
}

void opencl::log_program_binary(const kernel_object* kernel, const string& options) {
	if(kernel == NULL) return;
	
	try {
		// if the device is a nvidia gpu (and we are using the nvidia driver), log the ptx data
		size_t device_num = 0;
		vector<size_t> program_sizes = kernel->program->getInfo<CL_PROGRAM_BINARY_SIZES>();
		if(program_sizes.size() == 0) return;
		
		unsigned char** program_binaries = new unsigned char*[program_sizes.size()];
		for(size_t i = 0; i < program_sizes.size(); i++) {
			program_binaries[i] = new unsigned char[program_sizes[i]+1];
		}
		clGetProgramInfo((*kernel->program)(), CL_PROGRAM_BINARIES, program_sizes.size()*sizeof(unsigned char*), &program_binaries[0], NULL);
		
		string kernel_name = kernel->kernel->getInfo<CL_KERNEL_FUNCTION_NAME>();
		for(vector<device_object*>::iterator diter = devices.begin(); diter != devices.end(); diter++) {
			if(program_sizes[device_num] > 0) {
				if((*diter)->vendor_type != opencl::CLV_UNKNOWN) {
					string file_name = kernel_name + string("_") + size_t2string(device_num);
					if((*diter)->vendor_type == opencl::CLV_NVIDIA) {
						file_name += ".ptx";
					}
					else if((*diter)->vendor_type == opencl::CLV_INTEL || (*diter)->vendor_type == opencl::CLV_AMD) {
						file_name += ".asm";
					}
					else {
						file_name += ".bin";
					}
					
					fstream bin_file(file_name.c_str(), fstream::out | fstream::binary);
					if(!bin_file.is_open()) {
						a2e_error("couldn't save cl-binary file \"%s\"!", file_name.c_str());
						return;
					}
					
					bin_file.write((const char*)program_binaries[device_num], program_sizes[device_num]);
					bin_file.flush();
					bin_file.close();
					
#ifdef __APPLE__
					// this is a real elf binary on 10.7 now ...
					//system(("plutil -convert xml1 "+file_name).c_str());
#endif
				}
			}
			device_num++;
		}
		
		for(size_t i = 0; i < program_sizes.size(); i++) {
			delete [] program_binaries[i];
		}
		delete [] program_binaries;
	}
	__HANDLE_CL_EXCEPTION("log_program_binary")
}

void opencl::check_compilation(const bool ret, const string& filename) {
	if(!ret) {
		a2e_error("internal kernel \"%s\" didn't compile successfully!", filename.c_str());
		successful_internal_compilation = false;
	}
}

void opencl::reload_kernels() {
	destroy_kernels();
	
	successful_internal_compilation = true;
	check_compilation(add_kernel_file("PARTICLE INIT", make_kernel_path("particle_spawn.cl"), "particle_init", " -DA2E_PARTICLE_INIT") != NULL, "particle_spawn.cl");
	check_compilation(add_kernel_file("PARTICLE RESPAWN", make_kernel_path("particle_spawn.cl"), "particle_respawn") != NULL, "particle_spawn.cl");
	check_compilation(add_kernel_file("PARTICLE COMPUTE", make_kernel_path("particle_compute.cl"), "particle_compute") != NULL, "particle_compute.cl");
	
	// figure out which sorting local size we can use
	// a local size of 1024 can be used on fermi+ gpus
	size_t local_size_limit = std::max((size_t)512, devices[0]->max_wg_size); // default to 512
	for(const auto device : devices) {
		if(device->max_wg_size < local_size_limit) {
			local_size_limit = device->max_wg_size;
		}
	}
	const string lsl_str = " -DLOCAL_SIZE_LIMIT="+size_t2string(local_size_limit);
	check_compilation(add_kernel_file("PARTICLE SORT LOCAL", make_kernel_path("particle_sort.cl"), "bitonicSortLocal", lsl_str.c_str()) != NULL, "particle_sort.cl");
	check_compilation(add_kernel_file("PARTICLE SORT MERGE GLOBAL", make_kernel_path("particle_sort.cl"), "bitonicMergeGlobal", lsl_str.c_str()) != NULL, "particle_sort.cl");
	check_compilation(add_kernel_file("PARTICLE SORT MERGE LOCAL", make_kernel_path("particle_sort.cl"), "bitonicMergeLocal", lsl_str.c_str()) != NULL, "particle_sort.cl");
	
	// TODO: make tile size dependent on #cores
	/*check_compilation(add_kernel_file("INFERRED LIGHTING", make_kernel_path("ir_lighting.cl"), "ir_lighting", " -DA2E_IR_TILE_SIZE_X=4 -DA2E_IR_TILE_SIZE_Y=4") != NULL, "ir_lighting.cl");*/
	check_compilation(add_kernel_file("INFERRED LIGHTING", make_kernel_path("ir_lighting.cl"), "ir_lighting", " -DA2E_IR_TILE_SIZE_X=16 -DA2E_IR_TILE_SIZE_Y=16") != NULL, "ir_lighting.cl");
	
	if(successful_internal_compilation) a2e_debug("internal kernels loaded successfully!");
	else {
		// one or more kernels didn't compile, TODO: use fallback?
		a2e_error("there were problems loading/compiling the internal kernels!");
	}
}

void opencl::load_internal_kernels() {
	reload_kernels();
	
	set_active_device(opencl::FASTEST_GPU);
	//set_active_device(opencl::FASTEST_CPU);
}

void opencl::use_kernel(const string& identifier) {
	if(kernels.count(identifier) == 0) {
		a2e_error("kernel \"%s\" doesn't exist!", identifier.c_str());
		cur_kernel = NULL;
		return;
	}
	cur_kernel = kernels[identifier];
}

opencl::buffer_object* opencl::create_buffer_object(opencl::BUFFER_TYPE type, void* data) {
	try {
		buffers.push_back(new opencl::buffer_object());
		
		// type/flag validity check
		unsigned int vtype = 0;
		if(type & opencl::BT_USE_HOST_MEMORY) vtype |= opencl::BT_USE_HOST_MEMORY;
		if(type & opencl::BT_DELETE_AFTER_USE) vtype |= opencl::BT_DELETE_AFTER_USE;
		if(type & opencl::BT_BLOCK_ON_READ) vtype |= opencl::BT_BLOCK_ON_READ;
		if(type & opencl::BT_BLOCK_ON_WRITE) vtype |= opencl::BT_BLOCK_ON_WRITE;
		if(data != NULL && (type & opencl::BT_INITIAL_COPY) && !(vtype & opencl::BT_USE_HOST_MEMORY)) vtype |= opencl::BT_INITIAL_COPY;
		if(data != NULL && (type & opencl::BT_COPY_ON_USE)) vtype |= opencl::BT_COPY_ON_USE;
		if(data != NULL && (type & opencl::BT_READ_BACK_RESULT)) vtype |= opencl::BT_READ_BACK_RESULT;
		
		cl_mem_flags flags = 0;
		switch((EBUFFER_TYPE)(type & 0x03)) {
			case opencl::BT_READ_WRITE:
				vtype |= BT_READ_WRITE;
				flags |= CL_MEM_READ_WRITE;
				break;
			case opencl::BT_READ:
				vtype |= BT_READ;
				flags |= CL_MEM_READ_ONLY;
				break;
			case opencl::BT_WRITE:
				vtype |= BT_WRITE;
				flags |= CL_MEM_WRITE_ONLY;
				break;
			default:
				break;
		}
		if((vtype & opencl::BT_INITIAL_COPY) && !(vtype & opencl::BT_USE_HOST_MEMORY)) flags |= CL_MEM_COPY_HOST_PTR;
		if(data != NULL && (vtype & opencl::BT_USE_HOST_MEMORY)) flags |= CL_MEM_USE_HOST_PTR;
		if(data == NULL && (vtype & opencl::BT_USE_HOST_MEMORY)) flags |= CL_MEM_ALLOC_HOST_PTR;
		
		buffers.back()->type = vtype;
		buffers.back()->flags = flags;
		buffers.back()->data = data;
		return buffers.back();
	}
	__HANDLE_CL_EXCEPTION("create_buffer_object")
	return NULL;
}

opencl::buffer_object* opencl::create_buffer(opencl::BUFFER_TYPE type, size_t size, void* data) {
	if(size == 0) {
		return NULL;
	}
	
	try {
		buffer_object* buffer_obj = create_buffer_object(type, data);
		if(buffer_obj == NULL) return NULL;
		
		buffer_obj->size = size;
		buffer_obj->buffer = new cl::Buffer(*context, buffer_obj->flags, size,
											((buffer_obj->type & opencl::BT_INITIAL_COPY) || (buffer_obj->type & opencl::BT_USE_HOST_MEMORY) ? data : NULL),
											&ierr);
		return buffer_obj;
	}
	__HANDLE_CL_EXCEPTION("create_buffer")
	return NULL;
}

opencl::buffer_object* opencl::create_image2d_buffer(opencl::BUFFER_TYPE type, cl_channel_order channel_order, cl_channel_type channel_type, size_t width, size_t height, void* data) {
	try {
		buffer_object* buffer_obj = create_buffer_object(type, data);
		if(buffer_obj == NULL) return NULL;
		
		buffer_obj->format.image_channel_order = channel_order;
		buffer_obj->format.image_channel_data_type = channel_type;
		buffer_obj->origin.set(0, 0, 0);
		buffer_obj->region.set(width, height, 1); // depth must be 1 for 2d images
		buffer_obj->image_buffer = new cl::Image2D(*context, buffer_obj->flags, buffer_obj->format, width, height, 0, data, &ierr);
		return buffer_obj;
	}
	__HANDLE_CL_EXCEPTION("create_image2d_buffer")
	return NULL;
}

opencl::buffer_object* opencl::create_image3d_buffer(opencl::BUFFER_TYPE type, cl_channel_order channel_order, cl_channel_type channel_type, size_t width, size_t height, size_t depth, void* data) {
	try {
		buffer_object* buffer_obj = create_buffer_object(type, data);
		if(buffer_obj == NULL) return NULL;
		
		buffer_obj->format.image_channel_order = channel_order;
		buffer_obj->format.image_channel_data_type = channel_type;
		buffer_obj->origin.set(0, 0, 0);
		buffer_obj->region.set(width, height, depth);
		buffer_obj->image_buffer = new cl::Image3D(*context, buffer_obj->flags, buffer_obj->format, width, height, depth, 0, 0, data, &ierr);
		return buffer_obj;
	}
	__HANDLE_CL_EXCEPTION("create_image3d_buffer")
	return NULL;
}

opencl::buffer_object* opencl::create_ogl_buffer(opencl::BUFFER_TYPE type, GLuint ogl_buffer) {
	try {
		buffers.push_back(new opencl::buffer_object());
		
		// type/flag validity check
		unsigned int vtype = 0;
		if((type & opencl::BT_DELETE_AFTER_USE) != 0) vtype |= opencl::BT_DELETE_AFTER_USE;
		if((type & opencl::BT_BLOCK_ON_READ) != 0) vtype |= opencl::BT_BLOCK_ON_READ;
		if((type & opencl::BT_BLOCK_ON_WRITE) != 0) vtype |= opencl::BT_BLOCK_ON_WRITE;
		
		cl_mem_flags flags = 0;
		switch((EBUFFER_TYPE)(type & 0x03)) {
			case opencl::BT_READ_WRITE:
				vtype |= BT_READ_WRITE;
				flags |= CL_MEM_READ_WRITE;
				break;
			case opencl::BT_READ:
				vtype |= BT_READ;
				flags |= CL_MEM_READ_ONLY;
				break;
			case opencl::BT_WRITE:
				vtype |= BT_WRITE;
				flags |= CL_MEM_WRITE_ONLY;
				break;
			default:
				break;
		}
		
		vtype |= BT_OPENGL_BUFFER;
		
		buffers.back()->type = vtype;
		buffers.back()->ogl_buffer = ogl_buffer;
		buffers.back()->data = NULL;
		buffers.back()->size = 0;
		buffers.back()->buffer = new cl::BufferGL(*context, flags, ogl_buffer, &ierr);
		return buffers.back();
	}
	__HANDLE_CL_EXCEPTION("create_ogl_buffer")
	return NULL;
}

opencl::buffer_object* opencl::create_ogl_image2d_buffer(BUFFER_TYPE type, GLuint texture, GLenum target) {
	try {
		buffers.push_back(new opencl::buffer_object());
		
		// type/flag validity check
		unsigned int vtype = 0;
		if(type & opencl::BT_DELETE_AFTER_USE) vtype |= opencl::BT_DELETE_AFTER_USE;
		if(type & opencl::BT_BLOCK_ON_READ) vtype |= opencl::BT_BLOCK_ON_READ;
		if(type & opencl::BT_BLOCK_ON_WRITE) vtype |= opencl::BT_BLOCK_ON_WRITE;
		
		cl_mem_flags flags = 0;
		switch((EBUFFER_TYPE)(type & 0x03)) {
			case opencl::BT_READ_WRITE:
				vtype |= BT_READ_WRITE;
				flags |= CL_MEM_READ_WRITE;
				break;
			case opencl::BT_READ:
				vtype |= BT_READ;
				flags |= CL_MEM_READ_ONLY;
				break;
			case opencl::BT_WRITE:
				vtype |= BT_WRITE;
				flags |= CL_MEM_WRITE_ONLY;
				break;
			default:
				break;
		}
		
		vtype |= BT_OPENGL_BUFFER;
		
		buffers.back()->type = vtype;
		buffers.back()->ogl_buffer = texture;
		buffers.back()->data = NULL;
		buffers.back()->size = 0;
		buffers.back()->image_buffer = new cl::Image2DGL(*context, flags, target, 0, texture, &ierr);
		return buffers.back();
	}
	__HANDLE_CL_EXCEPTION("create_ogl_image2d_buffer")
	return NULL;
}

opencl::buffer_object* opencl::create_ogl_image2d_renderbuffer(BUFFER_TYPE type, GLuint renderbuffer) {
	try {
		buffers.push_back(new opencl::buffer_object());
		
		// type/flag validity check
		unsigned int vtype = 0;
		if(type & opencl::BT_DELETE_AFTER_USE) vtype |= opencl::BT_DELETE_AFTER_USE;
		if(type & opencl::BT_BLOCK_ON_READ) vtype |= opencl::BT_BLOCK_ON_READ;
		if(type & opencl::BT_BLOCK_ON_WRITE) vtype |= opencl::BT_BLOCK_ON_WRITE;
		
		cl_mem_flags flags = 0;
		switch((EBUFFER_TYPE)(type & 0x03)) {
			case opencl::BT_READ_WRITE:
				vtype |= BT_READ_WRITE;
				flags |= CL_MEM_READ_WRITE;
				break;
			case opencl::BT_READ:
				vtype |= BT_READ;
				flags |= CL_MEM_READ_ONLY;
				break;
			case opencl::BT_WRITE:
				vtype |= BT_WRITE;
				flags |= CL_MEM_WRITE_ONLY;
				break;
			default:
				break;
		}
		
		vtype |= BT_OPENGL_BUFFER;
		
		buffers.back()->type = vtype;
		buffers.back()->ogl_buffer = renderbuffer;
		buffers.back()->data = NULL;
		buffers.back()->size = 0;
		buffers.back()->buffer = new cl::BufferRenderGL(*context, flags, renderbuffer, &ierr);
		return buffers.back();
	}
	__HANDLE_CL_EXCEPTION("create_ogl_image2d_renderbuffer")
	return NULL;
}

void opencl::delete_buffer(opencl::buffer_object* buffer_obj) {
	// remove buffer from each associated kernel (and unset the kernel argument)
	for(map<kernel_object*, vector<unsigned int> >::iterator akiter = buffer_obj->associated_kernels.begin(); akiter != buffer_obj->associated_kernels.end(); akiter++) {
		for(vector<unsigned int>::iterator arg_iter = akiter->second.begin(); arg_iter != akiter->second.end(); arg_iter++) {
			akiter->first->args_passed[*arg_iter] = false;
			akiter->first->buffer_args.erase(*arg_iter);
		}
	}
	buffer_obj->associated_kernels.clear();
	if(buffer_obj->buffer != NULL) delete buffer_obj->buffer;
	if(buffer_obj->image_buffer != NULL) delete buffer_obj->image_buffer;
	buffers.erase(find(buffers.begin(), buffers.end(), buffer_obj));
}

void opencl::write_buffer(opencl::buffer_object* buffer_obj, const void* src, const size_t offset, const size_t size) {
	if(cur_kernel == NULL) {
		a2e_error("can't write buffer - no kernel currently in use!");
		return;
	}
	
	size_t write_size = size;
	if(write_size == 0) {
		if(buffer_obj->size == 0) {
			a2e_error("can't write 0 bytes (size of 0)!");
			return;
		}
		else write_size = buffer_obj->size;
	}
	
	size_t write_offset = offset;
	if(write_offset >= buffer_obj->size) {
		a2e_error("write offset (%d) out of bound!", write_offset);
		return;
	}
	if(write_offset+write_size > buffer_obj->size) {
		a2e_error("write offset (%d) or write size (%d) is too big - using write size of (%d) instead!",
				 write_offset, write_size, (buffer_obj->size - write_offset));
		write_size = buffer_obj->size - write_offset;
	}
	
	try {
		queues[active_device->device]->enqueueWriteBuffer(*buffer_obj->buffer, ((buffer_obj->type & opencl::BT_BLOCK_ON_WRITE) > 0),
																	  write_offset, write_size, src);
	}
	__HANDLE_CL_EXCEPTION("write_buffer")
}

void opencl::write_image2d(opencl::buffer_object* buffer_obj, const void* src, size2 origin, size2 region) {
	if(cur_kernel == NULL) {
		a2e_error("can't write buffer - no kernel currently in use!");
		return;
	}
	
	try {
		size3 origin3(origin.x, origin.y, 0); // origin z must be 0 for 2d images
		size3 region3(region.x, region.y, 1); // depth must be 1 for 2d images
		queues[active_device->device]->enqueueWriteImage(*buffer_obj->image_buffer, ((buffer_obj->type & opencl::BT_BLOCK_ON_WRITE) > 0),
																	 (cl::size_t<3>&)origin3, (cl::size_t<3>&)region3, 0, 0, (void*)src);
	}
	__HANDLE_CL_EXCEPTION("write_image2d")
}

void opencl::write_image3d(opencl::buffer_object* buffer_obj, const void* src, size3 origin, size3 region) {
	if(cur_kernel == NULL) {
		a2e_error("can't write buffer - no kernel currently in use!");
		return;
	}
	
	try {
		queues[active_device->device]->enqueueWriteImage(*buffer_obj->image_buffer, ((buffer_obj->type & opencl::BT_BLOCK_ON_WRITE) > 0),
																	 (cl::size_t<3>&)origin, (cl::size_t<3>&)region, 0, 0, (void*)src);
	}
	__HANDLE_CL_EXCEPTION("write_buffer")
}

void opencl::read_buffer(void* dst, opencl::buffer_object* buffer_obj) {
	if(cur_kernel == NULL) {
		a2e_error("can't read buffer - no kernel currently in use!");
		return;
	}
	
	try {
		queues[active_device->device]->enqueueReadBuffer(*buffer_obj->buffer, ((buffer_obj->type & opencl::BT_BLOCK_ON_READ) > 0),
																	 0, buffer_obj->size, dst);
	}
	__HANDLE_CL_EXCEPTION("read_buffer")
}

void opencl::set_active_device(opencl::OPENCL_DEVICE dev) {
	switch(dev) {
		case FASTEST_GPU:
			if(fastest_gpu != NULL) {
				active_device = fastest_gpu;
				return;
			}
			break;
		case FASTEST_CPU:
			if(fastest_cpu != NULL) {
				active_device = fastest_cpu;
				return;
			}
			break;
		case ALL_GPU:
			// TODO: ...
			break;
		case ALL_CPU:
			// TODO: ...
			break;
		case ALL_DEVICES:
			// TODO: ...
			break;
		default:
			break;
	}
	
	if((dev >= GPU0 && dev <= GPU255) || (dev >= CPU0 && dev <= CPU255)) {
		for(vector<device_object*>::iterator diter = devices.begin(); diter != devices.end(); diter++) {
			if((*diter)->type == dev) {
				active_device = *diter;
				return;
			}
		}
	}
	
	if(active_device != NULL) {
		a2e_error("can't use device %u - keeping current one (%u)!", dev, active_device->type);
	}
	else {
		// TODO: use _any_ device if there is at least one available ...
		a2e_error("can't use device %u and no other device is currently active!", dev);
	}
}

void opencl::set_kernel_range(const cl::NDRange& global, const cl::NDRange& local) {
	if(cur_kernel == NULL) return;
	
	memcpy(cur_kernel->global, global, sizeof(cl::NDRange));
	memcpy(cur_kernel->local, local, sizeof(cl::NDRange));
}

void opencl::run_kernel(kernel_object* kernel_obj) {
	try {
		bool all_set = true;
		for(unsigned int i = 0; i < kernel_obj->args_passed.size(); i++) {
			if(!kernel_obj->args_passed[i]) {
				a2e_error("argument #%u not set!", i);
				all_set = false;
			}
		}
		if(!all_set) return;
		
		vector<cl::Memory> gl_objects;
		for(map<unsigned int, buffer_object*>::iterator biter = kernel_obj->buffer_args.begin(); biter != kernel_obj->buffer_args.end(); biter++) {
			if((biter->second->type & opencl::BT_COPY_ON_USE) != 0) write_buffer(biter->second, biter->second->data);
			if((biter->second->type & opencl::BT_OPENGL_BUFFER) != 0 &&
			   !biter->second->manual_gl_sharing) {
				gl_objects.push_back(*(biter->second->buffer != NULL ? (cl::Memory*)biter->second->buffer : (cl::Memory*)biter->second->image_buffer));
				kernel_obj->has_ogl_buffers = true;
			}
		}
		if(!gl_objects.empty()) {
			queues[active_device->device]->enqueueAcquireGLObjects(&gl_objects);
		}
		
		cl::KernelFunctor func = kernel_obj->kernel->bind(*queues[active_device->device], *kernel_obj->global, *kernel_obj->local);
		//func().wait();
		func();
		
		for(map<unsigned int, buffer_object*>::iterator biter = kernel_obj->buffer_args.begin(); biter != kernel_obj->buffer_args.end(); biter++) {
			if((biter->second->type & opencl::BT_READ_BACK_RESULT) != 0) read_buffer(biter->second->data, biter->second);
		}
		for(map<unsigned int, buffer_object*>::reverse_iterator biter = kernel_obj->buffer_args.rbegin(); biter != kernel_obj->buffer_args.rend(); biter++) {
			if((biter->second->type & opencl::BT_DELETE_AFTER_USE) != 0) delete_buffer(biter->second);
		}
		
		if(kernel_obj->has_ogl_buffers && !gl_objects.empty()) {
			queues[active_device->device]->enqueueReleaseGLObjects(&gl_objects);
		}
	}
	__HANDLE_CL_EXCEPTION_EXT("run_kernel", (" - in kernel: "+kernel_obj->kernel_name).c_str())
}

void opencl::finish() {
	if(cur_kernel == NULL) return;
	queues[active_device->device]->finish();
}

void opencl::flush() {
	if(cur_kernel == NULL) return;
	queues[active_device->device]->flush();
}

void opencl::run_kernel() {
	run_kernel(cur_kernel);
}

void opencl::run_kernel(const char* kernel_identifier) {
	if(kernels.count(kernel_identifier) > 0) {
		run_kernel(kernels[kernel_identifier]);
	}
	a2e_error("kernel \"%s\" doesn't exist!", kernel_identifier);
}

bool opencl::set_kernel_argument(unsigned int index, opencl::buffer_object* arg) {
	if((arg->buffer != NULL && set_kernel_argument(index, (*arg->buffer)())) ||
	   (arg->image_buffer != NULL && set_kernel_argument(index, *(cl::Memory*)arg->image_buffer))) {
		cur_kernel->buffer_args[index] = arg;
		arg->associated_kernels[cur_kernel].push_back(index);
		return true;
	}
	return false;
}

bool opencl::set_kernel_argument(unsigned int index, const opencl::buffer_object* arg) {
	return set_kernel_argument(index, (opencl::buffer_object*)arg);
}

bool opencl::set_kernel_argument(unsigned int index, size_t size, void* arg) {
	try {
		cur_kernel->kernel->setArg(index, size, arg);
		cur_kernel->args_passed[index] = true;
		return true;
	}
	__HANDLE_CL_EXCEPTION("set_kernel_argument")
	return false;
}

opencl::device_object* opencl::get_device(opencl::OPENCL_DEVICE device) {
	if(device == FASTEST_GPU) return fastest_gpu;
	else if(device == FASTEST_CPU) return fastest_cpu;
	else {
		if((device >= GPU0 && device <= GPU255) || (device >= CPU0 && device <= CPU255)) {
			for(vector<device_object*>::iterator diter = devices.begin(); diter != devices.end(); diter++) {
				if((*diter)->type == device) {
					return *diter;
				}
			}
		}
	}
	return NULL;
}

opencl::device_object* opencl::get_active_device() {
	return active_device;
}

void* opencl::map_buffer(opencl::buffer_object* buffer_obj, EBUFFER_TYPE access_type, bool blocking) {
	try {
		cl_map_flags map_flags = CL_MAP_READ;
		switch((EBUFFER_TYPE)(access_type & 0x03)) {
			case opencl::BT_READ_WRITE: map_flags = CL_MAP_READ | CL_MAP_WRITE; break;
			case opencl::BT_READ: map_flags = CL_MAP_READ; break;
			case opencl::BT_WRITE: map_flags = CL_MAP_WRITE; break;
			default: break;
		}
		
		void* map_ptr = NULL;
		if(buffer_obj->buffer != NULL) {
			map_ptr = queues[active_device->device]->enqueueMapBuffer(*buffer_obj->buffer, blocking, map_flags, 0, buffer_obj->size);
		}
		else if(buffer_obj->image_buffer != NULL) {
			size_t row_pitch, slice_pitch;
			map_ptr = queues[active_device->device]->enqueueMapImage(*buffer_obj->image_buffer, blocking, map_flags,
																				 (cl::size_t<3>&)buffer_obj->origin,
																				 (cl::size_t<3>&)buffer_obj->region,
																				 &row_pitch, &slice_pitch);
		}
		else {
			a2e_error("unknown buffer object!");
			return NULL;
		}
		return map_ptr;
	}
	__HANDLE_CL_EXCEPTION("map_buffer")
	return NULL;
}

void opencl::unmap_buffer(opencl::buffer_object* buffer_obj, void* map_ptr) {
	try {
		void* buffer_ptr = NULL;
		if(buffer_obj->buffer != NULL) buffer_ptr = buffer_obj->buffer;
		else if(buffer_obj->image_buffer != NULL) buffer_ptr = buffer_obj->image_buffer;
		else {
			a2e_error("unknown buffer object!");
			return;
		}
		queues[active_device->device]->enqueueUnmapMemObject(*(cl::Memory*)buffer_ptr, map_ptr);
	}
	__HANDLE_CL_EXCEPTION("unmap_buffer")
}

bool opencl::has_vendor_device(OPENCL_VENDOR vendor_type) {
	for(vector<device_object*>::const_iterator dev_iter = devices.begin(); dev_iter != devices.end(); dev_iter++) {
		if((*dev_iter)->vendor_type == vendor_type) return true;
	}
	return false;
}

size_t opencl::get_kernel_work_group_size() {
	if(cur_kernel == NULL || active_device == NULL) return 0;
	
	try {
		return cur_kernel->kernel->getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(*active_device->device);
	}
	__HANDLE_CL_EXCEPTION("get_kernel_work_group_size")
	return 0;
}

cl::NDRange opencl::compute_local_kernel_range(const unsigned int dimensions) {
	cl::NDRange local;
	if(dimensions < 1 || dimensions > 3) {
		a2e_error("invalid dimensions number %d!", dimensions);
		return local;
	}
	if(cur_kernel == NULL || active_device == NULL) {
		dimensions == 1 ? local.set(1) : (dimensions == 2 ? local.set(1, 1) : local.set(1, 1, 1));
		return local;
	}
	
	// compute local work group size/dimensions
	size_t wgs = get_kernel_work_group_size();
	
	if(dimensions == 1) {
		local.set(wgs);
	}
	else if(dimensions == 2) {
		size_t local_x_size = 1, local_y_size = 1;
		size_t size = sizeof(size_t)*8;
		// get highest bit
		size_t hb, mask = 0;
		size_t one = 1; // ... -.-"
		for(hb = size-1; hb > 0; hb--) {
			mask |= (one << hb);
			if((wgs & mask) != 0) break;
		}
		size_t pot = (hb+1) >> 1;
		size_t rpot = hb-pot;
		local_x_size = one << pot;
		local_y_size = one << rpot;
		
		size_t rem = wgs ^ (one << hb);
		if(rem > 0) local_y_size += rem/local_x_size;
		
		local.set(local_x_size, local_y_size);
	}
	else if(dimensions == 3) {
		// TODO: code this! (same as dim 2 for the moment)
		
		size_t local_x_size = 1, local_y_size = 1;
		size_t size = sizeof(size_t)*8;
		// get highest bit
		size_t hb, mask = 0;
		size_t one = 1; // ... -.-"
		for(hb = size-1; hb > 0; hb--) {
			mask |= (one << hb);
			if((wgs & mask) != 0) break;
		}
		size_t pot = (hb+1) >> 1;
		size_t rpot = hb-pot;
		local_x_size = one << pot;
		local_y_size = one << rpot;
		
		size_t rem = wgs ^ (one << hb);
		if(rem > 0) local_y_size += rem/local_x_size;
		
		local.set(local_x_size, local_y_size, 1);
	}
	
	return local;
}

void opencl::set_manual_gl_sharing(buffer_object* gl_buffer_obj, const bool state) {
	if((gl_buffer_obj->type & opencl::BT_OPENGL_BUFFER) == 0 ||
	   gl_buffer_obj->ogl_buffer == 0) {
		a2e_error("this is not a gl object!");
		return;
	}
	
	gl_buffer_obj->manual_gl_sharing = state;
}

void opencl::acquire_gl_object(buffer_object* gl_buffer_obj) {
	vector<cl::Memory> gl_objects;
	gl_objects.push_back(*(gl_buffer_obj->buffer != NULL ?
						   (cl::Memory*)gl_buffer_obj->buffer :
						   (cl::Memory*)gl_buffer_obj->image_buffer));
	queues[active_device->device]->enqueueAcquireGLObjects(&gl_objects);
}

void opencl::release_gl_object(buffer_object* gl_buffer_obj) {
	vector<cl::Memory> gl_objects;
	gl_objects.push_back(*(gl_buffer_obj->buffer != NULL ?
						   (cl::Memory*)gl_buffer_obj->buffer :
						   (cl::Memory*)gl_buffer_obj->image_buffer));
	queues[active_device->device]->enqueueReleaseGLObjects(&gl_objects);
}
