#include <stdlib.h>
#include <math.h>
#include "vector3d.h"

//allocate matrix (m x n)
struct vector3d *vector3d_alloc(float x, float y, float z)
{
   struct vector3d *v;

   v    = (struct vector3d *)malloc(sizeof(struct vector3d));
   v->x = x;
   v->y = y;
   v->z = z;
   return(v);
}

void vector3d_free(struct vector3d *v)
{
	free(v);
}


//get modulus of a 3d vector sqrt(x^2+y^2+y^2)
float vector3d_modulus(struct vector3d *vector)
{
   float R;

   R  = vector->x * vector->x;
   R += vector->y * vector->y;
   R += vector->z * vector->z;
   return((float)sqrt(R));
}


//convert vector to a vector with same direction and modulus 1
void vector3d_normalize(struct vector3d *vector)
{
   float R;

   R          = vector3d_modulus(vector);
   vector->x /= R;
   vector->y /= R;
   vector->z /= R;
}


//calcuate vector dot-product  c = a . b
float vector3d_dot(struct vector3d *a, struct vector3d *b)
{
   return(a->x * b->x + a->y * b->y + a->z * b->z);
}


//calcuate vector cross-product  c = a x b
void vector3d_cross(struct vector3d *a, struct vector3d *b, struct vector3d *c)
{
   c->x = a->y * b->z - a->z * b->y;
   c->y = a->z * b->x - a->x * b->z;
   c->z = a->x * b->y - a->y * b->x;
}


//calcuate vector scalar-product  n = s x a
void vector3d_scale(float s, struct vector3d *a, struct vector3d *b)
{
   b->x = s * a->x;
   b->y = s * a->y;
   b->z = s * a->z;
}


//calcuate vector sum   c = a + b
void vector3d_add(struct vector3d *a, struct vector3d *b, struct vector3d *c)
{
   c->x = a->x + b->x;
   c->y = a->y + b->y;
   c->z = a->z + b->z;
}


//copy a to b
void vector3d_copy(struct vector3d *a, struct vector3d *b)
{
   b->x = a->x;
   b->y = a->y;
   b->z = a->z;
}
