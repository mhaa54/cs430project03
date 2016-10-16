#include "ppmrw_io.h"

#ifdef _WIN32
  #pragma warning(disable:4996)
#endif

/* Function to read PPM file
 * @param ipf[IN] 	  Input file name
 * @param imageR[OUT] Image Red Plane 
 * @param imageG[OUT] Image Red Plane 
 * @param imageB[OUT] Image Red Plane 
 * @param height[OUT] Image height
 * @param width[OUT]  Image width
 */
int readPPM( const char *ipf, unsigned char **imageR, unsigned char **imageG, unsigned char **imageB, int *height, int *width) 
{
	FILE *outFp = NULL;
	int level, imgSize, index = 0;
	char ch1, ch2;
	
	/*
	 * Check file name string is valid or not
	 */
	if(ipf == NULL)
	{
		printf("Check input file name\n");
		return ERROR;
	}

	/*
	 * Open the ppm file
	 */
	outFp = fopen(ipf, "r");
	if(outFp == NULL)
	{
		printf("Unable to open input file %s\n", ipf);
		return ERROR;
	}
	
	/*
	 * Get type of the file from header
	 */
	ch1 = getc(outFp);
	ch2 = getc(outFp);
	
	int ppmType = 0;
	
	/*
	 * Validate the header is valid or not
	 */
	if( ch1 == 'P' )
	{
		if( ch2 == '3' )
		{
			ppmType = 3;
		}
		else if( ch2 == '6' )
		{
			ppmType = 6;
		}
		else
		{
			printf("Not a ppm file\n");
			return ERROR;
		}
	}
	else
	{
		printf("Unknown file format\n");
		return ERROR;
	}

	/*
	 * Ignore whitespace
	 */
	ch1 = getc(outFp);
	
	/*
	 * Parse comment inside the file
	 */
	while( (ch1 = getc(outFp) ) == '#')
	{
		while( (ch1 = getc(outFp) ) != '\n'){}
	}

	/*
	 * Fetch width and height of the input ppm
	 */
	fseek ( outFp , -1, SEEK_CUR );
	fscanf(outFp, "%d %d\n%d\n", width, height, &level);
	
	/*
	 * Check the image is 8 bit per channel or more
	 */
	if( level > 255 )
	{
		printf("Not an image with 8 bit per channel \n");
		return ERROR;
	}
	
	/*
	 * Dynamically allocate memory to hold image buffers 
	 */
	*imageR = (unsigned char *)malloc((*height) * (*width) * sizeof(unsigned char));
	*imageG = (unsigned char *)malloc((*height) * (*width) * sizeof(unsigned char));
	*imageB = (unsigned char *)malloc((*height) * (*width) * sizeof(unsigned char));
	
	/*
	 * Check validity
	 */
	if(imageR == NULL || imageG == NULL || imageB == NULL)
	{
		printf("Memory allocation failed\n");
		return ERROR;
	}

	/*
	 * Calculate image size
	 */
	imgSize = (*height) * (*width);
	
	/*
	 * Scan depending upon ppm type
	 */
	switch (ppmType)
	{
		case 3:
		{
			/*
			 * P3 image with integers R1 G1 B1 R2 G2 B2 ...
			 */
			for(index = 0; index < imgSize; index ++)
			{
				int r,g,b;
				fscanf(outFp, "%d %d %d ", &r, &g, &b);
				(*imageR)[index] = (unsigned int)r;
				(*imageG)[index] = (unsigned int)g;
				(*imageB)[index] = (unsigned int)b;
			}
			
			break;
		}
		case 6:
		{	
			/*
			 * P6 image with charcters R1G1B1R2G2B2 ...
			 */
			for(index = 0; index < imgSize; index ++)
			{
				(*imageR)[index] = getc(outFp);
				(*imageG)[index] = getc(outFp);
				(*imageB)[index] = getc(outFp);
			}
			break;
		}
		default:
		{
			printf("Unknown file format\n");
			break;
		}
	}
	
	/*
	 * Close the file
	 */
	fclose(outFp);
	
	return SUCCESS;
}

