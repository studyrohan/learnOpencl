#include "learnOpencl.h"
#include "CL/cl.h"
#define VECTOR_SIZE    1024
#define KERNEL_SRC_MAX (1 << 20)

#define KERNEL(...) #__VA_ARGS__
//	//%x �� C ���������ڸ�ʽ�������һ�ָ�ʽ˵����,��Ҫ������ʮ��������ʽ�������������˵������
const char* KernelSourceCode = KERNEL(
	__kernel void hellocl(__global uint * buffer)
{

	size_t gidx = get_global_id(0);
	printf("gidx =  %x ", gidx);
	size_t gidy = get_global_id(1);
	printf("gidy =  %x ", gidy);
	size_t lidx = get_local_id(0);
	buffer[gidx + 4 * gidy] = (1 << gidx) | (0x10 << gidy);
	printf("value =  %x ", gidx + 4*gidy);
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
			//delete platforms;
		}
		cl_context_properties cps[3] = {
			CL_CONTEXT_PLATFORM,
			(cl_context_properties)platform,0
		};
		cl_context_properties* cprops = (NULL == platform) ? NULL : cps;
		//����������
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
		//Ѱ��Opencl  �豸
		//���Ȼ�ȡ�豸�б�ĳ���
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
		//��ȡ�豸�б�
		status = clGetContextInfo(
			context,
			CL_CONTEXT_DEVICES,
			deviceListSize,
			devices,
			NULL);


		int err;
		//for (cl_int k = 0; k < deviceListSize; k++)
		//{
		//	// 8. ��ȡ�ʹ�ӡ�����豸��name
		//	size_t length = 0;
		//	cl_device_id each_device = devices[k];
		//	err = clGetDeviceInfo(each_device, CL_DEVICE_NAME, 0, 0, &length);

		//	char* value = new char[length];
		//	err = clGetDeviceInfo(each_device, CL_DEVICE_NAME, length, value, 0);

		//	// 9. ��ȡ�ʹ�ӡ�����豸��version
		//	err = clGetDeviceInfo(each_device, CL_DEVICE_VERSION, 0, 0, &length);

		//	char* version = new char[length];
		//	err = clGetDeviceInfo(each_device, CL_DEVICE_VERSION, length, version, 0);

		//	delete[] value;
		//	delete[] version;
		//}

		if (status != CL_SUCCESS)
		{
			printf("error:Get Context info.\
					(clGetContextInfo)\n");
			return EXIT_FAILURE;
		}
		//װ���ں˳��򣬱���CL program,����cl�ں�ʵ��
		size_t sourceSize[] = { strlen(KernelSourceCode) };
		cl_program program = clCreateProgramWithSource(context, 1, &KernelSourceCode, sourceSize, &status);
		if (status != CL_SUCCESS)
		{
			printf("error:Loading Binary into cl_program.\
					(clGetContextInfo)\n");
			return EXIT_FAILURE;
		}

		//Ϊ����ָ�����豸���� CL program
		status = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
		if (status != CL_SUCCESS)
		{
			// ��ȡ������־�ĳ���
			size_t bufferSize = 0;
			clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &bufferSize);

			// �����㹻�Ŀռ����洢��־
			char * logBuffer = new char[bufferSize];
			if (!logBuffer) {
				std::cerr << "Failed to allocate memory for log buffer.\n";
				return -1;
			}

			// ��ȡ������־
			clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, bufferSize, logBuffer, NULL);

			// ��ӡ��־
			std::cout << "--- Build log ---\n" << logBuffer << std::endl;

			delete[] logBuffer;
			printf("error:Building Program \
					(clBuildProgram)\n");
			return EXIT_FAILURE;
		}
		//�õ�ָ�����ֵ��ں�ʵ�����
		cl_kernel kernel = clCreateKernel(program, "hellocl", &status);
		if (status != CL_SUCCESS)
		{
			printf("error:create kernel from Program \
					(clCreateKernel)\n");
			return EXIT_FAILURE;
		}

		//����һ��opencl command queue
		cl_command_queue commandQueue =
				clCreateCommandQueue(context, devices[0], 0, &status);
		if (status!= CL_SUCCESS)
		{
			printf("error:creating command error \
					(clCreateCommandQueue)\n");
			return EXIT_FAILURE;
		}

		// ����opencl buffer ����
		unsigned int* outbuffer = new unsigned int[4 * 4];
		memset(outbuffer, 0, 4 * 4 * 4);
		cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR, 4 * 4 * 4, NULL, &status);
		if (status!=CL_SUCCESS)
		{
			printf("Error:clCreateBuffer\
					(outputbuffer)\n");
			return EXIT_FAILURE;
		}

		// Ϊ�ں˳���������Ӧ�Ĳ���
		status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&outputBuffer);
		if (status!= CL_SUCCESS)
		{
			printf("Error,Set kernel argument.\
					(output)");
			return EXIT_FAILURE;
		}
		// ��һ��kernel����command queue
		size_t globalThreads[] = {4,4};
		size_t localThreads[] = { 2,2 };
		status = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, globalThreads, localThreads, 0, NULL, NULL);
		if (status != CL_SUCCESS)
		{
			printf("Error:Enqueueing kernel \n");
			return EXIT_FAILURE;
		}
		status = clFinish(commandQueue);
		if (status != CL_SUCCESS)
		{
			printf("Error:Finish command queue\n");
			return EXIT_FAILURE;
		}
		status = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0, 4 * 4 * 4,outbuffer,0, NULL, NULL);
		printf("out:\n");
		for (size_t i = 0; i < 16; i++)
		{
			printf("%x ", outbuffer[i]);
			if ((i + 1) % 4 == 0)
				printf("\n");

		}
		status = clReleaseKernel(kernel);
		status = clReleaseProgram(program);
		status = clReleaseContext(context);
		status = clReleaseMemObject(outputBuffer);
		status = clReleaseCommandQueue(commandQueue);
		free(devices);
		delete outbuffer;


	}


	return 0;
}

