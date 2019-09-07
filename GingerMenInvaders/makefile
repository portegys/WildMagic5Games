# Ginger Men Invaders makefile

CC = /usr/bin/g++

all: GingerMenInvadersSP GingerMenInvadersMP
	@echo Done

GingerMenInvadersSP: *.h *.hpp *.c *.cpp ObjMtl/*.h ObjMtl/*.inl ObjMtl/*.cpp
	@echo Building single-player game...
	@test -s ../fmod/lib/libfmodex.so || { echo "Please link appropriate ../fmod/lib .so to ../fmod/lib/libfmodex.so! Exiting..."; exit 1; }
	$(CC) -c -O2 -DUNIX -DNDEBUG -I ../../SDK/Include -I ../fmod/inc *.cpp *.c ObjMtl/*.cpp
	$(CC) -L /usr/lib/x86_64-linux-gnu -L ../../SDK/Library/Debug \
              *.o ../fmod/lib/libfmodex.so -o GingerMenInvadersSP \
              -lSM -lICE -lWm5GlxApplication -lWm5GlxGraphics -lWm5Imagics \
              -lWm5Physics -lWm5Mathematics -lWm5Core -lm -lGL -lGLU -lX11 -lXext -lXt -lpthread

GingerMenInvadersMP: *.h *.hpp *.c *.cpp ObjMtl/*.h ObjMtl/*.inl ObjMtl/*.cpp
	@echo Building multi-player game...
	@test -s ../fmod/lib/libfmodex.so || { echo "Please link appropriate ../fmod/lib .so to ../fmod/lib/libfmodex.so! Exiting..."; exit 1; }	
	$(CC) -c -O2 -DUNIX -DNETWORK -DNDEBUG -I ../../SDK/Include -I ../fmod/inc *.cpp *.c ObjMtl/*.cpp
	$(CC) -L /usr/lib/x86_64-linux-gnu -L ../../SDK/Library/Debug \
              *.o ../fmod/lib/libfmodex.so -o GingerMenInvadersMP \
              -lSM -lICE -lWm5GlxApplication -lWm5GlxGraphics -lWm5Imagics \
              -lWm5Physics -lWm5Mathematics -lWm5Core -lm -lGL -lGLU -lX11 -lXext -lXt -lpthread

clean:
	/bin/rm -f *.o
