main: src/main.c src/logic.c
	cc src/main.c src/logic.c -lSDL3 -lGL -lGLEW -o main
