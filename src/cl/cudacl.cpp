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

#if defined(A2E_CUDA_CL)

#include "opencl.h"
#include "cudacl_translator.h"
#include "zlib.h"

#if defined(__APPLE__)
#include <CUDA/cuda.h>
#include <CUDA/cudaGL.h>
#else
#include <cuda.h>
#include <cudaGL.h>
#endif

//
class A2E_API cudacl {
public:
	//
	cudacl() {
		CUresult cu_err = CUDA_SUCCESS;
		cu_err = cuInit(0);
		if(cu_err != CUDA_SUCCESS) {
			a2e_error("failed to initialize CUDA: %i", cu_err);
			valid = false;
		}
		
		//
		int driver_version = 0;
		cuDriverGetVersion(&driver_version);
		if((driver_version/1000) < 5) {
			a2e_error("A2E requires at least CUDA 5.0!");
			valid = false;
		}
		
		//
		int device_count = 0;
		if(cuDeviceGetCount(&device_count) != CUDA_SUCCESS) {
			a2e_error("cuDeviceGetCount failed!");
			valid = false;
		}
		if(device_count == 0) {
			a2e_error("there is no device that supports CUDA!");
			valid = false;
		}
	}
	~cudacl() {}
	
	// vars
	bool valid = true;
	string cache_path = "";
	string cc_target_str = "10";
	unsigned int cc_target = CU_TARGET_COMPUTE_10;
	vector<CUdevice*> devices;
	unordered_map<opencl::device_object*, const CUdevice*> device_map;
	unordered_map<const CUdevice*, CUcontext*> contexts;
	unordered_map<const CUdevice*, CUstream*> queues;
	unordered_map<opencl::buffer_object*, CUdeviceptr*> buffers;
	unordered_map<opencl::buffer_object*, CUgraphicsResource*> gl_buffers;
	unordered_map<CUgraphicsResource*, CUdeviceptr*> mapped_gl_buffers;
	
	struct cuda_kernel_object {
		CUmodule* module = nullptr;
		CUfunction* function = nullptr;
		const cudacl_kernel_info info;
		
		// <arg#, <arg size, arg ptr>>
		struct kernel_arg {
			size_t size = 0;
			void* ptr = nullptr;
			bool free_ptr = true;
		};
		unordered_map<cl_uint, kernel_arg> arguments;
		
		cuda_kernel_object(const cudacl_kernel_info& info_) : info(info_) {}
		~cuda_kernel_object() {
			for(const auto& arg : arguments) {
				if(arg.second.free_ptr &&
				   arg.second.ptr != nullptr) {
					free(arg.second.ptr);
				}
			}
			arguments.clear();
			
			if(function != nullptr) {
				delete function;
			}
			if(module != nullptr) {
				// no CU, since we shouldn't throw here and it doesn't really matter if the unload fails
				cuModuleUnload(*module);
				delete module;
			}
		}
	};
	unordered_map<opencl::kernel_object*, cuda_kernel_object*> kernels;
	
};

//
#define __ERROR_CODE_INFO(F) \
F(CUDA_SUCCESS) \
F(CUDA_ERROR_INVALID_VALUE) \
F(CUDA_ERROR_OUT_OF_MEMORY) \
F(CUDA_ERROR_NOT_INITIALIZED) \
F(CUDA_ERROR_DEINITIALIZED) \
F(CUDA_ERROR_PROFILER_DISABLED) \
F(CUDA_ERROR_PROFILER_NOT_INITIALIZED) \
F(CUDA_ERROR_PROFILER_ALREADY_STARTED) \
F(CUDA_ERROR_PROFILER_ALREADY_STOPPED) \
F(CUDA_ERROR_NO_DEVICE) \
F(CUDA_ERROR_INVALID_DEVICE) \
F(CUDA_ERROR_INVALID_IMAGE) \
F(CUDA_ERROR_INVALID_CONTEXT) \
F(CUDA_ERROR_CONTEXT_ALREADY_CURRENT) \
F(CUDA_ERROR_MAP_FAILED) \
F(CUDA_ERROR_UNMAP_FAILED) \
F(CUDA_ERROR_ARRAY_IS_MAPPED) \
F(CUDA_ERROR_ALREADY_MAPPED) \
F(CUDA_ERROR_NO_BINARY_FOR_GPU) \
F(CUDA_ERROR_ALREADY_ACQUIRED) \
F(CUDA_ERROR_NOT_MAPPED) \
F(CUDA_ERROR_NOT_MAPPED_AS_ARRAY) \
F(CUDA_ERROR_NOT_MAPPED_AS_POINTER) \
F(CUDA_ERROR_ECC_UNCORRECTABLE) \
F(CUDA_ERROR_UNSUPPORTED_LIMIT) \
F(CUDA_ERROR_CONTEXT_ALREADY_IN_USE) \
F(CUDA_ERROR_PEER_ACCESS_UNSUPPORTED) \
F(CUDA_ERROR_INVALID_SOURCE) \
F(CUDA_ERROR_FILE_NOT_FOUND) \
F(CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND) \
F(CUDA_ERROR_SHARED_OBJECT_INIT_FAILED) \
F(CUDA_ERROR_OPERATING_SYSTEM) \
F(CUDA_ERROR_INVALID_HANDLE) \
F(CUDA_ERROR_NOT_FOUND) \
F(CUDA_ERROR_NOT_READY) \
F(CUDA_ERROR_LAUNCH_FAILED) \
F(CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES) \
F(CUDA_ERROR_LAUNCH_TIMEOUT) \
F(CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING) \
F(CUDA_ERROR_PEER_ACCESS_ALREADY_ENABLED) \
F(CUDA_ERROR_PEER_ACCESS_NOT_ENABLED) \
F(CUDA_ERROR_PRIMARY_CONTEXT_ACTIVE) \
F(CUDA_ERROR_CONTEXT_IS_DESTROYED) \
F(CUDA_ERROR_ASSERT) \
F(CUDA_ERROR_TOO_MANY_PEERS) \
F(CUDA_ERROR_HOST_MEMORY_ALREADY_REGISTERED) \
F(CUDA_ERROR_HOST_MEMORY_NOT_REGISTERED) \
F(CUDA_ERROR_UNKNOWN)

#define __DECLARE_ERROR_CODE_TO_STRING(code) case code: return #code;

const char* opencl::error_code_to_string(cl_int error_code) {
	switch(error_code) {
		__ERROR_CODE_INFO(__DECLARE_ERROR_CODE_TO_STRING);
		default:
			return "UNKNOWN CUDA ERROR";
	}
}

