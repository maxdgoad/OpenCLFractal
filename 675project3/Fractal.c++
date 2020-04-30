// Fractal.cpp
// University of Kansas EECS 675 Spring2020 Project 3
// Max Goad

// GPU based Mandelbrot set and Julia set fractals

// compile with make
// make mandelbrot (for mandelbrot fractal)
// make julia (for julia fractal)

// OpenCL includes
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif

#include "CLWrapper.h"
#include "ImageWriter.h"
#include <fstream>
#include <iostream>
#include <string>

CLWrapper wrapper;

// All of the parameters needed for the algorithm
//////////////////////////////////////////////
int nRows;
int nCols;
int MaxIterations;
int MaxLengthSquared;
float realMin;
float realMax;
float imagMin;
float imagMax;
float juliaReal;
float juliaImag;
bool isJulia = false;
cl_float3 COLOR_1;
cl_float3 COLOR_2;
cl_float3 COLOR_3;
//////////////////////////////////////////////

// get such parameters from the input file
bool readParams(std::string file) {
  std::ifstream input;
  input.open(file);

  if (input.is_open()) {
    std::string temp;

    input >> temp;
    nCols = stoi(temp);
    input >> temp;
    nRows = stoi(temp);

    input >> temp;
    MaxIterations = stoi(temp);
    input >> temp;
    MaxLengthSquared = stoi(temp);

    input >> temp;
    realMin = stof(temp);
    input >> temp;
    realMax = stof(temp);
    input >> temp;
    imagMax = stof(temp);
    input >> temp;
    imagMin = stof(temp);

    // Julia point
    float real, imag;
    input >> temp;
    real = stof(temp);
    input >> temp;
    imag = stof(temp);
    juliaReal = real;
    juliaImag = imag;

    float r, g, b;

    input >> temp;
    r = stof(temp);
    input >> temp;
    g = stof(temp);
    input >> temp;
    b = stof(temp);
    COLOR_1 = {r, g, b};

    input >> temp;
    r = stof(temp);
    input >> temp;
    g = stof(temp);
    input >> temp;
    b = stof(temp);
    COLOR_2 = {r, g, b};

    input >> temp;
    r = stof(temp);
    input >> temp;
    g = stof(temp);
    input >> temp;
    b = stof(temp);
    COLOR_3 = {r, g, b};

    input.close();
    return true;
  }

  return false;
}

// since I am using float3 in the cl file, we must use this function to convert
// back to a format that png can use: unsigned char*
// There is probably a better way to do this.
// Using this function vs sending a blank realimage array to png adds about .25
// seconds to runtime for a 7000x7000 mandelbrot which I was okay with
unsigned char *convertfloat3toimage(cl_float3 *image, int rows, int cols,
                                    int numChannels) {
  numChannels = 3;
  unsigned char *realimage = new unsigned char[rows * cols * numChannels];

  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {

      int loc = r * cols * numChannels + c * numChannels;

      realimage[loc] =
          static_cast<unsigned char>(image[r * (cols) + c].s[0] * 255.0 + 0.5);
      realimage[loc + 1] =
          static_cast<unsigned char>(image[r * (cols) + c].s[1] * 255.0 + 0.5);
      realimage[loc + 2] =
          static_cast<unsigned char>(image[r * (cols) + c].s[2] * 255.0 + 0.5);
    }
  }

  return realimage;
}

int main(int argc, char *argv[]) {
  if (argc < 4)
    std::cerr << "Usage: " << argv[0] << "prog M/J paramsfile outputfile\n";
  else if (!readParams(argv[2]))
    std::cerr << "Incorrect format of " << argv[2] << std::endl;
  else {

    if (strcmp(argv[1], "J") == 0)
      isJulia = true;

    wrapper.N = nRows * nCols;
    wrapper.devIndex = wrapper.typicalOpenCLProlog(
        wrapper.devType); // from an example on the website, this will get an
                          // appropriate device to use

    int numChannels = 3; // R, G, B
    ImageWriter *iw = ImageWriter::create(argv[3], nCols, nRows, numChannels);
    if (iw == nullptr)
      exit(1);

    cl_float3 *image; // I am using cl_float3 for my array of 3 channel pixels
    if (wrapper.devIndex >= 0) {
      image = wrapper.makeFractal(nRows, nCols, realMax, realMin, imagMax,
                                  imagMin, MaxIterations, MaxLengthSquared,
                                  juliaReal, juliaImag, isJulia, COLOR_1,
                                  COLOR_2, COLOR_3); // tons of params
    }

    unsigned char *realimage = convertfloat3toimage(
        image, nRows, nCols,
        numChannels); // convert the float3 array to unsigned char*

    iw->writeImage(realimage);
    iw->closeImageFile();
    delete iw;
    delete[] image;
  }

  return 0;
}