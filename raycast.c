

#include "json.h"
#include "ppmrw_io.h"
#include <math.h>


// maximum number of objects
#define MAX_NODES 128

// distance to near clipping plane
#define zp 1

// distance to far clipping plane
#define fcp 200

// json array
node scene[MAX_NODES];

// number of nodes. Should be <= MAX_NODES
int nNodes;

// viewport size
int width, height;


// compute the intersection between ray and plane
// intersection should be between near clipping plane and far clipping plane
// otherwise it is discarded
int ray_plane(double *u, node *pNode, double *result)
{
  double *n = pNode->normal;
  double *p = pNode->position;

  double dot_npos = n[0] * p[0] + n[1] * p[1] + n[2] * p[2];
  double dot_nu   = n[0] * u[0] + n[1] * u[1] + n[2] * u[2];

  // almost divide by zero
  if (fabs(dot_nu) <= 0.0001)
    return 0;

  // compute the value of t where the ray intersects the plane
  double t = dot_npos / dot_nu;

  // result = [0,0,0] + T * u
  result[0] = t*u[0];
  result[1] = t*u[1];
  result[2] = t*u[2];

  // intersection is valid if z is between near and far clipping planes
  return (result[2] >= zp && result[2] <= fcp) ? 1 : 0;
}

// compute the intersection between ray and sphere
// intersection should be between near clipping plane and far clipping plane
// otherwise it is discarded
int ray_sphere(double *u, node *pNode, double *result)
{
  double *c = pNode->position;
  double r = pNode->radius;

  // tclose = pr + dot(u,c) ... but pr = [0,0,0]
  double tclose = u[0] * c[0] + u[1] * c[1] + u[2] * c[2];

  // compute xClose 
  double pclose[3];
  pclose[0] = tclose * u[0];
  pclose[1] = tclose * u[1];
  pclose[2] = tclose * u[2];

  double d = sqrt(
    (pclose[0] - c[0])*(pclose[0] - c[0]) +
    (pclose[1] - c[1])*(pclose[1] - c[1]) +
    (pclose[2] - c[2])*(pclose[2] - c[2]));
  if (d > r) // no intersection
    return 0;
  if (d == r) // one intersection
  {
    // out of clipping planes
    if (pclose[2] < zp || pclose[2] > fcp)
      return 0;

    // inside clipping planes...then copy the result
    memcpy(result, pclose, sizeof(double) * 3);
    return 1;
  }

  // default case, d < r; we have 2 intersections
  double a = sqrt(r*r - d*d);
  result[0] = (tclose - a)*u[0];
  result[1] = (tclose - a)*u[1];
  result[2] = (tclose - a)*u[2];
  return (result[2] >= zp && result[2] <= fcp) ? 1 : 0;
}


// return 1 if we found an intersection of the vector "u" with the scene
// pos is the resulting hit position in the space, and "index" the resulting object index
int shoot(double *u, double *pos, int *index)
{
  int closest_index = -1;
  double closest_distance = 0.0;
  double closest_intersection_point[3];
  double intersection_point[3];
  for (int i = 0; i < nNodes; i++)
  {
    if (strcmp(scene[i].type, "plane") == 0)
    {
      if (ray_plane(u, &scene[i], intersection_point))
      {
        double distance = sqrt(intersection_point[0] * intersection_point[0] + 
                               intersection_point[1] * intersection_point[1] + 
                               intersection_point[2] * intersection_point[2]);
        if (closest_index == -1 || distance < closest_distance)
        {
          closest_distance = distance;
          memcpy(closest_intersection_point, intersection_point, sizeof(double) * 3);
          closest_index = i;
        }
      }
    }
    else if (strcmp(scene[i].type, "sphere") == 0)
    {
      if (ray_sphere(u, &scene[i], intersection_point))
      {
        double distance = sqrt(intersection_point[0] * intersection_point[0] +
          intersection_point[1] * intersection_point[1] +
          intersection_point[2] * intersection_point[2]);
        if (closest_index == -1 || distance < closest_distance)
        {
          closest_distance = distance;
          memcpy(closest_intersection_point, intersection_point, sizeof(double) * 3);
          closest_index = i;
        }
      }
    }
  }

  if (closest_index >= 0)
  {
    *index = closest_index;
    memcpy(pos, closest_intersection_point, sizeof(double) * 3);
    return 1;
  }
  return 0;
}

