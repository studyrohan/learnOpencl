#include "learnOpencl.h"
#include "CL/cl.h"
#define VECTOR_SIZE    1024
#define KERNEL_SRC_MAX (1 << 20)

#define KERNEL(...) #__VA_ARGS__

const char* KernelSourceCode = KERNEL(
	__kernel void hellocl(__global uint * buffer)
{
	size_t gidx = get_global_id(0);
	size_t gidy = get_global_id(1);
	size_t lidx = get_local_id(0);
	Buffer[gidx + 4 * gidy] = (1 << gdix) | (0x10 << gidy);
}
);

int testOpencl()
{
	cl_int status = 0;
	size_t deviceListSize;
	cl_uint numPlatforms;
	cl_platform_id platform = NULL;
	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS)
	 {
		 return EXIT_FAILURE;
	 }
	if (numPlatforms>0)
	{
		cl_platform_id* platforms = (cl_platform_id*)malloc(numPlatforms * sizeof(cl_platform_id));
		status = clGetPlatformIDs(numPlatforms, platforms,NULL);
		if (status	!= CL_SUCCESS)
		{
			printf("error,get platform ids");
		}
		for (unsigned int i=0;i<numPlatforms;i++)
		{
			char pbuff[100];
			status = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(pbuff), pbuff,NULL);
			platform = platforms[i];

			if (!strcmp(pbuff,"Advanced Micro Devices, Inc."))
			{
				break;
			}
			delete platforms;
		}
		cl_context_properties cps[3] = {
			CL_CONTEXT_PLATFORM,
			(cl_context_properties)platform,0
		};
		cl_context_properties* cprops = (NULL == platform) ? NULL : cps;
		//生成上下文
		cl_context context = clCreateContextFromType(
			cprops,
			CL_DEVICE_TYPE_GPU,
			NULL,
			NULL,
			&status);

		if (status != CL_SUCCESS)
		{
			printf("error:creating Context.\
					(clcreateContextFromType)\n");
			return EXIT_FAILURE;
		}
		//寻找Opencl  设备
		//首先获取设备列表的长度
		status = clGetContextInfo(context,
			CL_CONTEXT_DEVICES,
			0, NULL,
			&deviceListSize);
		if (status != CL_SUCCESS)
		{
			printf("error:Get Context info.\
					(clGetContextInfo)\n");
			return EXIT_FAILURE;
		}
		cl_device_id* devices = (cl_device_id*)malloc(deviceListSize);

		if (devices == 0)
		{
			printf("no devices found.\n");
			return EXIT_FAILURE;
		}
		//获取设备列表
		status = clGetContextInfo(
			context,
			CL_CONTEXT_DEVICES,
			deviceListSize,
			devices,
			NULL);


		int err;
		for (cl_int k = 0; k < deviceListSize; k++)
		{
			// 8. 获取和打印各个设备的name
			size_t length = 0;
			cl_device_id each_device = devices[k];
			err = clGetDeviceInfo(each_device, CL_DEVICE_NAME, 0, 0, &length);


			char* value = new char[length];
			err = clGetDeviceInfo(each_device, CL_DEVICE_NAME, length, value, 0);



			// 9. 获取和打印各个设备的version
			err = clGetDeviceInfo(each_device, CL_DEVICE_VERSION, 0, 0, &length);


			char* version = new char[length];
			err = clGetDeviceInfo(each_device, CL_DEVICE_VERSION, length, version, 0);


			delete[] value;
			delete[] version;
		}

		if (status != CL_SUCCESS)
		{
			printf("error:Get Context info.\
					(clGetContextInfo)\n");
			return EXIT_FAILURE;
		}
		//装载内核程序，编译CL program,生成cl内核实例
		size_t sourceSize[] = { strlen(KernelSourceCode) };
		cl_program program = clCreateProgramWithSource(context, 1, &KernelSourceCode, sourceSize, &status);
		if (status != CL_SUCCESS)
		{
			printf("error:Loading Binary into cl_program.\
					(clGetContextInfo)\n");
			return EXIT_FAILURE;
		}

		//为所有指定的设备生成 CL program
		status = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
		if (status != CL_SUCCESS)
		{
			printf("error:Building Program \
					(clBuildProgram)\n");
			return EXIT_FAILURE;
		}
		//得到指定名字的内核实例句柄
		cl_kernel kernel = clCreateKernel(program, "hellocl", &status);
		if (status != CL_SUCCESS)
		{
			printf("error:create kernel from Program \
					(clCreateKernel)\n");
			return EXIT_FAILURE;
		}

		//创建一个opencl command queue
		cl_command_queue commandQueue =
				clCreateCommandQueue(context, devices[0], 0, &status);

	}


	return 0;
}

