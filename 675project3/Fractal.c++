//Fractal.cpp
//University of Kansas EECS 675 Spring2020 Project 3
//Max Goad

//GPU based Mandelbrot and Julia sets fractals
//I will also add logrithmic color blending and my own fractal function

//compile with make
//make mandelbrot (for mandelbrot fractal)
//make julia (for julia fractal)
//make max (for my own fractal function)

// OpenCL includes
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif

#include "ImageWriter.h"
#include "Complex.h"
#include "CLWrapper.h"
#include <iostream>
#include <string>
#include <fstream>


CLWrapper wrapper;

int nRows;
int nCols;
int MaxIterations;
int MaxLengthSquared;
float realMin;
float realMax;
float imagMin;
float imagMax;
Complex JuliaPoint;
Color COLOR_1;
Color COLOR_2;
Color COLOR_3;

bool readParams(std::string file)
{
    std::ifstream input;
    input.open(file);

    if(input.is_open())
    {
        std::string temp;

        input >> temp;
        nRows = stoi(temp);
        input >> temp;
        nCols = stoi(temp);

        input >> temp;
        MaxIterations = stoi(temp);
        input >> temp;
        MaxLengthSquared = stoi(temp);

        input >> temp;
        realMin = stof(temp);
        input >> temp;
        realMax = stof(temp);
        input >> temp;
        imagMin = stof(temp);
        input >> temp;
        imagMin = stof(temp);

        //Julia point
        float real,imag;
        input >> temp;
        real = stof(temp);
        input >> temp;
        imag = stof(temp);
        JuliaPoint = Complex(real,imag);

        float r,g,b;
        input >> temp;
        r = stof(temp);
        input >> temp;
        g = stof(temp);
        input >> temp;
        b = stof(temp);
        COLOR_1 = Color(r,g,b);

        input >> temp;
        r = stof(temp);
        input >> temp;
        g = stof(temp);
        input >> temp;
        b = stof(temp);
        COLOR_2 = Color(r,g,b);

        input >> temp;
        r = stof(temp);
        input >> temp;
        g = stof(temp);
        input >> temp;
        b = stof(temp);
        COLOR_3 = Color(r,g,b);

        input.close();
        return true;
    }

    return false;
}

unsigned char* convertfloat3toimage(cl_float3 *image, int rows, int cols, int numChannels)
{
    numChannels = 3;
    unsigned char * realimage = new unsigned char[rows*cols*numChannels];
    for (int r=0 ; r<rows ; r++)
    {
        for (int c=0 ; c<cols ; c++)
        {
            int loc = r*cols*numChannels + c*numChannels;
            unsigned char pixelVal = static_cast<unsigned char>(image[r+c].s[0] * 255.0);
            realimage[loc] = pixelVal;
            realimage[loc+1] = pixelVal;
            realimage[loc+2] = pixelVal;
            
        }
    }
    return realimage;

}

int main(int argc, char* argv[])
{
    if (argc < 4)
        std::cerr << "Usage: " << argv[0] << " M/J/MDG paramsfile outputfile\n";
    else if(!readParams(argv[2]))
        std::cerr << "Incorrect format of " << argv[2] << std::endl;
	else
	{
        wrapper.N = nRows * nCols;
        wrapper.devIndex = wrapper.typicalOpenCLProlog(wrapper.devType); //🙈🙈🙈🙈🙈🙈🙈


		int numChannels = 3; // R, G, B
		ImageWriter* iw = ImageWriter::create(argv[3], nCols, nRows, numChannels);
		if (iw == nullptr)
			exit(1);

        cl_float3* image;
		// We would launch a GPU kernel to get the data to be written; let's just
        // use a placeholder here:
        if (wrapper.devIndex >= 0)
        {
            //unsigned char* image = do_MatrixMultiply(devices[devIndex], N);
            image = wrapper.makeFractal(nRows, nCols, realMax,  realMin,  imagMax,  imagMin,  MaxIterations,  MaxLengthSquared);
        }

        unsigned char * realimage = convertfloat3toimage(image, nRows, nCols, numChannels);
        

		iw->writeImage(realimage);
		iw->closeImageFile();
		delete iw;
		delete [] image;
	}


    return 0;
}