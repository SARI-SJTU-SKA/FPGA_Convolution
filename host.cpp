/**********
Copyright (c) 2017, Xilinx, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********/
#include <CL/cl.h>
#include <math.h>
#include <cstdio>
#include <fstream>
#include <iosfwd>
#include <string>
#include <vector>
#include "config.h"

using std::ifstream;
using std::streamsize;
using std::string;
using std::vector;
using std::ios;

#define ERROR_CASE(err) \
    case err:           \
        return #err;    \
        break

float input_1[KERNEL_LEN], input_2[SEQUENCE_LEN];	 	/* COnvolution kernel sequence and input sequence */
float output[OUT_LEN];		/* ouput sequence */

int i, j;

const char *error_string(cl_int error_code) {
    switch (error_code) {
        ERROR_CASE(CL_SUCCESS);
        ERROR_CASE(CL_DEVICE_NOT_FOUND);
        ERROR_CASE(CL_DEVICE_NOT_AVAILABLE);
        ERROR_CASE(CL_COMPILER_NOT_AVAILABLE);
        ERROR_CASE(CL_MEM_OBJECT_ALLOCATION_FAILURE);
        ERROR_CASE(CL_OUT_OF_RESOURCES);
        ERROR_CASE(CL_OUT_OF_HOST_MEMORY);
        ERROR_CASE(CL_PROFILING_INFO_NOT_AVAILABLE);
        ERROR_CASE(CL_MEM_COPY_OVERLAP);
        ERROR_CASE(CL_IMAGE_FORMAT_MISMATCH);
        ERROR_CASE(CL_IMAGE_FORMAT_NOT_SUPPORTED);
        ERROR_CASE(CL_BUILD_PROGRAM_FAILURE);
        ERROR_CASE(CL_MAP_FAILURE);
        ERROR_CASE(CL_INVALID_VALUE);
        ERROR_CASE(CL_INVALID_DEVICE_TYPE);
        ERROR_CASE(CL_INVALID_PLATFORM);
        ERROR_CASE(CL_INVALID_DEVICE);
        ERROR_CASE(CL_INVALID_CONTEXT);
        ERROR_CASE(CL_INVALID_QUEUE_PROPERTIES);
        ERROR_CASE(CL_INVALID_COMMAND_QUEUE);
        ERROR_CASE(CL_INVALID_HOST_PTR);
        ERROR_CASE(CL_INVALID_MEM_OBJECT);
        ERROR_CASE(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
        ERROR_CASE(CL_INVALID_IMAGE_SIZE);
        ERROR_CASE(CL_INVALID_SAMPLER);
        ERROR_CASE(CL_INVALID_BINARY);
        ERROR_CASE(CL_INVALID_BUILD_OPTIONS);
        ERROR_CASE(CL_INVALID_PROGRAM);
        ERROR_CASE(CL_INVALID_PROGRAM_EXECUTABLE);
        ERROR_CASE(CL_INVALID_KERNEL_NAME);
        ERROR_CASE(CL_INVALID_KERNEL_DEFINITION);
        ERROR_CASE(CL_INVALID_KERNEL);
        ERROR_CASE(CL_INVALID_ARG_INDEX);
        ERROR_CASE(CL_INVALID_ARG_VALUE);
        ERROR_CASE(CL_INVALID_ARG_SIZE);
        ERROR_CASE(CL_INVALID_KERNEL_ARGS);
        ERROR_CASE(CL_INVALID_WORK_DIMENSION);
        ERROR_CASE(CL_INVALID_WORK_GROUP_SIZE);
        ERROR_CASE(CL_INVALID_WORK_ITEM_SIZE);
        ERROR_CASE(CL_INVALID_GLOBAL_OFFSET);
        ERROR_CASE(CL_INVALID_EVENT_WAIT_LIST);
        ERROR_CASE(CL_INVALID_EVENT);
        ERROR_CASE(CL_INVALID_OPERATION);
        ERROR_CASE(CL_INVALID_GL_OBJECT);
        ERROR_CASE(CL_INVALID_BUFFER_SIZE);
        ERROR_CASE(CL_INVALID_MIP_LEVEL);
        ERROR_CASE(CL_INVALID_GLOBAL_WORK_SIZE);
        ERROR_CASE(CL_COMPILE_PROGRAM_FAILURE);
        ERROR_CASE(CL_LINKER_NOT_AVAILABLE);
        ERROR_CASE(CL_LINK_PROGRAM_FAILURE);
        ERROR_CASE(CL_DEVICE_PARTITION_FAILED);
        ERROR_CASE(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
        ERROR_CASE(CL_INVALID_PROPERTY);
        ERROR_CASE(CL_INVALID_IMAGE_DESCRIPTOR);
        ERROR_CASE(CL_INVALID_COMPILER_OPTIONS);
        ERROR_CASE(CL_INVALID_LINKER_OPTIONS);
        ERROR_CASE(CL_INVALID_DEVICE_PARTITION_COUNT);
        default:
            printf("Unknown OpenCL Error (%d)\n", error_code);
            break;
    }
    return nullptr;
}

vector<unsigned char> readBinary(const std::string &fileName) {
    ifstream file(fileName, ios::binary | ios::ate);
    if (file) {
        file.seekg(0, ios::end);
        streamsize size = file.tellg();
        file.seekg(0, ios::beg);
        vector<unsigned char> buffer(size);
        file.read((char *)buffer.data(), size);
        return buffer;
    } else {
        fprintf(stderr, "%s not found\n", fileName.c_str());
        exit(-1);
    }
    return std::vector<unsigned char>(0);
}


int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s "
               "./xclbin/vector_addition.<emulation_mode>.<dsa>.xclbin\n",
               argv[0]);
        return EXIT_FAILURE;
    }
    char *binary_file_path = argv[1];
    cl_int err;

    cl_uint num_platforms;
    if ((err = clGetPlatformIDs(0, nullptr, &num_platforms))) {
        printf(
            "Fatal Error calling clGetPlatformIDs: %s\n"
            "This can be caused by an invalid installation of the OpenCL "
            "runtime. Please make sure the installation was successful.\n",
            error_string(err));
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }

    if (num_platforms == 0) {
        printf(
            "No platforms were found. This could be caused because the OpenCL "
            "icd was not installed in the /etc/OpenCL/vendors directory.\n");
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }

    vector<cl_platform_id> platforms(num_platforms + 1);
    if ((err = clGetPlatformIDs(num_platforms, platforms.data(), nullptr))) {
        printf("Error: Failed to find an OpenCL platform!\n");
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }

    string platform_name(1024, '\0');
    size_t actual_size = 0;
    if ((err = clGetPlatformInfo(platforms[0], CL_PLATFORM_NAME,
                                 platform_name.size(),
                                 (void *)platform_name.data(), &actual_size))) {
        printf("Error: Could not determine platform name!\n");
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }
    printf("Platform Name: %s\n", platform_name.c_str());

    cl_device_id device_id = 0;
    if ((err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 1, &device_id,
                              nullptr))) {
        printf(
            "Fatal Error calling clGetDeviceIDs: %s\n"
            "Unexpected error getting device IDs. This may happen if you are "
            "Targeting hardware or software emulation and the "
            "XCL_EMULATION_MODE environment variable is not set. Also makeyou "
            "have set the you have run the emconfigutil to setup the emulation "
            "environment.\n\n",
            error_string(err));
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }

    cl_context_properties props[3] = {CL_CONTEXT_PLATFORM,
                                      (cl_context_properties)platforms[0], 0};

    cl_context context = clCreateContext(props, 1, &device_id, nullptr, nullptr, &err);

    cl_command_queue command_queue = clCreateCommandQueue(
        context, device_id, CL_QUEUE_PROFILING_ENABLE, &err);
    if (err) {
        printf(
            "Fatal Error calling clCreateCommandQueue: %s\n"
            "Unexpected error creating a command queue.\n\n",
            error_string(err));
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }

    // Loading the file
    vector<unsigned char> binary = readBinary(binary_file_path);
    size_t binary_size = binary.size();
    //size_t incorrect_binary_size = binary.size() - 42;
    const unsigned char *binary_data = binary.data();

    cl_program program = clCreateProgramWithBinary(context, 1, &device_id, &binary_size,
                                        &binary_data, NULL, &err);
    if (err) {
        printf(
            "Fatal Error calling clCreateProgramWithBinary: %s\n"
            "Unexpected error creating a program from binary. Make sure you "
            "executed this program with the correct path to the binary.\n\n",
            error_string(err));
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }

    cl_kernel kernel = clCreateKernel(program, "conv", &err);
    if (err) {
        printf(
            "Fatal Error calling clCreateKernel: %s\n"
            "Unexpected error when creating kernel. Make sure you passed the "
            "correct binary into the executable of this program.\n\n",
            error_string(err));
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }

/////////////////////////////////////////////////////////////////////////////////////////////////

    /* Initiate Data */
        printf("/*** Input Sequence 1 ***/\n");
    	for (i = 0; i < KERNEL_LEN; i++)
    	{
    		//input_1[i] = (i % 9) + 1;
    		input_1[i] = i + 1.0;
    	}
    	printf("/*** Input Sequence 2 ***/\n");
    	for (j = 0; j < SEQUENCE_LEN; j++)
    	{
    		//input_2[j] = (j % 4) + 1;
    		input_2[j] = j + 1.0;
    	}
    	printf("/*** output Sequence ***/\n");
/////////////////////////////////////////////////////////////////////////////////////////////////////

    /* Create buffer*/

    size_t size_input_1 = KERNEL_LEN * sizeof(float);
    cl_mem buffer_input_1 = /*Buffer of input_1*/
    	clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, size_input_1, input_1, &err);
    if (err) {
        printf("Fatal Error calling clCreateBuffer: %s\n", error_string(err));
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }

    size_t size_input_2 = SEQUENCE_LEN * sizeof(float);
    cl_mem buffer_input_2 = /*Buffer of input_2*/
        clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, size_input_2, input_2, &err);
    if (err) {
        printf("Fatal Error calling clCreateBuffer: %s\n", error_string(err));
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }

    size_t size_output = OUT_LEN * sizeof(float);
    cl_mem buffer_output = /*Buffer of output*/
        clCreateBuffer(context, CL_MEM_WRITE_ONLY, size_output, nullptr, &err);
    if (err) {
        printf("Fatal Error calling clCreateBuffer: %s\n", error_string(err));
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }
///////////////////////////////////////////////////////////////////////////////////////

    /* Transfer data from/to buffer via clSetKernelArg() */

    if ((err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_input_1))) {
        printf("Fatal Error calling clSetKernelArg: %s\n", error_string(err));
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }

    if ((err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_input_2))) {
        printf("Fatal Error calling clSetKernelArg: %s\n", error_string(err));
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }

    if ((err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffer_output))) {
        printf("Fatal Error calling clSetKernelArg: %s\n", error_string(err));
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }

////////////////////////////////////////////////////////////////////////////////////////////


    size_t global = 1;
    size_t local = 1;
for(i = 0; i < 10; i++)
{
    if ((err = clEnqueueNDRangeKernel(command_queue, kernel, 1, nullptr,
                                      &global, &local, 0, nullptr, nullptr))) {
        printf("Fatal Error calling clEnqueueNDRangeKernel: %s\n",
               error_string(err));
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////

    /* Transfer data from buffer in device side to memory of host */

    if ((err = clEnqueueReadBuffer(command_queue, buffer_output, CL_TRUE, 0,
    		size_output, output, 0, nullptr, nullptr))) {
        printf("Fatal Error calling clEnqueueWriteBuffer: %s\n",
               error_string(err));
        printf("TEST FAILED\n");
        exit(EXIT_FAILURE);
    }

////////////////////////////////////////////////////////////////////////////////////////////////

/*	for (i = 0; i < OUT_LEN; i++)
	{
		printf("output[%d] = %f\n", i, output[i]);
	}*/

    clReleaseMemObject(buffer_input_1);
    clReleaseMemObject(buffer_input_2);
    clReleaseMemObject(buffer_output);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(command_queue);
	clReleaseContext(context);
	printf("Success\n");
}