class cudacl_exception : public exception {
protected:
	const int error_code;
	const string error_str;
public:
	cudacl_exception(const int& err_code) : error_code(err_code), error_str("") {}
	cudacl_exception(const string& err_str) : error_code(~0), error_str(err_str) {}
	cudacl_exception(const int& err_code, const string& err_str) : error_code(err_code), error_str(err_str) {}
	~cudacl_exception() throw() {}
	virtual const char* what() const throw ();
	virtual const int& code() const throw ();
};
const char* cudacl_exception::what() const throw () {
	return error_str.c_str();
}
const int& cudacl_exception::code() const throw () {
	return error_code;
}

#define __HANDLE_CL_EXCEPTION_START(func_str) __HANDLE_CL_EXCEPTION_START_EXT(func_str, "")
#define __HANDLE_CL_EXCEPTION_START_EXT(func_str, additional_info)				\
catch(cudacl_exception err) {													\
	a2e_error("line #%s, " func_str "(): %s (%d)%s!",							\
			  __LINE__, err.what(), err.code(), additional_info);
#define __HANDLE_CL_EXCEPTION_END }
#define __HANDLE_CL_EXCEPTION(func_str) __HANDLE_CL_EXCEPTION_START(func_str) __HANDLE_CL_EXCEPTION_END
#define __HANDLE_CL_EXCEPTION_EXT(func_str, additional_info) __HANDLE_CL_EXCEPTION_START_EXT(func_str, additional_info) __HANDLE_CL_EXCEPTION_END

//
#define CU(_CUDA_CALL) {														\
	CUresult _cu_err = _CUDA_CALL;												\
	/* check if call was successful, or if cuda is already shutting down, */	\
	/* in which case we just pretend nothing happened and continue ...    */	\
	if(_cu_err != CUDA_SUCCESS && _cu_err != CUDA_ERROR_DEINITIALIZED) {		\
		a2e_error("cuda driver error #%i: %s (%s)",								\
				  _cu_err, error_code_to_string(_cu_err), #_CUDA_CALL);			\
		throw cudacl_exception(CL_OUT_OF_RESOURCES, "cuda driver error");		\
	}																			\
}

opencl::opencl(const char* kernel_path, SDL_Window* wnd, const bool clear_cache) {
	opencl::sdl_wnd = wnd;
	opencl::kernel_path_str = kernel_path;
	
	context = nullptr;
	cur_kernel = nullptr;
	active_device = nullptr;
	
	fastest_cpu = nullptr;
	fastest_gpu = nullptr;
	
	build_options = "-I " + kernel_path_str;
	
	// clear cuda cache
	if(clear_cache) {
		// TODO (ignore this for now)
	}
	
	//
	ccl = new cudacl();
	if(!ccl->valid) {
		supported = false;
	}
	
	// check if the cache can be used
	ccl->cache_path = kernel_path_str.substr(0, kernel_path_str.rfind('/', kernel_path_str.length()-2)) + "/cache/";
	if(file_io::is_file(ccl->cache_path+"CACHECRC")) {
		file_io crc_file(ccl->cache_path+"CACHECRC", file_io::OPEN_TYPE::READ);
		if(crc_file.is_open()) {
			use_ptx_cache = true;
			
			auto& crc_fstream = *crc_file.get_filestream();
			unordered_map<string, unsigned int> cache_crcs;
			while(!crc_fstream.eof()) {
				string kernel_fname = "";
				unsigned int kernel_crc = 0;
				crc_fstream >> kernel_fname;
				crc_fstream >> hex >> kernel_crc >> dec;
				if(kernel_fname.empty()) continue;
				cache_crcs.insert({ kernel_fname, kernel_crc });
			}
			
			const auto kernel_files = core::get_file_list(kernel_path_str);
			for(const auto& kfile : kernel_files) {
				if(kfile.first[0] == '.') continue;
				stringstream buffer(stringstream::in | stringstream::out);
				if(!file_io::file_to_buffer(kernel_path+kfile.first, buffer)) {
					a2e_error("failed to read kernel source for \"%s\"!", kernel_path+kfile.first);
					return;
				}
				const string src(buffer.str());
				const unsigned int crc = (unsigned int)crc32(crc32(0L, Z_NULL, 0), (const Bytef*)src.c_str(), (uInt)src.size());
				
				//
				const auto cache_iter = cache_crcs.find(kfile.first);
				if(cache_iter == cache_crcs.end()) {
					use_ptx_cache = false;
					a2e_error("kernel file \"%s\" doesn't exist in cache!", kfile.first);
					break;
				}
				
				if(cache_iter->second != crc) {
					use_ptx_cache = false;
					a2e_error("CRC mismatch for kernel file \"%s\": %X (cache) != %X (current)", kfile.first, cache_iter->second, crc);
					break;
				}
			}
			crc_file.close();
		}
	}
	if(use_ptx_cache) {
		a2e_debug("using ptx cache!");
	}
}

opencl::~opencl() {
	a2e_debug("deleting opencl object");
	
	// delete cl/cuda buffers
	while(!buffers.empty()) {
		delete_buffer(buffers.back());
	}
	
	// delete kernels
	while(!ccl->kernels.empty()) {
		const auto iter = ccl->kernels.begin();
		delete iter->second;
		ccl->kernels.erase(iter);
	}
	destroy_kernels();
	
	// delete the rest (devices, contexts, streams)
	for(const auto& device : ccl->devices) {
		CUcontext* ctx = ccl->contexts[device];
		CUstream* stream = ccl->queues[device];
		CU(cuCtxSetCurrent(*ctx));
		CU(cuStreamDestroy(*stream));
		CU(cuCtxDestroy(*ctx));
		delete stream;
		delete device;
		delete ctx;
	}
	ccl->contexts.clear();
	ccl->queues.clear();
	ccl->devices.clear();
	
	for(const auto& cl_device : devices) {
		delete cl_device;
	}
	devices.clear();
	
	delete ccl;
	
	a2e_debug("opencl object deleted");
}

void opencl::init(bool use_platform_devices a2e_unused, const size_t platform_index a2e_unused, const set<string> device_restriction a2e_unused) {
	//
	if(!supported) return;
	
	platform_vendor = PLATFORM_VENDOR::NVIDIA;
	
	//
	int device_count = 0;
	unsigned int fastest_gpu_score = 0;
	unsigned int gpu_score = 0;
	cuDeviceGetCount(&device_count);
	for(int cur_device = 0; cur_device < device_count; cur_device++) {
		// get and create device
		CUdevice* cuda_dev = new CUdevice();
		CUresult cu_err = cuDeviceGet(cuda_dev, cur_device);
		ccl->devices.emplace_back(cuda_dev);
		if(cu_err != CUDA_SUCCESS) {
			a2e_error("failed to get device #%i: %i", cur_device, cu_err);
			continue;
		}
		CUdevice& cuda_device = *cuda_dev;
		
		// get all attributes
		char dev_name[256];
		memset(dev_name, 0, 256);
		CU(cuDeviceGetName(dev_name, 255, cuda_device));
		
		pair<int, int> cc;
		CU(cuDeviceComputeCapability(&cc.first, &cc.second, cuda_device));
		
		switch(cc.first) {
			case 0:
				a2e_error("invalid compute capability: %u.%u", cc.first, cc.second);
				break;
			case 1:
				switch(cc.second) {
					case 0:
						ccl->cc_target_str = "10";
						ccl->cc_target = CU_TARGET_COMPUTE_10;
						break;
					case 1:
						ccl->cc_target_str = "11";
						ccl->cc_target = CU_TARGET_COMPUTE_11;
						break;
					case 2:
						ccl->cc_target_str = "12";
						ccl->cc_target = CU_TARGET_COMPUTE_12;
						break;
					case 3:
					default: // ignore invalid ones ...
						ccl->cc_target_str = "13";
						ccl->cc_target = CU_TARGET_COMPUTE_13;
						break;
				}
				break;
			case 2:
				switch(cc.second) {
					case 0:
						ccl->cc_target_str = "20";
						ccl->cc_target = CU_TARGET_COMPUTE_20;
						break;
					case 1:
					default: // ignore invalid ones ...
						ccl->cc_target_str = "21";
						ccl->cc_target = CU_TARGET_COMPUTE_21;
						break;
				}
				break;
			case 3:
			default: // default higher ones to highest 3.x (drivers already mention sm_40 and sm_50)
				switch(cc.second) {
					case 0:
						ccl->cc_target_str = "30";
						ccl->cc_target = CU_TARGET_COMPUTE_30;
						break;
					case 2:
						// this is inofficial, but support it anyways ...
						ccl->cc_target_str = "30";
						ccl->cc_target = CU_TARGET_COMPUTE_30;
						break;
					case 5:
					default: // ignore invalid ones ...
						ccl->cc_target_str = "35";
						ccl->cc_target = CU_TARGET_COMPUTE_35;
						break;
				}
				break;
		}
		
		bool fp64 = cc.first > 1 || (cc.first == 1 && cc.second >= 3);
		string extensions = "cl_APPLE_gl_sharing cl_khr_byte_addressable_store cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_fp16 cl_nv_device_attribute_query cl_nv_pragma_unroll";
		if(fp64) extensions += " cl_khr_fp64";
		extensions += " "; // add additional space char, since some applications get confused otherwise
		
		size_t global_mem;
		CU(cuDeviceTotalMem(&global_mem, cuda_device));
		
		int vendor_id, proc_count, const_mem, local_mem, priv_mem, cache_size;
		CU(cuDeviceGetAttribute(&vendor_id, CU_DEVICE_ATTRIBUTE_PCI_DEVICE_ID, cuda_device));
		CU(cuDeviceGetAttribute(&proc_count, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, cuda_device));
		CU(cuDeviceGetAttribute(&const_mem, CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY, cuda_device));
		CU(cuDeviceGetAttribute(&local_mem, CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK, cuda_device));
		CU(cuDeviceGetAttribute(&priv_mem, CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK, cuda_device));
		CU(cuDeviceGetAttribute(&cache_size, CU_DEVICE_ATTRIBUTE_L2_CACHE_SIZE, cuda_device));
		cache_size = (cache_size < 0 ? 0 : cache_size);
		
		int warp_size, max_work_group_size, memory_pitch;
		tuple<int, int, int> max_work_item_size;
		tuple<int, int, int> max_grid_dim;
		CU(cuDeviceGetAttribute(&warp_size, CU_DEVICE_ATTRIBUTE_WARP_SIZE, cuda_device));
		CU(cuDeviceGetAttribute(&max_work_group_size, CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK, cuda_device));
		CU(cuDeviceGetAttribute(&memory_pitch, CU_DEVICE_ATTRIBUTE_MAX_PITCH, cuda_device));
		CU(cuDeviceGetAttribute(&get<0>(max_work_item_size), CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X, cuda_device));
		CU(cuDeviceGetAttribute(&get<1>(max_work_item_size), CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y, cuda_device));
		CU(cuDeviceGetAttribute(&get<2>(max_work_item_size), CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z, cuda_device));
		CU(cuDeviceGetAttribute(&get<0>(max_grid_dim), CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X, cuda_device));
		CU(cuDeviceGetAttribute(&get<1>(max_grid_dim), CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y, cuda_device));
		CU(cuDeviceGetAttribute(&get<2>(max_grid_dim), CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z, cuda_device));
		
		tuple<int, int> max_image_2d;
		tuple<int, int, int> max_image_3d;
		CU(cuDeviceGetAttribute(&get<0>(max_image_2d), CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_WIDTH, cuda_device));
		CU(cuDeviceGetAttribute(&get<1>(max_image_2d), CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_HEIGHT, cuda_device));
		CU(cuDeviceGetAttribute(&get<0>(max_image_3d), CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_WIDTH, cuda_device));
		CU(cuDeviceGetAttribute(&get<1>(max_image_3d), CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_HEIGHT, cuda_device));
		CU(cuDeviceGetAttribute(&get<2>(max_image_3d), CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_DEPTH, cuda_device));
		
		int clock_rate, mem_clock_rate, mem_bus_width, async_engine_count, tex_align;
		CU(cuDeviceGetAttribute(&clock_rate, CU_DEVICE_ATTRIBUTE_CLOCK_RATE, cuda_device));
		CU(cuDeviceGetAttribute(&mem_clock_rate, CU_DEVICE_ATTRIBUTE_MEMORY_CLOCK_RATE, cuda_device));
		CU(cuDeviceGetAttribute(&mem_bus_width, CU_DEVICE_ATTRIBUTE_GLOBAL_MEMORY_BUS_WIDTH, cuda_device));
		CU(cuDeviceGetAttribute(&async_engine_count, CU_DEVICE_ATTRIBUTE_ASYNC_ENGINE_COUNT, cuda_device));
		CU(cuDeviceGetAttribute(&tex_align, CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT, cuda_device));
		
		int exec_timeout, overlap, map_host_memory, integrated, concurrent, ecc, tcc, unified_memory;
		CU(cuDeviceGetAttribute(&exec_timeout, CU_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT, cuda_device));
		CU(cuDeviceGetAttribute(&overlap, CU_DEVICE_ATTRIBUTE_GPU_OVERLAP, cuda_device));
		CU(cuDeviceGetAttribute(&map_host_memory, CU_DEVICE_ATTRIBUTE_CAN_MAP_HOST_MEMORY, cuda_device));
		CU(cuDeviceGetAttribute(&integrated, CU_DEVICE_ATTRIBUTE_INTEGRATED, cuda_device));
		CU(cuDeviceGetAttribute(&concurrent, CU_DEVICE_ATTRIBUTE_CONCURRENT_KERNELS, cuda_device));
		CU(cuDeviceGetAttribute(&ecc, CU_DEVICE_ATTRIBUTE_ECC_ENABLED, cuda_device));
		CU(cuDeviceGetAttribute(&tcc, CU_DEVICE_ATTRIBUTE_TCC_DRIVER, cuda_device));
		CU(cuDeviceGetAttribute(&unified_memory, CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING, cuda_device));
		
		//
		opencl::device_object* device = new opencl::device_object();
		ccl->device_map[device] = cuda_dev;
		device->device = nullptr;
		device->internal_type = CL_DEVICE_TYPE_GPU;
		device->units = proc_count;
		device->clock = clock_rate / 1000;
		device->mem_size = global_mem;
		device->name = dev_name;
		device->vendor = "NVIDIA";
		device->version = "OpenCL 1.1";
		device->driver_version = "CLH 1.0";
		device->extensions = extensions;
		device->vendor_type = VENDOR::NVIDIA;
		device->type = (opencl::DEVICE_TYPE)cur_device;
		device->max_alloc = global_mem;
		device->max_wg_size = max_work_group_size;
		device->img_support = true;
		device->max_img_2d.set(get<0>(max_image_2d), get<1>(max_image_2d));
		device->max_img_3d.set(get<0>(max_image_3d), get<1>(max_image_3d), get<2>(max_image_3d));
		
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
		
		devices.push_back(device);
		
		// TYPE (Units: %, Clock: %): Name, Vendor, Version, Driver Version
		const string dev_type_str = "GPU ";
		a2e_debug("%s(Units: %u, Clock: %u MHz, Memory: %u MB): %s %s, %s / %s",
				  dev_type_str,
				  device->units,
				  device->clock,
				  (unsigned int)round(double(device->mem_size) / 1048576.0),
				  device->vendor,
				  device->name,
				  device->version,
				  device->driver_version);
	}
	
	// no supported devices found -> disable opencl/cudacl support
	if(devices.empty()) {
		a2e_error("no supported device found for this platform!");
		supported = false;
		return;
	}
	
	// create a (single) command queue (-> cuda context and stream) for each device
	for(const auto& device : ccl->devices) {
		CUcontext* ctx = new CUcontext();
		ccl->contexts[device] = ctx;
		CU(cuCtxCreate(ctx, CU_CTX_SCHED_AUTO, *device));
		
		CUstream* cuda_stream = new CUstream();
		CU(cuStreamCreate(cuda_stream, 0));
		ccl->queues[device] = cuda_stream;
	}
	CU(cuCtxSetCurrent(*ccl->contexts[ccl->devices[0]]));
	
	if(fastest_gpu != nullptr) a2e_debug("fastest GPU device: %s %s (score: %u)", fastest_gpu->vendor.c_str(), fastest_gpu->name.c_str(), fastest_gpu_score);
	
	// compile internal kernels	
	size_t local_size_limit = std::max((size_t)512, devices[0]->max_wg_size); // default to 512
	const string lsl_str = " -DLOCAL_SIZE_LIMIT="+size_t2string(local_size_limit);
	
	internal_kernels = { // first time init:
		make_tuple("PARTICLE_INIT", "particle_spawn.cl", "particle_init", " -DA2E_PARTICLE_INIT"),
		make_tuple("PARTICLE_RESPAWN", "particle_spawn.cl", "particle_respawn", ""),
		make_tuple("PARTICLE_COMPUTE", "particle_compute.cl", "particle_compute", ""),
		make_tuple("PARTICLE_SORT_LOCAL", "particle_sort.cl", "bitonicSortLocal", lsl_str),
		make_tuple("PARTICLE_SORT_MERGE_GLOBAL", "particle_sort.cl", "bitonicMergeGlobal", lsl_str),
		make_tuple("PARTICLE_SORT_MERGE_LOCAL", "particle_sort.cl", "bitonicMergeLocal", lsl_str),
		make_tuple("PARTICLE_COMPUTE_DISTANCES", "particle_sort.cl", "compute_distances", lsl_str)
	};
	
	// TODO: support this in cuda
	// TODO: make tile size dependent on #cores
	/*string ir_lighting_flags = (" -DA2E_IR_TILE_SIZE_X="+uint2string(A2E_IR_TILE_SIZE_X) +
								" -DA2E_IR_TILE_SIZE_Y="+uint2string(A2E_IR_TILE_SIZE_Y) +
								" -DA2E_LOCAL_ATOMICS");
	// only add the inferred lighting kernel if there is support for local memory atomics
	bool local_atomics_support = true;
	if(local_atomics_support) {
		internal_kernels.emplace_back("INFERRED_LIGHTING", "ir_lighting.cl", "ir_lighting", ir_lighting_flags);
	}*/
	
	load_internal_kernels();
}

opencl::kernel_object* opencl::add_kernel_src(const string& identifier, const string& src, const string& func_name, const string additional_options) {
	a2e_debug("compiling \"%s\" kernel!", identifier);
	string options = build_options;
	string nvcc_log = "";
	string build_cmd = "";
	const string tmp_name = "/tmp/cudacl_tmp_"+identifier+"_"+size_t2string(SDL_GetPerformanceCounter());
	try {
		if(kernels.count(identifier) != 0) {
			a2e_error("kernel \"%s\" already exists!", identifier);
			return kernels[identifier];
		}
		
		// add kernel
		opencl::kernel_object* kernel = new opencl::kernel_object();
		kernels[identifier] = kernel;
		kernel->kernel_name = identifier;
		kernel->kernel = nullptr;
		kernel->global = new cl::NDRange(1);
		kernel->local = new cl::NDRange(1);
		const cudacl_kernel_info* kernel_info = nullptr;
		vector<cudacl_kernel_info> kernels_info;
		
		//
		stringstream ptx_buffer(stringstream::in | stringstream::out);
		if(!use_ptx_cache) {
			if(!additional_options.empty()) {
				// convert all -DDEFINEs to -D DEFINE
				options += " " + core::find_and_replace(additional_options, "-D", "-D ");
			}
			
			string cuda_source = "";
			cudacl_translate(tmp_name, src.c_str(), options, cuda_source, kernels_info);
			
			// create tmp cu file
			fstream cu_file(tmp_name+".cu", fstream::out);
			cu_file << cuda_source << endl;
			cu_file.close();
			
			// nvcc compile
			build_cmd = "/usr/local/cuda/bin/nvcc --ptx --machine 64 -arch sm_" + ccl->cc_target_str + " -O3";
			build_cmd += " " + options;
			build_cmd += " -D NVIDIA";
			build_cmd += " -D GPU";
			build_cmd += " -D PLATFORM_"+platform_vendor_to_str(platform_vendor);
			build_cmd += " -o "+tmp_name+".ptx";
			build_cmd += " "+tmp_name+".cu";
			//build_cmd += " 2>&1";
			core::system(build_cmd.c_str(), nvcc_log);
			
			// read ptx
			if(!file_io::file_to_buffer(tmp_name+".ptx", ptx_buffer)) {
				throw cudacl_exception("ptx file doesn't exist!");
			}
			
			bool found = false;
			for(const auto& info : kernels_info) {
				if(info.name == func_name) {
					kernel_info = &info;
					kernel->arg_count = (unsigned int)info.parameters.size();
					found = true;
					break;
				}
			}
			if(!found) throw cudacl_exception("kernel function \""+func_name+"\" does not exist in source file!");
		}
		else {
			// read cached ptx
			if(!file_io::file_to_buffer(ccl->cache_path+identifier+"_"+ccl->cc_target_str+".ptx", ptx_buffer)) {
				throw cudacl_exception("ptx file doesn't exist!");
			}
			
			// read cached kernel info
			stringstream info_buffer(stringstream::in | stringstream::out);
			if(!file_io::file_to_buffer(ccl->cache_path+identifier+"_info_"+ccl->cc_target_str+".txt", info_buffer)) {
				throw cudacl_exception("info file doesn't exist!");
			}
			
			string info_kernel_name = "";
			unsigned int info_param_count = 0;
			info_buffer >> info_kernel_name;
			info_buffer >> info_param_count;
			kernel->arg_count = info_param_count;
			vector<cudacl_kernel_info::kernel_param> kernel_parameters;
			for(size_t i = 0; i < info_param_count; i++) {
				string param_name = "";
				unsigned int p0 = 0, p1 = 0, p2 = 0;
				info_buffer >> param_name;
				info_buffer >> p0;
				info_buffer >> p1;
				info_buffer >> p2;
				kernel_parameters.emplace_back(param_name, (CUDACL_PARAM_ADDRESS_SPACE)p0, (CUDACL_PARAM_TYPE)p1, (CUDACL_PARAM_ACCESS)p2);
			}
			if(info_buffer.fail() || info_buffer.bad()) {
				throw cudacl_exception("failed to parse cached kernel info file!");
			}
			
			kernels_info.emplace_back(info_kernel_name, kernel_parameters);
			kernel_info = &kernels_info.back();
		}
		kernel->args_passed.insert(kernel->args_passed.begin(),
								   kernel->arg_count,
								   false);
		
		// create cuda module (== opencl program)
		array<CUjit_option, 1> jit_options { { CU_JIT_TARGET } };
		size_t* jit_option_values = new size_t[1]; // size_t for correct alignment ...
		jit_option_values[0] = 0;
		*((unsigned int*)jit_option_values) = ccl->cc_target;
		
		// use the binary/ptx of the first device for now
		CUmodule* module = new CUmodule();
		CU(cuModuleLoadDataEx(module,
							  ptx_buffer.str().c_str(),
							  (unsigned int)jit_options.size(),
							  &jit_options[0],
							  (void**)jit_option_values));
		
		delete [] jit_option_values;
		
		// create cuda function (== opencl kernel)
		CUfunction* cuda_func = new CUfunction();
		CU(cuModuleGetFunction(cuda_func, *module, func_name.c_str()));
		
		cudacl::cuda_kernel_object* cuda_kernel = new cudacl::cuda_kernel_object(*kernel_info);
		cuda_kernel->module = module;
		cuda_kernel->function = cuda_func;
		ccl->kernels.insert(make_pair(kernel, cuda_kernel));
	}
	__HANDLE_CL_EXCEPTION_START("add_kernel")
		// print out build log and build options
		a2e_error("nvcc log (%s): %s", identifier, nvcc_log);
		a2e_error("build command (%s): %s", identifier, build_cmd);
	__HANDLE_CL_EXCEPTION_END
	
	// cleanup
	if(!use_ptx_cache) {
		string dummy = "";
		core::system("rm "+tmp_name+".ptx", dummy);
		core::system("rm "+tmp_name+".cu", dummy);
	}
	
	return kernels[identifier];
}

void opencl::log_program_binary(const kernel_object* kernel) {
	if(kernel == nullptr) return;
}

opencl::buffer_object* opencl::create_buffer_object(opencl::BUFFER_TYPE type, void* data) {
	try {
		opencl::buffer_object* buffer = new opencl::buffer_object();
		buffers.push_back(buffer);
		
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
		
		buffer->type = vtype;
		buffer->flags = flags;
		buffer->data = data;
		return buffer;
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
		buffer_obj->buffer = nullptr;
		
		CUdeviceptr* cuda_mem = new CUdeviceptr();
		
		unsigned int cu_flags = 0;
		if(buffer_obj->flags & CL_MEM_USE_HOST_PTR) {
			cu_flags |= CU_MEMHOSTALLOC_DEVICEMAP;
			CU(cuMemHostRegister(buffer_obj->data, size, cu_flags));
			CU(cuMemHostGetDevicePointer(cuda_mem, buffer_obj->data, 0));
		}
		else if(buffer_obj->flags & CL_MEM_ALLOC_HOST_PTR) {
			if(buffer_obj->flags & CL_MEM_READ_ONLY) cu_flags |= CU_MEMHOSTALLOC_WRITECOMBINED;
			cu_flags |= CU_MEMHOSTALLOC_DEVICEMAP;
			CU(cuMemHostAlloc(&buffer_obj->data, size, cu_flags));
			CU(cuMemHostGetDevicePointer(cuda_mem, buffer_obj->data, 0));
		}
		else if(buffer_obj->flags & CL_MEM_COPY_HOST_PTR) {
			CU(cuMemAlloc(cuda_mem, size));
			CU(cuMemcpyHtoD(*cuda_mem, buffer_obj->data, size));
		}
		else {
			CU(cuMemAlloc(cuda_mem, size));
		}
		ccl->buffers[buffer_obj] = cuda_mem;
		return buffer_obj;
	}
	__HANDLE_CL_EXCEPTION("create_buffer")
	return nullptr;
}

opencl::buffer_object* opencl::create_image2d_buffer(opencl::BUFFER_TYPE type a2e_unused, cl_channel_order channel_order a2e_unused, cl_channel_type channel_type a2e_unused, size_t width a2e_unused, size_t height a2e_unused, void* data a2e_unused) {
	// TODO
	/*try {
		buffer_object* buffer_obj = create_buffer_object(type, data);
		if(buffer_obj == nullptr) return nullptr;
		
		buffer_obj->format.image_channel_order = channel_order;
		buffer_obj->format.image_channel_data_type = channel_type;
		buffer_obj->origin.set(0, 0, 0);
		buffer_obj->region.set(width, height, 1); // depth must be 1 for 2d images
		buffer_obj->image_buffer = new cl::Image2D(*context, buffer_obj->flags, buffer_obj->format, width, height, 0, data, &ierr);
		return buffer_obj;
	}
	__HANDLE_CL_EXCEPTION("create_image2d_buffer")*/
	return nullptr;
}

opencl::buffer_object* opencl::create_image3d_buffer(opencl::BUFFER_TYPE type a2e_unused, cl_channel_order channel_order a2e_unused, cl_channel_type channel_type a2e_unused, size_t width a2e_unused, size_t height a2e_unused, size_t depth a2e_unused, void* data a2e_unused) {
	// TODO
	/*try {
		buffer_object* buffer_obj = create_buffer_object(type, data);
		if(buffer_obj == nullptr) return nullptr;
		
		buffer_obj->format.image_channel_order = channel_order;
		buffer_obj->format.image_channel_data_type = channel_type;
		buffer_obj->origin.set(0, 0, 0);
		buffer_obj->region.set(width, height, depth);
		buffer_obj->image_buffer = new cl::Image3D(*context, buffer_obj->flags, buffer_obj->format, width, height, depth, 0, 0, data, &ierr);
		return buffer_obj;
	}
	__HANDLE_CL_EXCEPTION("create_image3d_buffer")*/
	return nullptr;
}

opencl::buffer_object* opencl::create_ogl_buffer(opencl::BUFFER_TYPE type, GLuint ogl_buffer) {
	try {
		opencl::buffer_object* buffer = new opencl::buffer_object();
		buffers.push_back(buffer);
		
		// type/flag validity check
		unsigned int vtype = 0;
		if((type & opencl::BT_DELETE_AFTER_USE) != 0) vtype |= opencl::BT_DELETE_AFTER_USE;
		if((type & opencl::BT_BLOCK_ON_READ) != 0) vtype |= opencl::BT_BLOCK_ON_READ;
		if((type & opencl::BT_BLOCK_ON_WRITE) != 0) vtype |= opencl::BT_BLOCK_ON_WRITE;
		
		unsigned int cuda_flags = 0;
		switch((EBUFFER_TYPE)(type & 0x03)) {
			case opencl::BT_READ_WRITE:
				vtype |= BT_READ_WRITE;
				cuda_flags = CU_GRAPHICS_REGISTER_FLAGS_NONE;
				break;
			case opencl::BT_READ:
				vtype |= BT_READ;
				cuda_flags = CU_GRAPHICS_REGISTER_FLAGS_READ_ONLY;
				break;
			case opencl::BT_WRITE:
				vtype |= BT_WRITE;
				cuda_flags = CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD;
				break;
			default:
				break;
		}
		
		vtype |= BT_OPENGL_BUFFER;
		
		buffer->type = vtype;
		buffer->ogl_buffer = ogl_buffer;
		buffer->data = nullptr;
		buffer->size = 0;
		buffer->buffer = nullptr;
		
		CUgraphicsResource* cuda_gl_buffer = new CUgraphicsResource();
		CU(cuGraphicsGLRegisterBuffer(cuda_gl_buffer, ogl_buffer, cuda_flags));
		ccl->gl_buffers[buffer] = cuda_gl_buffer;
		
		return buffer;
	}
	__HANDLE_CL_EXCEPTION("create_ogl_buffer")
	return nullptr;
}

opencl::buffer_object* opencl::create_ogl_image2d_buffer(BUFFER_TYPE type a2e_unused, GLuint texture a2e_unused, GLenum target a2e_unused) {
	// TODO
	/*try {
		opencl::buffer_object* buffer = new opencl::buffer_object();
		buffers.push_back(buffer);
		
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
		
		buffer->type = vtype;
		buffer->ogl_buffer = texture;
		buffer->data = nullptr;
		buffer->size = 0;
		buffer->image_buffer = new cl::Image2DGL(*context, flags, target, 0, texture, &ierr);
		return buffer;
	}
	__HANDLE_CL_EXCEPTION("create_ogl_image2d_buffer")*/
	return nullptr;
}

opencl::buffer_object* opencl::create_ogl_image2d_renderbuffer(BUFFER_TYPE type a2e_unused, GLuint renderbuffer a2e_unused) {
	// TODO
	/*try {
		opencl::buffer_object* buffer = new opencl::buffer_object();
		buffers.push_back(buffer);
		
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
		
		buffer->type = vtype;
		buffer->ogl_buffer = renderbuffer;
		buffer->data = nullptr;
		buffer->size = 0;
		buffer->buffer = new cl::BufferRenderGL(*context, flags, renderbuffer, &ierr);
		return buffer;
	}
	__HANDLE_CL_EXCEPTION("create_ogl_image2d_renderbuffer")*/
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
	
	// normal buffer
	const auto buffer_iter = ccl->buffers.find(buffer_obj);
	if(buffer_iter != ccl->buffers.end()) {
		CUdeviceptr* cuda_mem = buffer_iter->second;
		if(cuda_mem != nullptr && *cuda_mem != 0) {
			if(buffer_obj->flags & CL_MEM_USE_HOST_PTR) {
				CU(cuMemHostUnregister(buffer_obj->data));
			}
			else if(buffer_obj->flags & CL_MEM_ALLOC_HOST_PTR) {
				CU(cuMemFreeHost(buffer_obj->data));
			}
			else {
				CU(cuMemFree(*cuda_mem));
			}
			delete cuda_mem;
		}
		ccl->buffers.erase(buffer_iter);
	}
	
	// unregister resource (+potential unmap)
	const auto gl_buffer_iter = ccl->gl_buffers.find(buffer_obj);
	if(gl_buffer_iter != ccl->gl_buffers.end()) {
		CUgraphicsResource* res = gl_buffer_iter->second;
		if(buffer_obj->ogl_buffer != 0) {
			if(ccl->mapped_gl_buffers.count(res) > 0) {
				// resource is still mapped -> unmap
				release_gl_object(buffer_obj);
			}
			CU(cuGraphicsUnregisterResource(*res));
			delete res;
		}
		ccl->gl_buffers.erase(gl_buffer_iter);
	}
	
	// TODO: image buffers
	//if(buffer_obj->image_buffer != nullptr) delete buffer_obj->image_buffer;
	const auto iter = find(begin(buffers), end(buffers), buffer_obj);
	if(iter != end(buffers)) buffers.erase(iter);
	delete buffer_obj;
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
	
	//
	CUdeviceptr* cuda_mem = ccl->buffers[buffer_obj];
	CUstream stream = *ccl->queues[ccl->device_map[active_device]];
	if((buffer_obj->type & opencl::BT_BLOCK_ON_WRITE) > 0) {
		// blocking write: wait until everything has completed in the cmdqueue
		finish();
		
		//
		if(offset == 0) {
			CU(cuMemcpyHtoD(*cuda_mem, src, write_size));
		}
		else {
			// if there is an offset, we have to call the cuda runtime to copy the memory
			//unsigned char* dptr = ((unsigned char*)cuda_mem) + write_offset;
			//CURT(cudaMemcpy(dptr, src, write_size, cudaMemcpyHostToDevice));
			assert(false && "write_buffer with offset not implemented yet!");
		}
	}
	else {
		//
		if(offset == 0) {
			CU(cuMemcpyHtoDAsync(*cuda_mem, src, write_size, stream));
		}
		else {
			// if there is an offset, we have to call the cuda runtime to copy the memory
			//unsigned char* dptr = ((unsigned char*)cuda_mem) + write_offset;
			//CURT(cudaMemcpyAsync(dptr, src, write_size, cudaMemcpyHostToDevice, stream));
			assert(false && "write_buffer with offset not implemented yet!");
		}
	}
}

void opencl::write_image2d(opencl::buffer_object* buffer_obj a2e_unused, const void* src a2e_unused, size2 origin a2e_unused, size2 region a2e_unused) {
	// TODO
	/*try {
		size3 origin3(origin.x, origin.y, 0); // origin z must be 0 for 2d images
		size3 region3(region.x, region.y, 1); // depth must be 1 for 2d images
		queues[active_device->device]->enqueueWriteImage(*buffer_obj->image_buffer, ((buffer_obj->type & opencl::BT_BLOCK_ON_WRITE) > 0),
														 (cl::size_t<3>&)origin3, (cl::size_t<3>&)region3, 0, 0, (void*)src);
	}
	__HANDLE_CL_EXCEPTION("write_image2d")*/
}

void opencl::write_image3d(opencl::buffer_object* buffer_obj a2e_unused, const void* src a2e_unused, size3 origin a2e_unused, size3 region a2e_unused) {
	// TODO
	/*try {
		queues[active_device->device]->enqueueWriteImage(*buffer_obj->image_buffer, ((buffer_obj->type & opencl::BT_BLOCK_ON_WRITE) > 0),
														 (cl::size_t<3>&)origin, (cl::size_t<3>&)region, 0, 0, (void*)src);
	}
	__HANDLE_CL_EXCEPTION("write_buffer")*/
}

void opencl::read_buffer(void* dst a2e_unused, opencl::buffer_object* buffer_obj a2e_unused) {
	// TODO
	/*try {
		queues[active_device->device]->enqueueReadBuffer(*buffer_obj->buffer, ((buffer_obj->type & opencl::BT_BLOCK_ON_READ) > 0),
														 0, buffer_obj->size, dst);
	}
	__HANDLE_CL_EXCEPTION("read_buffer")*/
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
		
		//
		cudacl::cuda_kernel_object* kernel = ccl->kernels[kernel_obj];
		CUfunction* cuda_function = kernel->function;
		CUstream* stream = ccl->queues[ccl->device_map[active_device]];
		
		// check if all arguments have been set
		if(kernel->arguments.size() != kernel->info.parameters.size()) {
			throw cudacl_exception(CL_INVALID_KERNEL_ARGS);
		}
		
		// pre kernel-launch stuff:
		vector<opencl::buffer_object*> gl_objects;
		for(const auto& buffer_arg : kernel_obj->buffer_args) {
			if((buffer_arg.second->type & opencl::BT_COPY_ON_USE) != 0) {
				write_buffer(buffer_arg.second, buffer_arg.second->data);
			}
			if((buffer_arg.second->type & opencl::BT_OPENGL_BUFFER) != 0 &&
			   !buffer_arg.second->manual_gl_sharing) {
				gl_objects.push_back(buffer_arg.second);
				kernel_obj->has_ogl_buffers = true;
			}
		}
		if(!gl_objects.empty()) {
			for(const auto& obj : gl_objects) {
				acquire_gl_object(obj);
			}
		}
		
		// create kernel arguments block for cuda
		void** kernel_arguments = new void*[kernel->arguments.size()];
		for(cl_uint i = 0; i < kernel->arguments.size(); i++) {
			kernel_arguments[i] = kernel->arguments[i].ptr;
		}
		
		uint3 global_dim((unsigned int)(*cur_kernel->global)[0],
						 (unsigned int)(*cur_kernel->global)[1],
						 (unsigned int)(*cur_kernel->global)[2]);
		uint3 local_dim((unsigned int)(*cur_kernel->local)[0],
						(unsigned int)(*cur_kernel->local)[1],
						(unsigned int)(*cur_kernel->local)[2]);
		global_dim.max(uint3(1)); // dimensions must at least be 1 in cuda (== 0 in opencl)
		local_dim.max(uint3(1));
		
		//cout << "launching kernel " << kernel_obj->kernel_name  << ": " << global_dim << ", " << local_dim << endl;
		
		CU(cuLaunchKernel(*cuda_function,
						  global_dim.x, global_dim.y, global_dim.z,
						  local_dim.x, local_dim.y, local_dim.z,
						  0, // 0 = auto?
						  *stream,
						  kernel_arguments,
						  nullptr));
		//finish();
		
		delete [] kernel_arguments;
		
		// post kernel-run stuff:
		for(const auto& buffer_arg : kernel_obj->buffer_args) {
			if((buffer_arg.second->type & opencl::BT_READ_BACK_RESULT) != 0) {
				read_buffer(buffer_arg.second->data, buffer_arg.second);
			}
		}
		
		for_each(begin(kernel_obj->buffer_args), end(kernel_obj->buffer_args),
				 [this](const pair<const unsigned int, buffer_object*>& buffer_arg) {
					 if((buffer_arg.second->type & opencl::BT_DELETE_AFTER_USE) != 0) {
						 this->delete_buffer(buffer_arg.second);
					 }
				 });
		
		if(kernel_obj->has_ogl_buffers && !gl_objects.empty()) {
			for(const auto& obj : gl_objects) {
				release_gl_object(obj);
			}
		}
	}
	__HANDLE_CL_EXCEPTION_EXT("run_kernel", (" - in kernel: "+kernel_obj->kernel_name).c_str())
}