/* @brief 读取OpenCL源码
 * @param[in] file OpenCL 源文件
 * @param[out] size 返回源码的大小
 * @return 源码
 */
char* readKernel(const char* file, size_t* n)
{
	FILE* fp = fopen(file, "rb");
	char* src = static_cast<char*>(malloc(KERNEL_SRC_MAX));
	if (src == nullptr)
	{
		fclose(fp);
		return nullptr;
	}
	*n = fread(src, 1, KERNEL_SRC_MAX, fp);
	fclose(fp);
	return src;
}


int testOpenclProgram()
{
	/* 获取平台ID */
	cl_platform_id  pid;
	cl_int err = clGetPlatformIDs(1, &pid, nullptr);
	if (err != CL_SUCCESS)
	{
		fprintf(stderr, "failed to get platform id\n");
		return EXIT_FAILURE;
	}

	/* 获取设备ID */
	cl_device_id did;
	err = clGetDeviceIDs(pid, CL_DEVICE_TYPE_GPU, 1, &did, nullptr);
	if (err != CL_SUCCESS)
	{
		fprintf(stderr, "failed to get device id\n");
		return EXIT_FAILURE;
	}

	/* 创建上下文 */
	cl_context ctx = clCreateContext(nullptr, 1, &did, nullptr, nullptr, &err);
	if (err != CL_SUCCESS)
	{
		fprintf(stderr, "failed to create context\n");
		return EXIT_FAILURE;
	}

	/* 创建指令队列 */
	cl_command_queue cmd = clCreateCommandQueueWithProperties(ctx, did, 0, &err);
	if (err != CL_SUCCESS)
	{
		fprintf(stderr, "failed to create command queue\n");
		return EXIT_FAILURE;
	}

	/* 创建数据 */
	cl_float* x = static_cast<float*>(malloc(VECTOR_SIZE * sizeof(cl_float)));
	cl_float* y = static_cast<float*>(malloc(VECTOR_SIZE * sizeof(cl_float)));
	if (x == nullptr || y == nullptr)
	{
		fprintf(stderr, "RAM bad alloc\n");
		return EXIT_FAILURE;
	}

	for (int i = 0; i < VECTOR_SIZE; i++)
	{
		x[i] = 1.0f * i;
		y[i] = 2.0f * i;
	}

	/* 在设备（GPU）上分配内存 */
	cl_mem X = clCreateBuffer(ctx, CL_MEM_READ_WRITE, VECTOR_SIZE * sizeof(cl_float), nullptr, &err);
	if (err != CL_SUCCESS)
	{
		fprintf(stderr, "GRAM bad alloc\n");
		return EXIT_FAILURE;
	}

	cl_mem Y = clCreateBuffer(ctx, CL_MEM_READ_WRITE, VECTOR_SIZE * sizeof(cl_float), nullptr, &err);
	if (err != CL_SUCCESS)
	{
		fprintf(stderr, "GRAM bad alloc\n");
		return EXIT_FAILURE;
	}

	/* 将数据拷贝到设备（GPU）上 */
	err = clEnqueueWriteBuffer(cmd, X, CL_TRUE, 0, VECTOR_SIZE * sizeof(cl_float), x, 0, nullptr, nullptr);
	if (err != CL_SUCCESS)
	{
		fprintf(stderr, "failed to write buffer\n");
		return EXIT_FAILURE;
	}

	err = clEnqueueWriteBuffer(cmd, Y, CL_TRUE, 0, VECTOR_SIZE * sizeof(cl_float), y, 0, nullptr, nullptr);
	if (err != CL_SUCCESS)
	{
		fprintf(stderr, "failed to write buffer\n");
		return EXIT_FAILURE;
	}

	/* 读取OpenCL源码 */
	size_t srcSize = 0;
	char* src = readKernel("add.cl", &srcSize);
	if (src == nullptr)
	{
		fprintf(stderr, "failed to read source\n");
		return EXIT_FAILURE;
	}

	printf("%.*s\n", static_cast<unsigned int>(srcSize), src);

	/* 创建程序 */
	cl_program program = clCreateProgramWithSource(ctx, 1, const_cast<const char**>(&src), &srcSize, &err);
	free(src);
	if (err != CL_SUCCESS)
	{
		fprintf(stderr, "failed to create program\n");
		return EXIT_FAILURE;
	}

	/* 构建程序 */
	err = clBuildProgram(program, 1, &did, nullptr, nullptr, nullptr);
	if (err != CL_SUCCESS)
	{
		size_t len = 0;
		char msg[8 * 1024];
		clGetProgramBuildInfo(program, did, CL_PROGRAM_BUILD_LOG, sizeof(msg), msg, &len);
		fprintf(stderr, "failed to build program: %*s\n", static_cast<unsigned int>(len), msg);
		return EXIT_FAILURE;
	}

	/* 加载kernel函数 */
	cl_kernel kernelVecAdd = clCreateKernel(program, "vecAdd", &err);
	if (err != CL_SUCCESS)
	{

		fprintf(stderr, "failed to create kernel\n");
		return EXIT_FAILURE;
	}

	/* 设置参数 */
	err = clSetKernelArg(kernelVecAdd, 0, sizeof(cl_mem), (void*)&X);
	if (err != CL_SUCCESS)
	{

		fprintf(stderr, "failed to set arg\n");
		return EXIT_FAILURE;
	}

	err = clSetKernelArg(kernelVecAdd, 1, sizeof(cl_mem), (void*)&Y);
	if (err != CL_SUCCESS)
	{

		fprintf(stderr, "failed to set arg\n");
		return EXIT_FAILURE;
	}

	/* 运行kernel函数 */
	size_t globalItems = VECTOR_SIZE; // 总工作项数
	size_t localItems = 64;           // 一个工作组的工作项数
	err = clEnqueueNDRangeKernel(cmd, kernelVecAdd, 1, nullptr, &globalItems, &localItems, 0, nullptr, nullptr);
	if (err != CL_SUCCESS)
	{

		fprintf(stderr, "failed to execute kernel\n");
		return EXIT_FAILURE;
	}

	/* 读取运行结果 */
	err = clEnqueueReadBuffer(cmd, X, CL_TRUE, 0, VECTOR_SIZE * sizeof(cl_float), x, 0, nullptr, nullptr);
	if (err != CL_SUCCESS)
	{

		fprintf(stderr, "failed to read buffer\n");
		return EXIT_FAILURE;
	}

	for (int i = 0; i < VECTOR_SIZE; i++)
	{
		printf("%f  ", x[i]);
	}
	printf("\n");

	clFinish(cmd);
	clReleaseKernel(kernelVecAdd);
	clReleaseProgram(program);
	clReleaseMemObject(X);
	clReleaseMemObject(Y);
	clReleaseCommandQueue(cmd);
	clReleaseContext(ctx);
	free(x);
	free(y);
	return 0;
}
