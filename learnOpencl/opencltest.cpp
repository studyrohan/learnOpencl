#include "learnOpencl.h"
#include "CL/cl.h"

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

		}

	}



}