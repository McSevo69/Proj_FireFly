#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

#include "ppmIO.h"
#include "visualizer.h"
#include "colors.h"

#define HEIGHT 720
#define WIDTH 1280

typedef int dataType;

static int burning = -2;
static int maxT = 3;

//bits representing index
//4 bits: SIGN|TOPDOWN|SIGN|LEFTRIGHT
//1: minus, 0: plus
enum compass {S = 0b1100, SW = 0b1101, W = 0b0001, NW = 0b0101, N = 0b0100, NE = 0b0111, E = 0b0011, SE = 0b1111, RAND = -1};

//Rules (by S. Staudinger)
//full fire (all inner neighbors are burning) burns down quicker
//wind strength = radius (flying sparks)

dataType* getNeighbors(dataType *dataSet, dataType *neighborhood, int x, int y, int radius) {
	int neighSize = (2*radius+1)*(2*radius+1);

	int centeredIdx = (neighSize-1)/2;
	int dsIdx = y*WIDTH+x;

	neighborhood[centeredIdx] = dataSet[dsIdx]; //<-- center cell

	//cross around center cell -> 4*r
	for (int i=1; i <= radius; i++) {
		neighborhood[centeredIdx-i*(2*radius+1)] = ((y - i) < 0) ? 1 //any number higher than $burning is okay here
				: dataSet[dsIdx-i*WIDTH]; //above
		neighborhood[centeredIdx+i*(2*radius+1)] = ((y + i) >= HEIGHT) ? 1
				: dataSet[dsIdx+i*WIDTH]; //below
		neighborhood[centeredIdx-i] = ((x - i) < 0) ? 1
				: dataSet[dsIdx-i]; //left
		neighborhood[centeredIdx+i] = ((x + i) >= WIDTH) ? 1
				: dataSet[dsIdx+i]; // right
	}

	//squares around x -> 4*r^2
	for (int i=1; i <= radius; i++) {
		for (int j=1; j <= radius; j++) {

			neighborhood[centeredIdx-i*(2*radius+1)-j] = ((y - i) < 0 || (x - j) < 0) ? 1 :
				dataSet[dsIdx-i*WIDTH-j]; //above left

			neighborhood[centeredIdx-i*(2*radius+1)+j] = ((y - i) < 0 || (x + j) >= WIDTH) ? 1 :
				dataSet[dsIdx-i*WIDTH+j]; //above right

			neighborhood[centeredIdx+i*(2*radius+1)-j] = ((y + i) >= HEIGHT || (x - j) < 0) ? 1 :
				dataSet[dsIdx+i*WIDTH-j]; //below left

			neighborhood[centeredIdx+i*(2*radius+1)+j] = ((y + i) >= HEIGHT || (x + j) >= WIDTH) ? 1 :
				dataSet[dsIdx+i*WIDTH+j]; //below right
		}
	}

	return neighborhood;
}

float verifyResults(int *outVector, int *expectedVector, int size) {
	int errors = 0;
	for (int i = 0; i < size; i++) {
		if (outVector[i] != expectedVector[i]) {
			printf("%d != %d \n", outVector[i], expectedVector[i]);
			errors++;
		}
	}

	printf("elements: %d, errors: %d\n", size, errors);

	return 100.0 - ((float) errors/ (float) size)*100;
}

//csv reading taken from: https://ideone.com/l23He
const char* getfield(char* line, int num) {
	const char* tok;
	for (tok = strtok(line, ",");
			tok && *tok;
			tok = strtok(NULL, ",\n"))
	{
		if (!--num)
			return tok;
	}
	return NULL;
}

//initialization
void init(dataType *a, float normal, float dry, int burningOnes) {

	for (int i=0; i<WIDTH*HEIGHT; i++) a[i] = 0;

	int randomIdx;
	srand(time(NULL));

	int items = floor(WIDTH*HEIGHT*normal);

	// filled ones (normal)
	for (int i=0; i<items; i++) {
		randomIdx = floor(( (float) rand() / RAND_MAX) * ( (float) WIDTH*HEIGHT));
		a[randomIdx] = getTreeColor(1);
	}

	int itemsDry = floor(WIDTH*HEIGHT*dry);

	// filled ones dry
	for (int i=0; i<itemsDry; i++) {
		randomIdx = floor(( (float) rand() / RAND_MAX) * ( (float) WIDTH*HEIGHT));
		a[randomIdx] = getTreeColor(5);
	}
}

