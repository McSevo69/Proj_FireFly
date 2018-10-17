#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <stdbool.h>
#include <limits.h>
#include "Maxfiles.h"
#include <MaxSLiCInterface.h>

#include "ppmIO.h"
#include "colors.h"
#include "types.h"

static dataType burning = -2;
static int maxT = 3;
static int maxWind = 4;

//bits representing index
//4 bits: SIGN|TOPDOWN|SIGN|LEFTRIGHT
//1: minus, 0: plus
enum compass {S = 0b1100, SW = 0b1101, W = 0b0001, NW = 0b0101, N = 0b0100, NE = 0b0111, E = 0b0011, SE = 0b1111, RAND = -1};

//Rules (by S. Staudinger)
//full fire (all inner neighbors are burning) burns down quicker
//wind strength = radius (flying sparks)

double checkAccuracy(dataType** results, dataType** expected, int itRes, int itExp) {
	int errors = 0;

	int maxIt = (itRes >= itExp) ? itRes : itExp;
	int expItIdx, resItIdx;

	for (int i=0; i < maxIt; ++i) {
		expItIdx = (i < itExp) ? i : itExp - 1;
		resItIdx = (i < itRes) ? i : itRes - 1;
		for (int j=0; j < Vectors_width*Vectors_height; ++j) {
			if (results[resItIdx][j] != expected[expItIdx][j])
				++errors;
		}
	}

	return 1 - ( (double) (errors/maxIt) / (double) (Vectors_width*Vectors_height) );
}

dataType* getNeighbors(dataType *dataSet, dataType *neighborhood, int x, int y, int radius) {
	int neighSize = (2*radius+1)*(2*radius+1);

	int centeredIdx = (neighSize-1)/2;
	int dsIdx = y*Vectors_width+x;

	neighborhood[centeredIdx] = dataSet[dsIdx]; //<-- center cell

	//cross around center cell -> 4*r
	for (int i=1; i <= radius; ++i) {
		neighborhood[centeredIdx-i*(2*radius+1)] = ((y - i) < 0) ? 1 //any number higher than $burning is okay here
				: dataSet[dsIdx-i*Vectors_width]; //above
		neighborhood[centeredIdx+i*(2*radius+1)] = ((y + i) >= Vectors_height) ? 1
				: dataSet[dsIdx+i*Vectors_width]; //below
		neighborhood[centeredIdx-i] = ((x - i) < 0) ? 1
				: dataSet[dsIdx-i]; //left
		neighborhood[centeredIdx+i] = ((x + i) >= Vectors_width) ? 1
				: dataSet[dsIdx+i]; // right
	}

	//squares around x -> 4*r^2
	for (int i = 1; i <= radius; ++i) {
		for (int j = 1; j <= radius; ++j) {
			neighborhood[centeredIdx-i*(2*radius+1)-j] = ((y - i) < 0 || (x - j) < 0) ? 1 :
				dataSet[dsIdx-i*Vectors_width-j]; //above left

			neighborhood[centeredIdx-i*(2*radius+1)+j] = ((y - i) < 0 || (x + j) >= Vectors_width) ? 1 :
				dataSet[dsIdx-i*Vectors_width+j]; //above right

			neighborhood[centeredIdx+i*(2*radius+1)-j] = ((y + i) >= Vectors_height || (x - j) < 0) ? 1 :
				dataSet[dsIdx+i*Vectors_width-j]; //below left

			neighborhood[centeredIdx+i*(2*radius+1)+j] = ((y + i) >= Vectors_height || (x + j) >= Vectors_width) ? 1 :
				dataSet[dsIdx+i*Vectors_width+j]; //below right
		}
	}

	return neighborhood;
}

