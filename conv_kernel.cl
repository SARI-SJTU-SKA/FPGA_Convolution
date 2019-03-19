/*Convolution Algorithm Kernel*/
#include "config.h"

__kernel __attribute__((reqd_work_group_size(1, 1, 1)))
void conv(      __global float* input_1, //input sequence 1
                __global float* input_2, //input sequence 2
                __global float* output)  //output sequence

{
/*	int global_size = get_global_size(0);
	int global_id   = get_global_id(0);

	int kernel_point_size_1  = M;
	int kernel_point_size_2  = N;
	int kernel_point_start_1 = global_id * kernel_point_size_1;
	int kernel_point_start_2 = global_id * kernel_point_size_2;
	int kernel_point_end_1 = kernel_point_start_1 + kernel_point_size_1;
	int kernel_point_end_2 = kernel_point_start_2 + kernel_point_size_2;*/

	local float temp_data_1[M] __attribute__((xcl_array_partition(complete, 1)));
	local float temp_data_2[PARA][2 * M + N - 1] __attribute__((xcl_array_partition(complete, 2)));
	local float temp_output[PARA][M + N - 1] __attribute__((xcl_array_partition(complete, 2)));

	convolution: for(int num = 0; num < NUMs; num += PARA)
	{
		//Reading the inputs from DDR to BRAM
		__attribute__((xcl_pipeline_loop))
		read_in1: for(int iter = 0; iter < M; iter++)
		{
			temp_data_1[iter] = input_1[iter];
		}

		__attribute__((xcl_pipeline_loop))
		read_in2_1: for(int iter = 0, i = 0, j = 0; iter < PARA * M; iter++, j++)
		{
			if (j == M) {j = 0; i++;}
			temp_data_2[i][j] = 0.0;
		}

		__attribute__((xcl_pipeline_loop))
		read_in2_2: for(int iter = num * N, i = 0, j = M; iter < (num + PARA) * N; iter++, j++)
		{
			if (j == M + N) {j = M; i++;}
			temp_data_2[i][j] = input_2[iter];
		}

		__attribute__((xcl_pipeline_loop))
		read_in2_3: for(int iter = 0, i = 0, j = M + N; iter < PARA * (M - 1); iter++, j++)
		{
			if (j == 2 * M + N - 1) {j = M + N; i++;}
			temp_data_2[i][j] = 0.0;
		}

		//Convolution
		loop0: for(int t = 0; t < PARA; t++)
		{
			__attribute__((xcl_pipeline_loop))
			loop1: for(int i = 0; i < M + N - 1; i++)
			{
					temp_output[t][i] = 0.0;
					__attribute__((opencl_unroll_hint))
					loop2: for(int j = 0; j < M; j++)
					{
						temp_output[t][i] += temp_data_1[j] * temp_data_2[t][i - j + M];
					}
			}
		}

		//temp_output to output
		output1: for(int i = 0; i < PARA; i++)
		{
			__attribute__((xcl_pipeline_loop))
		    output2: for(int j = 0; j < M + N - 1; j++)
		    {
		    	output[(num + i) * (M + N - 1) + j] = temp_output[i][j];
		    }
		}
	}
}

/*
//Convolution
for(i = 0; i < M + N - 1; i++)
{
	temp_output[i] = 0.0;

	if(i < M)
	{
		temp = i;
	}
	else temp = M - 1;

	for(j = 0; j <= temp; j++)
	{
		temp_output[i] += (input_1[j] * temp_data[i - j]);
	}
}
*/
