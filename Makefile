CFLAGS = -I ./include
##LIB    = ./libggfonts.so
LFLAGS = -lrt -lX11 -lGLU -lGL -pthread -lm #-lXrandr

all: rocketlander

rocketlander: rocketlander.cpp log.cpp mytime.cpp ppm.cpp \
    ramonR.cpp ramonR.h \
    abrahamA.cpp abrahamA.h \
    patrickE.cpp patrickE.h \
    nicholasP.cpp nicholasP.h
	
	g++ $(CFLAGS) rocketlander.cpp log.cpp mytime.cpp ppm.cpp\
	    nicholasP.cpp \
	    ramonR.cpp \
	    abrahamA.cpp \
	    patrickE.cpp \
	    libggfonts.a \
	    /usr/lib/x86_64-linux-gnu/libopenal.so \
	    /usr/lib/x86_64-linux-gnu/libalut.so \
	    -Wall -Wextra $(LFLAGS) -DUSE_OPENAL_SOUND -o rocketlander

clean:
	rm -f rocketlander
	rm -f *.o
