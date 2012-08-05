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

#if !defined(A2E_NO_OPENCL)

#define __ERROR_CODE_INFO_CL_11(F) \
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

#if defined(CL_VERSION_1_2)
#define __ERROR_CODE_INFO_CL_12(F) \
F(CL_MISALIGNED_SUB_BUFFER_OFFSET) \
F(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST) \
F(CL_COMPILE_PROGRAM_FAILURE) \
F(CL_LINKER_NOT_AVAILABLE) \
F(CL_LINK_PROGRAM_FAILURE) \
F(CL_DEVICE_PARTITION_FAILED) \
F(CL_KERNEL_ARG_INFO_NOT_AVAILABLE) \
F(CL_INVALID_GLOBAL_WORK_SIZE) \
F(CL_INVALID_PROPERTY) \
F(CL_INVALID_IMAGE_DESCRIPTOR) \
F(CL_INVALID_COMPILER_OPTIONS) \
F(CL_INVALID_LINKER_OPTIONS) \
F(CL_INVALID_DEVICE_PARTITION_COUNT)
#define __ERROR_CODE_INFO(F) __ERROR_CODE_INFO_CL_11(F) __ERROR_CODE_INFO_CL_12(F)
#else
#define __ERROR_CODE_INFO(F) __ERROR_CODE_INFO_CL_11(F)
#endif

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
			  __LINE__, err.what(), err.err(), error_code_to_string(err.err()), additional_info);
#define __HANDLE_CL_EXCEPTION_END }
#define __HANDLE_CL_EXCEPTION(func_str) __HANDLE_CL_EXCEPTION_START(func_str) __HANDLE_CL_EXCEPTION_END
#define __HANDLE_CL_EXCEPTION_EXT(func_str, additional_info) __HANDLE_CL_EXCEPTION_START_EXT(func_str, additional_info) __HANDLE_CL_EXCEPTION_END

/*! creates a opencl object
 */
