//clwrapper.c++

#include "CLWrapper.h"

CLWrapper::CLWrapper(){}
CLWrapper::~CLWrapper(){}

void CLWrapper::checkStatus(std::string where, cl_int status, bool abortOnError)
{
    if (debug || (status != 0))
		std::cout << "Step " << where << ", status = " << status << '\n';
	if ((status != 0) && abortOnError)
		exit(1);
}

void CLWrapper::reportPlatformInformation(const cl_platform_id& platformIn)
{
	NameTable what[] = {
		{ "CL_PLATFORM_PROFILE:    ", CL_PLATFORM_PROFILE },
		{ "CL_PLATFORM_VERSION:    ", CL_PLATFORM_VERSION },
		{ "CL_PLATFORM_NAME:       ", CL_PLATFORM_NAME },
		{ "CL_PLATFORM_VENDOR:     ", CL_PLATFORM_VENDOR },
		{ "CL_PLATFORM_EXTENSIONS: ", CL_PLATFORM_EXTENSIONS },
		{ "", 0 }
	};
	size_t size;
	char* buf = nullptr;
	int bufLength = 0;
	std::cout << "===============================================\n";
	std::cout << "========== PLATFORM INFORMATION ===============\n";
	std::cout << "===============================================\n";
	for (int i=0 ; what[i].value != 0 ; i++)
	{
		clGetPlatformInfo(platformIn, what[i].value, 0, nullptr, &size);
		if (size > bufLength)
		{
			if (buf != nullptr)
				delete [] buf;
			buf = new char[size];
			bufLength = size;
		}
		clGetPlatformInfo(platformIn, what[i].value, bufLength, buf, &size);
		std::cout << what[i].name << buf << '\n';
	}
	std::cout << "================= END =========================\n\n";
	if (buf != nullptr)
		delete [] buf;
}