void setSomeTreesOnFire(dataType *dataIn, int size, int burningOnes) {
	int burningTrees = 0, randomIdx = 0;
	int maxTries = 10000000, x = 0;

	srand(time(NULL));

	while (burningTrees < burningOnes && x < maxTries) {
		randomIdx = floor(( (float) rand() / RAND_MAX) * ( (float) size));
		if (getInflammability(dataIn[randomIdx]) > 2) {
			//printf("Idx %d burning...\n", randomIdx);
			dataIn[randomIdx] = burning;
			burningTrees++;
		}
		x++;
	}
}

int hasBurningNeighbors(int* dataset, int x, int y, int width, int height, int radius, enum compass wind) {

	int cnt;
	int idx = y*width+x;

	cnt = (y - 1 >= 0) ? dataset[idx - WIDTH] < -1 : 0;
	cnt += (y - 1 >= 0 && x + 1 < width) ? dataset[idx - WIDTH + 1] < -1 : 0;
	cnt += (x + 1 < width) ? dataset[idx + 1] < -1 : 0;
	cnt += (y + 1 < height && x + 1 < width) ? dataset[idx + WIDTH + 1] < -1 : 0;
	cnt += (y + 1 < height) ? dataset[idx + WIDTH] < -1 : 0;
	cnt += (y + 1 < height && x - 1 >= 0) ? dataset[idx + WIDTH - 1] < -1 : 0;
	cnt += (x - 1 >= 0) ? dataset[idx - 1] < -1 : 0;
	cnt += (y - 1 >= 0 && x - 1 >= 0) ? dataset[idx - WIDTH - 1] < -1 : 0;

	if (cnt > 6) return -1; //full fire

	if (wind & 5 == 0 || radius < 2) return cnt; //->no wind

	//NEIGHBORS
	dataType *neighborhood = malloc((2*radius+1)*(2*radius+1)*sizeof(dataType));
	neighborhood = getNeighbors(dataset, neighborhood, x, y, radius);

	int moveUpDown = (int) pow(-1, (wind & 8) >> 3) * ((wind >> 2) & 1);
	int moveLeftRight = (int) pow(-1, (wind & 3) >> 1) * (wind & 1);

	int centerCell = ((2*radius+1)*(2*radius+1)-1)/2;

	if (wind % 2 == 0) { //vertical
		for (int i = 2; i <= radius; i++) cnt += neighborhood[centerCell+moveUpDown*i*(2*radius+1)] < -1;

		for (int a = 2; a <= radius; a++) {
			for (int b = 1; b < a; b++) {
				cnt += neighborhood[centerCell+moveUpDown*a*(2*radius+1)+b] < -1; //counting left/right ones
				cnt += neighborhood[centerCell+moveUpDown*a*(2*radius+1)-b] < -1;
			}
		}
	} else if (wind % 8 <= 3) { //horizontal
		for (int i = 2; i <= radius; i++) cnt += neighborhood[centerCell+moveLeftRight*i] < -1;

		for (int a = 2; a <= radius; a++) {
			for (int b = 1; b < a; b++) {
				cnt += neighborhood[centerCell+moveLeftRight*a+b*(2*radius+1)] < -1; //counting above/below ones
				cnt += neighborhood[centerCell+moveLeftRight*a-b*(2*radius+1)] < -1;
			}
		}
	} else {
		for (int i = 2; i <= radius; i++) cnt += neighborhood[centerCell+moveUpDown*i*(2*radius+1)+moveLeftRight*i] < -1; //diagonal

		int diagIdx = 0;

		for (int a = 2; a <= radius; a++) {
			for (int b = 1; b < a; b++) {
				diagIdx = centerCell+moveUpDown*a*(2*radius+1)+moveLeftRight*a;
				cnt += neighborhood[diagIdx+(-1)*moveUpDown*b*(2*radius+1)] < -1; //times (-1) as we go in the opposite direction
				cnt += neighborhood[diagIdx+(-1)*moveLeftRight*b] < -1;
			}
		}
	}

	free(neighborhood);

	return cnt;
}