//initialization
void init(int *a, float normal, float dry) {

	for (int i=0; i<Vectors_width*Vectors_height; ++i) a[i] = 0;

	int randomIdx;
	srand(time(NULL));

	int items = floor(Vectors_width*Vectors_height*normal);

	// filled ones (normal)
	for (int i=0; i<items; ++i) {
		randomIdx = floor(( (float) rand() / RAND_MAX) * ( (float) Vectors_width*Vectors_height));
		a[randomIdx] = getTreeColor(2);
	}

	int itemsDry = floor(Vectors_width*Vectors_height*dry);

	// filled ones dry
	for (int i=0; i<itemsDry; ++i) {
		randomIdx = floor(( (float) rand() / RAND_MAX) * ( (float) Vectors_width*Vectors_height));
		a[randomIdx] = getTreeColor(5);
	}
}

void makeItRealistic(int *dataIn, int width, int height, float rate) {

	int randomIdx, normalisedOnes = 0, maxTries = 10000000, x = 0;
	srand(time(NULL));

	int items = floor(width*height*rate);

	while (normalisedOnes < items && x < maxTries) {
		randomIdx = floor(( (float) rand() / RAND_MAX) * ( (float) height*width));
		if (dataIn[randomIdx] > 0) {
			dataIn[randomIdx] = 0x052200; //dark green
			normalisedOnes++;
		}
		x++;
	}
}

void setSomeTreesOnFire(dataType *dataIn, int size, int burningOnes) {
	int burningTrees = 0, randomIdx = 0;
	int maxTries = 10000000, x = 0;

	srand(time(NULL));

	while (burningTrees < burningOnes && x < maxTries) {
		randomIdx = floor(( (float) rand() / RAND_MAX) * ( (float) size));
		if (dataIn[randomIdx] > 2) {
			dataIn[randomIdx] = burning;
			burningTrees++;
		}
		x++;
	}
}

int hasBurningNeighbors(dataType* dataset, int x, int y, int width, int height, int radius, enum compass wind) {

	int cnt;
	int idx = y*width+x;

	cnt = (y - 1 >= 0) ? dataset[idx - Vectors_width] < -1 : 0;
	cnt += (y - 1 >= 0 && x + 1 < width) ? dataset[idx - Vectors_width + 1] < -1 : 0;
	cnt += (x + 1 < width) ? dataset[idx + 1] < -1 : 0;
	cnt += (y + 1 < height && x + 1 < width) ? dataset[idx + Vectors_width + 1] < -1 : 0;
	cnt += (y + 1 < height) ? dataset[idx + Vectors_width] < -1 : 0;
	cnt += (y + 1 < height && x - 1 >= 0) ? dataset[idx + Vectors_width - 1] < -1 : 0;
	cnt += (x - 1 >= 0) ? dataset[idx - 1] < -1 : 0;
	cnt += (y - 1 >= 0 && x - 1 >= 0) ? dataset[idx - Vectors_width - 1] < -1 : 0;

	if (cnt > 6) return -1; //full fire

	if ((wind & 5) == 0 || radius < 2 || cnt) return cnt; //->no wind or burning neighbor found

	//NEIGHBORS
	dataType *neighborhood = malloc((2*radius+1)*(2*radius+1)*sizeof(dataType));
	neighborhood = getNeighbors(dataset, neighborhood, x, y, radius);

	int moveUpDown = (int) pow(-1, (wind & 8) >> 3) * ((wind >> 2) & 1);
	int moveLeftRight = (int) pow(-1, (wind & 3) >> 1) * (wind & 1);

	int centerCell = ((2*radius+1)*(2*radius+1)-1)/2;

	if (wind % 2 == 0) { //vertical
		for (int i = 2; i <= radius; i++) cnt += neighborhood[centerCell+moveUpDown*i*(2*radius+1)] < -1;

		for (int a = 2; a <= radius; ++a) {
			for (int b = 1; b < a; ++b) {
				cnt += neighborhood[centerCell+moveUpDown*a*(2*radius+1)+b] < -1; //counting left/right ones
				cnt += neighborhood[centerCell+moveUpDown*a*(2*radius+1)-b] < -1;
			}
		}
	} else if (wind % 8 <= 3) { //horizontal
		for (int i = 2; i <= radius; ++i) cnt += neighborhood[centerCell+moveLeftRight*i] < -1;

		for (int a = 2; a <= radius; ++a) {
			for (int b = 1; b < a; ++b) {
				cnt += neighborhood[centerCell+moveLeftRight*a+b*(2*radius+1)] < -1; //counting above/below ones
				cnt += neighborhood[centerCell+moveLeftRight*a-b*(2*radius+1)] < -1;
			}
		}
	} else {
		for (int i = 2; i <= radius; ++i) cnt += neighborhood[centerCell+moveUpDown*i*(2*radius+1)+moveLeftRight*i] < -1; //diagonal

		int diagIdx = 0;

		for (int a = 2; a <= radius; ++a) {
			for (int b = 1; b < a; ++b) {
				diagIdx = centerCell+moveUpDown*a*(2*radius+1)+moveLeftRight*a;
				cnt += neighborhood[diagIdx+(-1)*moveUpDown*b*(2*radius+1)] < -1; //times (-1) as we go in the opposite direction
				cnt += neighborhood[diagIdx+(-1)*moveLeftRight*b] < -1;
			}
		}
	}

	free(neighborhood);

	return cnt;
}

