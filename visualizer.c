#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SHAPE_SIZE 1

typedef int dataType;

void startVisualisation(int width, int height, int iterations, dataType **dataOut, dataType **paramsOut) {

	SDL_Window* Main_Window;
	SDL_Renderer* Main_Renderer;
	SDL_Surface* Loading_Surf;
	SDL_Texture* Background_Tx;
	SDL_Texture* BurntCell;
	SDL_Texture* Fire1Cell;
	SDL_Texture* Fire2Cell;
	SDL_Texture* Fire3Cell;
	SDL_Texture* Fire4Cell;
	SDL_Texture* Fire5Cell;
	SDL_Texture* TreeCellNormal;
	SDL_Texture* TreeCellDry;
	SDL_Texture* RiverCell;
	SDL_Texture* RoadCell;
	SDL_Texture* StreetCell;
	SDL_Texture* GrassCellNormal;
	SDL_Texture* GrassCellDry;

	/* Rectangles for drawing which will specify source (inside the texture)
	and target (on the screen) for rendering our textures. */
	SDL_Rect SrcR;
	SDL_Rect DestR;

	SrcR.x = 0;
	SrcR.y = 0;
	SrcR.w = SHAPE_SIZE;
	SrcR.h = SHAPE_SIZE;

	DestR.w = SHAPE_SIZE;
	DestR.h = SHAPE_SIZE;

	/* Before we can render anything, we need a window and a renderer */
	Main_Window = SDL_CreateWindow("Forest-fire model",
	SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * SHAPE_SIZE, height * SHAPE_SIZE, 0);
	Main_Renderer = SDL_CreateRenderer(Main_Window, -1, SDL_RENDERER_ACCELERATED);

	/* The loading of the background texture. Since SDL_LoadBMP() returns
	a surface, we convert it to a texture afterwards for fast accelerated
	blitting. */
	Loading_Surf = SDL_LoadBMP("resources/shadow.bmp");
	Background_Tx = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
	SDL_FreeSurface(Loading_Surf); /* we got the texture now -> free surface */

	/* Load an additional texture */
	Loading_Surf = SDL_LoadBMP("resources/burnt.bmp");
	BurntCell = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
	SDL_FreeSurface(Loading_Surf);

	/* Load an additional texture */
	Loading_Surf = SDL_LoadBMP("resources/fire1.bmp");
	Fire1Cell = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
	SDL_FreeSurface(Loading_Surf);

	/* Load an additional texture */
	Loading_Surf = SDL_LoadBMP("resources/fire2.bmp");
	Fire2Cell = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
	SDL_FreeSurface(Loading_Surf);

	/* Load an additional texture */
	Loading_Surf = SDL_LoadBMP("resources/fire3.bmp");
	Fire3Cell = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
	SDL_FreeSurface(Loading_Surf);

	/* Load an additional texture */
	Loading_Surf = SDL_LoadBMP("resources/fire4.bmp");
	Fire4Cell = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
	SDL_FreeSurface(Loading_Surf);

	/* Load an additional texture */
	Loading_Surf = SDL_LoadBMP("resources/fire5.bmp");
	Fire5Cell = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
	SDL_FreeSurface(Loading_Surf);

	/* Load an additional texture */
	Loading_Surf = SDL_LoadBMP("resources/tree_normal.bmp");
	TreeCellNormal = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
	SDL_FreeSurface(Loading_Surf);

	/* Load an additional texture */
	Loading_Surf = SDL_LoadBMP("resources/tree_dry.bmp");
	TreeCellDry = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
	SDL_FreeSurface(Loading_Surf);

	/* Load an additional texture */
	Loading_Surf = SDL_LoadBMP("resources/river.bmp");
	RiverCell = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
	SDL_FreeSurface(Loading_Surf);

	/* Load an additional texture */
	Loading_Surf = SDL_LoadBMP("resources/road_nature.bmp");
	RoadCell = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
	SDL_FreeSurface(Loading_Surf);

	/* Load an additional texture */
	Loading_Surf = SDL_LoadBMP("resources/street.bmp");
	StreetCell = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
	SDL_FreeSurface(Loading_Surf);

	/* Load an additional texture */
	Loading_Surf = SDL_LoadBMP("resources/grass_dry.bmp");
	GrassCellDry = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
	SDL_FreeSurface(Loading_Surf);

	/* Load an additional texture */
	Loading_Surf = SDL_LoadBMP("resources/grass.bmp");
	GrassCellNormal = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
	SDL_FreeSurface(Loading_Surf);

	TTF_Init();

	//Font part taken from https://stackoverflow.com/questions/22886500/how-to-render-text-in-sdl2
	TTF_Font* Sans = TTF_OpenFont("resources/arial.ttf", 24); //this opens a font style and sets a size

	SDL_Color White = {255, 255, 255};  // this is the color in rgb format, maxing out all would give you the color white, and it will be your text's color

	SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, "Dummy", White); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first

	SDL_Texture* Message = SDL_CreateTextureFromSurface(Main_Renderer, surfaceMessage); //now you can convert it into a texture

	SDL_Rect Message_rect; //create a rect
	Message_rect.x = width-100;  //controls the rect's x coordinate 
	Message_rect.y = 0; // controls the rect's y coordinte
	Message_rect.w = 100; // controls the width of the rect
	Message_rect.h = 100; // controls the height of the rect

	/* now 'plotting'*/
	int it;
	int currentCell;

	for (it = 0; it < iterations; ++it) {

		/* render background, whereas NULL for source and destination
		rectangles just means "use the default" */
		SDL_RenderCopy(Main_Renderer, Background_Tx, NULL, NULL);
		

		/* render the current animation step of our shape */
		//SDL_RenderCopy(Main_Renderer, BlueShapes, &SrcR, &DestR);

		for (int j=0; j<height; j++) {
			for (int k=0; k<width; k++) {
				currentCell = dataOut[it][j*width+k];
				if (currentCell > 12) {
					DestR.x = SHAPE_SIZE*2*(k) / 2;
					DestR.y = SHAPE_SIZE*2*(j) / 2;
					SDL_UpdateTexture(Fire5Cell, NULL, NULL, Loading_Surf->pitch);
					SDL_RenderCopy(Main_Renderer, Fire5Cell, &SrcR, &DestR);
				} else if (currentCell == 12) {
					DestR.x = SHAPE_SIZE*2*(k) / 2;
					DestR.y = SHAPE_SIZE*2*(j) / 2;
					SDL_UpdateTexture(Fire4Cell, NULL, NULL, Loading_Surf->pitch);
					SDL_RenderCopy(Main_Renderer, Fire4Cell, &SrcR, &DestR);
				}else if (currentCell == 11) {
					DestR.x = SHAPE_SIZE*2*(k) / 2;
					DestR.y = SHAPE_SIZE*2*(j) / 2;
					SDL_UpdateTexture(Fire3Cell, NULL, NULL, Loading_Surf->pitch);
					SDL_RenderCopy(Main_Renderer, Fire3Cell, &SrcR, &DestR);
				}else if (currentCell == 10) {
					DestR.x = SHAPE_SIZE*2*(k) / 2;
					DestR.y = SHAPE_SIZE*2*(j) / 2;
					SDL_UpdateTexture(Fire2Cell, NULL, NULL, Loading_Surf->pitch);
					SDL_RenderCopy(Main_Renderer, Fire2Cell, &SrcR, &DestR);
				}else if (currentCell == 9) {
					DestR.x = SHAPE_SIZE*2*(k) / 2;
					DestR.y = SHAPE_SIZE*2*(j) / 2;
					SDL_UpdateTexture(Fire1Cell, NULL, NULL, Loading_Surf->pitch);
					SDL_RenderCopy(Main_Renderer, Fire1Cell, &SrcR, &DestR);
				}else if (currentCell == 8) {
					DestR.x = SHAPE_SIZE*2*(k) / 2;
					DestR.y = SHAPE_SIZE*2*(j) / 2;
					SDL_UpdateTexture(BurntCell, NULL, NULL, Loading_Surf->pitch);
					SDL_RenderCopy(Main_Renderer, BurntCell, &SrcR, &DestR);
				} else if (currentCell == 7) {
					DestR.x = SHAPE_SIZE*2*(k) / 2;
					DestR.y = SHAPE_SIZE*2*(j) / 2;
					SDL_UpdateTexture(GrassCellNormal, NULL, NULL, Loading_Surf->pitch);
					SDL_RenderCopy(Main_Renderer, GrassCellNormal, &SrcR, &DestR);
				} else if (currentCell == 6) {
					DestR.x = SHAPE_SIZE*2*(k) / 2;
					DestR.y = SHAPE_SIZE*2*(j) / 2;
					SDL_UpdateTexture(GrassCellDry, NULL, NULL, Loading_Surf->pitch);
					SDL_RenderCopy(Main_Renderer, GrassCellDry, &SrcR, &DestR);
				} else if (currentCell == 5) {
					DestR.x = SHAPE_SIZE*2*(k) / 2;
					DestR.y = SHAPE_SIZE*2*(j) / 2;
					SDL_UpdateTexture(StreetCell, NULL, NULL, Loading_Surf->pitch);
					SDL_RenderCopy(Main_Renderer, StreetCell, &SrcR, &DestR);
				} else if (currentCell == 4) {
					DestR.x = SHAPE_SIZE*2*(k) / 2;
					DestR.y = SHAPE_SIZE*2*(j) / 2;
					SDL_UpdateTexture(RoadCell, NULL, NULL, Loading_Surf->pitch);
					SDL_RenderCopy(Main_Renderer, RoadCell, &SrcR, &DestR);
				} else if (currentCell == 3) {
					DestR.x = SHAPE_SIZE*2*(k) / 2;
					DestR.y = SHAPE_SIZE*2*(j) / 2;
					SDL_UpdateTexture(RiverCell, NULL, NULL, Loading_Surf->pitch);
					SDL_RenderCopy(Main_Renderer, RiverCell, &SrcR, &DestR);
				} else if (currentCell == 2) {
					DestR.x = SHAPE_SIZE*2*(k) / 2;
					DestR.y = SHAPE_SIZE*2*(j) / 2;
					SDL_UpdateTexture(TreeCellDry, NULL, NULL, Loading_Surf->pitch);
					SDL_RenderCopy(Main_Renderer, TreeCellDry, &SrcR, &DestR);
				} else if (currentCell == 1) {
					DestR.x = SHAPE_SIZE*2*(k) / 2;
					DestR.y = SHAPE_SIZE*2*(j) / 2;
					SDL_UpdateTexture(TreeCellNormal, NULL, NULL, Loading_Surf->pitch);
					SDL_RenderCopy(Main_Renderer, TreeCellNormal, &SrcR, &DestR);
				} else {
					DestR.x = SHAPE_SIZE*2*(k) / 2;
					DestR.y = SHAPE_SIZE*2*(j) / 2;
					SDL_UpdateTexture(Background_Tx, NULL, NULL, Loading_Surf->pitch);
					SDL_RenderCopy(Main_Renderer, Background_Tx, &SrcR, &DestR);
				}
			}
		}

		char *msg = "";

		switch(paramsOut[it][0]) {
			case 0: msg = "No wind"; break;
			case 1: msg = "W"; break;
			case 2: msg = "No wind"; break;
			case 3: msg = "E"; break;
			case 4: msg = "N"; break;
			case 5: msg = "NW"; break;
			case 6: msg = "N"; break;
			case 7: msg = "NE"; break;
			case 8: msg = "No wind"; break;
			case 9: msg = "W"; break;
			case 10: msg = "No wind"; break;
			case 11: msg = "E"; break;
			case 12: msg = "S"; break;
			case 13: msg = "SW"; break;
			case 14: msg = "S"; break;
			case 15: msg = "SE"; break;
			default: break;
		}		

		surfaceMessage = TTF_RenderText_Solid(Sans, msg, White); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first

		Message = SDL_CreateTextureFromSurface(Main_Renderer, surfaceMessage); //now you can convert it into a texture

		SDL_RenderCopy(Main_Renderer, Message, NULL, &Message_rect);
		SDL_RenderPresent(Main_Renderer);

		int cnt = 0;
		while(++cnt < 5) {
			SDL_Event event;
			SDL_PollEvent( &event );
			SDL_Delay(1);
		}
	}

	SDL_DestroyTexture(Fire1Cell);
	SDL_DestroyTexture(Fire2Cell);
	SDL_DestroyTexture(Fire3Cell);
	SDL_DestroyTexture(Fire4Cell);
	SDL_DestroyTexture(Fire5Cell);
	SDL_DestroyTexture(BurntCell);
	SDL_DestroyTexture(TreeCellNormal);
	SDL_DestroyTexture(TreeCellDry);
	SDL_DestroyTexture(RoadCell);
	SDL_DestroyTexture(StreetCell);
	SDL_DestroyTexture(GrassCellNormal);
	SDL_DestroyTexture(GrassCellDry);
	SDL_DestroyTexture(Background_Tx);
	SDL_DestroyRenderer(Main_Renderer);
	SDL_DestroyWindow(Main_Window);
	SDL_Quit();

}