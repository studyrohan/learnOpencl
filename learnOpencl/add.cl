__kernel void vecAdd(__global float *X, __global float *Y)
{
	const int idx = get_global_id(0);

	X[idx] = X[idx] + Y[idx];
}