int getNewCellState(dataType* dataset, int x, int y, int width, int height, int radius, enum compass wind) {

	int idx = y*width+x;

	if (dataset[idx] > 2) { //tree dry
		int burnState = hasBurningNeighbors(dataset, x, y, width, height, radius, wind);

		if (burnState == -1) return burning + 1;
		else if (burnState > 0) return burning + 1;
		else return dataset[idx];
	} else if(dataset[idx] > 0) { //tree normal
		int burnState = hasBurningNeighbors(dataset, x, y, width, height, radius, wind);

		if (burnState == -1) return burning + 1;
		else if (burnState > 1) return burning -1;
		else return dataset[idx];
	} else return (dataset[idx] < -1) ? dataset[idx] + 1 : dataset[idx];

}

void transformInputImage(int* datasetIn, dataType* datasetOut, int width, int height) {
	for (int i=0; i<width*height; ++i)
		datasetOut[i] = getInflammability(datasetIn[i]);
}

/**
 * Transition matrix could be added
 *
 *
 **/
int getWindStrength(int prevWind, int itCnt, int windChangeIntervall) {

	if (itCnt % windChangeIntervall) {
		return (prevWind == 0) ? 0 : prevWind;
	} else {
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
}

/**
 *
 *
 **/
int getWindDirection(int prevWindDir, int itCnt, int windChangeIntervall) {

	if (itCnt % windChangeIntervall) {
		return (prevWindDir == -1) ? 0 : prevWindDir;
	} else {
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
}

void initParams(dataType** params, char* file, int iterations) {
	char buffer[8000000];
	char *ptr;

	char *record,*line;
	int x = -1, y = -1;

	FILE* pstream = fopen(file, "r");

	if(pstream == NULL) {
		printf("WARNING: Could not open params file.\n");
		return;
	}

	x = -1;
	while((line = fgets(buffer, sizeof(buffer), pstream)) !=NULL && ++x < iterations) {
		y = -1;
		record = strtok(line, ",");
		while(y++ < 2 && record != NULL) {
			params[x][y] = strtoul(record, &ptr, 10);
			record = strtok(NULL, ",");
		}
	}

	fclose(pstream);
	printf("Params file loading successful...\n");
}

void manageParams(dataType* paramsIn, dataType* paramsOut, int windStrength,
		enum compass windDir, int windChangeIntervall, int itCnt) {

	enum compass wind = (windDir == -1) ? getWindDirection(paramsIn[0], itCnt, windChangeIntervall) : windDir;

	int radius = (windStrength == 0) ? getWindStrength(paramsIn[1], itCnt, windChangeIntervall) : windStrength; //paramsIn are the previous settings

	paramsOut[0] = wind;
	paramsOut[1] = radius;

}

void VectorsCPU(dataType *dataIn, dataType *dataOut, dataType* paramsIn, int *minValue) {
	*minValue = INT_MAX;
	for (int y = 0; y < Vectors_height; ++y) {
		for (int x = 0; x < Vectors_width; ++x) {
			int idx = y*Vectors_width+x;
			dataOut[idx] = getNewCellState(dataIn, x, y, Vectors_width, Vectors_height, paramsIn[1], paramsIn[0]);
			if (dataOut[idx] < *minValue) *minValue = dataOut[idx];
		}
	}
}

void printDataset(int *dataset, int width, int height) {
	printf("\n");
	for (int y = 1; y + 1 < height; ++y) {
		for (int x = 1; x + 1 < width; ++x) {
			int idx = y*width+x;
			printf("%d ", dataset[idx]);
		}
		printf("\n");
	}
}

void showHelpMessage(char *argv[]) {
	printf("==============================USAGE==================================\n");
	printf("%s [options]\n", argv[0]);
	printf("Simulation options:\n");
	printf("  -i | --iterations <INT>\tNumber of iterations to calculate\n");
	printf("  -f | --firecells <INT>\tInitial random fire cells\n");
	printf("  -r | --radius <INT>\t\tWind strength (max: %d, 0=random)\n", maxWind);
	printf("  -w | --wind <DIR>\t\tWind direction (N, NE, E, ..., RAND)\n");
	printf("  -c | --changetime <INT>\tWind changing intervall (default: 10)\n");
	printf("  -t | --time <INT>\t\tAdditional fire duration (max: %d)\n", maxT);
	printf("  -n | --noise <FLOAT>\t\tNoise ratio for input images.\n");
	printf("Options for random datasets:\n");
	printf("  -s | --standard <FLOAT>\tStandard tree ratio\n");
	printf("  -d | --dry <FLOAT>\t\tDry tree ratio\n");
	printf("General options:\n");
	printf("  -h | --help\t\t\tPrint help message\n");
	printf("  -I | --inImage <PATH>\t\tPath for input image\n");
	printf("  -P | --params <PATH>\t\tPath for input params file\n");
	printf("  -B | --benchmark\t\tCompare DFE/CPU results\n");
	printf("  -X | --export\t\t\tExport results to csv\n");
	printf("=====================================================================\n");
}

int convertArgToInt(char * str) {
	if (strcmp ("--inImage", str) == 0) return 1;
	else if (strcmp ("-I", str) == 0) return 1;
	else if (strcmp ("--noise", str) == 0) return 2;
	else if (strcmp ("-n", str) == 0) return 2;
	else if (strcmp ("--iterations", str) == 0) return 3;
	else if (strcmp ("-i", str) == 0) return 3;
	else if (strcmp ("--time", str) == 0) return 5;
	else if (strcmp ("-t", str) == 0) return 5;
	else if (strcmp ("--firecells", str) == 0) return 6;
	else if (strcmp ("-f", str) == 0) return 6;
	else if (strcmp ("--dry", str) == 0) return 7;
	else if (strcmp ("-d", str) == 0) return 7;
	else if (strcmp ("--standard", str) == 0) return 8;
	else if (strcmp ("-s", str) == 0) return 8;
	else if (strcmp ("--radius", str) == 0) return 9;
	else if (strcmp ("-r", str) == 0) return 9;
	else if (strcmp ("--wind", str) == 0) return 10;
	else if (strcmp ("-w", str) == 0) return 10;
	else if (strcmp ("--benchmark", str) == 0) return 11;
	else if (strcmp ("-B", str) == 0) return 11;
	else if (strcmp ("--export", str) == 0) return 12;
	else if (strcmp ("-X", str) == 0) return 12;
	else if (strcmp ("--help", str) == 0) return 13;
	else if (strcmp ("-h", str) == 0) return 13;
	else if (strcmp ("--params", str) == 0) return 14;
	else if (strcmp ("-P", str) == 0) return 14;
	else if (strcmp ("--changetime", str) == 0) return 15;
	else if (strcmp ("-c", str) == 0) return 15;
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
	printf("            ### #### ####  ### #### ###   ### # ####  ###            \n");
	printf("            #   #  # #  #  #   #     #    #   # #  #  #              \n");
	printf("            ### #  # # #   ###  #    #    ### # # #   ###            \n");
	printf("            #   #  # #  #  #     #   #    #   # #  #  #              \n");
	printf("            #   #### #   # ### ####  #    #   # #   # ###            \n");
	printf("=====================================================================\n");

	float normal = 0.2, dry = 0.35, noiseRatio = 0;
	char *inPath = "in.ppm", *windString = "RAND", *paramsInPath = "params.csv";
	enum compass wind = RAND;
	size_t burningOnes = 3, radius = 1, windChangeIntervall = 10;
	bool inSet = 0, paramsGiven = 0, windSet = 0,
		noiseDesired = 0, exportIt = 0, showHelp = 0, benchmarkIt = 0;

	//commandline parameter parsing
	for (int i=0; i<argc; ++i) {
		switch(convertArgToInt(argv[i])) {
			case 1: inPath = argv[++i]; inSet = 1; break;
			case 2: noiseDesired = 1; noiseRatio = atof(argv[++i]); break;
			case 3: it = atoi(argv[++i]); break;
			case 5: t = atoi(argv[++i]); break;
			case 6: burningOnes = atoi(argv[++i]); break;
			case 7: dry = atof(argv[++i]); break;
			case 8: normal = atof(argv[++i]); break;
			case 9: radius = atoi(argv[++i]); break;
			case 10: windString = argv[++i]; windSet = 1; break;
			case 11: benchmarkIt = 1; break;
			case 12: exportIt = 1; break;
			case 13: showHelp = 1; break;
			case 14: paramsInPath = argv[++i]; paramsGiven = 1; break;
			case 15: windChangeIntervall = atoi(argv[++i]); break;
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
	printf("iterations: %d, additional burn time: %d, initial fire cells: %ld\nradius: %ld, wind direction: %d, wind change intervall: %zu\n",
		it, t, burningOnes, radius, wind, windChangeIntervall);
	if (!inSet) printf("normal ratio: %f, dry ratio: %f\n", normal, dry);
	printf("=====================================================================\n");

	if (t > 0) burning -= (t > maxT) ? maxT : t;

	int * dataIn = calloc(Vectors_width*Vectors_height, sizeof(int));
	dataType * dataBuffer = calloc(Vectors_width*Vectors_height, sizeof(dataType));
	dataType ** dataOutDFE = malloc(it*sizeof(dataType*));
	for (int i=0; i<it; ++i) dataOutDFE[i] = calloc(Vectors_width*Vectors_height, sizeof(dataType));

	dataType ** paramsOut = malloc(it*sizeof(dataType*));
	for (int i=0; i<it; ++i) paramsOut[i] = calloc(2, sizeof(dataType));

	if (paramsGiven) initParams(paramsOut, paramsInPath, it);

	if (normal > 1 || normal < 0 ) {
		printf("WARNING: parameter -s/--standard %f not in range [0, 1]\nDefault (0.2) is used.\n", normal);
		normal = 0.4;
	}

	if (dry > 1 || dry < 0 ) {
		printf("WARNING: parameter -d/--dry %f not in range [0, 1]\nDefault (0.35) is used.\n", dry);
		dry = 0.35;
	}

	if (burningOnes >= Vectors_width*Vectors_height) {
		printf("WARNING: parameter -f/--firecells %ld not in range [0, Vectors_width*Vectors_height]\nDefault (3) is used.\n", burningOnes);
		burningOnes = 3;
	}

	if (noiseRatio < 0 || noiseRatio > 1) {
		printf("WARNING: parameter -n/--noise %f not in range [0, 1]\nDefault (0.0) is used.\n", noiseRatio);
		burningOnes = 3;
	}

	if (!inSet) {
		printf("WARNING: parameter -I/--inImage not set. Input data is generated.\n");
		init(dataIn, normal, dry);
		transformInputImage(dataIn, dataBuffer, Vectors_width, Vectors_height);
	} else {
		printf("Loading image...\n");
		int width = 0, height = 0;
		loadImage(inPath, &dataIn, &width, &height, 0);
		if (noiseDesired) makeItRealistic(dataIn, width, height, noiseRatio);
		transformInputImage(dataIn, dataBuffer, width, height);
	}

	if (burningOnes > 0) setSomeTreesOnFire(dataBuffer, Vectors_width*Vectors_height, burningOnes);

	printf("Running DFE...\n");
	int16_t * minValues = calloc(8, sizeof(int16_t));
	int x = 0;
	int elements = Vectors_width*Vectors_height;
	bool convergedDFE = false;
	int overlap = (Vectors_maxRadius + 1) * Vectors_width;
	gettimeofday(&begin, NULL);
	if (!paramsGiven) manageParams(paramsOut[0], paramsOut[0], radius, wind, windChangeIntervall, 0);
	
	Vectors(burning, elements, paramsOut[0][0], paramsOut[0][1],
		dataBuffer, &dataBuffer[elements/6-overlap], &dataBuffer[2*elements/6-overlap], 
		&dataBuffer[3*elements/6-overlap], &dataBuffer[4*elements/6-overlap], &dataBuffer[5*elements/6-overlap],
		minValues, dataOutDFE[0], &dataOutDFE[0][elements/6], &dataOutDFE[0][2*elements/6],
		&dataOutDFE[0][3*elements/6], &dataOutDFE[0][4*elements/6], &dataOutDFE[0][5*elements/6]);

	while (++x < it && !convergedDFE) {
		if (!paramsGiven) manageParams(paramsOut[x-1], paramsOut[x], radius, wind, windChangeIntervall, x);

		Vectors(burning, elements, paramsOut[x][0], paramsOut[x][1],
			dataOutDFE[x-1], &dataOutDFE[x-1][elements/6-overlap], &dataOutDFE[x-1][2*elements/6-overlap],
			&dataOutDFE[x-1][3*elements/6-overlap], &dataOutDFE[x-1][4*elements/6-overlap], &dataOutDFE[x-1][5*elements/6-overlap],
			minValues, dataOutDFE[x], &dataOutDFE[x][elements/6], &dataOutDFE[x][2*elements/6],
			&dataOutDFE[x][3*elements/6], &dataOutDFE[x][4*elements/6], &dataOutDFE[x][5*elements/6]);

		if (minValues[0] >= -1 && minValues[1] >= -1 && minValues[2] >= -1 && minValues[3] >= -1 && minValues[4] >= -1 && minValues[5] >= -1) 
			convergedDFE = true;
	}
	gettimeofday(&end, NULL);

	free(minValues);

	timeSpent += (end.tv_sec - begin.tv_sec) +
            ((end.tv_usec - begin.tv_usec)/1000000.0);
	printf("Time DFE: %lf\n", timeSpent);

	if (convergedDFE) printf("Converged after %d iterations\n", x);
	else printf("Hasn't converged after %d iterations\n", x);

	if (benchmarkIt) {
		dataType ** dataOut = malloc(it*sizeof(dataType*));
		for (int i=0; i<it; ++i) dataOut[i] = calloc(Vectors_width*Vectors_height, sizeof(dataType));

		printf("Running CPU...\n");
		bool convergedCPU = false;
		int y = 0, minValueCPU = -2;
		gettimeofday(&begin, NULL);
		VectorsCPU(dataBuffer, dataOut[0], paramsOut[0], &minValueCPU);
		while (++y < it && convergedCPU == 0) {
			VectorsCPU(dataOut[y-1], dataOut[y], paramsOut[y], &minValueCPU);
			if (minValueCPU >= -1) convergedCPU = true;
		}
		gettimeofday(&end, NULL);
		timeSpentCPU += (end.tv_sec - begin.tv_sec) +
				((end.tv_usec - begin.tv_usec)/1000000.0);
		printf("Time CPU: %lf\n", timeSpentCPU);

		if (convergedCPU) printf("Converged after %d iterations\n", y);
		else printf("Hasn't converged after %d iterations\n", y);

		double acc = checkAccuracy(dataOutDFE, dataOut, x, y);
		printf("The accuracy measured is: %f\n", acc);
		printf("Speedup: %f\n", timeSpentCPU/timeSpent);

		FILE* time_res;
		char filename[16];
		snprintf(filename, sizeof(filename), "benchmark.csv");
		time_res = fopen(filename, "a");
		fprintf(time_res, "%d, %d, %d, %f, %f, %f, %f\n", Vectors_width, Vectors_height, it, timeSpentCPU, timeSpent, timeSpentCPU/timeSpent, acc);
		fclose(time_res);
		printf("Benchmark results written to %s\n", filename);

		for (int i=0; i<it; ++i) free(dataOut[i]);
		free(dataOut);
	}

	if (exportIt) {
		printf("Exporting results...\n");
		FILE* results;
		FILE* paramsFile;

		int calculatedIts = (x < it) ? x : it;

		char filename[64];
		snprintf(filename, sizeof(filename), "results_%dx%d_%d.csv", Vectors_width, Vectors_height, calculatedIts);

		results = fopen(filename, "wb");

		char paramsFileName[64];
		snprintf(paramsFileName, sizeof(paramsFileName), "params_%dx%d_%d.csv", Vectors_width, Vectors_height, calculatedIts);

		paramsFile = fopen(paramsFileName, "wb");

		dataType* imageBuffer = malloc(Vectors_width * Vectors_height * sizeof(dataType));

		//exporting - first it
		fprintf(paramsFile, "%d,%d\n", paramsOut[0][0], paramsOut[0][1]);

		//printing dataIn as well
		for(int i = 0; i < Vectors_width*Vectors_height-1; ++i) {
			fprintf(results, "%d,", dataIn[i]);
		}
		fprintf(results, "%d\n", dataIn[Vectors_width*Vectors_height-1]);

		for(int i = 0; i < Vectors_width*Vectors_height-1; ++i) {
			if (dataOutDFE[0][i] < 0) fprintf(results, "%d,", dataOutDFE[0][i]);
			else fprintf(results, " ,");
			imageBuffer[i] = dataOutDFE[0][i];
		}

		if (dataOutDFE[0][Vectors_width*Vectors_height-1] < 0) fprintf(results, "%d\n", dataOutDFE[0][Vectors_width*Vectors_height-1]);
		else fprintf(results, " \n");
		imageBuffer[Vectors_width*Vectors_height-1] = dataOutDFE[0][Vectors_width*Vectors_height-1];

		for(int j = 1; j < calculatedIts; ++j) {
			for(int i = 0; i < Vectors_width*Vectors_height-1; ++i) {
				if (imageBuffer[i] != dataOutDFE[j][i] && imageBuffer != 0) {
					fprintf(results, "%i,", dataOutDFE[j][i]);
					imageBuffer[i] = dataOutDFE[0][i];
				} else fprintf(results, " ,");
			}

			if (imageBuffer[Vectors_width*Vectors_height-1] != dataOutDFE[j][Vectors_width*Vectors_height-1] && imageBuffer != 0) {
				fprintf(results, "%i", dataOutDFE[j][Vectors_width*Vectors_height-1]);
				imageBuffer[Vectors_width*Vectors_height-1] = dataOutDFE[0][Vectors_width*Vectors_height-1];
			} else fprintf(results, " ");

			fprintf(results, "\n");
			fprintf(paramsFile, "%d,%d\n", paramsOut[j][0], paramsOut[j][1]);
		}

		fclose(results);
		fclose(paramsFile);
		free(imageBuffer);

		printf("Exporting done.\n");
	}

	free(dataIn);
	free(dataBuffer);
	for (int i=0; i<it; ++i) free(dataOutDFE[i]);
	free(dataOutDFE);

	for (int i=0; i<it; ++i) free(paramsOut[i]);
	free(paramsOut);

	printf("Goodbye!\n");

	return 0;
}
