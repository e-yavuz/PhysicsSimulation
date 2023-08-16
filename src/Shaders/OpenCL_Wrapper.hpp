#ifndef OPENCL_WRAPPER_HPP
#define OPENCL_WRAPPER_HPP

#include <fcntl.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <OpenCL/OpenCL.h>
#include <opencl-c-base.h>
#include <stdexcept>

class OpenCL_WRAPPER
{
public:
    OpenCL_WRAPPER() {}
    OpenCL_WRAPPER(std::string kernelFile)
    {
        std::ifstream in(kernelFile);
        content = std::string((std::istreambuf_iterator<char>(in)), 
    std::istreambuf_iterator<char>());
        char* KernelSource = content.data();

        // Connect to a compute device
        //
        int gpu = 1;
        err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
        if (err != CL_SUCCESS)
            throw(std::runtime_error("Error: Failed to create a device group!\n"));
    
        // Create a compute context 
        //
        context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
        if (!context)
            throw(std::runtime_error("Error: Failed to create a compute context!\n"));
    
        // Create a command commands
        //
        commands = clCreateCommandQueue(context, device_id, 0, &err);
        if (!commands)
            throw(std::runtime_error("Error: Failed to create a command commands!\n"));
    
        // Create the compute program from the source buffer
        //
        program = clCreateProgramWithSource(context, 1, (const char **) & KernelSource, NULL, &err);
        if (!program)
            throw(std::runtime_error("Error: Failed to create compute program!\n"));
    
        // Build the program executable
        //
        err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
        if (err != CL_SUCCESS)
        {
            size_t len;
            char buffer[2048];
    
            clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
            printf("%s\n", buffer);
            throw(std::runtime_error("Error: Failed to build program executable!\n"));
        }
    
        // Create the compute kernel in the program we wish to run
        //
        kernel = clCreateKernel(program, "naive_Update", &err);
        if (!kernel || err != CL_SUCCESS)
            throw(std::runtime_error("Error: Failed to create compute kernel!\n"));
    }

    ~OpenCL_WRAPPER()
    {
        clReleaseProgram(program);
        clReleaseKernel(kernel);
        clReleaseCommandQueue(commands);
        clReleaseContext(context);
    }

    void RunKernel_naive_Update(float* hostRadiusObjects, float* hostInitPositions, float* hostFinalPositions, uint N)
    {
        cl_mem deviceRadiusObjects;
        cl_mem deviceInitPositions;
        cl_mem deviceFinalPositions;
        ulong local;
        // Create the input and output arrays in device memory for our calculation
        //
        deviceRadiusObjects = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(cl_float) * N, NULL, NULL);
        deviceInitPositions = clCreateBuffer(context,  CL_MEM_READ_ONLY,  3*sizeof(cl_float) * N, NULL, NULL);
        deviceFinalPositions = clCreateBuffer(context,  CL_MEM_WRITE_ONLY, 3*sizeof(cl_float) * N, NULL, NULL);
        if (!deviceRadiusObjects || !deviceInitPositions || !deviceFinalPositions)
            throw(std::runtime_error("Error: Failed to allocate device memory!\n"));
        
        // Write our data set into the input array in device memory 
        //
        err = clEnqueueWriteBuffer(commands, deviceRadiusObjects, CL_TRUE, 0, sizeof(cl_float) * N, hostRadiusObjects, 0, NULL, NULL);
        err |= clEnqueueWriteBuffer(commands, deviceInitPositions, CL_TRUE, 0, 3*sizeof(cl_float) * N, hostInitPositions, 0, NULL, NULL);
        err |= clEnqueueWriteBuffer(commands, deviceFinalPositions, CL_TRUE, 0, 3*sizeof(cl_float) * N, hostFinalPositions, 0, NULL, NULL);
        if (err != CL_SUCCESS)
            throw(std::runtime_error("Error: Failed to write to source array!\n"));
    
        // Set the arguments to our compute kernel
        //
        err = 0;
        err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &deviceInitPositions);
        err  |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &deviceFinalPositions);
        err  |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &deviceRadiusObjects);
        err |= clSetKernelArg(kernel, 3, sizeof(uint), &N);
        if (err != CL_SUCCESS)
            throw(std::runtime_error("Error: Failed to set kernel arguments!"));
    
        // Get the maximum work group size for executing the kernel on the device
        //
        err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
        if (err != CL_SUCCESS)
            throw(std::runtime_error("Error: Failed to retrieve kernel work group info!"));
    
        // Execute the kernel over the entire range of our 1d input data set
        // using the maximum number of work group items for this device
        //
        local = 256;
        ulong global = ((N/local)+1)*local;
        err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
        if (err)
            throw(std::runtime_error("Error: Failed to execute kernel!\n ERR: " + std::to_string(err)));
    
        // Wait for the command commands to get serviced before reading back results
        //
        clFinish(commands);

        // Read back the results from the device to verify the output
        //
        err = clEnqueueReadBuffer( commands, deviceFinalPositions, CL_TRUE, 0, 3 * sizeof(cl_float) * N, hostFinalPositions, 0, NULL, NULL );
        if (err != CL_SUCCESS)
            throw(std::runtime_error("Error: Failed to read output array! %d\n"));

        clReleaseMemObject(deviceRadiusObjects);
        clReleaseMemObject(deviceInitPositions);
        clReleaseMemObject(deviceFinalPositions);
    }
private:
    int err;                            // error code returned from api calls
    uint correct;                       // number of correct results returned
 
    cl_device_id device_id;             // compute device id 
    cl_context context;                 // compute context
    cl_command_queue commands;          // compute command queue
    cl_program program;                 // compute program
    cl_kernel kernel;                   // compute kernel
    std::string content;                // kernel tokens
};

#endif