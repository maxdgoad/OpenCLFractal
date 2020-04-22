__kernel
Color computeColor(__global Complex R, __global Complex J, __global int MaxIterations, __global float MaxLengthSquared)
{
    /*
	// Get the work-item's unique ID
	int idx = get_global_id(0);

	// Add the corresponding locations of
	//  'A' and 'B', and store the result in 'C'.
	C[2*idx] = cos(A[idx]);
	C[2*idx+1] = sin(B[idx]);
    */
    Color c;

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
}