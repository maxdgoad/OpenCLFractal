//#include "Complex.h"

__kernel
void computeColor(__global float* Q, __global unsigned char* C, int N)
{
    /*
	// Get the work-item's unique ID
	int idx = get_global_id(0);

	// Add the corresponding locations of
	//  'A' and 'B', and store the result in 'C'.
	C[2*idx] = cos(A[idx]);
	C[2*idx+1] = sin(B[idx]);
    

    int MaxIterations = 1000; // just set these here for now
    int MaxLengthSquared = 4;

    // Get the work-item's unique ID
	int col = get_global_id(0);
	int row = get_global_id(1);
	//if ((row < N) && (col < N))
	//{
		
		C[row*N + col] = "3.3";
	//}


    /*

    int rep;
    for(rep = 0; rep < MaxIterations; rep++)
    {
        Complex X = R*R + S;
        if (X.LengthSquared() > MaxLengthSquared)
            break;
        R = X;
    }
    if (rep >= MaxIterations)
        c = COLOR_1;
    else
    {
        Float f = (float)(((float) rep)/((float)MaxIterations)); // 0 < f < 1
        c = (1.0 - f)*COLOR_2 + f*COLOR_3;
    }
    return color;
    */
}