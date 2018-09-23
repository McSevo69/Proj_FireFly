#include "types.h"

/**
 * \brief starts a visualisation
 *
 * \param [in] width Width of the image 
 * \param [in] height Height of the image
 * \param [in] iterations number of iterations to display
 * \param [in] dataIn the initial dataset
 * \param [in] dataOut the full dataset
 * \param [in] paramsOut parameterset which was used for calculating the dataset
 */
void startVisualisation(int width, int height, int iterations, int* dataIn, dataType **dataOut, dataType **paramsOut);

/**
 * \brief starts a visualisation
 *
 * \param [in] fileName in format "results_WIDTHxHEIGHT_IT.csv"
 */
void startVisualisationFromFile(char* fileName);