int getNewCellState(int* dataset, int x, int y, int width, int height, int radius, enum compass wind) {

	int idx = y*width+x;

	if (dataset[idx] < -1) return dataset[idx] + 1;
	else {
		if (getInflammability(dataset[idx]) > 3) { //tree dry
			int burnState = hasBurningNeighbors(dataset, x, y, width, height, radius, wind);

			if (burnState == -1) return burning + 1;
			else if (burnState > 0) return burning;
			else return dataset[idx];
		} else if(getInflammability(dataset[idx]) > 0) { //tree normal
			int burnState = hasBurningNeighbors(dataset, x, y, width, height, radius, wind);

			if (burnState == -1) return burning;
			else if (burnState > 1) return burning -1;
			else return dataset[idx];
		} else return dataset[idx];
	}
}

/**
 * Transition matrix
 * 
 *
 **/
int getWindStrength(int prevWind) {
	srand(time(NULL));

	double randNr = (double) rand() / (double) RAND_MAX;

	switch(prevWind) {
		case 1: 
				if (randNr <= 0.5) {
					return 1;
				} else if (randNr <= 0.8) {
					return 2;
				} else return 3;
		case 2: 
				if (randNr <= 0.5) {
					return 1;
				} else if (randNr <= 0.8) {
					return 2;
				} else return 3;
		case 3:
				if (randNr <= 0.25) {
					return 1;
				} else if (randNr <= 0.5) {
					return 2;
				} else if (randNr <= 0.75) {
					return 3;
				} else return 4;
		case 4: 
				if (randNr <= 0.25) {
					return 2;
				} else if (randNr <= 0.5) {
					return 3;
				} else return 4;
		default: return 3;
	}
}

/**
 * 
 *
 **/
int getWindDirection(int prevWindDir) {
	srand(time(NULL));

	int randNr = (int) rand() % 11;

	switch(randNr) {
		case 0: return 0b1100;
		case 1: return 0b1101;
		case 2: return 0b0001;
		case 3: return 0b0101;
		case 4: return 0b0100;
		case 5: return 0b0111;
		case 6: return 0b0011;
		case 7: return 0b1111;
		default: return prevWindDir;
	}
}

void VectorsCPU(dataType *dataIn, dataType *dataOut, int* paramsIn, int* paramsOut, int radius, enum compass wind, int itCnt) {

	enum compass windToGo;
	int radiusToGo;

	//srand(time(NULL));

	//printf("I'm here: %d\n", dataOut[idx]);
	if (wind == -1) windToGo = (itCnt % 10 == 0) ? getWindDirection(paramsIn[0]) : paramsIn[0];
	else windToGo = wind;

	//if (radius == 0) radiusToGo = (int) rand() % (3 + 1 - 0) + 1;
	if (radius == 0) radiusToGo = (itCnt % 10 == 0) ? getWindStrength(paramsIn[1]) : paramsIn[1];
	else radiusToGo = radius;

	paramsOut[0] = windToGo;
	paramsOut[1] = radiusToGo;

	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int idx = y*WIDTH+x;
			dataOut[idx] = getNewCellState(dataIn, x, y, WIDTH, HEIGHT, radiusToGo, windToGo);
		}
	}
}

void printDataset(int *dataset, int width, int height) {
	printf("\n");
	for (int y = 1; y + 1 < height; y++) {
		for (int x = 1; x + 1 < width; x++) {
			int idx = y*width+x;
			printf("%d ", dataset[idx]);
		}
		printf("\n");
	}
}

void showHelpMessage(char *argv[]) {
	printf("=============================USAGE===================================\n");
	printf("%s [options]\n", argv[0]);
	printf("Simulation options:\n");
	printf("  -i | --iterations <INT>\tNumber of iterations to calculate\n");
	printf("  -f | --firecells <INT>\tInitial random fire cells\n");
	printf("  -r | --radius <INT>\t\tWind strength (max 4, 0=random)\n");
	printf("  -w | --wind <DIR>\t\tWind direction (N, NE, E, ..., RAND)\n");
	printf("  -t | --time <INT>\t\tFire duration (default=1)\n");
	printf("Options for random datasets:\n");
	printf("  -n | --normal <FLOAT>\t\tNormal tree ratio\n");
	printf("  -d | --dry <FLOAT>\t\tDry tree ratio\n");
	printf("General options:\n");
	printf("  -h | --help\t\t\tPrint help message\n");
	printf("  -I | --inImage <PATH>\t\tPath for input image\n");
	printf("  -B | --benchmark\t\tCompare CPU/DFE runtime results\n");
	printf("  -v | --visualize\t\tVisualize results\n");
	printf("  -X | --export\t\t\tExport results to csv\n");
	printf("=====================================================================\n");
}

