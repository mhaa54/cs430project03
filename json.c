#include "json.h"
#include <math.h>


// line counter
int line = 1;

// next_c() wraps the getc() function and provides error checking and line
// number maintenance
int next_c(FILE* json)
{
  int c;
  do
  {
    c = fgetc(json);

    // check enter chars in windows/linux/mac
    if (c == '\n' || (c == '\r'))
      line++;
    else if (c == EOF)
    {
      fprintf(stderr, "Error: Unexpected end of file on line number %d\n", line);
      fclose(json);
      exit(1);
    }
    else
      break;
  } while (1);
  return c;
}

// expect_c() checks that the next character is d. If it is not it emits
// an error.
void expect_c(FILE* json, int d)
{
  int c = next_c(json);
  if (c == d)
    return;
  fprintf(stderr, "Error: Expected '%c' on line %d\n", d, line);
  fclose(json);
  exit(1);
}

// skip_ws() skips white space in the file.
void skip_ws(FILE* json)
{
  int c = next_c(json);
  while (c == ' ')
    c = next_c(json);
  ungetc(c, json);
}

// next_string() gets the next string from the file handle and emits an error
// if a string can not be obtained.
char* next_string(FILE* json)
{
  char buffer[129];
  int c = next_c(json);
  if (c != '"')
  {
    fprintf(stderr, "Error: Expected string on line %d.\n", line);
    fclose(json);
    exit(1);
  }
  c = next_c(json);
  int i = 0;
  while (c != '"')
  {
    if (i >= 128)
    {
      fprintf(stderr, "Error: Strings longer than 128	characters in length are not supported.\n");
      fclose(json);
      exit(1);
    }
    if (c == '\\')
    {
      fprintf(stderr, "Error: Strings with escape codes are not supported.\n");
      fclose(json);
      exit(1);
    }
    if (c < 32 || c > 126)
    {
      fprintf(stderr, "Error: Strings may contain only ascii characters.\n");
      fclose(json);
      exit(1);
    }
    buffer[i] = c;
    i++;
    c = next_c(json);
  }
  buffer[i] = 0;
  return strdup(buffer);
}

// next_number(f) returns the next floating point value on the json file
double next_number(FILE* json)
{
  double value;
  if (fscanf(json, "%lf", &value) != 1)
  {
    fprintf(stderr, "Error, line number %d; expected floating point value\n", line);
    fclose(json);
    exit(1);
  }
  return value;
}

// next_vector(f) returns the next 3D vector on the json file
double* next_vector(FILE* json)
{
  double* v = malloc(3 * sizeof(double));
  expect_c(json, '[');
  skip_ws(json);
  v[0] = next_number(json);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  v[1] = next_number(json);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  v[2] = next_number(json);
  skip_ws(json);
  expect_c(json, ']');
  return v;
}


