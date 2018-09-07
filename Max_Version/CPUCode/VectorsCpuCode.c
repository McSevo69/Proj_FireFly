#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "Maxfiles.h"
#include <MaxSLiCInterface.h>

typedef uint8_t dataType;

static dataType burning = 3;
// 2 - burnt
// 1 - empty
// 0 - filled

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

void fillWithRandom(float *randSet, int size) {
	for (int i=0; i<size; i++) randSet[i] = (float) rand() / RAND_MAX;
}
 
void loadInputData(char *inPath, dataType *inData, int size) {
	
	FILE *dataFile = fopen(inPath, "r");

	char line[1024];
	while (fgets(line, 1024, dataFile)) {
		char* tmp = strdup(line);
		for (int i=0; i<size; i++) inData[i] = atoi(getfield(tmp, i));
		// NOTE strtok clobbers tmp
		free(tmp);
	}

	fclose(dataFile);
}

//initialization
void init(dataType *a, float empty) {

	for (int i=0; i<Vectors_width*Vectors_height; i++) a[i] = 1;

	int randomIndex;
	srand(time(NULL));

	int items = floor(Vectors_width*Vectors_height*(1-empty));

	// filled ones
	for (int i=0; i<items; i++) {
		randomIndex = floor(( (float) rand() / RAND_MAX) * ( (float) Vectors_width*Vectors_height));		
		a[randomIndex] = 0;
	}
}
 
int hasBurningNeighbors(dataType* dataset, int x, int y, int width, int height) {

	int cnt;
	int index = y*width+x;
	
	cnt = (y - 1 >= 0) ? dataset[index - width] > 2 : 0;
	cnt += (y - 1 >= 0 && x + 1 < width) ? dataset[index - width + 1] > 2 : 0;
	cnt += (x + 1 < width) ? dataset[index + 1] > 2 : 0;
	cnt += (y + 1 < height && x + 1 < width) ? dataset[index + width + 1] > 2 : 0;
	cnt += (y + 1 < height) ? dataset[index + width] > 2 : 0;
	cnt += (y + 1 < height && x - 1 >= 0) ? dataset[index + width - 1] > 2 : 0;
	cnt += (x - 1 >= 0) ? dataset[index - 1] > 2 : 0;
	cnt += (y - 1 >= 0 && x - 1 >= 0) ? dataset[index - width - 1] > 2 : 0;

	return cnt;		
}

int getNewCellState(dataType* dataset, int x, int y, int width, int height, float prop_f, float prop_p) {

	int index = y*width+x;

	if (dataset[index] >= 2) return dataset[index] - 1;
	else if(dataset[index] == 1) return (prop_p > (float)rand() / RAND_MAX) ? 0 : dataset[index];
	else return (hasBurningNeighbors(dataset, x, y, width, height) 
		|| prop_f > (float)rand() / RAND_MAX) ? burning : dataset[index];
}

void VectorsCPU(dataType *dataIn, dataType *dataOut, float prop_f, float prop_p) {

	for (int y = 0; y < Vectors_height; y++) {
		for (int x = 0; x < Vectors_width; x++) {
			int index = y*Vectors_width+x;
			dataOut[index] = getNewCellState(dataIn, x, y, Vectors_width, Vectors_height, prop_f, prop_p);			
		}
	}
	
}

void printDataset(int *dataset, int width, int height) {
	printf("\n");
	for (int y = 1; y + 1 < height; y++) {
		for (int x = 1; x + 1 < width; x++) {
			int index = y*width+x;
			printf("%d ", dataset[index]);              
		}
		printf("\n");
	}
}

int convertArgToInt(char * str) {
	if (strcmp ("-in", str) == 0) return 1;
	else if (strcmp ("-out", str) == 0) return 2;
	else if (strcmp ("-it", str) == 0) return 3;
	else if (strcmp ("-benchmark", str) == 0) return 4;
	else if (strcmp ("-t", str) == 0) return 5;
	else if (strcmp ("-f", str) == 0) return 6;
	else if (strcmp ("-p", str) == 0) return 7;
	else if (strcmp ("-empty", str) == 0) return 8;
	else return 0;
}

