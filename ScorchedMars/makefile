# Scorched Mars makefile

CC = /usr/bin/g++

all: ScorchedMarsSP ScorchedMarsMP
	@echo Done

ScorchedMarsSP: *.h *.hpp *.c *.cpp
	@echo Building single-player game...
	@test -s ../fmod/lib/libfmodex.so || { echo "Please link appropriate ../fmod/lib .so to ../fmod/lib/libfmodex.so! Exiting..."; exit 1; }
	$(CC) -c -O2 -DUNIX -DNDEBUG -I /usr/X11R6/include -I ../../SDK/Include -I ../fmod/inc *.cpp *.c
	$(CC) -L /usr/lib/x86_64-linux-gnu -L ../../SDK/Library/Debug \
              *.o ../fmod/lib/libfmodex.so -o ScorchedMarsSP \
              -lSM -lICE -lWm5GlxApplication -lWm5GlxGraphics -lWm5Imagics \
              -lWm5Physics -lWm5Mathematics -lWm5Core -lm -lGL -lGLU -lX11 -lXext -lXt -lpthread

ScorchedMarsMP: *.h *.hpp *.c *.cpp
	@echo Building multi-player game...
	@test -s ../fmod/lib/libfmodex.so || { echo "Please link appropriate ../fmod/lib .so to ../fmod/lib/libfmodex.so! Exiting..."; exit 1; }	
	$(CC) -c -O2 -DUNIX -DNETWORK -DNDEBUG -I /usr/X11R6/include -I ../../SDK/Include -I ../fmod/inc *.cpp *.c
	$(CC) -L /usr/lib/x86_64-linux-gnu -L ../../SDK/Library/Debug \
              *.o ../fmod/lib/libfmodex.so -o ScorchedMarsMP \
              -lSM -lICE -lWm5GlxApplication -lWm5GlxGraphics -lWm5Imagics \
              -lWm5Physics -lWm5Mathematics -lWm5Core -lm -lGL -lGLU -lX11 -lXext -lXt -lpthread

clean:
	/bin/rm -f *.o