// next_object(f,node) returns the next 3D node record on the json file f
void next_object(FILE *json, node *pNode)
{
  int c = next_c(json);

  if (c != '{')
  {
    fprintf(stderr, "Error on line number %d. Expected character '{' instead of '%c'\n", line, c);
    fclose(json);
    exit(1);
  }

  skip_ws(json);
  c = next_c(json);

  while (c != '}')
  {
    if (c != '"')
    {
      fprintf(stderr, "Error on line number %d. Expected character '\"' instead of '%c'\n", line, c);
      fclose(json);
      exit(1);
    }

    ungetc(c, json);
    char *name = next_string(json);

    if (strcmp(name, "type") == 0)
    {
      skip_ws(json);
      expect_c(json, ':');
      skip_ws(json);
      pNode->type = next_string(json);
      if (strcmp(pNode->type, "camera") != 0 && strcmp(pNode->type, "plane") != 0 && strcmp(pNode->type, "sphere") != 0 && strcmp(pNode->type, "light") != 0)
      {
        fprintf(stderr, "Error on line number %d. Valid objects are: camera, plane and sphere\n", line);
        fclose(json);
        exit(1);
      }
    }
		else if (strcmp(name, "radial-a0") == 0)
		{
			skip_ws(json);
			expect_c(json, ':');
			skip_ws(json);
			double v = next_number(json);
			pNode->radial[0] = v;
		}
		else if (strcmp(name, "radial-a1") == 0)
		{
			skip_ws(json);
			expect_c(json, ':');
			skip_ws(json);
			double v = next_number(json);
			pNode->radial[1] = v;
		}
		else if (strcmp(name, "radial-a2") == 0)
		{
			skip_ws(json);
			expect_c(json, ':');
			skip_ws(json);
			double v = next_number(json);
			pNode->radial[2] = v;
		}
		else if (strcmp(name, "theta") == 0)
		{
			skip_ws(json);
			expect_c(json, ':');
			skip_ws(json);
			pNode->theta = next_number(json);

			// converting degress to radians
			pNode->theta *= 3.1415926535897932384 / 180.0;
		}
		else if (strcmp(name, "ns") == 0)
		{
			skip_ws(json);
			expect_c(json, ':');
			skip_ws(json);
			pNode->ns = next_number(json);
			// should be 0 or larger
			if (pNode->ns < 0.0)
				pNode->ns = 0.0;
		}
		else if (strcmp(name, "angular-a0") == 0)
		{
			skip_ws(json);
			expect_c(json, ':');
			skip_ws(json);
			pNode->angular = next_number(json);
		}
		else if ((strcmp(name, "width") == 0) || (strcmp(name, "height") == 0) || (strcmp(name, "radius") == 0))
    {
      skip_ws(json);
      expect_c(json, ':');
      skip_ws(json);
      double v = next_number(json);
      if (strcmp(name, "width") == 0)
        pNode->width = v;
      else if (strcmp(name, "height") == 0)
        pNode->height = v;
      else
        pNode->radius = v;
    }
    else if ((strcmp(name, "color") == 0) || (strcmp(name, "position") == 0) || (strcmp(name, "normal") == 0) || (strcmp(name, "direction") == 0) || (strcmp(name, "diffuse_color") == 0) || (strcmp(name, "specular_color") == 0))
    {
      skip_ws(json);
      expect_c(json, ':');
      skip_ws(json);
      double *v = next_vector(json);
      if (strcmp(name, "color") == 0)
        pNode->color = v;
      else if (strcmp(name, "position") == 0)
        pNode->position = v;
			else if (strcmp(name, "diffuse_color") == 0)
				pNode->diffuse = v;
			else if (strcmp(name, "specular_color") == 0)
				pNode->specular = v;
			else
      {
        // normalize, in case it is not
        double norm = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        if (norm > 0.0 && norm != 1.0)
        {
          v[0] /= norm;
          v[1] /= norm;
          v[2] /= norm;
        }
        pNode->normal = v;
      }
    }
    else
    {
      fprintf(stderr, "Invalid object type on line number %d\n", line);
      fclose(json);
      exit(1);
    }

    skip_ws(json);
    c = next_c(json);

    // not every line ends with ',' ... just ignore ','
    if (c == ',')
    {
      skip_ws(json);
      c = next_c(json);
    }

  }
	//printf("loaded\n");
}


// read_scene() returns the scene from the file, and the number of objects
void read_scene(const char *filename, int *n, node *scene)
{
	*n = 0;
	FILE *json = fopen(filename, "rt");
	if (json == NULL)
	{
		fprintf(stderr, "Json file %s not found\n", filename);
		exit(1);
	}

  skip_ws(json);
	int c = next_c(json);
	
	if (c != '[') 
	{
		fprintf(stderr, "Invalid scene. It should start with '[' token\n");
		fclose(json);		
		exit(1);
	} 
  skip_ws(json);
  c = next_c(json);
  while (c != ']')
	{
		if (c == '{') 
		{
			ungetc(c, json);
			next_object(json, &scene[*n]);
			*n = *n + 1;
		}
		else
		{
			fprintf(stderr, "Invalid scene. Expected object starting with character '{' instead of '%c'\n", c);
			fclose(json);		
      exit(1);
		}
			
		skip_ws(json);
		c = next_c(json);
    // not every object ends with ',' ... just ignore ','
    if (c == ',')
    {
      skip_ws(json);
      c = next_c(json);
    }
  }
	fclose(json);
}
 
 
