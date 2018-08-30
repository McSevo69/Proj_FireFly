CC := gcc
CFLAGS := -Wall -O3 -pedantic -std=c99
OBJECTS := ff.c ppmIO.c visualizer.c colors.c
LIBRARIES := -lm -lSDL2 -lSDL2_ttf

.Phony:
all : main

main : $(OBJECTS)
	$(CC) -o main $(OBJECTS) $(CFLAGS) $(LIBRARIES)

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(LIBRARIES)

clean:
	for file in main *.o; do if [ -f $${file} ]; then rm $${file}; fi; done

run: main
	./main
