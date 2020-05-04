
// this complex number code was repurposed from code found online
// source: http://www.davidespataro.it/complex-number-opencl/

////////////////////////////////////////////////////////////////

typedef float2 cl_float_complex;

typedef cl_float_complex cl_complex;
typedef float TYPE;

inline TYPE cl_complex_real_part(const cl_complex *n) { return n->x; }

inline TYPE cl_complex_imaginary_part(const cl_complex *n) { return n->y; }
inline TYPE cl_complex_lengthSquared(const cl_complex *n) {
  return (n->x * n->x) + (n->y * n->y);
}

inline cl_complex cl_complex_add(const cl_complex *a, const cl_complex *b) {
  return (cl_complex)(a->x + b->x, a->y + b->y);
}

inline cl_complex cl_complex_subtract(const cl_complex *a,
                                      const cl_complex *b) {
  return (cl_complex)(a->x - b->x, a->y - b->y);
}

inline cl_complex cl_complex_divide(const cl_complex *a, const cl_complex *b) {
  return (cl_complex)((a->x * b->x + a->y * b->y) / (b->x * b->x + b->y * b->y),
                      (a->y * b->x - a->x * b->y) /
                          (b->x * b->x + b->y * b->y));
}

inline cl_complex cl_complex_multiply(const cl_complex *a,
                                      const cl_complex *b) {
  return (cl_complex)(a->x * b->x - a->y * b->y, a->x * b->y + a->y * b->x);
}

////////////////////////////////////////////////////////////////

__kernel void computeColor(__global float *R, __global float *Ri,
                           __global float3 *C, int N, int nRows, int nCols,
                           int MaxIterations, int MaxLengthSquared,
                           float juliaReal, float juliaImag, int isJulia,
                           float3 color1, float3 color2, float3 color3) {

  int col = get_global_id(0);
  int row = get_global_id(1);

  if (col < nCols && row < nRows) {

    // this will the "R" complex number
    cl_complex R1;

    R1.x = R[row * nCols + col];
    R1.y = Ri[row * nCols + col];

    // this will be the "S" complex number (=R if mandelbrot, =JuliaPoint if
    // julia fractal)
    cl_complex S1;

    if (isJulia == 1) {
      // julia set
      S1.x = -.765;
      S1.y = .11;
    } else {
      // mandelbrot set
      S1.x = R1.x;
      S1.y = R1.y;
    }

    // iterations algorithm
    cl_complex X;

    int rep;
    for (rep = 1; rep <= MaxIterations; rep++) {

      //Mandelbrot/Julia
      // X = R^2 + S 
      cl_complex rsquared = cl_complex_multiply(&R1, &R1); //R^2
      X = cl_complex_add(&rsquared, &S1); // + S 

      if (cl_complex_lengthSquared(&X) > MaxLengthSquared)
        break;
      R1 = X;
      
    }


    //purple/blue colors
    if (rep == 0)
    {
      C[row * nCols + col] = color1;
    }
    else
    {
      C[row * nCols + col] = (float3) {(float)(8*rep % 256)/256, (float)(rep*rep%256)/256, (float)(19*rep%256)/256} ;
    }
    
    // project spec color assignment
    /*
    if (rep >= MaxIterations) {
      C[row * nCols + col] = color1;
    } else {
      float f = (float)(rep) / (float)MaxIterations; // 0 < f < 1
      C[row * nCols + col] = (float3)((1.0f - f) * color2 + f * color3);
    }
    */
    
  }
}