opencl::opencl(const char* kernel_path, file_io* f_, SDL_Window* wnd, const bool clear_cache) :
buffer(stringstream::in | stringstream::out)
{
	opencl::f = f_;
	opencl::sdl_wnd = wnd;
	opencl::kernel_path_str = kernel_path;
	
	context = nullptr;
	cur_kernel = nullptr;
	active_device = nullptr;
	
	fastest_cpu = nullptr;
	fastest_gpu = nullptr;
	
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
	
#if !defined(__APPLE__)
	//nv_build_options = " -cl-nv-verbose";
	//nv_build_options = " -check-kernel-functions";
	//nv_build_options += " -nvptx-mad-enable -inline-all";
#else
	build_options += " -cl-auto-vectorize-enable";
#endif
	
	// clear opencl cache
	if(clear_cache) {
#if defined(__APPLE__)
		system("rm -R ~/Library/Caches/com.apple.opencl > /dev/null 2>&1");
		// TODO: delete app specific cache (~/Library/Caches/$identifier/com.apple.opencl)
#elif defined(__WINDOWS__)
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
	
	for(const auto& buf : buffers) {
		delete buf->buffer;
	}
	buffers.clear();
	
	destroy_kernels();
	
	if(context != nullptr) delete context;
	
	internal_devices.clear();
	devices.clear();
	
	a2e_debug("opencl object deleted");
}

void opencl::destroy_kernels() {
	for(const auto& k : kernels) {
		delete k.second;
	}
	kernels.clear();
	cur_kernel = nullptr;
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
		
#if defined(__APPLE__)
		platform_vendor = PLATFORM_VENDOR::APPLE;
		
		cl_context_properties cl_properties[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[platform_index](),
#if !defined(A2E_IOS) // TODO: sharing isn't supported on iOS yet (code path exists, but fails with a gles sharegroup)
			CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)CGLGetShareGroup(CGLGetCurrentContext()),
#endif
			0 };
		
		// create a context with all platform devices (this works fine since 10.7)
		context = new cl::Context(internal_devices, cl_properties, clLogMessagesToStdoutAPPLE, nullptr, &ierr);
		
#else
		// context with gl share group (cl/gl interop)
#if defined(__WINDOWS__)
		cl_context_properties cl_properties[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[platform_index](),
			CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
			CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
			0
		};
#else // Linux, hopefully *BSD too
		SDL_SysWMinfo wm_info;
		SDL_VERSION(&wm_info.version);
		if(SDL_GetWindowWMInfo(sdl_wnd, &wm_info) != 1) {
			a2e_error("couldn't get window manger info!");
			return;
		}
		
		cl_context_properties cl_properties[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[platform_index](),
			CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
			CL_GLX_DISPLAY_KHR, (cl_context_properties)wm_info.info.x11.display,
			0
		};
#endif
		
		if(use_platform_devices) {
			context = new cl::Context(internal_devices, cl_properties, nullptr, nullptr, &ierr);
		}
		else {
			context = new cl::Context(CL_DEVICE_TYPE_ALL, cl_properties, nullptr, nullptr, &ierr);
		}
#endif

#if !defined(__APPLE__)
		const string platform_str = platforms[platform_index].getInfo<CL_PLATFORM_NAME>();
		const string platform_vendor_str = core::str_to_lower(platform_str);
		if(platform_vendor_str.find("nvidia") != string::npos) {
			platform_vendor = PLATFORM_VENDOR::NVIDIA;
		}
		else if(platform_vendor_str.find("amd") != string::npos) {
			platform_vendor = PLATFORM_VENDOR::AMD;
		}
		else if(platform_vendor_str.find("intel") != string::npos) {
			platform_vendor = PLATFORM_VENDOR::INTEL;
		}
#endif

		a2e_debug("opencl platform #%u vendor: %s", platform_index, platform_vendor_to_str(platform_vendor));
		
		internal_devices.clear();
		internal_devices = context->getInfo<CL_CONTEXT_DEVICES>();
		a2e_debug("%u opencl device%s found!", internal_devices.size(), (internal_devices.size() > 1 ? "s" : ""));
		
		a2e_debug("opencl context successfully created!");
		
		string dev_type_str;
		unsigned int gpu_counter = (unsigned int)DEVICE_TYPE::GPU0;
		unsigned int cpu_counter = (unsigned int)DEVICE_TYPE::CPU0;
		unsigned int fastest_cpu_score = 0;
		unsigned int fastest_gpu_score = 0;
		unsigned int cpu_score = 0;
		unsigned int gpu_score = 0;
		for(const auto& internal_device : internal_devices) {
			dev_type_str = "";
			
			opencl::device_object* device = new opencl::device_object();
			device->device = &internal_device;
			device->internal_type = internal_device.getInfo<CL_DEVICE_TYPE>();
			device->units = internal_device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
			device->clock = internal_device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>();
			device->mem_size = internal_device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
			device->name = internal_device.getInfo<CL_DEVICE_NAME>();
			device->vendor = internal_device.getInfo<CL_DEVICE_VENDOR>();
			device->version = internal_device.getInfo<CL_DEVICE_VERSION>();
			device->driver_version = internal_device.getInfo<CL_DRIVER_VERSION>();
			device->extensions = internal_device.getInfo<CL_DEVICE_EXTENSIONS>();
			
			device->max_alloc = internal_device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
			device->max_wg_size = internal_device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
			device->img_support = internal_device.getInfo<CL_DEVICE_IMAGE_SUPPORT>() == 1;
			device->max_img_2d.set(internal_device.getInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>(),
								   internal_device.getInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>());
			device->max_img_3d.set(internal_device.getInfo<CL_DEVICE_IMAGE3D_MAX_WIDTH>(),
								   internal_device.getInfo<CL_DEVICE_IMAGE3D_MAX_HEIGHT>(),
								   internal_device.getInfo<CL_DEVICE_IMAGE3D_MAX_DEPTH>());

			device->vendor_type = VENDOR::UNKNOWN;
			string vendor_str = core::str_to_lower(device->vendor);
			if(strstr(vendor_str.c_str(), "nvidia") != nullptr) {
				device->vendor_type = VENDOR::NVIDIA;
			}
			else if(strstr(vendor_str.c_str(), "amd") != nullptr) {
				device->vendor_type = VENDOR::AMD;
			}
			else if(strstr(vendor_str.c_str(), "ati") != nullptr) {
				device->vendor_type = VENDOR::ATI;
			}
			else if(strstr(vendor_str.c_str(), "intel") != nullptr) {
				device->vendor_type = VENDOR::INTEL;
			}
			else if(strstr(vendor_str.c_str(), "apple") != nullptr) {
				device->vendor_type = VENDOR::APPLE;
			}
			
			if(device->internal_type & CL_DEVICE_TYPE_CPU) {
				device->type = (opencl::DEVICE_TYPE)cpu_counter;
				cpu_counter++;
				dev_type_str += "CPU ";
				
				if(fastest_cpu == nullptr) {
					fastest_cpu = device;
					fastest_cpu_score = device->units * device->clock;
				}
				else {
					cpu_score = device->units * device->clock;
					if(cpu_score > fastest_cpu_score) {
						fastest_cpu = device;
					}
				}
			}
			if(device->internal_type & CL_DEVICE_TYPE_GPU) {
				device->type = (opencl::DEVICE_TYPE)gpu_counter;
				gpu_counter++;
				dev_type_str += "GPU ";
				
				if(fastest_gpu == nullptr) {
					fastest_gpu = device;
					fastest_gpu_score = device->units * device->clock;
				}
				else {
					gpu_score = device->units * device->clock;
					if(gpu_score > fastest_gpu_score) {
						fastest_gpu = device;
					}
				}
			}
			if(device->internal_type & CL_DEVICE_TYPE_ACCELERATOR) {
				dev_type_str += "Accelerator ";
			}
			if(device->internal_type & CL_DEVICE_TYPE_DEFAULT) {
				dev_type_str += "Default ";
			}
			
			// cl_khr_byte_addressable_store support is mandatory
			if(device->extensions.find("cl_khr_byte_addressable_store") == string::npos) {
				a2e_msg("opencl device \"%s %s\" does not support \"cl_khr_byte_addressable_store\"!", device->vendor, device->name);
				delete device;
				continue;
			}
			devices.push_back(device);
			
			// TYPE (Units: %, Clock: %): Name, Vendor, Version, Driver Version
			a2e_debug("%s(Units: %u, Clock: %u MHz, Memory: %u MB): %s %s, %s / %s",
					 dev_type_str,
					 device->units,
					 device->clock,
					 (unsigned int)(device->mem_size / 1024ul / 1024ul),
					 device->vendor,
					 device->name,
					 device->version,
					 device->driver_version);
		}
		
		// no supported devices found
		if(devices.empty()) {
			throw a2e_exception("no supported device found for this platform!");
		}
		
		// create a (single) command queue for each device
		for(const auto& device : devices) {
			queues[device->device] = new cl::CommandQueue(*context, *device->device, 0, &ierr);
		}
		
		if(fastest_cpu != nullptr) a2e_debug("fastest CPU device: %s %s (score: %u)", fastest_cpu->vendor.c_str(), fastest_cpu->name.c_str(), fastest_cpu_score);
		if(fastest_gpu != nullptr) a2e_debug("fastest GPU device: %s %s (score: %u)", fastest_gpu->vendor.c_str(), fastest_gpu->name.c_str(), fastest_gpu_score);
		
		// compile internal kernels
		//for(const auto& device : devices) cout << "max wg size: " << device->max_wg_size << endl;
		
		size_t local_size_limit = std::max((size_t)512, devices[0]->max_wg_size); // default to 512
		bool local_atomics_support = true;
		for(const auto& device : devices) {
			if(device->max_wg_size < local_size_limit) {
				local_size_limit = device->max_wg_size;
			}
			if(device->extensions.find("cl_khr_local_int32_base_atomics") == string::npos ||
			   device->extensions.find("cl_khr_local_int32_extended_atomics") == string::npos) {
				local_atomics_support = false;
			}
		}
		const string lsl_str = " -DLOCAL_SIZE_LIMIT="+size_t2string(local_size_limit);
		
		
		internal_kernels = { // first time init:
			make_tuple("PARTICLE INIT", "particle_spawn.cl", "particle_init", " -DA2E_PARTICLE_INIT"),
			make_tuple("PARTICLE RESPAWN", "particle_spawn.cl", "particle_respawn", ""),
			make_tuple("PARTICLE COMPUTE", "particle_compute.cl", "particle_compute", ""),
			make_tuple("PARTICLE SORT LOCAL", "particle_sort.cl", "bitonicSortLocal", lsl_str),
			make_tuple("PARTICLE SORT MERGE GLOBAL", "particle_sort.cl", "bitonicMergeGlobal", lsl_str),
			make_tuple("PARTICLE SORT MERGE LOCAL", "particle_sort.cl", "bitonicMergeLocal", lsl_str),
			make_tuple("PARTICLE COMPUTE DISTANCES", "particle_sort.cl", "compute_distances", lsl_str)
		};
		
		// TODO: make tile size dependent on #cores
		string ir_lighting_flags = (" -DA2E_IR_TILE_SIZE_X="+uint2string(A2E_IR_TILE_SIZE_X) +
									" -DA2E_IR_TILE_SIZE_Y="+uint2string(A2E_IR_TILE_SIZE_Y) +
									" -DA2E_LOCAL_ATOMICS");
		// only add the inferred lighting kernel if there is support for local memory atomics
		if(local_atomics_support) {
			internal_kernels.emplace_back("INFERRED LIGHTING", "ir_lighting.cl", "ir_lighting", ir_lighting_flags);
		}
		
		load_internal_kernels();
	}
	__HANDLE_CL_EXCEPTION_START("init")
		// try another time w/o using the platform devices
		if(platform_index+1 < platforms.size()) {
			a2e_debug("trying next platform ...");
			init(use_platform_devices, platform_index+1);
		}
	__HANDLE_CL_EXCEPTION_END
	catch(a2e_exception& e) {
		a2e_debug("%s", e.what());
		// try another time w/o using the platform devices
		if(platform_index+1 < platforms.size()) {
			a2e_debug("trying next platform ...");
			init(use_platform_devices, platform_index+1);
		}
	}
	
	// if absolutely no devices on any platform are supported, disable opencl support
	if(devices.empty()) {
		supported = false;
		return;
	}
	
	if(ro_formats.empty() && wo_formats.empty() && rw_formats.empty()) {
#if 0
		// context has been created, query image format information
		context->getSupportedImageFormats(CL_MEM_READ_ONLY, CL_MEM_OBJECT_IMAGE2D, &ro_formats);
		context->getSupportedImageFormats(CL_MEM_WRITE_ONLY, CL_MEM_OBJECT_IMAGE2D, &wo_formats);
		context->getSupportedImageFormats(CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE2D, &rw_formats);
		
		//
		array<pair<vector<cl::ImageFormat>&, string>, 3> formats {
			{
				{ ro_formats, "read-only" },
				{ wo_formats, "write-only" },
				{ rw_formats, "read-write" },
			}
		};
		for(const auto& frmts : formats) {
			cout << "## " << frmts.second << " formats:" << endl;
			for(const auto& format : frmts.first) {
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
		}
#endif
	}
}

