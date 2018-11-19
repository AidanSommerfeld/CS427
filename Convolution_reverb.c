/*****************************************************
* Reverb filter:                                     *
*  Purpose: Take a .dat pcm encoded file, apply a    *
*           reverb effect, and output a .dat pcm     *
*           encoded file.                            *
*                                                    *
* This code was created for the class Introduction   *
* to Computer Audio: Fall 2018                       *
*                                                    *
* Created by Aidan Sommerfeld on 11/19/2018          *
*****************************************************/

#include <cstdio>
#include <fstream>
#include <string>
using namespace std;


/**************************************************************************
* Array struct                                                            *
* Created by casablanca on Aug 21, 2010                                   *
* and edited by Matteo Furlan on Jan 4, 2017                              *
* found here:                                                             *
* https://stackoverflow.com/questions/3536153/c-dynamically-growing-array *
* it was repurposed for use of long double precision floats               *
**************************************************************************/
typedef struct {
	long double *array;
	size_t used;
	size_t size;
} Array;

void initArray(Array *a, size_t initialSize);
void insertArray(Array *a, long double element);
void freeArray(Array *a);
/*************************************************************************/
void reverb(Array *input, Array * imp);

void input(char* input, int length);

/*****************************************************
* main():                                            *
*  Purpose: Aquires the file names from the user and *
*           creates arrays that are passed to the    *
*           reverb function.                         *
*                                                    *
*
*     Array sampleTime: stores the time column from  *
*                       the original file. This will *
*                       not be modified.             *
*                                                    *
*           Array left: stores the values from the   *
*                       left channel.                *
*                                                    *
*          Array right: stores the values from the   *
*                       right channel.               *
*                                                    *
*           Once the reverb is applied by the        *
*           function, the result is output to the    *
*           output file.                             *
*****************************************************/
int main()
{
	char inFileName[128];
	char impulseName[128];
	char outFileName[128];
	printf("Reverb Filter\n\tThis program will apply a finite impulse response to the indicated file.\nEnter the name of the input file: ");
	input(inFileName, 128);
	printf("Enter the name of the impulse file: ");
	input(impulseName, 128);
	printf("Enter the name of the output file: ");
	input(outFileName, 128);

	FILE *inFile;
	if (inFile = fopen(inFileName, "r"))
	{
		printf("%s succesfully opened for reading\n", inFileName);
	}
	else
	{
		printf("%s not found\nexiting...\n", inFileName);
		return 1;
	}

	FILE *impulseFile;
	if (impulseFile = fopen(impulseName, "r"))
	{
		printf("%s succesfully opened for reading\n", impulseName);
	}
	else
	{
		printf("%s not found\nexiting...\n", impulseName);
		return 2;
	}

	FILE *outFile;
	if (outFile = fopen(outFileName, "w"))
	{
		printf("%s succesfully opened for writing\n", inFileName);
	}
	else
	{
		printf("%s not found\nexiting...\n", inFileName);
		return 2;
	}


	Array sampleTime;
	Array left;
	Array right;
	Array impLeft;
	Array impRight;

	//initialize each array
	initArray(&sampleTime, 44100);
	initArray(&left, 44100);
	initArray(&right, 44100);
	initArray(&impLeft, 44100);
	initArray(&impRight, 44100);

	//read from the input file
	char headerLine1[256];
	char headerLine2[256];
	fgets(headerLine1, 256, (FILE*)inFile);
	fgets(headerLine2, 256, (FILE*)inFile);

	char tempSampleTime[128];
	char tempLeft[128];
	char tempRight[128];

	long double tempLeftD;
	long double tempRightD;
	long double tempSampleTimeD;

	while (!feof(inFile))
	{
		fscanf(inFile, "%s%s%s", tempSampleTime, tempLeft, tempRight);
		char *leftEnd;
		char *rightEnd;
		char *sampleTimeEnd;
		//convert cstrings to long double
		tempLeftD = strtold(tempLeft, &leftEnd);
		tempRightD = strtold(tempRight, &rightEnd);
		tempSampleTimeD = strtold(tempSampleTime, &sampleTimeEnd);

		printf("%Lf %Le %Le\n", tempSampleTimeD, tempLeftD, tempRightD);

		//add values to the array
		insertArray(&sampleTime, tempSampleTimeD);
		insertArray(&left, tempLeftD);
		insertArray(&right, tempRightD);
	}

	//close input file
	fclose(inFile);


	//read from the impulse file

	char ignoreHeader[256];
	fgets(ignoreHeader, 256, (FILE*)impulseFile);
	fgets(ignoreHeader, 256, (FILE*)impulseFile);

	char tempImpSampleTime[128];
	char tempImpLeft[128];
	char tempImpRight[128];

	long double tempImpLeftD;
	long double tempImpRightD;
	long double tempImpSampleTimeD;
	while (!feof(impulseFile))
	{
		fscanf(impulseFile, "%s%s%s", tempImpSampleTime, tempImpLeft, tempImpRight);
		char *leftEnd;
		char *rightEnd;
		//convert cstrings to long double
		tempImpLeftD = strtold(tempImpLeft, &leftEnd);
		tempImpRightD = strtold(tempImpRight, &rightEnd);


		printf("%Le %Le\n", tempImpLeftD, tempImpRightD);
		//add to the array
		insertArray(&impLeft, tempImpLeftD);
		insertArray(&impRight, tempImpRightD);

	}

	//close impulse file
	fclose(impulseFile);

	//do reverb 

	reverb(&left, &impLeft);
	reverb(&right, &impRight);

	//make new file
	fprintf(outFile, "%s%s", headerLine1, headerLine2);
	for (int i = 0; (i < sampleTime.used && i < left.used && i < right.used); i++)
	{
		fprintf(outFile, "%15Lf%20Le%19Le\n", sampleTime.array[i], left.array[i], right.array[i]);
	}


	//close output file
	fclose(outFile);

	//free memory for each array
	freeArray(&sampleTime);
	freeArray(&left);
	freeArray(&right);
	freeArray(&impLeft);
	freeArray(&impRight);
	return 0;
}

