CPP = g++ -std=c++11 -fPIC
INCLUDES = -I../Packed3DArray

CFLAGS = -O -c $(INCLUDES)

#PNG = -llibpng
PNG = -lpng

OBJS = ImageWriter.o PNGImageWriter.o

../lib/libCOGLImageWriter.dll: libCOGLImageWriter.dll
	copy libCOGLImageWriter.dll ../lib

libCOGLImageWriter.dll: $(OBJS)
	g++ -shared -o libCOGLImageWriter.dll $(OBJS) $(PNG)

ImageWriter.o: ImageWriter.c++
	$(CPP) $(CFLAGS) ImageWriter.c++

PNGImageWriter.o: PNGImageWriter.c++
	$(CPP) $(CFLAGS) PNGImageWriter.c++
