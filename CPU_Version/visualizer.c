#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "colors.h"

#define SHAPE_SIZE 1

typedef int dataType;

void startVisualisation(int width, int height, int iterations, dataType **dataOut, dataType **paramsOut) {

	SDL_Window* Main_Window;
	SDL_Renderer* Main_Renderer;
	SDL_Surface* Loading_Surf;

	/* Before we can render anything, we need a window and a renderer */
	Main_Window = SDL_CreateWindow("Forest-fire model",
	SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * SHAPE_SIZE, height * SHAPE_SIZE, 0);
	Main_Renderer = SDL_CreateRenderer(Main_Window, -1, SDL_RENDERER_ACCELERATED);

	/* now 'plotting'*/
	int it;
	int currentCell;

	dataType* image = calloc(width*height, sizeof(dataType));
	int currentColor = 0, hasChanged = 0;

	for (it = 0; it < iterations; ++it) {

		hasChanged = 0;

		for (int j=0; j<height; ++j) {
			for (int k=0; k<width; ++k) {
				int idx = j * width + k;
				currentCell = dataOut[it][idx];
				int colorToDraw = getColorForRendering(currentCell);

				if (image[idx] != colorToDraw) {
					if (currentColor != colorToDraw) {
						hasChanged = 1;
						SDL_SetRenderDrawColor(Main_Renderer, colorToDraw >> 16, colorToDraw >> 8 & 0xFF, colorToDraw & 0xFF, 255);
						currentColor = colorToDraw;
					}
					SDL_RenderDrawPoint(Main_Renderer, k, j);
					image[idx] = colorToDraw;
				}
			}
		}


		SDL_RenderPresent(Main_Renderer);

		int cnt = 0;
		while(++cnt < 50) {
			SDL_Event event;
			SDL_PollEvent( &event );
			SDL_Delay(1);
		}

		if(!hasChanged) {
			printf("Completed after %d iterations.\n", it);
			break;
		}
	}

	free(image);

	SDL_DestroyRenderer(Main_Renderer);
	SDL_DestroyWindow(Main_Window);
	SDL_Quit();

}
