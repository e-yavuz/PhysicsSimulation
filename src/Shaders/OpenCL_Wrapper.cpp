// #include "OpenCL_Wrapper.hpp"
// #include <OpenCL/OpenCL.h>
// #include <opencl-c-base.h>
// #include <stdexcept>
// #include <sys/types.h>

// // void OpenCL_WRAPPER::RunKernel_naive_Update(float* hostRadiusObjects, float* hostInitPositions, float* hostFinalPositions, uint N)
// // {
// //     cl_mem deviceRadiusObjects;
// //     cl_mem deviceInitPositions;
// //     cl_mem deviceFinalPositions;
// //     ulong local;
// //     // Create the input and output arrays in device memory for our calculation
// //     //
// //     deviceRadiusObjects = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(float) * N, NULL, NULL);
// //     deviceInitPositions = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(float3) * N, NULL, NULL);
// //     deviceFinalPositions = clCreateBuffer(context,  CL_MEM_WRITE_ONLY, sizeof(float3) * N, NULL, NULL);
// //     if (!deviceRadiusObjects || !deviceInitPositions || !deviceFinalPositions)
// //         throw(std::runtime_error("Error: Failed to allocate device memory!\n"));
    
// //     // Write our data set into the input array in device memory 
// //     //
// //     err = clEnqueueWriteBuffer(commands, deviceRadiusObjects, CL_TRUE, 0, sizeof(float) * N, hostRadiusObjects, 0, NULL, NULL);
// //     err |= clEnqueueWriteBuffer(commands, deviceInitPositions, CL_TRUE, 0, sizeof(float3) * N, hostInitPositions, 0, NULL, NULL);
// //     err |= clEnqueueWriteBuffer(commands, deviceFinalPositions, CL_TRUE, 0, sizeof(float3) * N, hostFinalPositions, 0, NULL, NULL);
// //     if (err != CL_SUCCESS)
// //         throw(std::runtime_error("Error: Failed to write to source array!\n"));
 
// //     // Set the arguments to our compute kernel
// //     //
// //     err = 0;
// //     err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &deviceInitPositions);
// //     err  |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &deviceFinalPositions);
// //     err  |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &deviceRadiusObjects);
// //     err |= clSetKernelArg(kernel, 3, sizeof(uint), &deviceInitPositions);
// //     if (err != CL_SUCCESS)
// //         throw(std::runtime_error("Error: Failed to set kernel arguments!"));
 
// //     // Get the maximum work group size for executing the kernel on the device
// //     //
// //     err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
// //     if (err != CL_SUCCESS)
// //         throw(std::runtime_error("Error: Failed to retrieve kernel work group info!"));
 
// //     // Execute the kernel over the entire range of our 1d input data set
// //     // using the maximum number of work group items for this device
// //     //
// //     ulong global = N;
// //     err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
// //     if (err)
// //         throw(std::runtime_error("Error: Failed to execute kernel!\n"));
 
// //     // Wait for the command commands to get serviced before reading back results
// //     //
// //     clFinish(commands);

// //     // Read back the results from the device to verify the output
// //     //
// //     err = clEnqueueReadBuffer( commands, deviceFinalPositions, CL_TRUE, 0, sizeof(float3) * N, hostFinalPositions, 0, NULL, NULL );
// //     if (err != CL_SUCCESS)
// //         throw(std::runtime_error("Error: Failed to read output array! %d\n"));

// //     clReleaseMemObject(deviceRadiusObjects);
// //     clReleaseMemObject(deviceInitPositions);
// //     clReleaseMemObject(deviceFinalPositions);
// // }