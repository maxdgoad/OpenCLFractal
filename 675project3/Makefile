# uncomment the following lines if running on linux
#CF_EXTRA = -Wno-deprecated-declarations
#OPENCL_LIB = -lOpenCL
#uncomment the following lines if running on Mac OS
CF_EXTRA = -DCL_SILENCE_DEPRECATION
OPENCL_LIB = -framework OpenCL
#end: linux versus Mac OS


PROG = Fractal
WRAP = CLWrapper

CPP = g++ -fPIC -std=c++11
LINK = g++ -fPIC

INC = -I../Packed3DArray -I../ImageWriter
CFLAGS = -g -c $(INC) $(CF_EXTRA)

LIB = ../lib/libCOGLImageWriter.so

$(PROG): $(PROG).o $(WRAP).o $(LIB)
	$(LINK) -g -o $(PROG) $(PROG).o  $(WRAP).o $(LIB) $(OPENCL_LIB)

../lib/libCOGLImageWriter.so: ../ImageWriter/ImageWriter.h ../ImageWriter/ImageWriter.c++
	(cd ../ImageWriter; make)

$(PROG).o: $(PROG).c++
	$(CPP) $(CFLAGS) $(PROG).c++
$(WRAP).o: $(WRAP).h $(WRAP).c++
	$(CPP) $(CFLAGS) $(WRAP).c++

mandelbrot:
	./$(PROG) M mandelbrotparams.txt mandelbrot.png
julia:
	./$(PROG) J juliaparams.txt julia.png

clean: 
	rm *.o Fractal