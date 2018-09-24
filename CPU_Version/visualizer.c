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
#define DELAY 50

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

void startVisualisation(int width, int height, int iterations, int* dataIn, dataType **dataOut, dataType **paramsOut) {

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
	int currentColor = 0, hasChanged = 0, windPower = 0, windDir = 0, windGoes = 0, cnt = 0;
	int colorToDraw = 0, idx = 0;

	//plot initial state
	for (int j=0; j<height; ++j) {
		for (int k=0; k<width; ++k) {
			colorToDraw = dataIn[j*width+k];
			SDL_SetRenderDrawColor(Main_Renderer, colorToDraw >> 16, (colorToDraw >> 8) & 0xFF, colorToDraw & 0xFF, 255);	
			SDL_RenderDrawPoint(Main_Renderer, k, j);
			image[idx] = colorToDraw;			
		}
	}

	SDL_RenderPresent(Main_Renderer);

	while(++cnt < DELAY) {
		SDL_Event event;
		SDL_PollEvent( &event );
		SDL_Delay(1);
	}

	for (it=0; it<iterations; ++it) {

		hasChanged = 0;
		for (int j=0; j<height; ++j) {
			for (int k=0; k<width; ++k) {
				int idx = j * width + k;
				currentCell = dataOut[it][idx];
				int colorToDraw = (currentCell >= 0) ? image[idx] : getColorForRendering(currentCell);

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

		if(!hasChanged) {
			printf("Completed after %d iterations.\n", it);
			break;
		}

		cnt = 0;
		while(++cnt < DELAY) {
			SDL_Event event;
			SDL_PollEvent( &event );
			SDL_Delay(1);
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

void startVisualisationFromFile(char* fileName) {
	
	//File must be opened before strtok operations are done
	FILE* fstream = fopen(fileName, "r");
	
	if(fstream == NULL) {
		printf("ERROR: file opening failed.\n");
		return;
	}

	//reading in width/height/it from file name
	char *token;
	int width, height, it;

	token = strtok(fileName, "_");
	token = strtok(NULL, "x");
	width = atoi(token);

	token = strtok(NULL, "_");
	height = atoi(token);

	token = strtok(NULL, ".");
	it = atoi(token);

	char buffer[8000000];
	char *ptr;

	char *record,*line;
	int x = -2, y = -1, numBuf;
	
	int* dataIn = calloc(width*height, sizeof(int));
	dataType ** dataOut = (dataType **) malloc(it*sizeof(dataType*));
	for (int i=0; i<it; ++i) dataOut[i] = calloc(width*height, sizeof(dataType));

	dataType ** paramsOut = (dataType **) malloc(it*sizeof(dataType*));
	for (int i=0; i<it; ++i) paramsOut[i] = calloc(2, sizeof(dataType));

	while((line = fgets(buffer, sizeof(buffer), fstream)) !=NULL && ++x < it) {
		y = -1;		
		record = strtok(line, ",");

		while(y++ < width*height && record != NULL) {
			numBuf = strtoul(record, &ptr, 10);
			if (x>=0) dataOut[x][y] = (numBuf != 0) ? numBuf : dataIn[y];
			else dataIn[y] = numBuf;
							
			record = strtok(NULL, ",");			
		}
	}

	fclose(fstream);
	printf("Results file loading successful...\n");

	char paramsFileName[1024];
	snprintf(paramsFileName, sizeof(paramsFileName), "params_%dx%d_%d.csv", width, height, it);

	//Params file
	FILE* pstream = fopen(paramsFileName, "r");
	
	if(pstream == NULL) {
		printf("WARNING: Could not open params file.\n");
	} else {
		x = -1;
		while((line = fgets(buffer, sizeof(buffer), pstream)) !=NULL && ++x < it) {
			y = -1;		
			record = strtok(line, ",");
			while(y++ < 2 && record != NULL) {
				paramsOut[x][y] = strtoul(record, &ptr, 10);	
				record = strtok(NULL, ",");		
			}
		}

		fclose(pstream);
		printf("Params file loading successful...\n");
	}	

	startVisualisation(width, height, it, dataIn, dataOut, paramsOut);

	free(dataIn);
	for (int i=0; i<it; ++i) free(dataOut[i]);
	free(dataOut);
}
