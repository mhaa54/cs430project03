#ifndef _JSON_h_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#pragma warning(disable:4996)
#endif

// json node
typedef struct node
{
	char *type;
	double radius;
	double width;
	double height;
	double *color;
	double *position;
	double *normal;
	double theta;
	double angular;	// a0, only for light
	double radial[3];	// a0, a1, a2 ... only for light
	double *diffuse;
	double *specular;
	double ns;	// shininness
} node;

// next_c() wraps the getc() function and provides error checking and line
// number maintenance
int next_c(FILE* json);

// expect_c() checks that the next character is d. If it is not it emits an error.
void expect_c(FILE* json, int d);

// skip_ws() skips white space in the file.
void skip_ws(FILE* json);

// next_string() gets the next string from the file handle and emits an error
// if a string can not be obtained.
char* next_string(FILE* json);

// next_number(f) returns the next floating point value on the json file
double next_number(FILE* json);

// next_vector(f) returns the next 3D vector on the json file
double* next_vector(FILE* json);

// next_object(f,node) returns the next 3D node record on the json file f
void next_object(FILE *json, node *pNode);
 
// read_scene() returns the scene from the file, and the number of objects
void read_scene(const char *filename, int *n, node *scene);
 
#endif
