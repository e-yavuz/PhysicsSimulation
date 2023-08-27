#ifndef OPENCL_WRAPPER_HPP
#define OPENCL_WRAPPER_HPP

#include <cassert>
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
    OpenCL_WRAPPER(std::string kernelFile, ulong localBlockSize) : localBlockSize(localBlockSize)
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
            printf("OPENCL: %s\n", buffer);
            throw(std::runtime_error("Error: Failed to build program executable!\n"));
        }
    
        // Create the Naive compute kernel in the program we wish to run
        //
        naiveKernel = clCreateKernel(program, "naive_Update", &err);
        if (!naiveKernel || err != CL_SUCCESS)
            throw(std::runtime_error("Error: Failed to create compute kernel!\n"));

        // Create the Spatial Hash compute kernel in the program we wish to run
        //
        spatialHashKernel = clCreateKernel(program, "spatial_Hash_Update", &err);
        if (!spatialHashKernel || err != CL_SUCCESS)
            throw(std::runtime_error("Error: Failed to create compute kernel!\n"));
    }

    ~OpenCL_WRAPPER()
    {
        clReleaseProgram(program);
        clReleaseKernel(naiveKernel);
        clReleaseKernel(spatialHashKernel);
        clReleaseCommandQueue(commands);
        clReleaseContext(context);
    }

    void RunKernel_naive_Update(float* hostRadiusObjects, float* hostInitPositions, float* hostFinalPositions, const uint N)
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
    
        // Set the arguments to our compute naiveKernel
        //
        err = 0;
        err  = clSetKernelArg(naiveKernel, 0, sizeof(cl_mem), &deviceInitPositions);
        err  |= clSetKernelArg(naiveKernel, 1, sizeof(cl_mem), &deviceFinalPositions);
        err  |= clSetKernelArg(naiveKernel, 2, sizeof(cl_mem), &deviceRadiusObjects);
        err |= clSetKernelArg(naiveKernel, 3, sizeof(uint), &N);
        if (err != CL_SUCCESS)
            throw(std::runtime_error("Error: Failed to set naiveKernel arguments!"));
    
        // Get the maximum work group size for executing the naiveKernel on the device
        //
        err = clGetKernelWorkGroupInfo(naiveKernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
        if (err != CL_SUCCESS)
            throw(std::runtime_error("Error: Failed to retrieve naiveKernel work group info!"));
    
        // Execute the naiveKernel over the entire range of our 1d input data set
        // using the maximum number of work group items for this device
        //
        local = 16;
        ulong globalThreadCount = ((N/local)+1)*local;
        err = clEnqueueNDRangeKernel(commands, naiveKernel, 1, NULL, &globalThreadCount, &local, 0, NULL, NULL);
        if (err)
            throw(std::runtime_error("Error: Failed to execute naiveKernel!\n ERR: " + std::to_string(err)));
    
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

    void RunKernel_Spatial_Hash(float* hostRadiusObjects, float* hostInitPositions, float* hostFinalPositions, uint* hostSpatialIndicies, uint2* hostthreadBlockToSpatialHashMetaData, uint2* hostspatialHashGridMetaData, const uint spatialHashGridDim, const uint N, const ulong threadBlockCount)
    {
        cl_mem deviceRadiusObjects;
        cl_mem deviceInitPositions;
        cl_mem deviceFinalPositions;
        cl_mem devicethreadBlockToSpatialHashMetaData;
        cl_mem devicespatialHashGridMetaData;
        cl_mem deviceSpatialIndicies;
        ulong local;
        uint gridCount = spatialHashGridDim * spatialHashGridDim;

        // Create the input and output arrays in device memory for our calculation
        //
        deviceRadiusObjects = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(cl_float) * N, NULL, NULL);
        deviceInitPositions = clCreateBuffer(context,  CL_MEM_READ_ONLY,  3*sizeof(cl_float) * N, NULL, NULL);
        deviceFinalPositions = clCreateBuffer(context,  CL_MEM_WRITE_ONLY, 3*sizeof(cl_float) * N, NULL, NULL);
        deviceSpatialIndicies = clCreateBuffer(context,  CL_MEM_READ_ONLY, sizeof(cl_uint) * N, NULL, NULL);
        devicethreadBlockToSpatialHashMetaData = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(cl_uint2) * threadBlockCount, NULL, NULL);
        devicespatialHashGridMetaData = clCreateBuffer(context,  CL_MEM_READ_ONLY, sizeof(cl_uint2) * gridCount, NULL, NULL);
        if (!deviceRadiusObjects || !deviceInitPositions || !deviceFinalPositions)
            throw(std::runtime_error("Error: Failed to allocate device memory!\n"));
        
        // Write our data set into the input array in device memory 
        //
        err = clEnqueueWriteBuffer(commands, deviceRadiusObjects, CL_TRUE, 0, sizeof(cl_float) * N, hostRadiusObjects, 0, NULL, NULL);
        err |= clEnqueueWriteBuffer(commands, deviceInitPositions, CL_TRUE, 0, 3*sizeof(cl_float) * N, hostInitPositions, 0, NULL, NULL);
        err |= clEnqueueWriteBuffer(commands, deviceFinalPositions, CL_TRUE, 0, 3*sizeof(cl_float) * N, hostFinalPositions, 0, NULL, NULL);
        err |= clEnqueueWriteBuffer(commands, deviceSpatialIndicies, CL_TRUE, 0, sizeof(cl_uint) * N, hostSpatialIndicies, 0, NULL, NULL);
        err |= clEnqueueWriteBuffer(commands, devicethreadBlockToSpatialHashMetaData, CL_TRUE, 0, sizeof(cl_uint2) * threadBlockCount, hostthreadBlockToSpatialHashMetaData, 0, NULL, NULL);
        err |= clEnqueueWriteBuffer(commands, devicespatialHashGridMetaData, CL_TRUE, 0, sizeof(cl_uint2) * gridCount, hostspatialHashGridMetaData, 0, NULL, NULL);
        if (err != CL_SUCCESS)
            throw(std::runtime_error("Error: Failed to write to source array!\n"));
    
        // Set the arguments to our compute spatialHashKernel
        //
        err = 0;
        err  = clSetKernelArg(spatialHashKernel, 0, sizeof(cl_mem), &deviceInitPositions);
        err  |= clSetKernelArg(spatialHashKernel, 1, sizeof(cl_mem), &deviceFinalPositions);
        err  |= clSetKernelArg(spatialHashKernel, 2, sizeof(cl_mem), &deviceRadiusObjects);
        err  |= clSetKernelArg(spatialHashKernel, 3, sizeof(cl_mem), &deviceSpatialIndicies);
        err  |= clSetKernelArg(spatialHashKernel, 4, sizeof(cl_mem), &devicethreadBlockToSpatialHashMetaData);
        err  |= clSetKernelArg(spatialHashKernel, 5, sizeof(cl_mem), &devicespatialHashGridMetaData);
        err |= clSetKernelArg(spatialHashKernel, 6, sizeof(uint), &spatialHashGridDim);
        if (err != CL_SUCCESS)
            throw(std::runtime_error("Error: Failed to set spatialHashKernel arguments!"));

        // Initalize local threadBlock size
        //
        local = localBlockSize;
        ulong globalThreadCount = threadBlockCount*local;
    
        // Execute the spatialHashKernel over the entire range of our 1d input data set
        // using the maximum number of work group items for this device
        //
        err = clEnqueueNDRangeKernel(commands, spatialHashKernel, 1, NULL, &globalThreadCount, &local, 0, NULL, NULL);
        if (err)
            throw(std::runtime_error("Error: Failed to execute spatialHashKernel!\n ERR: " + std::to_string(err)));
    
        // Wait for the command commands to get serviced before reading back results
        //
        clFinish(commands);

        // Read back the results from the device
        //
        err = clEnqueueReadBuffer( commands, deviceFinalPositions, CL_TRUE, 0, 3 * sizeof(cl_float) * N, hostFinalPositions, 0, NULL, NULL );
        if (err != CL_SUCCESS)
            throw(std::runtime_error("Error: Failed to read output array! %d\n"));

        clReleaseMemObject(deviceRadiusObjects);
        clReleaseMemObject(deviceInitPositions);
        clReleaseMemObject(deviceFinalPositions);
        clReleaseMemObject(devicethreadBlockToSpatialHashMetaData);
        clReleaseMemObject(devicespatialHashGridMetaData);
    }
    

    ulong localBlockSize;

private:
    int err;                            // error code returned from api calls
    uint correct;                       // number of correct results returned
 
    cl_device_id device_id;             // compute device id 
    cl_context context;                 // compute context
    cl_command_queue commands;          // compute command queue
    cl_program program;                 // compute program
    cl_kernel naiveKernel;              // Naive compute kernel
    cl_kernel spatialHashKernel;        // Spatial Hash compute kernel
    std::string content;                // kernel tokens
};

#endif