#include "json.h"
#include "ppmrw_io.h"
#include <math.h>

// default shinniness factor for specular reflection (used wuieh ns==0)
#define SHININNESS 20

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
// tValue is the distance from pr in the ray where the intersection ocurrs
int ray_plane(double *pr, double *ur, node *pNode, double *result, double *tValue)
{
  double *n = pNode->normal;
  double *p = pNode->position;

  double dot_nu   = n[0] * ur[0] + n[1] * ur[1] + n[2] * ur[2];

  // almost divide by zero
  if (fabs(dot_nu) <= 0.0001)
    return 0;

	double dot_npr_p0 = n[0] * (pr[0]-p[0]) + n[1] * (pr[1]-p[1]) + n[2] * (pr[2]-p[2]);
	// compute the value of t where the ray intersects the plane
  double t = -dot_npr_p0 / dot_nu;

  // result = [0,0,0] + T * u
  result[0] = pr[0] + t*ur[0];
  result[1] = pr[1] + t*ur[1];
  result[2] = pr[2] + t*ur[2];

	*tValue = t;
	return 1;
}

void normalize(double *v)
{
	double norm = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	if (norm)
	{
		double factor = 1.0 / norm;
		v[0] *= factor;
		v[1] *= factor;
		v[2] *= factor;
	}
}

double distance_two_points(double *p, double *q)
{
	return sqrt((p[0] - q[0])*(p[0] - q[0]) + (p[1] - q[1])*(p[1] - q[1]) + (p[2] - q[2])*(p[2] - q[2]));
}

