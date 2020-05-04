// my opencl wrapper class
// this will handle lots of the cpu side opencl functions and variables that i
// need without making the Fractal.c++ file too messy (as if it won't already be
// very messy)

#ifndef CLWRAPPER_H
#define CLWRAPPER_H

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif

#include <iostream>

struct NameTable {
  std::string name;
  int value;
};

class CLWrapper {

private:
  bool debug = false;

public:
  CLWrapper();
  ~CLWrapper();

  cl_device_type devType = CL_DEVICE_TYPE_DEFAULT;
  size_t N = 0;

  // 1) Platforms
  cl_uint numPlatforms = 0;
  cl_platform_id *platforms = nullptr;
  cl_platform_id curPlatform;
  // 2) Devices
  cl_uint numDevices = 0;
  cl_device_id *devices = nullptr;

  int devIndex;

  // these functions are largely adapted from the code in matrixMultiplyV1.cpp
  // on website
  ////////////////////////////////////////////////////////////////
  int typicalOpenCLProlog(cl_device_type desiredDeviceType);
  void checkStatus(std::string where, cl_int status, bool abortOnError);
  void reportPlatformInformation(const cl_platform_id &platformIn);

  void doTheKernelLaunch(cl_device_id dev, float *R, float *Ri,
                         cl_float3 *output, size_t N, int nRows, int nCols,
                         int MaxIterations, int MaxLengthSquared,
                         float juliaReal, float juliaImag, bool isJulia,
                         cl_float3 COLOR_1, cl_float3 COLOR_2,
                         cl_float3 COLOR_3);
  const char *readSource(const char *kernelPath);
  void showProgramBuildLog(cl_program pgm, cl_device_id dev);

  ////////////////////////////////////////////////////////////////

  // this function was NOT adapted from any code on the website
  // this would be the equivalent of the do_matrixMultiply function from
  // matrixMultiplyV1.cpp on website inits arrays and does pixel to complex
  // mapping
  cl_float3 *makeFractal(int nRows, int nCols, float realMax, float realMin,
                         float imagMax, float imagMin, int MaxIterations,
                         int MaxLengthSquared, float juliaReal, float juliaImag,
                         bool isJulia, cl_float3 COLOR_1, cl_float3 COLOR_2,
                         cl_float3 COLOR_3);
};

#endif
