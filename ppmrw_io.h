#ifndef PPM_h
#define PPM_h

/* include header files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define return status */
#define SUCCESS (0)
#define ERROR	(-1)

/* Function to read PPM file
 * @param ipf[IN] 	  Input file name
 * @param imageR[OUT] Image Red Plane 
 * @param imageG[OUT] Image Red Plane 
 * @param imageB[OUT] Image Red Plane 
 * @param height[OUT] Image height
 * @param width[OUT]  Image width
 */
int readPPM( const char *ipf, unsigned char **imageR, unsigned char **imageG, unsigned char **imageB, int *height, int *width);

/* Function to write PPM file in P3 format
 * @param imageR[IN] Image Red Plane 
 * @param imageG[IN] Image Red Plane 
 * @param imageB[IN] Image Red Plane 
 * @param height[IN] Image height
 * @param width[IN]  Image width
 * @param opf[IN] 	  Output file name
 */
int writePPM3(const unsigned char *imageR, const unsigned char *imageG, const unsigned char *imageB, int height, int width, const char *opf);

/* Function to write PPM file in P6 format
 * @param imageR[IN] Image Red Plane 
 * @param imageG[IN] Image Red Plane 
 * @param imageB[IN] Image Red Plane 
 * @param height[IN] Image height
 * @param width[IN]  Image width
 * @param opf[IN] 	  Output file name
 */
int writePPM6(const unsigned char *imageR, const unsigned char *imageG, const unsigned char *imageB, int height, int width, const char *opf);

#endif