/*****************************************************
* input():                                           *
*  Purpose: Gets and cleans user input to replace    *
*           newline character with a null            *
*           terminator.                              *
*****************************************************/
void input(char *input, int length) {

	fgets(input, length, stdin);
	for (int i = 0; i < length; i++)
	{
		if (input[i] == '\n')
		{
			input[i] = '\0';
			break;
		}
	}
}

/*****************************************************
* reverb(Array *input, Array *imp):                  *
*  Purpose:  plays the impulse response on every     *
*            sample.                                 *
*                                                    *
*  Follows the form:                                 *
* processed[n] = imp[0]*input[i]                     *
*                + imp[1]* input[i-1]                *
*                + ... + imp[n]*[i-n]                *
*                                                    *
*                note: if i-n is less than 0, stop   *
                        and move on.                 *
*****************************************************/
void reverb(Array *input, Array *imp)
{
	Array processed;
	initArray(&processed, input->used);

	//calculate reverb
	for (int i = 0; i < input->used; i++)
	{
		long double newVal = 0;
		for (int j = 0; j < imp->used; j++)
		{
			if (i - j < 0)
				break;
			newVal += imp->array[j] * input->array[i - j];
		}
		processed.array[i] = newVal / 10;
	}

	//set a to processed
	for (int i = 0; i < input->used; i++)
	{
		input->array[i] = processed.array[i];
	}

	//free processed memory
	freeArray(&processed);
}

/**************************************************************************
* Dynamic C array functions                                               *
* Created by casablanca on Aug 21, 2010                                   *
* and edited by Matteo Furlan on Jan 4, 2017                              *
* found here:                                                             *
* https://stackoverflow.com/questions/3536153/c-dynamically-growing-array *
* it was repurposed for use of long double precision floats               *
**************************************************************************/

/*****************************************************
* initArray(Array *a, size_t initialSize):           *
*  Purpose: create a dynamic array with an initial   *
*           specified small size.                    *
*****************************************************/
void initArray(Array *a, size_t initialSize) {
	a->array = (long double *)malloc(initialSize * sizeof(long double));
	a->used = 0;
	a->size = initialSize;
}



/*****************************************************
* insertArray(Array *a, long double element):        *
*  Purpose: create a dynamic array with an initial   *
*           specified small size.                    *
*****************************************************/
void insertArray(Array *a, long double element) {
	// a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
	// Therefore a->used can go up to a->size 
	if (a->used == a->size) {
		a->size *= 2;
		a->array = (long double *)realloc(a->array, a->size * sizeof(long double));
	}
	a->array[a->used++] = element;
}


/*****************************************************
* freeArray(Array *a):                               *
*  Purpose: free memory from the arrays to avoid     *
*           memory leaks.                            *
*****************************************************/
void freeArray(Array *a) {
	free(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}