// to the ray casting, and save it into filename
void ray_casting(const char *filename)
{
  // do some validations
  if (nNodes <= 0)
  {
    fprintf(stderr, "Empty scene\n");
    exit(1);
  }

  // look for camera object
  int found = 0;
  double w, h;
  for (int i = 0; i < nNodes; i++)
  {
    if (strcmp(scene[i].type, "camera") == 0)
    {
      found = 1;
      w = scene[i].width;
      h = scene[i].height;
      if (w <= 0.0 || w > 4096.0 || h< 0.0 || h > 4096.0)
      {
        fprintf(stderr, "Invalid camera. Please, check the scene\n");
        exit(1);
      }
    }
  }
  if (found == 0)
  {
    fprintf(stderr, "Camera object not found. Invalid scene\n");
    exit(1);
  }


  // creating image buffer
  unsigned char *imageR = NULL, *imageG = NULL, *imageB = NULL;
  /*
  * Dynamically allocate memory to hold image buffers
  */
  imageR = (unsigned char *)malloc(height * width * sizeof(unsigned char));
  imageG = (unsigned char *)malloc(height * width * sizeof(unsigned char));
  imageB = (unsigned char *)malloc(height * width * sizeof(unsigned char));

  /*
  * Check validity
  */
  if (imageR == NULL || imageG == NULL || imageB == NULL)
  {
    fprintf(stderr, "Memory allocation failed for the image\n");
    exit(1);
  }

  // erasing image
  int s = width * height;
  memset(imageR, 0, s);
  memset(imageG, 0, s);
  memset(imageB, 0, s);

  // the height of one pixel 
  double pixheight = h / (double)height;

  // the width of one pixel 
  double pixwidth = w / (double)width;
  
  // for each row 
  for(int i = 0; i < height; i++)
  { 
    // y coord of row 
    double py = -h/2.0 + pixheight * (i + 0.5);

    // for each column 
    for(int j = 0; j < width; j++)
    { 
      // x coord of column 
      double px = -w/2.0 + pixwidth * (j + 0.5);

      // z coord is on screen 
      double pz = zp;

      // length of p vector
      double norm = sqrt(px*px + py*py + pz*pz);

      // unit ray vector 
      double ur[3] = {px/norm, py/norm, pz/norm};

      // return position of first hit 
      double hit[3];
      int index;
      if (shoot(ur, hit, &index))
      {
        // pixel colored by object hit 
        int k = (height-1-i) * width + j;
        
        // object should have a color... otherwise, nothing to do!
        if (scene[index].color != NULL) 
        {
          imageR[k] = (unsigned char)(scene[index].color[0] * 255.0);
          imageG[k] = (unsigned char)(scene[index].color[1] * 255.0);
          imageB[k] = (unsigned char)(scene[index].color[2] * 255.0);
        }
      }
    } 
  }  


  // save ppm file
  writePPM3(imageR, imageG, imageB, height, width, filename);

  // free image
  free(imageR);
  free(imageG);
  free(imageB);
}

void free_scene()
{
  for (int i = 0; i < nNodes; i++)
  {
    if (scene[i].color != NULL)
      free(scene[i].color);
    if (scene[i].position != NULL)
      free(scene[i].position);
    if (scene[i].normal != NULL)
      free(scene[i].normal);
    if (scene[i].type != NULL)
      free(scene[i].type);
  }
}

int main(int argc, char *argv[])
{
	if (argc != 5)
	{
		fprintf(stderr, "Please, use raycast <width> <height> <input.json> <output.ppm>\n");
		return 1;
	}

	width = atoi(argv[1]);
	if (width <= 0 || width > 4906)
	{
		fprintf(stderr, "Wrong output image size. Please, use any size between 1..4096\n");
		return 1;
	}
	
	height = atoi(argv[2]);
	if (height <= 0 || height > 4906)
	{
		fprintf(stderr, "Wrong output image size. Please, use any size between 1..4096\n");
		return 1;
	}

  // clear all possible nodes of the array
  memset(scene, 0, sizeof(node) * MAX_NODES);
	read_scene(argv[3], &nNodes, scene);
	ray_casting(argv[4]);
  free_scene();
	return 0;
} 
