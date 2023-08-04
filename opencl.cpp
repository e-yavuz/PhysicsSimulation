#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <OpenCL/opencl.h>
#include <iostream>
#include <vector>

#define DATA_SIZE (1024)

 
int main(int argc, char** argv)
{
    int err;                            // error code returned from api calls
    uint correct;                       // number of correct results returned
 
    size_t global;                      // global domain size for our calculation
    size_t local;                       // local domain size for our calculation
 
    cl_device_id device_id;             // compute device id 
    cl_context context;                 // compute context
    cl_command_queue commands;          // compute command queue
    cl_program program;                 // compute program
    cl_kernel kernel;                   // compute kernel
    
    cl_mem position;                       // device memory used for the input position array
    cl_mem velocity;                       // device memory used for the input position array

    float cohesion_force = 1.f;
    float seperation_force = 1.f;
    float alignment_force = 1.f;

    std::ifstream in("src/Shaders/Collision.cl");

    std::string content(std::string((std::istreambuf_iterator<char>(in)), 
    std::istreambuf_iterator<char>()));
    const char* KernelSource = content.data();
    
    // Pull in test data
    int bucket_size, tree_size, number_of_boids;

    std::cin >> bucket_size >> tree_size >> number_of_boids;

    size_t sz = bucket_size * tree_size * 2;

    std::vector<float> position_v(sz);
    std::vector<float> velocity_v(sz);

    std::vector<float> desired_position_v(sz);
    std::vector<float> desired_velocity_v(sz);

    for(int i = 0; i < sz; i++)
        std::cin >> position_v[i];
    for(int i = 0; i < sz; i++)
        std::cin >> velocity_v[i];
    for(int i = 0; i < sz; i++)
        std::cin >> desired_position_v[i];
    for(int i = 0; i < sz; i++)
        std::cin >> desired_velocity_v[i];
    
    // Connect to a compute device
    //
    int gpu = 1;
    err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to create a device group!\n");
        return EXIT_FAILURE;
    }
  
    // Create a compute context 
    //
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    if (!context)
    {
        printf("Error: Failed to create a compute context!\n");
        return EXIT_FAILURE;
    }
 
    // Create a command commands
    //
    commands = clCreateCommandQueue(context, device_id, 0, &err);
    if (!commands)
    {
        printf("Error: Failed to create a command commands!\n");
        return EXIT_FAILURE;
    }
 
    // Create the compute program from the source buffer
    //
    program = clCreateProgramWithSource(context, 1, (const char **) & KernelSource, NULL, &err);
    if (!program)
    {
        printf("Error: Failed to create compute program!\n");
        return EXIT_FAILURE;
    }
 
    // Build the program executable
    //
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];
 
        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        exit(1);
    }
 
    // Create the compute kernel in the program we wish to run
    //
    kernel = clCreateKernel(program, "update", &err);
    if (!kernel || err != CL_SUCCESS)
    {
        printf("Error: Failed to create compute kernel!\n");
        exit(1);
    }
 
    // Create the input and output arrays in device memory for our calculation
    //
    position = clCreateBuffer(context,  CL_MEM_READ_WRITE,  sizeof(float) * position_v.size(), NULL, NULL);
    velocity = clCreateBuffer(context,  CL_MEM_READ_WRITE,  sizeof(float) * velocity_v.size(), NULL, NULL);
    if (!position || !velocity)
    {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }    
    
    // Write our data set into the input array in device memory 
    //
    err = clEnqueueWriteBuffer(commands, position, CL_TRUE, 0, sizeof(float) * position_v.size(), position_v.data(), 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(commands, velocity, CL_TRUE, 0, sizeof(float) * velocity_v.size(), velocity_v.data(), 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to write to source array!\n");
        exit(1);
    }
 
    // Set the arguments to our compute kernel
    //
    err = 0;
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &position);
    err  |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &velocity);
    err |= clSetKernelArg(kernel, 2, sizeof(float), &cohesion_force);
    err  |= clSetKernelArg(kernel, 3, sizeof(float), &seperation_force);
    err |= clSetKernelArg(kernel, 4, sizeof(float), &alignment_force);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to set kernel arguments! %d\n", err);
        exit(1);
    }
 
    // Get the maximum work group size for executing the kernel on the device
    //
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to retrieve kernel work group info! %d\n", err);
        exit(1);
    }
 
    // Execute the kernel over the entire range of our 1d input data set
    // using the maximum number of work group items for this device
    //
    global = sz/2;
    std::cout << global << " ";
    local = 128;
    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    if (err)
    {
        printf("Error: Failed to execute kernel!\n");
        return EXIT_FAILURE;
    }
 
    // Wait for the command commands to get serviced before reading back results
    //
    clFinish(commands);

    // for(float elm: position_v)
    //     std::cout << elm << " ";
    // std::cout << "\n";
    // for(float elm: velocity_v)
    //     std::cout << elm << " ";
    //     std::cout << "\n";
 
    // Read back the results from the device to verify the output
    //
    err = clEnqueueReadBuffer( commands, position, CL_TRUE, 0, sizeof(float) * sz, position_v.data(), 0, NULL, NULL );
    err |= clEnqueueReadBuffer( commands, velocity, CL_TRUE, 0, sizeof(float) * sz, velocity_v.data(), 0, NULL, NULL );  
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to read output array! %d\n", err);
        exit(1);
    }
    
    // Validate result
    correct = 0;
    for(int i = 0; i < sz; i++)
    {
        if(position_v[i] == desired_position_v[i])
            correct++;
            
        if(velocity_v[i] == desired_velocity_v[i])
            correct++;
    }
    // for(float elm: position_v)
    //     std::cout << elm << " ";
    // std::cout << "\n";
    // for(float elm: velocity_v)
    //     std::cout << elm << " ";
    
    // // Print a brief summary detailing the results
    // //
    // printf("Computed '%d/%d' correct values!\n", correct, (int)sz*2);
    
    // // Shutdown and cleanup
    //
    clReleaseMemObject(velocity);
    clReleaseMemObject(position);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);
 
    return 0;
}