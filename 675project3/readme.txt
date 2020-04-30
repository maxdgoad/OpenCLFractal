Max Goad
KUID:2914224
University of Kansas 
EECS675 Spring2020 Project3
Fractals

notes on running/compiling:

to compile:
make

IMPORTANT NOTE ABOUT COMPILING:
***I was not able to load the libCOGLImageWriter.so from the lib folder and had to copy/paste it into
the 675project3 directory***

to run:
make mandelbrot
make julia

*for making julia and mandelbrot fractals respectively
each has their own parameters text file*



notes on design/project in general:

I have done fractal makers in Java and JavaScript before (only on the cpu)
and just lived with the slow speed. But WOW I couldn't believe how 
fast even 8000x8000 images with 1000 iterations processed
on my meager laptop gpu. Even running on the cycle server GPUs I found this to be much more memory constrained
than time constrained (ran out of memory far before I felt like I was waiting a long time).

Maybe using float3 for the pixels is too much.

I had to use floats instead of doubles because my laptop GPU does not support double.

Also I did some messing around with the complex functions, the iterations algorithm, and the colors
and made some interesting images found in the coolgenerations folder.