int main(int argc, char *argv[]) {
	//time measurement
	struct timeval begin, end;
	double timeSpentCPU = 0, timeSpent = 0;

	int it=10, t = 0;  //t<-burn duration

	printf("=================\n");
	printf("Forest fire stream\n");
	printf("Precision: %ld\n",sizeof(dataType)*8);
	printf("=================\n");
	
	size_t benchmark = 0, inSet = 0, outSet = 0;
	float f = 0.002, p = 0.08, empty = 0.75;
	char *inPath, *outPath = "out.csv";

	float *randSet = malloc(Vectors_width*Vectors_height*sizeof(float));

	//commandline parameter parsing
	for (int i=0; i<argc; i++) {
		switch(convertArgToInt(argv[i])) {
			case 1: inPath = argv[++i]; inSet = 1; break;
			case 2: outPath = argv[++i]; outSet = 1; break;
			case 3: it = atoi(argv[++i]); break;
			case 4: benchmark = 1; break;
			case 5: t = atoi(argv[++i]); break;
			case 6: f = atof(argv[++i]); break;
			case 7: p = atof(argv[++i]); break;
			case 8: empty = atof(argv[++i]); break;
			default: break;
		}
	}

	printf("==========PARAMETERS========\n");
	printf("it: %d, f: %f, p: %f, t: %d, empty: %f\n", it, f, p, t, empty);
	printf("============================\n");

	if (t > 0) burning += t;

	dataType * dataIn = calloc(Vectors_width*Vectors_height, sizeof(dataType));
	dataType ** dataOut = malloc(it*sizeof(dataType*));
	for (int i=0; i<it; i++) dataOut[i] = calloc(Vectors_width*Vectors_height, sizeof(dataType));

	dataType ** dataOutDFE = malloc(it*sizeof(dataType*));
	for (int i=0; i<it; i++) dataOutDFE[i] = calloc(Vectors_width*Vectors_height, sizeof(dataType));

	if (empty > 1 || empty < 0 ) {
		printf("WARNING: parameter empty %f not in range [0, 1]\nDefault (0.3) is used.\n", empty);
		empty = 0.3;
	}

	if (!inSet) {
		printf("No input path (param: -in) set. Input data is generated.\n");
		init(dataIn, empty);
	} else {
		printf("Loading input file.\n");
		loadInputData(inPath, dataIn, Vectors_width*Vectors_height);
	}

	if (!outSet) {
		printf("WARNING: No output path (param: -out) set. Default is used.\n");
	}

	if (benchmark) {		
		printf("Running CPU...\n");
		gettimeofday(&begin, NULL);
		VectorsCPU(dataIn, dataOut[0], f, p);
		for (int i=1; i<it; i++) VectorsCPU(dataOut[i-1], dataOut[i], f, p);
		gettimeofday(&end, NULL);
		timeSpentCPU += (end.tv_sec - begin.tv_sec) +
		    ((end.tv_usec - begin.tv_usec)/1000000.0);
		printf("Time CPU: %lf\n", timeSpentCPU);
	}


	printf("Running DFE...\n");
	fillWithRandom(randSet, Vectors_width*Vectors_height);
	gettimeofday(&begin, NULL);	
	Vectors(Vectors_width*Vectors_height, f, p, burning, dataIn, randSet, dataOutDFE[0]);
	for (int i=1; i<it; i++) {
		//fillWithRandom(randSet, Vectors_width*Vectors_height);
		Vectors(Vectors_width*Vectors_height, f, p, burning, dataOutDFE[i-1], randSet, dataOutDFE[i]);
	}
	gettimeofday(&end, NULL);
	timeSpent += (end.tv_sec - begin.tv_sec) +
            ((end.tv_usec - begin.tv_usec)/1000000.0);
	printf("Time DFE: %lf\n", timeSpent);
	
	//Exporting benchmark results
	if (benchmark) {
		FILE* time_res;
		char filename[32];
		printf("Exporting benchmark to file...\n");
		snprintf(filename, sizeof(filename), "benchmark.csv");
		time_res = fopen(filename,"a");
		fprintf(time_res, "%d, %d, %d, %f, %f\n", Vectors_width, Vectors_height, it, timeSpentCPU, timeSpent);
		fclose(time_res);
	}

	printf("Exporting results...\n");
	FILE* results;
	
	if (outSet) {
		results = fopen(outPath, "w");
	} else {
		char filename[1024];
		snprintf(filename, sizeof(filename), "results_%dx%d_%d.csv", Vectors_width, Vectors_height, it);

		results = fopen(filename, "w");
	}
	
	//printing in-data
	for(int i = 0; i < Vectors_width*Vectors_height-1; ++i){
		fprintf(results, "%u,", dataIn[i]);
	}
		
	fprintf(results, "%u", dataIn[Vectors_width*Vectors_height-1]);
	fprintf(results, "\n");


	//exporting
	for (int j = 0; j < it; j++) {

		for(int i = 0; i < Vectors_width*Vectors_height-1; ++i){
			fprintf(results, "%u,", dataOutDFE[j][i]);
		}
		fprintf(results, "%u", dataOutDFE[j][Vectors_width*Vectors_height-1]);

		fprintf(results, "\n");
	}
	
	fclose(results);

	free(dataIn);
	for (int i=0; i<it; i++) free(dataOut[i]);
	free(dataOut);

	for (int i=0; i<it; i++) free(dataOutDFE[i]);
	free(dataOutDFE);

	free(randSet);
	
	return 0;
}