void opencl::finish() {
	if(active_device == nullptr) return;
	CU(cuStreamSynchronize(*ccl->queues[ccl->device_map[active_device]]));
}

void opencl::flush() {
	return; // nothing?
}

void opencl::activate_context() {
	if(active_device == nullptr) return;
	CU(cuCtxSetCurrent(*ccl->contexts.at(ccl->device_map.at(active_device))));
}

void opencl::deactivate_context() {
	CU(cuCtxSetCurrent(nullptr));
}

bool opencl::set_kernel_argument(const unsigned int& index, opencl::buffer_object* arg) {
	if(!set_kernel_argument(index, 0, (void*)arg)) return false;
	cur_kernel->buffer_args[index] = arg;
	arg->associated_kernels[cur_kernel].push_back(index);
	return true;
}

bool opencl::set_kernel_argument(const unsigned int& index, const opencl::buffer_object* arg) {
	return set_kernel_argument(index, (opencl::buffer_object*)arg);
}

bool opencl::set_kernel_argument(const unsigned int& index, size_t size, void* arg) {
	try {
		// alloc memory for new argument and copy data
		cudacl::cuda_kernel_object* kernel = ccl->kernels[cur_kernel];
		auto& kernel_arg = kernel->arguments[index];
		
		// delete old arg data (if there is any)
		if(kernel_arg.free_ptr && kernel_arg.ptr != nullptr) {
			free(kernel_arg.ptr);
		}
		kernel_arg.size = 0;
		kernel_arg.ptr = nullptr;
		kernel_arg.free_ptr = true;
		
		switch(kernel->info.get_parameter_type(index)) {
			case CUDACL_PARAM_TYPE::BUFFER:
			case CUDACL_PARAM_TYPE::IMAGE_2D:
			case CUDACL_PARAM_TYPE::IMAGE_3D:
				kernel_arg.size = sizeof(void*);
				if(arg != NULL) {
					kernel_arg.free_ptr = false;
					opencl::buffer_object* buffer = (opencl::buffer_object*)arg;
					if(buffer->ogl_buffer == 0) {
						kernel_arg.ptr = (void*)ccl->buffers[buffer];
					}
					else {
						CUgraphicsResource* res = ccl->gl_buffers[buffer];
						const auto iter = ccl->mapped_gl_buffers.find(res);
						CUdeviceptr* dev_ptr = nullptr;
						if(iter != ccl->mapped_gl_buffers.end()) {
							dev_ptr = iter->second;
						}
						else {
							dev_ptr = new CUdeviceptr();
							size_t size_ret = 0;
							CU(cuGraphicsResourceGetMappedPointer(dev_ptr, &size_ret, *res));
							ccl->mapped_gl_buffers.insert({res, dev_ptr});
						}
						kernel_arg.ptr = (void*)dev_ptr;
					}
				}
				else {
					kernel_arg.ptr = malloc(kernel_arg.size);
					memset(kernel_arg.ptr, 0, kernel_arg.size);
				}
				break;
			case CUDACL_PARAM_TYPE::SAMPLER:
				kernel_arg.size = sizeof(void*);
				kernel_arg.ptr = malloc(kernel_arg.size);
				memset(kernel_arg.ptr, 0, kernel_arg.size);
				break;
			default:
				kernel_arg.size = size;
				if(kernel_arg.size > 0) {
					kernel_arg.ptr = malloc(kernel_arg.size);
					memcpy(kernel_arg.ptr, arg, kernel_arg.size);
				}
				break;
		}
		cur_kernel->args_passed[index] = true;
		return true;
	}
	__HANDLE_CL_EXCEPTION("set_kernel_argument")
	return false;
}

