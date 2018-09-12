CC := gcc 
CFLAGS := -Wall -O3 -std=c99
OBJECTS := ff.c ppmIO.c visualizer.c colors.c
LIBRARIES := -lSDL2 -lSDL2_image -lm
INCLUDE := -I${PWD}/libs/include

.Phony:
all : main

main : $(OBJECTS)
	$(CC) -o main $(OBJECTS) $(CFLAGS) $(INCLUDE) $(LIBRARIES)

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDE)

clean:
	for file in main *.o; do if [ -f $${file} ]; then rm $${file}; fi; done

run: main
	./main