int convertArgToInt(char * str) {
	if (strcmp ("--inImage", str) == 0) return 1;
	else if (strcmp ("-I", str) == 0) return 1;
	else if (strcmp ("--iterations", str) == 0) return 3;
	else if (strcmp ("-i", str) == 0) return 3;
	else if (strcmp ("--benchmark", str) == 0) return 4;
	else if (strcmp ("-B", str) == 0) return 4;
	else if (strcmp ("--time", str) == 0) return 5;
	else if (strcmp ("-t", str) == 0) return 5;
	else if (strcmp ("--firecells", str) == 0) return 6;
	else if (strcmp ("-f", str) == 0) return 6;
	else if (strcmp ("--normal", str) == 0) return 7;
	else if (strcmp ("-n", str) == 0) return 7;
	else if (strcmp ("--dry", str) == 0) return 8;
	else if (strcmp ("-d", str) == 0) return 8;
	else if (strcmp ("--radius", str) == 0) return 9;
	else if (strcmp ("-r", str) == 0) return 9;
	else if (strcmp ("--wind", str) == 0) return 10;
	else if (strcmp ("-w", str) == 0) return 10;
	else if (strcmp ("--visualize", str) == 0) return 11;
	else if (strcmp ("-v", str) == 0) return 11;
	else if (strcmp ("--export", str) == 0) return 12;
	else if (strcmp ("-X", str) == 0) return 12;
	else if (strcmp ("--help", str) == 0) return 13;
	else if (strcmp ("-h", str) == 0) return 13;
	else return 0;
}

enum compass convertWindToEnum(char * str) {
	if (strcmp ("N", str) == 0) return N;
	else if (strcmp ("NE", str) == 0) return NE;
	else if (strcmp ("E", str) == 0) return E;
	else if (strcmp ("SE", str) == 0) return SE;
	else if (strcmp ("S", str) == 0) return S;
	else if (strcmp ("SW", str) == 0) return SW;
	else if (strcmp ("W", str) == 0) return W;
	else if (strcmp ("NW", str) == 0) return NW;
	else if (strcmp ("RAND", str) == 0) return RAND;
	else return 0;
}