void* opencl::map_buffer(opencl::buffer_object* buffer_obj a2e_unused, EBUFFER_TYPE access_type a2e_unused, bool blocking a2e_unused) {
	// TODO
	/*try {
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
	__HANDLE_CL_EXCEPTION("map_buffer")*/
	return nullptr;
}

void opencl::unmap_buffer(opencl::buffer_object* buffer_obj a2e_unused, void* map_ptr a2e_unused) {
	// TODO
	/*try {
		void* buffer_ptr = nullptr;
		if(buffer_obj->buffer != nullptr) buffer_ptr = buffer_obj->buffer;
		else if(buffer_obj->image_buffer != nullptr) buffer_ptr = buffer_obj->image_buffer;
		else {
			a2e_error("unknown buffer object!");
			return;
		}
		queues[active_device->device]->enqueueUnmapMemObject(*(cl::Memory*)buffer_ptr, map_ptr);
	}
	__HANDLE_CL_EXCEPTION("unmap_buffer")*/
}

size_t opencl::get_kernel_work_group_size() {
	if(cur_kernel == nullptr || active_device == nullptr) return 0;
	
	try {
		CUfunction* cuda_function = ccl->kernels[cur_kernel]->function;
		int ret = 0;
		CU(cuFuncGetAttribute(&ret, CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK, *cuda_function));
		return ret;
	}
	__HANDLE_CL_EXCEPTION("get_kernel_work_group_size")
	return 0;
}

void opencl::acquire_gl_object(buffer_object* gl_buffer_obj) {
	CUstream stream = *ccl->queues[ccl->device_map[active_device]];
	CUgraphicsResource* cuda_gl_buffer = ccl->gl_buffers[gl_buffer_obj];
	CU(cuGraphicsMapResources(1, cuda_gl_buffer, stream));
}

void opencl::release_gl_object(buffer_object* gl_buffer_obj) {
	CUstream stream = *ccl->queues[ccl->device_map[active_device]];
	CUgraphicsResource* cuda_gl_buffer = ccl->gl_buffers[gl_buffer_obj];
	const auto iter = ccl->mapped_gl_buffers.find(cuda_gl_buffer);
	if(iter != ccl->mapped_gl_buffers.end()) {
		delete iter->second;
		ccl->mapped_gl_buffers.erase(iter);
	}
	CU(cuGraphicsUnmapResources(1, cuda_gl_buffer, stream));
}

#endif
