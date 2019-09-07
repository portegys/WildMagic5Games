# SpanTracker Castle Demo makefile

CC = /usr/bin/g++

all: Castle
	@echo Done

Castle: *.h *.cpp integrator/*.h integrator/*.cpp
	@(cd GUI/src; make)
	$(CC) -c -std=gnu++11 -O2 -DUNIX -DNDEBUG -I ../../SDK/Include *.cpp integrator/*.cpp -I GUI/src
	$(CC) -L /usr/lib/x86_64-linux-gnu -L ../../SDK/Library/Debug -L GUI/lib \
              *.o -o Castle -lSM -lICE -lWm5GlxApplication -lWm5GlxGraphics -lWm5Imagics \
              -lWm5Physics -lWm5Mathematics -lWm5Core -lm -lGL -lGLU -lX11 -lXext -lXt -lpthread \
              -lsxmlgui -lglpng -lstdc++

clean:
	/bin/rm -f *.o
	@(cd GUI/src; make clean)