bool opencl::is_cpu_support() {
	// if a fastest cpu exists, we do have cpu support
	return (fastest_cpu != nullptr);
}

bool opencl::is_gpu_support() {
	// if a fastest gpu exists, we do have gpu support
	return (fastest_gpu != nullptr);
}

opencl::kernel_object* opencl::add_kernel_file(const string& identifier, const string& file_name, const string& func_name, const string additional_options) {
	if(kernels.count(identifier) != 0) {
		a2e_error("kernel \"%s\" already exists!", identifier);
		return kernels[identifier];
	}
	
	core::reset(buffer);
	if(!f->file_to_buffer(file_name, buffer)) {
		return nullptr;
	}
	string kernel_data(buffer.str());
	
	// work around caching bug and modify source on each load, TODO: check if this still exists (still present in 10.6.2)
	kernel_data = "#define __" + core::str_to_upper(func_name) +  "_BUILD_TIME__ " + uint2string((unsigned int)time(nullptr)) + "\n" + kernel_data;
	
	// check if this is an external kernel (and hasn't been added before)
	if(external_kernels.count(identifier) == 0 &&
	   none_of(begin(internal_kernels), end(internal_kernels),
			   [&identifier](const decltype(internal_kernels)::value_type& int_kernel) {
				   return (get<0>(int_kernel) == identifier);
			   })) {
		// if so, add it to the external kernel list
		external_kernels.insert(make_pair(identifier,
										  make_tuple(file_name, func_name, additional_options)));
	}
	
	return add_kernel_src(identifier, kernel_data, func_name, additional_options);
}