int main(int argc, char *argv[]) {
	//time measurement
	struct timeval begin, end;
	double timeSpentCPU = 0, timeSpent = 0;

	int it = 10, t = 0;  //t<-burn duration

	printf("=====================================================================\n");
	printf("          ### #### ##### ### #### ###  ### # ##### ###    \n");
	printf("          #   #  # #  #  #   #     #   #   # #  #  #   \n");
	printf("          ### #  # # #   ###  #    #   ### # # #   ### \n");
	printf("          #   #  # #  #  #     #   #   #   # #  #  #\n");
	printf("          #   #### #   # ### ####  #   #   # #   # ###\n");
	printf("=====================================================================\n");

	size_t benchmark = 0, inSet = 0, outSet = 0;
	float normal = 0.2, dry = 0.35;
	char *inPath, *outPath = "out.csv", *windString = "RAND";
	enum compass wind = RAND;
	int windSet = 0, burningOnes = 3;
	int radius = 1, vis = 0, exportIt = 0, showHelp = 0;

	//commandline parameter parsing
	for (int i=0; i<argc; i++) {
		switch(convertArgToInt(argv[i])) {
			case 1: inPath = argv[++i]; inSet = 1; break;
			case 3: it = atoi(argv[++i]); break;
			case 4: benchmark = 1; break;
			case 5: t = atoi(argv[++i]); break;
			case 6: burningOnes = atoi(argv[++i]); break;
			case 7: dry = atof(argv[++i]); break;
			case 8: normal = atof(argv[++i]); break;
			case 9: radius = atoi(argv[++i]); break;
			case 10: windString = argv[++i]; windSet = 1; break;
			case 11: vis = 1; break;
			case 12: exportIt = 1; break;
			case 13: showHelp = 1; break;
			default: break;
		}
	}

	if (showHelp) {
		showHelpMessage(argv);
		return 0;
	}

	if (windSet) {
		wind = convertWindToEnum(windString);
	}

	printf("=============================PARAMETERS==============================\n");
	printf("it: %d, t: %d, initial fire cells: %d, radius: %d, wind direction: %d\n", it, t, burningOnes, radius, wind);
	if (!inSet) printf("normal: %f, dry: %f\n", normal, dry);
	printf("=====================================================================\n");

	if (t > 0) burning -= (t > maxT) ? maxT : t;

	dataType * dataIn = calloc(WIDTH*HEIGHT, sizeof(dataType));
	dataType ** dataOut = malloc(it*sizeof(dataType*));
	for (int i=0; i<it; i++) dataOut[i] = calloc(WIDTH*HEIGHT, sizeof(dataType));

	int ** paramsOut = malloc(it*sizeof(int*));
	for (int i=0; i<it; i++) paramsOut[i] = calloc(2, sizeof(int));

	if (normal > 1 || normal < 0 ) {
		printf("WARNING: parameter normal %f not in range [0, 1]\nDefault (0.2) is used.\n", normal);
		normal = 0.4;
	}

	if (dry > 1 || dry < 0 ) {
		printf("WARNING: parameter dry %f not in range [0, 1]\nDefault (0.35) is used.\n", dry);
		dry = 0.35;
	}

	if (burningOnes < 1 || burningOnes >= WIDTH*HEIGHT) {
		printf("WARNING: parameter b %d not in range [1, WIDTH*HEIGHT]\nDefault (3) is used.\n", burningOnes);
		burningOnes = 3;
	}

	if (!inSet) {
		printf("No input path (param: -in) set. Input data is generated.\n");
		init(dataIn, normal, dry, burningOnes);
		setSomeTreesOnFire(dataIn, WIDTH*HEIGHT, burningOnes);
	} else {
		printf("Loading image...\n");
		int width = 0, height = 0;
		loadImage(inPath, &dataIn, &width, &height, 0);
		setSomeTreesOnFire(dataIn, WIDTH*HEIGHT, burningOnes);
	}

	printf("Running CPU...\n");
	gettimeofday(&begin, NULL);
	VectorsCPU(dataIn, dataOut[0], paramsOut[0], paramsOut[0], radius, wind, 0);
	for (int i=1; i<it; i++) VectorsCPU(dataOut[i-1], dataOut[i], paramsOut[i-1], paramsOut[i], radius, wind, i);
	gettimeofday(&end, NULL);
	timeSpentCPU += (end.tv_sec - begin.tv_sec) +
            ((end.tv_usec - begin.tv_usec)/1000000.0);
	printf("Time CPU: %lf\n", timeSpentCPU);

	if (exportIt) {
		printf("Exporting results...\n");
		FILE* results;
		FILE* paramsFile;

		char filename[1024];
		snprintf(filename, sizeof(filename), "results_%dx%d_%d.csv", WIDTH, HEIGHT, it);

		results = fopen(filename, "w");

		char paramsFileName[1024];
		snprintf(paramsFileName, sizeof(paramsFileName), "params_%dx%d_%d.csv", WIDTH, HEIGHT, it);

		paramsFile = fopen(paramsFileName, "w");

		//exporting
		for (int j = 0; j < it; j++) {

			fprintf(paramsFile, "%d,%d\n", paramsOut[j][0], paramsOut[j][1]);

			for(int i = 0; i < WIDTH*HEIGHT-1; ++i){
				fprintf(results, "%u,", dataOut[j][i]);
			}
			fprintf(results, "%u", dataOut[j][WIDTH*HEIGHT-1]);

			fprintf(results, "\n");
		}

		fclose(results);
		fclose(paramsFile);

		printf("Exporting done.\n");
	}

	if (vis) {
		printf("Visualising...\n");
		startVisualisation(WIDTH, HEIGHT, it, dataOut, paramsOut);
	}

	free(dataIn);
	for (int i=0; i<it; i++) free(dataOut[i]);
	free(dataOut);

	for (int i=0; i<it; i++) free(paramsOut[i]);
	free(paramsOut);

	printf("Goodbye!\n");

	return 0;
}