int CLWrapper::typicalOpenCLProlog(cl_device_type desiredDeviceType)
{
    //-----------------------------------------------------
	// Discover and query the platforms
	//-----------------------------------------------------

	cl_int status = clGetPlatformIDs(0, nullptr, &numPlatforms);
	checkStatus("clGetPlatformIDs-0", status, true);

	platforms = new cl_platform_id[numPlatforms];
 
	status = clGetPlatformIDs(numPlatforms, platforms, nullptr);
	checkStatus("clGetPlatformIDs-1", status, true);
	int which = 0;
	if (numPlatforms > 1)
	{
		std::cout << "Found " << numPlatforms << " platforms:\n";
		for (int i=0 ; i<numPlatforms ; i++)
		{
			std::cout << i << ": ";
			reportPlatformInformation(platforms[i]);
		}
		which = -1;
		while ((which < 0) || (which >= numPlatforms))
		{
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

	status = clGetDeviceIDs(curPlatform, desiredDeviceType, 0, nullptr, &numDevices);
	checkStatus("clGetDeviceIDs-0", status, true);
	if (numDevices <= 0)
	{
		std::cout << "No devices on platform!\n";
		return -1;
	}

	devices = new cl_device_id[numDevices];

	status = clGetDeviceIDs(curPlatform, desiredDeviceType, numDevices, devices, nullptr);
	checkStatus("clGetDeviceIDs-1", status, true);
	// Find a device that supports double precision arithmetic
	int* possibleDevs = new int[numDevices];
	int nPossibleDevs = 0;
	std::cout << "\nLooking for a device that supports double precision...\n";
	for (int idx=0 ; idx<numDevices ; idx++)
	{
		size_t extLength;
		clGetDeviceInfo(devices[idx], CL_DEVICE_EXTENSIONS, 0, nullptr, &extLength);
		char* extString = new char[extLength+1];
		clGetDeviceInfo(devices[idx], CL_DEVICE_EXTENSIONS, extLength+1, extString, nullptr);
		const char* fp64 = strstr(extString, "cl_khr_fp64");
		if (fp64 != nullptr) // this device supports double precision
			possibleDevs[nPossibleDevs++] = idx;
		delete [] extString;
	}
	if (nPossibleDevs == 0)
	{
		std::cerr << "\nNo device supports double precision.\n";
		return -1;
	}
	size_t nameLength;
	for (int i=0 ; i<nPossibleDevs ; i++)
	{
		clGetDeviceInfo(devices[possibleDevs[i]], CL_DEVICE_NAME, 0, nullptr, &nameLength);
		char* name = new char[nameLength+1];
		clGetDeviceInfo(devices[possibleDevs[i]], CL_DEVICE_NAME, nameLength+1, name, nullptr);
		std::cout << "Device " << i << ": [" << name << "] supports double precision.\n";
		delete [] name;
	}
	if (nPossibleDevs == 1)
	{
		std::cout << "\nNo other device in the requested device category supports double precision.\n"
		          << "You may want to try the -a command line option to see if there are others.\n"
		          << "For now, I will use the one I found.\n";
		return possibleDevs[0];
	}
	int devIndex = -1;
	while ((devIndex < 0) || (devIndex >= nPossibleDevs))
	{
		std::cout << "Which device do you want to use? ";
		std::cin >> devIndex;
	}
	return possibleDevs[devIndex];
}

void CLWrapper::doTheKernelLaunch(cl_device_id dev, Complex* input, Color* output, size_t N)
{
	//------------------------------------------------------------------------
	// Create a context for some or all of the devices on the platform
	// (Here we are including all devices.)
	//------------------------------------------------------------------------

	cl_int status;
	cl_context context = clCreateContext(nullptr, 1, &dev, nullptr, nullptr, &status);
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

	size_t inputDatasize = sizeof(Complex) * N;
	size_t outputDatasize = sizeof(Color) * N;
    

	cl_mem d_Q = clCreateBuffer( // Input array on the device
		context, CL_MEM_READ_ONLY, inputDatasize, nullptr, &status);
	checkStatus("clCreateBuffer-Q", status, true);

	cl_mem d_C = clCreateBuffer( // Output complex on the device
		context, CL_MEM_WRITE_ONLY, outputDatasize, nullptr, &status);
	checkStatus("clCreateBuffer-C", status, true);

	//-----------------------------------------------------
	// Use the command queue to encode requests to
	//         write host data to the device buffers
	//----------------------------------------------------- 

	status = clEnqueueWriteBuffer(cmdQueue, 
		d_Q, CL_FALSE, 0, inputDatasize,                         
		&input, 0, nullptr, nullptr);
	checkStatus("clEnqueueWriteBuffer-Q", status, true);


	//-----------------------------------------------------
	// Create, compile, and link the program
	//----------------------------------------------------- 

	const char* programSource[] = { readSource("fractal.cl") };
	cl_program program = clCreateProgramWithSource(context, 
		1, programSource, nullptr, &status);
	checkStatus("clCreateProgramWithSource", status, true);

	status = clBuildProgram(program, 1, &dev, nullptr, nullptr, nullptr);
	if (status != 0)
		showProgramBuildLog(program, dev);
	checkStatus("clBuildProgram", status, true);

	//----------------------------------------------------------------------
	// Create a kernel using a "__kernel" function in the ".cl" file
	//----------------------------------------------------------------------

	cl_kernel kernel = clCreateKernel(program, "Fractal", &status);

	//-----------------------------------------------------
	// Set the kernel arguments
	//----------------------------------------------------- 

	status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_Q);
	checkStatus("clSetKernelArg-Q", status, true);

	status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_C);
	checkStatus("clSetKernelArg-C", status, true);
	status = clSetKernelArg(kernel, 3, sizeof(int), &N);
	checkStatus("clSetKernelArg-N", status, true);

	//-----------------------------------------------------
	// Configure the work-item structure
	//----------------------------------------------------- 

	size_t localWorkSize[] = { 16, 16 };
	size_t globalWorkSize[2];
	// Global work size needs to be at least NxN, but it must
	// also be a multiple of local size in each dimension:
	for (int d=0 ; d<2 ; d++)
	{
		globalWorkSize[d] = N;
		if (globalWorkSize[d]%localWorkSize[d] != 0)
			globalWorkSize[d] = ((N / localWorkSize[d]) + 1) * localWorkSize[d];
	}

	//-----------------------------------------------------
	// Enqueue the kernel for execution
	//----------------------------------------------------- 

	status = clEnqueueNDRangeKernel(cmdQueue, kernel,
		2, // number dimensions in grid
		nullptr, globalWorkSize, // globalOffset, globalSize
		localWorkSize,
		0, nullptr, nullptr); // event information, if needed
	checkStatus("clEnqueueNDRangeKernel", status, true);

	//-----------------------------------------------------
	// Read the output buffer back to the host
	//----------------------------------------------------- 

	clEnqueueReadBuffer(cmdQueue, 
		d_C, CL_TRUE, 0, outputDatasize, 
		output, 0, nullptr, nullptr);

	//-----------------------------------------------------
	// Release OpenCL resources
	//----------------------------------------------------- 

	// Free OpenCL resources
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmdQueue);
	clReleaseMemObject(d_Q);
	clReleaseMemObject(d_C);
	clReleaseContext(context);

	// Free host resources
	delete [] platforms;
	delete [] devices;
}

void CLWrapper::showProgramBuildLog(cl_program pgm, cl_device_id dev)
{
	size_t size;
	clGetProgramBuildInfo(pgm, dev, CL_PROGRAM_BUILD_LOG, 0, nullptr, &size);
	char* log = new char[size+1];
	clGetProgramBuildInfo(pgm, dev, CL_PROGRAM_BUILD_LOG, size+1, log, nullptr);
	std::cout << "LOG:\n" << log << "\n\n";
	delete [] log;
}

const char* CLWrapper::readSource(const char* kernelPath)
{
   printf("Program file is: %s\n", kernelPath);

   FILE* fp = fopen(kernelPath, "rb");
   if(!fp)
   {
      printf("Could not open kernel file\n");
      exit(-1);
   }
   int status = fseek(fp, 0, SEEK_END);
   if(status != 0)
   {
      printf("Error seeking to end of file\n");
      exit(-1);
   }
   long int size = ftell(fp);
   if(size < 0)
   {
      printf("Error getting file position\n");
      exit(-1);
   }

   rewind(fp);

   char* source = new char[size + 1];

   for (int i = 0; i < size+1; i++)
   {
      source[i]='\0';
   }

   fread(source, 1, size, fp);
   source[size] = '\0';

   return source;
}


