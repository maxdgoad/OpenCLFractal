//my opencl wrapper class
//this will handle lots of the cpu side opencl functions and variables that i need
//without making the Fractal.c++ file too messy (as if it won't already be very messy)

#ifndef CLWRAPPER_H
#define CLWRAPPER_H

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif

#include <iostream>
#include "Complex.h"

//a simple color class
class Color
{
private:
    //color values between 0 and 1
    float R;
    float G;
    float B;
public:
    Color(){}
    Color(float R, float G,float B) : R(R), G(G), B(B) {}
    Color operator*(float f){return Color(f*R, f*G, f*B);}

};

struct NameTable
{
	std::string name;
	int value;
};

class CLWrapper
{

private:
    bool debug = false;


public:
    CLWrapper();
    ~CLWrapper();

    cl_device_type devType = CL_DEVICE_TYPE_DEFAULT;
	size_t N = 0;

    //1) Platforms
    cl_uint numPlatforms = 0;
    cl_platform_id* platforms = nullptr;
    cl_platform_id curPlatform;
    // 2) Devices
    cl_uint numDevices = 0;
    cl_device_id* devices = nullptr;

    int devIndex;

    int typicalOpenCLProlog(cl_device_type desiredDeviceType);
    void checkStatus(std::string where, cl_int status, bool abortOnError);
    void reportPlatformInformation(const cl_platform_id& platformIn);

    void doTheKernelLaunch(cl_device_id dev, Complex* input, unsigned char* output, size_t N);
    const char* readSource(const char* kernelPath);
    void showProgramBuildLog(cl_program pgm, cl_device_id dev);

    unsigned char* makeFractal(int nRows, int nCols, int realMax, int realMin, int imagMax, int imagMin, int MaxIterations, int MaxLengthSquared);

};

#endif