double dot_product(double *a, double *b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void subtract(double *p, double * q, double *result)
{
	result[0] = p[0] - q[0];
	result[1] = p[1] - q[1];
	result[2] = p[2] - q[2];
}

void reflect(double *L, double *N, double *R)
{
	double dot = dot_product(N, N);
	R[0] = 2 * dot * N[0] - L[0];
	R[1] = 2 * dot * N[1] - L[1];
	R[2] = 2 * dot * N[2] - L[2];
	normalize(R);
}

void clamp(double *v, double minimum, double maximum)
{
	for (int i = 0; i < 3; i++)
		if (v[i] < minimum)
			v[i] = minimum;
		else if (v[i] > maximum)
			v[i] = maximum;
}

// compute the intersection between ray and sphere
// intersection should be between near clipping plane and far clipping plane
// otherwise it is discarded
// tValue is the distance from pr in the ray where the intersection ocurrs
int ray_sphere(double *pr, double *u, node *pNode, double *result, double *tValue)
{
  double *c = pNode->position;
  double r = pNode->radius;

  // tclose = pr + dot(u,c) ... but pr = [0,0,0]
  double tclose = u[0] * (c[0]-pr[0]) + u[1] * (c[1]-pr[1]) + u[2] * (c[2]-pr[2]);

  // compute xClose 
  double pclose[3];
  pclose[0] = pr[0] + tclose * u[0];
  pclose[1] = pr[1] + tclose * u[1];
  pclose[2] = pr[2] + tclose * u[2];

  double d = distance_two_points(pclose, c);
  if (d > r) // no intersection
    return 0;
  if (d == r) // one intersection
  {
		*tValue = tclose;
    memcpy(result, pclose, sizeof(double) * 3);
    return 1;
  }

  // default case, d < r; we have 2 intersections
  double a = sqrt(r*r - d*d);
  result[0] = pr[0] + (tclose - a)*u[0];
  result[1] = pr[1] + (tclose - a)*u[1];
  result[2] = pr[2] + (tclose - a)*u[2];
	*tValue = tclose - a;
  return (result[2] >= zp && result[2] <= fcp) ? 1 : 0;
}

// return 1 if we found an intersection of the vector "u" with the scene
// pos is the resulting hit position in the space, and "index" the resulting object index
int shoot(double *p, double *u, double *pos, int *index)
{
  int closest_index = -1;
  double closest_distance = 0.0;
  double closest_intersection_point[3];
  double intersection_point[3];
	double t;
  for (int i = 0; i < nNodes; i++)
  {
    if (strcmp(scene[i].type, "plane") == 0)
    {
      if (ray_plane(p, u, &scene[i], intersection_point, &t) && t>=0.0)
      {
				// intersection is valid if z is between near and far clipping planes
				if (intersection_point[2] >= zp && intersection_point[2] <= fcp)	// clipping to near and far planes
				{
					double distance = distance_two_points(p, intersection_point);
					if (closest_index == -1 || distance < closest_distance)
					{
						closest_distance = distance;
						memcpy(closest_intersection_point, intersection_point, sizeof(double) * 3);
						closest_index = i;
					}
				}
      }
    }
    else if (strcmp(scene[i].type, "sphere") == 0)
    {
      if (ray_sphere(p, u, &scene[i], intersection_point, &t) && t>= 0.0)
      {
        double distance = distance_two_points(p, intersection_point);
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

void Phong(double *L, double *N, double *R, double *V, double *hitPoint, node *obj, node *light, double *result)
{
	
	double dot_diffuse = dot_product(N,L);
	double dot_specular = dot_product(R,V);

	if (obj->diffuse != NULL && dot_diffuse > 0.0)
	{
		result[0] = dot_diffuse * obj->diffuse[0];
		result[1] = dot_diffuse * obj->diffuse[1];
		result[2] = dot_diffuse * obj->diffuse[2];
	}
	
	if (obj->specular != NULL && dot_specular > 0)
	{
		double spec_factor;
		if (obj->ns == 0.0)
			spec_factor = pow(dot_specular, SHININNESS);
		else 
			spec_factor = pow(dot_specular, obj->ns);
		result[0] += spec_factor * obj->specular[0];
		result[1] += spec_factor * obj->specular[1];
		result[2] += spec_factor * obj->specular[2];
	}
	
	// light color should attenuate the color
	if (light->color)
	{
		result[0] *= light->color[0];
		result[1] *= light->color[1];
		result[2] *= light->color[2];
	}

	// light normal == light direction for spotlight
	if (light->normal != NULL && light->theta > 0.0)	// is spotlight??
	{
		// minus dot product between "direction" and light vector -L
		// because < direction, -L >
		double dot = -dot_product(light->normal, L);
		// angular attenuation (for spot lights)
		double fang = 0.0f;

		if (fabs(dot) >= cos(light->theta))
			fang = pow(dot, light->angular);
		result[0] *= fang;
		result[1] *= fang;
		result[2] *= fang;
	}
	else if (light->radial[0] != 0.0 || light->radial[1] != 0.0 || light->radial[2] != 0.0)
	{
		// it is a point light!
		// distance from light to point
		double d = distance_two_points(light->position, hitPoint);
		if (d > 0.0)
		{
			double deno = d * d * light->radial[2] + d * light->radial[1] + light->radial[0];
			if (fabs(deno) > 0.0001)
			{
				double frad = 1.0 / deno;
				if (frad < 0.0)
					frad = 0.0;
				if (frad < 1.0 && frad >=0.0)
				{
					result[0] *= frad;
					result[1] *= frad;
					result[2] *= frad;
				}
			}
		}
	}
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
  // Dynamically allocate memory to hold image buffers
  imageR = (unsigned char *)malloc(height * width * sizeof(unsigned char));
  imageG = (unsigned char *)malloc(height * width * sizeof(unsigned char));
  imageB = (unsigned char *)malloc(height * width * sizeof(unsigned char));

  // Check validity
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
  
	double eyePos[3] = { 0.0, 0.0,0.0 };

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
			double colorIJ[3] = { 0,0,0 };

			// check if an object is hit, and in that case, if the hit is between near and far clipping planes
			if (shoot(eyePos, ur, hit, &index) && hit[2] >= zp && hit[2] <= fcp)
			{
				// inside clipping planes...
				// now, loop over all lights
				for (int k = 0; k < nNodes; k++) if (scene[k].type[0] == 'l')	// for each light
				{
					// ray origing (fron light to object)
					double *Ron = scene[k].position;

					// ray direction
					double Rdn[3];
					// light position - hit
					subtract(hit, Ron, Rdn);

					normalize(Rdn);

					// look for the closest ray intersection
					double objHit[3];
					int objIndex = index;
					if (shoot(Ron, Rdn, objHit, &objIndex))
					{
						// the object is the closets to the light direction
						if (objIndex == index)
						{
							double resultColor[3] = {0.0, 0.0, 0.0};
							double N[3], L[3], V[3];
							// acum light
							if (scene[index].type[0] == 's')	// sphere
								//N = Ron - scene[index].position 
								subtract(hit, scene[index].position, N);
							else if (scene[index].type[0] == 'p')	// plane
								memcpy(N, scene[index].normal, sizeof(double) * 3);
							normalize(N);

							// L = light position - hit
							subtract(scene[k].position, hit, L);
							normalize(L);

							// computing reflext ray
							double R[3];
							reflect(L, N, R);

							// view vector = (0,0,0) - hit 
							V[0] = -hit[0];
							V[1] = -hit[1];
							V[2] = -hit[2];
							normalize(V);

							// compute light model with L, N, R, Diffuse and Specular
							Phong(L, N, R, V, hit, &scene[index], &scene[k], resultColor);
							colorIJ[0] += resultColor[0];
							colorIJ[1] += resultColor[1];
							colorIJ[2] += resultColor[2];
						}
					}
				}

				// need to be  sure that every color component is betwqeen 0 and 1 before conversion
				// pixel colored by object hit 
				int k = (height - 1 - i) * width + j;

				clamp(colorIJ, 0.0, 1.0);
				imageR[k] = (unsigned char)(colorIJ[0] * 255.0);
				imageG[k] = (unsigned char)(colorIJ[1] * 255.0);
				imageB[k] = (unsigned char)(colorIJ[2] * 255.0);
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
	// first erase dynamic mem
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
		if (scene[i].diffuse != NULL)
			free(scene[i].diffuse);
		if (scene[i].specular != NULL)
			free(scene[i].specular);
	}

	// clear all nodes with zeros
	memset(scene, 0, sizeof(node) * MAX_NODES);
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
