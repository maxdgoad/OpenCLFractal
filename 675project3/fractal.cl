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
void computeColor(__global float* Q, __global __float3* C, int N)
{
    __float3 c = {1,1,1};
    
    /*

    cl_complex X;
    X.x = 3.0;
    X.y = 4.0;
    int MaxIterations = 100;
    float MaxLengthSquared = 4.0;

    int rep;
    for(rep = 0; rep < MaxIterations; rep++)
    {
        if (cl_complex_lengthSquared(&X) > MaxLengthSquared)
            break;
    }
    if (rep >= MaxIterations)
        c = c;
    else
    {
        float f = (float)(((float) rep)/((float)MaxIterations)); // 0 < f < 1
        //c = (unsigned char) ((1.0 - f)*(0.4) + f*(0.9));
    }
    */

    int col = get_global_id(0);
    int row = get_global_id(1);
    printf("%d\n", col);
    C[col] = c;
    
}