/* @brief ��ȡOpenCLԴ��
 * @param[in] file OpenCL Դ�ļ�
 * @param[out] size ����Դ��Ĵ�С
 * @return Դ��
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
	/* ��ȡƽ̨ID */
	cl_platform_id  pid;
	cl_int err = clGetPlatformIDs(1, &pid, nullptr);
	if (err != CL_SUCCESS)
	{
		fprintf(stderr, "failed to get platform id\n");
		return EXIT_FAILURE;
	}

	/* ��ȡ�豸ID */
	cl_device_id did;
	err = clGetDeviceIDs(pid, CL_DEVICE_TYPE_GPU, 1, &did, nullptr);
	if (err != CL_SUCCESS)
	{
		fprintf(stderr, "failed to get device id\n");
		return EXIT_FAILURE;
	}

	/* ���������� */
	cl_context ctx = clCreateContext(nullptr, 1, &did, nullptr, nullptr, &err);
	if (err != CL_SUCCESS)
	{
		fprintf(stderr, "failed to create context\n");
		return EXIT_FAILURE;
	}

	/* ����ָ����� */
	cl_command_queue cmd = clCreateCommandQueueWithProperties(ctx, did, 0, &err);
	if (err != CL_SUCCESS)
	{
		fprintf(stderr, "failed to create command queue\n");
		return EXIT_FAILURE;
	}

	/* �������� */
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

	/* ���豸��GPU���Ϸ����ڴ� */
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

	/* �����ݿ������豸��GPU���� */
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

	/* ��ȡOpenCLԴ�� */
	size_t srcSize = 0;
	char* src = readKernel("add.cl", &srcSize);
	if (src == nullptr)
	{
		fprintf(stderr, "failed to read source\n");
		return EXIT_FAILURE;
	}

	printf("%.*s\n", static_cast<unsigned int>(srcSize), src);

	/* �������� */
	cl_program program = clCreateProgramWithSource(ctx, 1, const_cast<const char**>(&src), &srcSize, &err);
	free(src);
	if (err != CL_SUCCESS)
	{
		fprintf(stderr, "failed to create program\n");
		return EXIT_FAILURE;
	}

	/* �������� */
	err = clBuildProgram(program, 1, &did, nullptr, nullptr, nullptr);
	if (err != CL_SUCCESS)
	{
		size_t len = 0;
		char msg[8 * 1024];
		clGetProgramBuildInfo(program, did, CL_PROGRAM_BUILD_LOG, sizeof(msg), msg, &len);
		fprintf(stderr, "failed to build program: %*s\n", static_cast<unsigned int>(len), msg);
		return EXIT_FAILURE;
	}

	/* ����kernel���� */
	cl_kernel kernelVecAdd = clCreateKernel(program, "vecAdd", &err);
	if (err != CL_SUCCESS)
	{

		fprintf(stderr, "failed to create kernel\n");
		return EXIT_FAILURE;
	}

	/* ���ò��� */
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

	/* ����kernel���� */
	size_t globalItems = VECTOR_SIZE; // �ܹ�������
	size_t localItems = 64;           // һ��������Ĺ�������
	err = clEnqueueNDRangeKernel(cmd, kernelVecAdd, 1, nullptr, &globalItems, &localItems, 0, nullptr, nullptr);
	if (err != CL_SUCCESS)
	{

		fprintf(stderr, "failed to execute kernel\n");
		return EXIT_FAILURE;
	}

	/* ��ȡ���н�� */
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