opencl::kernel_object* opencl::add_kernel_src(const string& identifier, const string& src, const string& func_name, const string additional_options) {
	a2e_debug("compiling \"%s\" kernel!", identifier);
	string options = build_options;
	try {
		if(kernels.count(identifier) != 0) {
			a2e_error("kernel \"%s\" already exists!", identifier);
			return kernels[identifier];
		}
		
		if(!additional_options.empty()) {
			options += (additional_options[0] != ' ' ? " " : "") + additional_options;
		}
		
#if defined(__APPLE__)
		//options += " -D__APPLE__"; // not necessary any more
#else
		// workaround for the nvidia compiler which apparently defines __APPLE__
		options += " -DUNDEF__APPLE__";
#endif
#if defined(__WINDOWS__)
		options += " -D__WINDOWS__"; // TODO: still necessary?
#endif
		
		// add kernel
		kernels[identifier] = new opencl::kernel_object();
		kernels[identifier]->kernel_name = identifier;
		cl::Program::Sources source(1, make_pair(src.c_str(), src.length()));
		kernels[identifier]->program = new cl::Program(*context, source);
		
		// compile for each device independently to add device-specific defines
		for(const auto& device : devices) {
			vector<cl::Device> cur_device;
			cur_device.push_back(*device->device);
			
			string device_options = "";
			switch(device->vendor_type) {
				case VENDOR::NVIDIA:
					device_options += nv_build_options;
					device_options += " -DNVIDIA";
					break;
				case VENDOR::ATI:
					device_options += " -DATI";
					break;
				case VENDOR::INTEL:
					device_options += " -DINTEL";
					break;
				case VENDOR::AMD:
					device_options += " -DAMD";
					break;
				case VENDOR::APPLE:
					device_options += " -DAPPLE_ARM";
					break;
				case VENDOR::UNKNOWN:
					device_options += " -DUNKNOWN_VENDOR";
					break;
			}
			if((device->internal_type & CL_DEVICE_TYPE_CPU) != 0) device_options += " -DCPU";
			if((device->internal_type & CL_DEVICE_TYPE_GPU) != 0) device_options += " -DGPU";
			if((device->internal_type & CL_DEVICE_TYPE_ACCELERATOR) != 0) device_options += " -DACCELERATOR";
			
			device_options += " -DPLATFORM_"+platform_vendor_to_str(platform_vendor);
			
			kernels[identifier]->program->build(cur_device, (options+device_options).c_str());
		}
		
		kernels[identifier]->kernel = new cl::Kernel(*kernels[identifier]->program, func_name.c_str(), &ierr);
		kernels[identifier]->global = new cl::NDRange(1);
		kernels[identifier]->local = new cl::NDRange(1);
		
		kernels[identifier]->arg_count = kernels[identifier]->kernel->getInfo<CL_KERNEL_NUM_ARGS>();
		kernels[identifier]->args_passed.insert(kernels[identifier]->args_passed.begin(), kernels[identifier]->arg_count, false);

		// print out build log
		/*for(const auto& internal_device : internal_devices) {
			char build_log[CLINFO_STR_SIZE];
			memset(build_log, 0, CLINFO_STR_SIZE);
			kernels[identifier]->program->getBuildInfo(internal_device, CL_PROGRAM_BUILD_LOG, &build_log);
			a2e_debug("build log: %s", build_log);
		}*/
		
		/*size_t device_num = 0;
		for(const auto& device : internal_devices) {
			a2e_log("%s (dev #%u): kernel local memory: %u", identifier, device_num,
					kernels[identifier]->kernel->getWorkGroupInfo<CL_KERNEL_LOCAL_MEM_SIZE>(device));
			a2e_log("%s (dev #%u): work group size: %u", identifier, device_num,
					kernels[identifier]->kernel->getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device));
			device_num++;
		}*/
	}
	__HANDLE_CL_EXCEPTION_START("add_kernel")
		// print out build log
		for(const auto& internal_device : internal_devices) {
			char build_log[CLINFO_STR_SIZE];
			memset(build_log, 0, CLINFO_STR_SIZE);
			kernels[identifier]->program->getBuildInfo(internal_device, CL_PROGRAM_BUILD_LOG, &build_log);
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

void opencl::log_program_binary(const kernel_object* kernel) {
	if(kernel == nullptr) return;
	
	try {
		// if the device is a nvidia gpu (and we are using the nvidia driver), log the ptx data
		size_t device_num = 0;
		vector<size_t> program_sizes = kernel->program->getInfo<CL_PROGRAM_BINARY_SIZES>();
		if(program_sizes.size() == 0) return;
		
		unsigned char** program_binaries = new unsigned char*[program_sizes.size()];
		for(size_t i = 0; i < program_sizes.size(); i++) {
			program_binaries[i] = new unsigned char[program_sizes[i]+1];
		}
		clGetProgramInfo((*kernel->program)(), CL_PROGRAM_BINARIES, program_sizes.size()*sizeof(unsigned char*), &program_binaries[0], nullptr);
		
		string kernel_name = kernel->kernel->getInfo<CL_KERNEL_FUNCTION_NAME>();
		for(const auto& device : devices) {
			if(program_sizes[device_num] > 0) {
				if(device->vendor_type != VENDOR::UNKNOWN) {
					string file_name = kernel_name + string("_") + size_t2string(device_num);
					if(device->vendor_type == VENDOR::NVIDIA) {
						file_name += ".ptx";
					}
					else if(device->vendor_type == VENDOR::INTEL || device->vendor_type == VENDOR::AMD) {
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
					
#if defined(__APPLE__)
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
	
	for(const auto& int_kernel : internal_kernels) {
		check_compilation(add_kernel_file(get<0>(int_kernel),
										  make_kernel_path(get<1>(int_kernel)),
										  get<2>(int_kernel),
										  get<3>(int_kernel)) != nullptr,
						  get<1>(int_kernel));
	}
	
	if(successful_internal_compilation) a2e_debug("internal kernels loaded successfully!");
	else {
		// one or more kernels didn't compile, TODO: use fallback?
		a2e_error("there were problems loading/compiling the internal kernels!");
	}
	
	// load external kernels
	for(const auto& ext_kernel : external_kernels) {
		add_kernel_file(ext_kernel.first,
						get<0>(ext_kernel.second).c_str(),
						get<1>(ext_kernel.second),
						get<2>(ext_kernel.second).c_str());
	}
	if(!external_kernels.empty()) a2e_debug("external kernels loaded successfully!");
}

void opencl::load_internal_kernels() {
	reload_kernels();
	
	if(is_gpu_support()) set_active_device(DEVICE_TYPE::FASTEST_GPU);
	else if(is_cpu_support()) set_active_device(DEVICE_TYPE::FASTEST_CPU);
}

void opencl::use_kernel(const string& identifier) {
	if(kernels.count(identifier) == 0) {
		a2e_error("kernel \"%s\" doesn't exist!", identifier.c_str());
		cur_kernel = nullptr;
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
		if(data != nullptr && (type & opencl::BT_INITIAL_COPY) && !(vtype & opencl::BT_USE_HOST_MEMORY)) vtype |= opencl::BT_INITIAL_COPY;
		if(data != nullptr && (type & opencl::BT_COPY_ON_USE)) vtype |= opencl::BT_COPY_ON_USE;
		if(data != nullptr && (type & opencl::BT_READ_BACK_RESULT)) vtype |= opencl::BT_READ_BACK_RESULT;
		
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
		if(data != nullptr && (vtype & opencl::BT_USE_HOST_MEMORY)) flags |= CL_MEM_USE_HOST_PTR;
		if(data == nullptr && (vtype & opencl::BT_USE_HOST_MEMORY)) flags |= CL_MEM_ALLOC_HOST_PTR;
		
		buffers.back()->type = vtype;
		buffers.back()->flags = flags;
		buffers.back()->data = data;
		return buffers.back();
	}
	__HANDLE_CL_EXCEPTION("create_buffer_object")
	return nullptr;
}

opencl::buffer_object* opencl::create_buffer(opencl::BUFFER_TYPE type, size_t size, void* data) {
	if(size == 0) {
		return nullptr;
	}
	
	try {
		buffer_object* buffer_obj = create_buffer_object(type, data);
		if(buffer_obj == nullptr) return nullptr;
		
		buffer_obj->size = size;
		buffer_obj->buffer = new cl::Buffer(*context, buffer_obj->flags, size,
											((buffer_obj->type & opencl::BT_INITIAL_COPY) || (buffer_obj->type & opencl::BT_USE_HOST_MEMORY) ? data : nullptr),
											&ierr);
		return buffer_obj;
	}
	__HANDLE_CL_EXCEPTION("create_buffer")
	return nullptr;
}

opencl::buffer_object* opencl::create_image2d_buffer(opencl::BUFFER_TYPE type, cl_channel_order channel_order, cl_channel_type channel_type, size_t width, size_t height, void* data) {
	try {
		buffer_object* buffer_obj = create_buffer_object(type, data);
		if(buffer_obj == nullptr) return nullptr;
		
		buffer_obj->format.image_channel_order = channel_order;
		buffer_obj->format.image_channel_data_type = channel_type;
		buffer_obj->origin.set(0, 0, 0);
		buffer_obj->region.set(width, height, 1); // depth must be 1 for 2d images
		buffer_obj->image_buffer = new cl::Image2D(*context, buffer_obj->flags, buffer_obj->format, width, height, 0, data, &ierr);
		return buffer_obj;
	}
	__HANDLE_CL_EXCEPTION("create_image2d_buffer")
	return nullptr;
}

opencl::buffer_object* opencl::create_image3d_buffer(opencl::BUFFER_TYPE type, cl_channel_order channel_order, cl_channel_type channel_type, size_t width, size_t height, size_t depth, void* data) {
	try {
		buffer_object* buffer_obj = create_buffer_object(type, data);
		if(buffer_obj == nullptr) return nullptr;
		
		buffer_obj->format.image_channel_order = channel_order;
		buffer_obj->format.image_channel_data_type = channel_type;
		buffer_obj->origin.set(0, 0, 0);
		buffer_obj->region.set(width, height, depth);
		buffer_obj->image_buffer = new cl::Image3D(*context, buffer_obj->flags, buffer_obj->format, width, height, depth, 0, 0, data, &ierr);
		return buffer_obj;
	}
	__HANDLE_CL_EXCEPTION("create_image3d_buffer")
	return nullptr;
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
		buffers.back()->data = nullptr;
		buffers.back()->size = 0;
		buffers.back()->buffer = new cl::BufferGL(*context, flags, ogl_buffer, &ierr);
		return buffers.back();
	}
	__HANDLE_CL_EXCEPTION("create_ogl_buffer")
	return nullptr;
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
		buffers.back()->data = nullptr;
		buffers.back()->size = 0;
		buffers.back()->image_buffer = new cl::Image2DGL(*context, flags, target, 0, texture, &ierr);
		return buffers.back();
	}
	__HANDLE_CL_EXCEPTION("create_ogl_image2d_buffer")
	return nullptr;
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
		buffers.back()->data = nullptr;
		buffers.back()->size = 0;
		buffers.back()->buffer = new cl::BufferRenderGL(*context, flags, renderbuffer, &ierr);
		return buffers.back();
	}
	__HANDLE_CL_EXCEPTION("create_ogl_image2d_renderbuffer")
	return nullptr;
}

void opencl::delete_buffer(opencl::buffer_object* buffer_obj) {
	// remove buffer from each associated kernel (and unset the kernel argument)
	for(const auto& associated_kernel : buffer_obj->associated_kernels) {
		for(const auto& arg_num : associated_kernel.second) {
			associated_kernel.first->args_passed[arg_num] = false;
			associated_kernel.first->buffer_args.erase(arg_num);
		}
	}
	buffer_obj->associated_kernels.clear();
	if(buffer_obj->buffer != nullptr) delete buffer_obj->buffer;
	if(buffer_obj->image_buffer != nullptr) delete buffer_obj->image_buffer;
	const auto iter = find(begin(buffers), end(buffers), buffer_obj);
	if(iter != end(buffers)) buffers.erase(iter);
}

void opencl::write_buffer(opencl::buffer_object* buffer_obj, const void* src, const size_t offset, const size_t size) {
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
	try {
		size3 origin3(origin.x, origin.y, 0); // origin z must be 0 for 2d images
		size3 region3(region.x, region.y, 1); // depth must be 1 for 2d images
		queues[active_device->device]->enqueueWriteImage(*buffer_obj->image_buffer, ((buffer_obj->type & opencl::BT_BLOCK_ON_WRITE) > 0),
														 (cl::size_t<3>&)origin3, (cl::size_t<3>&)region3, 0, 0, (void*)src);
	}
	__HANDLE_CL_EXCEPTION("write_image2d")
}

void opencl::write_image3d(opencl::buffer_object* buffer_obj, const void* src, size3 origin, size3 region) {
	try {
		queues[active_device->device]->enqueueWriteImage(*buffer_obj->image_buffer, ((buffer_obj->type & opencl::BT_BLOCK_ON_WRITE) > 0),
														 (cl::size_t<3>&)origin, (cl::size_t<3>&)region, 0, 0, (void*)src);
	}
	__HANDLE_CL_EXCEPTION("write_buffer")
}

void opencl::read_buffer(void* dst, opencl::buffer_object* buffer_obj) {
	try {
		queues[active_device->device]->enqueueReadBuffer(*buffer_obj->buffer, ((buffer_obj->type & opencl::BT_BLOCK_ON_READ) > 0),
														 0, buffer_obj->size, dst);
	}
	__HANDLE_CL_EXCEPTION("read_buffer")
}

void opencl::set_active_device(opencl::DEVICE_TYPE dev) {
	switch(dev) {
		case DEVICE_TYPE::FASTEST_GPU:
			if(fastest_gpu != nullptr) {
				active_device = fastest_gpu;
				return;
			}
			break;
		case DEVICE_TYPE::FASTEST_CPU:
			if(fastest_cpu != nullptr) {
				active_device = fastest_cpu;
				return;
			}
			break;
		case DEVICE_TYPE::ALL_GPU:
			// TODO: ...
			break;
		case DEVICE_TYPE::ALL_CPU:
			// TODO: ...
			break;
		case DEVICE_TYPE::ALL_DEVICES:
			// TODO: ...
			break;
		case DEVICE_TYPE::NONE:
		default:
			break;
	}
	
	if((dev >= DEVICE_TYPE::GPU0 && dev <= DEVICE_TYPE::GPU255) ||
	   (dev >= DEVICE_TYPE::CPU0 && dev <= DEVICE_TYPE::CPU255)) {
		for(const auto& device : devices) {
			if(device->type == dev) {
				active_device = device;
				return;
			}
		}
	}
	
	if(active_device != nullptr) {
		a2e_error("can't use device %u - keeping current one (%u)!", dev, active_device->type);
	}
	else {
		// TODO: use _any_ device if there is at least one available ...
		a2e_error("can't use device %u and no other device is currently active!", dev);
	}
}

void opencl::set_kernel_range(const cl::NDRange& global, const cl::NDRange& local) {
	if(cur_kernel == nullptr) return;
	
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
		for(const auto& buffer_arg : kernel_obj->buffer_args) {
			if((buffer_arg.second->type & opencl::BT_COPY_ON_USE) != 0) write_buffer(buffer_arg.second, buffer_arg.second->data);
			if((buffer_arg.second->type & opencl::BT_OPENGL_BUFFER) != 0 &&
			   !buffer_arg.second->manual_gl_sharing) {
				gl_objects.push_back(*(buffer_arg.second->buffer != nullptr ? (cl::Memory*)buffer_arg.second->buffer : (cl::Memory*)buffer_arg.second->image_buffer));
				kernel_obj->has_ogl_buffers = true;
			}
		}
		if(!gl_objects.empty()) {
			queues[active_device->device]->enqueueAcquireGLObjects(&gl_objects);
		}
		
		cl::KernelFunctor func = kernel_obj->kernel->bind(*queues[active_device->device], *kernel_obj->global, *kernel_obj->local);
		//func().wait();
		func();
		
		for(const auto& buffer_arg : kernel_obj->buffer_args) {
			if((buffer_arg.second->type & opencl::BT_READ_BACK_RESULT) != 0) read_buffer(buffer_arg.second->data, buffer_arg.second);
		}
		
		for_each(begin(kernel_obj->buffer_args), end(kernel_obj->buffer_args),
				 [this](const pair<const unsigned int, buffer_object*>& buffer_arg) {
					 if((buffer_arg.second->type & opencl::BT_DELETE_AFTER_USE) != 0) {
						 this->delete_buffer(buffer_arg.second);
					 }
				 });
		
		if(kernel_obj->has_ogl_buffers && !gl_objects.empty()) {
			queues[active_device->device]->enqueueReleaseGLObjects(&gl_objects);
		}
	}
	__HANDLE_CL_EXCEPTION_EXT("run_kernel", (" - in kernel: "+kernel_obj->kernel_name).c_str())
}

void opencl::finish() {
	queues[active_device->device]->finish();
}

void opencl::flush() {
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

bool opencl::set_kernel_argument(const unsigned int& index, opencl::buffer_object* arg) {
	if((arg->buffer != nullptr && set_kernel_argument(index, (*arg->buffer)())) ||
	   (arg->image_buffer != nullptr && set_kernel_argument(index, *(cl::Memory*)arg->image_buffer))) {
		cur_kernel->buffer_args[index] = arg;
		arg->associated_kernels[cur_kernel].push_back(index);
		return true;
	}
	return false;
}

bool opencl::set_kernel_argument(const unsigned int& index, const opencl::buffer_object* arg) {
	return set_kernel_argument(index, (opencl::buffer_object*)arg);
}

bool opencl::set_kernel_argument(const unsigned int& index, size_t size, void* arg) {
	try {
		cur_kernel->kernel->setArg(index, size, arg);
		cur_kernel->args_passed[index] = true;
		return true;
	}
	__HANDLE_CL_EXCEPTION("set_kernel_argument")
	return false;
}

opencl::device_object* opencl::get_device(opencl::DEVICE_TYPE device) {
	if(device == DEVICE_TYPE::FASTEST_GPU) return fastest_gpu;
	else if(device == DEVICE_TYPE::FASTEST_CPU) return fastest_cpu;
	else {
		if((device >= DEVICE_TYPE::GPU0 && device <= DEVICE_TYPE::GPU255) ||
		   (device >= DEVICE_TYPE::CPU0 && device <= DEVICE_TYPE::CPU255)) {
			for(const auto& dev : devices) {
				if(dev->type == device) {
					return dev;
				}
			}
		}
	}
	return nullptr;
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
		
		void* map_ptr = nullptr;
		if(buffer_obj->buffer != nullptr) {
			map_ptr = queues[active_device->device]->enqueueMapBuffer(*buffer_obj->buffer, blocking, map_flags, 0, buffer_obj->size);
		}
		else if(buffer_obj->image_buffer != nullptr) {
			size_t row_pitch, slice_pitch;
			map_ptr = queues[active_device->device]->enqueueMapImage(*buffer_obj->image_buffer, blocking, map_flags,
																				 (cl::size_t<3>&)buffer_obj->origin,
																				 (cl::size_t<3>&)buffer_obj->region,
																				 &row_pitch, &slice_pitch);
		}
		else {
			a2e_error("unknown buffer object!");
			return nullptr;
		}
		return map_ptr;
	}
	__HANDLE_CL_EXCEPTION("map_buffer")
	return nullptr;
}

void opencl::unmap_buffer(opencl::buffer_object* buffer_obj, void* map_ptr) {
	try {
		void* buffer_ptr = nullptr;
		if(buffer_obj->buffer != nullptr) buffer_ptr = buffer_obj->buffer;
		else if(buffer_obj->image_buffer != nullptr) buffer_ptr = buffer_obj->image_buffer;
		else {
			a2e_error("unknown buffer object!");
			return;
		}
		queues[active_device->device]->enqueueUnmapMemObject(*(cl::Memory*)buffer_ptr, map_ptr);
	}
	__HANDLE_CL_EXCEPTION("unmap_buffer")
}

bool opencl::has_vendor_device(opencl::VENDOR vendor_type) {
	for(const auto& device : devices) {
		if(device->vendor_type == vendor_type) return true;
	}
	return false;
}

size_t opencl::get_kernel_work_group_size() {
	if(cur_kernel == nullptr || active_device == nullptr) return 0;
	
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
	if(cur_kernel == nullptr || active_device == nullptr) {
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
	gl_objects.push_back(*(gl_buffer_obj->buffer != nullptr ?
						   (cl::Memory*)gl_buffer_obj->buffer :
						   (cl::Memory*)gl_buffer_obj->image_buffer));
	queues[active_device->device]->enqueueAcquireGLObjects(&gl_objects);
}

void opencl::release_gl_object(buffer_object* gl_buffer_obj) {
	vector<cl::Memory> gl_objects;
	gl_objects.push_back(*(gl_buffer_obj->buffer != nullptr ?
						   (cl::Memory*)gl_buffer_obj->buffer :
						   (cl::Memory*)gl_buffer_obj->image_buffer));
	queues[active_device->device]->enqueueReleaseGLObjects(&gl_objects);
}

string opencl::platform_vendor_to_str(const opencl::PLATFORM_VENDOR pvendor) const {
	switch(pvendor) {
		case PLATFORM_VENDOR::NVIDIA: return "NVIDIA";
		case PLATFORM_VENDOR::INTEL: return "INTEL";
		case PLATFORM_VENDOR::AMD: return "AMD";
		case PLATFORM_VENDOR::APPLE: return "APPLE";
		case PLATFORM_VENDOR::UNKNOWN: break;
	}
	return "UNKNOWN";
}

#endif
