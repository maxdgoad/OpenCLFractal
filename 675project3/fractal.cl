//#include "Complex.h"

typedef float2 cl_float_complex;

typedef cl_float_complex cl_complex;
typedef float TYPE;


inline TYPE cl_complex_real_part(const cl_complex* n){
	return n->x;
}

inline TYPE cl_complex_imaginary_part(const cl_complex* n){
	return n->y;
}


inline TYPE cl_complex_lengthSquared(const cl_complex* n){
	return (n->x*n->x)+(n->y*n->y);
}


inline cl_complex cl_complex_add(const cl_complex* a, const cl_complex* b){
	return (cl_complex)( a->x + b->x, a->y + b->y );
}

inline cl_complex cl_complex_multiply(const cl_complex* a, const cl_complex* b){
	return (cl_complex)(a->x*b->x - a->y*b->y,  a->x*b->y + a->y*b->x);
}

__kernel
void computeColor(__global float* R, __global float* Ri, __global float3* C, int N, int nRows, int nCols)
{
    int col = get_global_id(0);
    int row = get_global_id(1);

    if(col < nCols && row < nRows)
    {
        cl_complex R1;

        R1.x = R[row*nCols + col];
        R1.y = Ri[row*nCols + col];

        cl_complex S1;

        //julia set test
        S1.x = (float) -0.765;
        S1.y = (float) 0.11;

        //mandelbrot set test
        S1 = R1;


        //float realS = S[row*nCols + col];
        //float imagS = Si[row*nCols + col];

        int MaxIterations = 1000;
        float MaxLengthSquared = 4.0;

        cl_complex X;

        int rep;
        for(rep = 0; rep < MaxIterations; rep++)
        {
            cl_complex rsquared = cl_complex_multiply(&R1, &R1);
            X = cl_complex_add(&rsquared, &S1); // mandelbrot
            if(cl_complex_lengthSquared(&X) > MaxLengthSquared)
                break;
            R1 = X;
        }

        if(rep >= MaxIterations)
        {
            C[row*nCols + col] = (float3){0.1, 0.1, 0.1};
        }
        else
        {
            float f = float(rep)/(float)MaxIterations; // 0 < f < 1
            float first = (1.0 - f) + f;
             C[row*nCols + col] = (float3) {first, f, 0};
        }

    }
    
}