/* Function to write PPM file in P3 format
 * @param imageR[IN] Image Red Plane 
 * @param imageG[IN] Image Red Plane 
 * @param imageB[IN] Image Red Plane 
 * @param height[IN] Image height
 * @param width[IN]  Image width
 * @param opf[IN] 	  Output file name
 */
int writePPM3(const unsigned char *imageR, const unsigned char *imageG, const unsigned char *imageB, int height, int width, const char *opf) 
{
	FILE *inFp = NULL;
	int ch, i, j, index = 0;
	
	/*
	 * Sample signature to comment inside image
	 */
	char signature[] = {35, 71, 101, 110, 101, 114, 97, 116, 101, 100, 32, 98, 121, 32, 68, 121, 
			110, 65, 108, 103, 111, 40, 105, 110, 102, 111, 64, 100, 121, 110, 97, 108, 103, 111, 46, 99, 111, 109, 41, 0};
	
	/*
	 * Do input validation
	 */
	if(imageR == NULL || imageG == NULL || imageB == NULL || opf == NULL)
	{
		printf("Memory not allocated properly\n");
		return ERROR;
	}

	/*
	 * Open the file to write
	 */
	inFp = fopen(opf, "w");

	if(inFp == NULL)
	{
		printf("Unable to write file\n");
		return ERROR;
	}

	/*
	 * Write in P3 format
	 */
	putc('P', inFp);
	putc('3', inFp);
	fprintf(inFp, "\n%s\n", signature);
	
	fprintf(inFp, "%d %d\n%d\n", width, height, 255);
	
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			fprintf(inFp, "%d %d %d ", imageR[index], imageG[index], imageB[index]);
			index++;
		}
		fprintf(inFp, "\n");
	}
	
	/*
	 * Close the file
	 */
	putc(EOF, inFp);
	fclose(inFp);
	
	return SUCCESS;
}

/* Function to write PPM file in P6 format
 * @param imageR[IN] Image Red Plane 
 * @param imageG[IN] Image Red Plane 
 * @param imageB[IN] Image Red Plane 
 * @param height[IN] Image height
 * @param width[IN]  Image width
 * @param opf[IN] 	  Output file name
 */
int writePPM6(const unsigned char *imageR, const unsigned char *imageG, const unsigned char *imageB, int height, int width, const char *opf) 
{
	FILE *inFp = NULL;
	int ch, i, j, index = 0;
	
	/*
	 * Sample signature to comment inside image
	 */
	char signature[] = {35, 71, 101, 110, 101, 114, 97, 116, 101, 100, 32, 98, 121, 32, 68, 121, 
			110, 65, 108, 103, 111, 40, 105, 110, 102, 111, 64, 100, 121, 110, 97, 108, 103, 111, 46, 99, 111, 109, 41, 0};
	
	/*
	 * Do input validation
	 */
	if(imageR == NULL || imageG == NULL || imageB == NULL || opf == NULL)
	{
		printf("Memory not allocated properly\n");
		return ERROR;
	}

	/*
	 * Open the file to write
	 */
	inFp = fopen(opf, "w");

	if(inFp == NULL)
	{
		printf("Unable to open file\n");
		return ERROR;
	}

	/*
	 * Write in P6 format
	 */
	putc('P', inFp);
	putc('6', inFp);
	fprintf(inFp, "\n%s\n", signature);
	
	fprintf(inFp, "%d %d\n%d\n", width, height, 255);

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			fprintf(inFp, "%c%c%c", imageR[index], imageG[index], imageB[index]);
			index++;
		}
	}
	
	/*
	 * Close the file
	 */
	putc(EOF, inFp);
	fclose(inFp);
	
	return SUCCESS;
}
