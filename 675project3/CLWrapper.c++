// clwrapper.c++

#include "CLWrapper.h"

CLWrapper::CLWrapper() {}
CLWrapper::~CLWrapper() {}

void CLWrapper::checkStatus(std::string where, cl_int status,
                            bool abortOnError) {
  if (debug || (status != 0))
    std::cout << "Step " << where << ", status = " << status << '\n';
  if ((status != 0) && abortOnError)
    exit(1);
}

void CLWrapper::reportPlatformInformation(const cl_platform_id &platformIn) {
  NameTable what[] = {{"CL_PLATFORM_PROFILE:    ", CL_PLATFORM_PROFILE},
                      {"CL_PLATFORM_VERSION:    ", CL_PLATFORM_VERSION},
                      {"CL_PLATFORM_NAME:       ", CL_PLATFORM_NAME},
                      {"CL_PLATFORM_VENDOR:     ", CL_PLATFORM_VENDOR},
                      {"CL_PLATFORM_EXTENSIONS: ", CL_PLATFORM_EXTENSIONS},
                      {"", 0}};
  size_t size;
  char *buf = nullptr;
  int bufLength = 0;
  std::cout << "===============================================\n";
  std::cout << "========== PLATFORM INFORMATION ===============\n";
  std::cout << "===============================================\n";
  for (int i = 0; what[i].value != 0; i++) {
    clGetPlatformInfo(platformIn, what[i].value, 0, nullptr, &size);
    if (size > bufLength) {
      if (buf != nullptr)
        delete[] buf;
      buf = new char[size];
      bufLength = size;
    }
    clGetPlatformInfo(platformIn, what[i].value, bufLength, buf, &size);
    std::cout << what[i].name << buf << '\n';
  }
  std::cout << "================= END =========================\n\n";
  if (buf != nullptr)
    delete[] buf;
}

int CLWrapper::typicalOpenCLProlog(cl_device_type desiredDeviceType) {
  //-----------------------------------------------------
  // Discover and query the platforms
  //-----------------------------------------------------

  cl_int status = clGetPlatformIDs(0, nullptr, &numPlatforms);
  checkStatus("clGetPlatformIDs-0", status, true);

  platforms = new cl_platform_id[numPlatforms];

  status = clGetPlatformIDs(numPlatforms, platforms, nullptr);
  checkStatus("clGetPlatformIDs-1", status, true);
  int which = 0;
  if (numPlatforms > 1) {
    std::cout << "Found " << numPlatforms << " platforms:\n";
    for (int i = 0; i < numPlatforms; i++) {
      std::cout << i << ": ";
      reportPlatformInformation(platforms[i]);
    }
    which = -1;
    while ((which < 0) || (which >= numPlatforms)) {
      std::cout << "Which platform do you want to use? ";
      std::cin >> which;
    }
  }
  curPlatform = platforms[which];

  std::cout << "Selected platform: ";
  reportPlatformInformation(curPlatform);

  //----------------------------------------------------------
  // Discover and initialize the devices on a platform
  //----------------------------------------------------------

  status =
      clGetDeviceIDs(curPlatform, desiredDeviceType, 0, nullptr, &numDevices);
  checkStatus("clGetDeviceIDs-0", status, true);
  if (numDevices <= 0) {
    std::cout << "No devices on platform!\n";
    return -1;
  }

  devices = new cl_device_id[numDevices];

  status = clGetDeviceIDs(curPlatform, desiredDeviceType, numDevices, devices,
                          nullptr);
  checkStatus("clGetDeviceIDs-1", status, true);
  // Find a device that supports double precision arithmetic
  int *possibleDevs = new int[numDevices];
  int nPossibleDevs = 0;
  for (int idx = 0; idx < numDevices; idx++) {
    size_t extLength;
    clGetDeviceInfo(devices[idx], CL_DEVICE_EXTENSIONS, 0, nullptr, &extLength);
    char *extString = new char[extLength + 1];
    clGetDeviceInfo(devices[idx], CL_DEVICE_EXTENSIONS, extLength + 1,
                    extString, nullptr);

    possibleDevs[nPossibleDevs++] = idx;
    delete[] extString;
  }

  size_t nameLength;
  for (int i = 0; i < nPossibleDevs; i++) {
    clGetDeviceInfo(devices[possibleDevs[i]], CL_DEVICE_NAME, 0, nullptr,
                    &nameLength);
    char *name = new char[nameLength + 1];
    clGetDeviceInfo(devices[possibleDevs[i]], CL_DEVICE_NAME, nameLength + 1,
                    name, nullptr);
    std::cout << "Device " << i << ": [" << name << "] found.\n";
    delete[] name;
  }
  if (nPossibleDevs == 1) {
    std::cout << "\nNo other device in the requested device category found "
              << "You may want to try the -a command line option to see if "
                 "there are others.\n"
              << "For now, I will use the one I found.\n";
    return possibleDevs[0];
  }
  int devIndex = -1;
  while ((devIndex < 0) || (devIndex >= nPossibleDevs)) {
    std::cout << "Which device do you want to use? ";
    std::cin >> devIndex;
  }
  return possibleDevs[devIndex];
}

