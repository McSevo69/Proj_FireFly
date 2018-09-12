#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "colors.h"
#include "types.h"

#define SHAPE_SIZE 1
#define N_PATH "resources/north.png"
#define NE_PATH "resources/ne.png"
#define E_PATH "resources/east.png"
#define SE_PATH "resources/se.png"
#define S_PATH "resources/south.png"
#define SW_PATH "resources/sw.png"
#define W_PATH "resources/west.png"
#define NW_PATH "resources/nw.png"
#define NOWIND_PATH "resources/nowind.png"
#define LIGHTWIND_PATH "resources/lightwind.png"
#define MEDIUMWIND_PATH "resources/mediumwind.png"
#define STRONGWIND_PATH "resources/strongwind.png"

char* getWindDirPath(int windCode) {
	switch(windCode) {
		case 4: return N_PATH;
		case 6: return N_PATH;
		case 7: return NE_PATH;
		case 3: return E_PATH;
		case 11: return E_PATH;
		case 15: return SE_PATH;
		case 12: return S_PATH;
		case 14: return S_PATH;
		case 13: return SW_PATH;
		case 1: return W_PATH;
		case 9: return W_PATH;
		case 5: return NW_PATH;
		default: return NOWIND_PATH;
	}
}

char* getWindPowerPath(int windPowerCode) {
	switch(windPowerCode) {
		case 2: return LIGHTWIND_PATH;
		case 3: return MEDIUMWIND_PATH;
		case 4: return STRONGWIND_PATH;
		default: return NOWIND_PATH;
	}
}

void startVisualisation(int width, int height, int iterations, dataType **dataOut, dataType **paramsOut) {

	SDL_Window* Main_Window;
	SDL_Renderer* Main_Renderer;
	
	SDL_Texture *windImg = NULL;
	SDL_Rect texr; texr.x = width-100; texr.y = 20; texr.w = 80; texr.h = 80; 

	/* Before we can render anything, we need a window and a renderer */
	Main_Window = SDL_CreateWindow("Forest-fire model",
	SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * SHAPE_SIZE, height * SHAPE_SIZE, 0);
	Main_Renderer = SDL_CreateRenderer(Main_Window, -1, SDL_RENDERER_ACCELERATED);	

	SDL_RenderClear(Main_Renderer);

	/* now 'plotting'*/
	int it;
	int currentCell;

	dataType* image = calloc(width*height, sizeof(dataType));
	int currentColor = 0, hasChanged = 0, windPower = 0, windDir = 0, windGoes = 0;

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

		windPower = paramsOut[it][1];
		windDir = paramsOut[it][0];
		windGoes = windDir & 5;

		if (windGoes) {
			windImg = IMG_LoadTexture(Main_Renderer, getWindPowerPath(windPower));
			SDL_RenderCopy(Main_Renderer, windImg, NULL, &texr);
		} else {
			windImg = IMG_LoadTexture(Main_Renderer, NOWIND_PATH);
			SDL_RenderCopy(Main_Renderer, windImg, NULL, &texr);
		}		

		if (windPower > 1 && windGoes) {
			windImg = IMG_LoadTexture(Main_Renderer, getWindDirPath(windDir));
			SDL_RenderCopy(Main_Renderer, windImg, NULL, &texr);
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

	char ch;
	printf("Press ENTER for closing...");

	SDL_RenderClear(Main_Renderer);

	while(1) {
		ch = fgetc(stdin);
		if(ch==0x0A) break;	
	};

	free(image);

	SDL_DestroyTexture(windImg);
	SDL_DestroyRenderer(Main_Renderer);
	SDL_DestroyWindow(Main_Window);
	SDL_Quit();

}
