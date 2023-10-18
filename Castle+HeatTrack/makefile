# SpanTracker Castle Demo makefile

CC = /usr/bin/g++

all: Castle
	@echo Done

Castle: *.h *.cpp
	@(cd GUI/src; make)
	$(CC) -c -O2 -DUNIX -DNDEBUG -I ../../SDK/Include *.cpp integrator/*.c -I GUI/src
	$(CC) -L /usr/lib/x86_64-linux-gnu -L ../../SDK/Library/Debug -L GUI/lib \
              *.o -o Castle -lSM -lICE -lWm5GlxApplication -lWm5GlxGraphics -lWm5Imagics \
              -lWm5Physics -lWm5Mathematics -lWm5Core -lm -lGL -lGLU -lX11 -lXext -lXt -lpthread \
              -lsxmlgui -lglpng

clean:
	/bin/rm -f *.o
	@(cd GUI/src; make clean)