void CLWrapper::doTheKernelLaunch(cl_device_id dev, float *R, float *Ri,
                                  cl_float3 *output, size_t N, int nRows,
                                  int nCols, int MaxIterations,
                                  int MaxLengthSquared, float juliaReal,
                                  float juliaImag, bool isJulia,
                                  cl_float3 COLOR_1, cl_float3 COLOR_2,
                                  cl_float3 COLOR_3) {

  //------------------------------------------------------------------------
  // Create a context for some or all of the devices on the platform
  // (Here we are including all devices.)
  //------------------------------------------------------------------------

  cl_int status;
  cl_context context =
      clCreateContext(nullptr, 1, &dev, nullptr, nullptr, &status);
  checkStatus("clCreateContext", status, true);

  //-------------------------------------------------------------
  // Create a command queue for one device in the context
  // (There is one queue per device per context.)
  //-------------------------------------------------------------

  cl_command_queue cmdQueue = clCreateCommandQueue(context, dev, 0, &status);
  checkStatus("clCreateCommandQueue", status, true);

  //----------------------------------------------------------
  // Create device buffers associated with the context
  //----------------------------------------------------------

  size_t datasize = N * sizeof(float);
  size_t outputsize =
      N * sizeof(cl_float3); // the cpu side version of float3 in opencl

  cl_mem d_R = clCreateBuffer( // Input array on the device
      context, CL_MEM_READ_ONLY, datasize, nullptr, &status);
  checkStatus("clCreateBuffer-R", status, true);

  cl_mem d_Ri = clCreateBuffer( // Input array on the device
      context, CL_MEM_READ_ONLY, datasize, nullptr, &status);
  checkStatus("clCreateBuffer-Ri", status, true);

  cl_mem d_C = clCreateBuffer( // Output array on the device
      context, CL_MEM_WRITE_ONLY, outputsize, nullptr, &status);
  checkStatus("clCreateBuffer-C", status, true);

  //-----------------------------------------------------
  // Use the command queue to encode requests to
  //         write host data to the device buffers
  //-----------------------------------------------------

  status = clEnqueueWriteBuffer(cmdQueue, d_R, CL_FALSE, 0, datasize, R, 0,
                                nullptr, nullptr);
  checkStatus("clEnqueueWriteBuffer-R", status, true);

  status = clEnqueueWriteBuffer(cmdQueue, d_Ri, CL_FALSE, 0, datasize, Ri, 0,
                                nullptr, nullptr);
  checkStatus("clEnqueueWriteBuffer-Ri", status, true);

  //-----------------------------------------------------
  // Create, compile, and link the program
  //-----------------------------------------------------

  const char *programSource[] = {readSource("fractal.cl")};
  cl_program program =
      clCreateProgramWithSource(context, 1, programSource, nullptr, &status);
  checkStatus("clCreateProgramWithSource", status, true);

  status = clBuildProgram(program, 1, &dev, nullptr, nullptr, nullptr);
  if (status != 0)
    showProgramBuildLog(program, dev);
  checkStatus("clBuildProgram", status, true);

  //----------------------------------------------------------------------
  // Create a kernel using a "__kernel" function in the ".cl" file
  //----------------------------------------------------------------------

  cl_kernel kernel = clCreateKernel(program, "computeColor", &status);

  //-----------------------------------------------------
  // Set the kernel arguments
  //-----------------------------------------------------

  // bool not supported in opencl?
  int julia = 0;
  if (isJulia)
    julia = 1;

  // lots of params
  status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_R);
  checkStatus("clSetKernelArg-R", status, true);
  status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_Ri);
  checkStatus("clSetKernelArg-Ri", status, true);
  status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_C);
  checkStatus("clSetKernelArg-C", status, true);
  status = clSetKernelArg(kernel, 3, sizeof(int), &N);
  checkStatus("clSetKernelArg-N", status, true);
  status = clSetKernelArg(kernel, 4, sizeof(int), &nRows);
  checkStatus("clSetKernelArg-rows", status, true);
  status = clSetKernelArg(kernel, 5, sizeof(int), &nCols);
  checkStatus("clSetKernelArg-cols", status, true);
  status = clSetKernelArg(kernel, 6, sizeof(int), &MaxIterations);
  checkStatus("clSetKernelArg-it", status, true);
  status = clSetKernelArg(kernel, 7, sizeof(int), &MaxLengthSquared);
  checkStatus("clSetKernelArg-length", status, true);
  status = clSetKernelArg(kernel, 8, sizeof(float), &juliaReal);
  checkStatus("clSetKernelArg-juliareal", status, true);
  status = clSetKernelArg(kernel, 9, sizeof(float), &juliaImag);
  checkStatus("clSetKernelArg-juliaimag", status, true);
  status = clSetKernelArg(kernel, 10, sizeof(int), &julia);
  checkStatus("clSetKernelArg-juliaimag", status, true);
  status = clSetKernelArg(kernel, 11, sizeof(cl_float3), &COLOR_1);
  checkStatus("clSetKernelArg-color1", status, true);
  status = clSetKernelArg(kernel, 12, sizeof(cl_float3), &COLOR_2);
  checkStatus("clSetKernelArg-color2", status, true);
  status = clSetKernelArg(kernel, 13, sizeof(cl_float3), &COLOR_3);
  checkStatus("clSetKernelArg-color3", status, true);

  //-----------------------------------------------------
  // Configure the work-item structure
  //-----------------------------------------------------

  size_t localWorkSize[] = {16, 16};
  size_t globalWorkSize[2];
  // Global work size needs to be at least NxN, but it must
  // also be a multiple of local size in each dimension:
  for (int d = 0; d < 2; d++) {
    globalWorkSize[d] = nCols; // assuming nCols >= nRows, but will still work
                               // (slowly) otherwise
    if (globalWorkSize[d] % localWorkSize[d] != 0)
      globalWorkSize[d] = ((nCols / localWorkSize[d]) + 1) * localWorkSize[d];
  }

  /*
  //I did find this automatic worksize code to be slower than the above code
 size_t globalWorkSize[] = { N };
 size_t* globalWorkOffset = nullptr; // ==> offset=0 in all dims
 size_t* localWorkSize = nullptr; // ==> OpenCL runtime will pick sizes
 */

  //-----------------------------------------------------
  // Enqueue the kernel for execution
  //-----------------------------------------------------

  status = clEnqueueNDRangeKernel(
      cmdQueue, kernel,
      2,                                   // number dimensions in grid
      nullptr, globalWorkSize,             // globalOffset, globalSize
      localWorkSize, 0, nullptr, nullptr); // event information, if needed
  checkStatus("clEnqueueNDRangeKernel", status, true);

  //-----------------------------------------------------
  // Read the output buffer back to the host
  //-----------------------------------------------------

  clEnqueueReadBuffer(cmdQueue, d_C, CL_TRUE, 0, outputsize, output, 0, nullptr,
                      nullptr);

  //-----------------------------------------------------
  // Release OpenCL resources
  //-----------------------------------------------------

  // Free OpenCL resources
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseCommandQueue(cmdQueue);
  clReleaseMemObject(d_R);
  clReleaseMemObject(d_Ri);
  clReleaseMemObject(d_C);
  clReleaseContext(context);

  // Free host resources
  delete[] platforms;
  delete[] devices;
}

