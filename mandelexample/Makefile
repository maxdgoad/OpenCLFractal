# uncomment the following lines if running on linux
CF_EXTRA = -Wno-deprecated-declarations
# 32-bit
OPENCL_LIB = "C:\Windows\System32\OpenCL.dll"
#OPENCL_LIB = -LOpenCl
#uncomment the following lines if running on Mac OS
# CF_EXTRA = -DCL_SILENCE_DEPRECATION
# OPENCL_LIB = -framework OpenCL
#end: linux versus Mac OS
HEADERS = -I"C:/Users/derek/Desktop/OpenCL-Headers-master/CL"

PROG = Fractal
WRAP = CLWrapper

CPP = g++ -fPIC -std=c++11
LINK = g++ -fPIC

INC = -I../Packed3DArray -I../ImageWriter
CFLAGS = -g -c $(INC) $(CF_EXTRA)

LIB = ../lib/libCOGLImageWriter.dll

$(PROG): $(PROG).o $(WRAP).o $(LIB)
	$(LINK) $(HEADERS) -g -o $(PROG) $(PROG).o  $(WRAP).o $(LIB) $(OPENCL_LIB)

../lib/libCOGLImageWriter.dll: ../ImageWriter/ImageWriter.h ../ImageWriter/ImageWriter.c++
	(cd ../ImageWriter; make)

$(PROG).o: $(PROG).c++
	$(CPP) $(CFLAGS) $(PROG).c++
$(WRAP).o: $(WRAP).h $(WRAP).c++
	$(CPP) $(CFLAGS) $(WRAP).c++

mandelbrot:
	.\$(PROG) M mandelbrotparams.txt image.png
julia:
	(export DYLD_LIBRARY_PATH=../lib; ./$(PROG) J juliaparams.txt julia.png)

clean: 
	rm *.o Fractal