void CLWrapper::showProgramBuildLog(cl_program pgm, cl_device_id dev) {
  size_t size;
  clGetProgramBuildInfo(pgm, dev, CL_PROGRAM_BUILD_LOG, 0, nullptr, &size);
  char *log = new char[size + 1];
  clGetProgramBuildInfo(pgm, dev, CL_PROGRAM_BUILD_LOG, size + 1, log, nullptr);
  std::cout << "LOG:\n" << log << "\n\n";
  delete[] log;
}

const char *CLWrapper::readSource(const char *kernelPath) {
  printf("Program file is: %s\n", kernelPath);

  FILE *fp = fopen(kernelPath, "rb");
  if (!fp) {
    printf("Could not open kernel file\n");
    exit(-1);
  }
  int status = fseek(fp, 0, SEEK_END);
  if (status != 0) {
    printf("Error seeking to end of file\n");
    exit(-1);
  }
  long int size = ftell(fp);
  if (size < 0) {
    printf("Error getting file position\n");
    exit(-1);
  }

  rewind(fp);

  char *source = new char[size + 1];

  for (int i = 0; i < size + 1; i++) {
    source[i] = '\0';
  }

  fread(source, 1, size, fp);
  source[size] = '\0';

  return source;
}

cl_float3 *CLWrapper::makeFractal(int nRows, int nCols, float realMax,
                                  float realMin, float imagMax, float imagMin,
                                  int MaxIterations, int MaxLengthSquared,
                                  float juliaReal, float juliaImag,
                                  bool isJulia, cl_float3 COLOR_1,
                                  cl_float3 COLOR_2, cl_float3 COLOR_3) {

  // N is row*cols
  float *R = new float[N];
  float *Ri = new float[N];
  cl_float3 *colors = new cl_float3[N];

  // this converts the pixel locations to complex coordinates
  // I am using 2 SEPARATE arrays to hold corresponding real and imaginary parts
  for (int row = 0; row < nRows; row++)
    for (int col = 0; col < nCols; col++) {
      R[row * nCols + col] =
          realMin + ((float)col / (float)(nCols - 1)) * (realMax - realMin);
      Ri[row * nCols + col] =
          imagMin + ((float)row / (float)(nRows - 1)) * (imagMax - imagMin);
      colors[row * nCols + col] = {0.0, 0.0, 0.0};
    }

  doTheKernelLaunch(devices[devIndex], R, Ri, colors, N, nRows, nCols,
                    MaxIterations, MaxLengthSquared, juliaReal, juliaImag,
                    isJulia, COLOR_1, COLOR_2, COLOR_3); // lots of params

  delete[] R;
  delete[] Ri;
  return colors;
}
