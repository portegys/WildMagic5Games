#ifndef VECTOR3D__H
#define VECTOR3D__H

// vector3d structure
struct vector3d
{
   float x, y, z;
};

//allocate matrix (m x n)
struct vector3d *vector3d_alloc(float x, float y, float z);
void vector3d_free(struct vector3d *);

//get length of a 3d vector sqrt(x^2+y^2+y^2)
float vector3d_length(struct vector3d *vector);

//convert vector to a vector with same direction and modulus 1
void vector3d_normalize(struct vector3d *vector);

//calcuate vector dot-product  c = a . b
float vector3d_dot(struct vector3d *a, struct vector3d *b);

//calcuate vector cross-product  c = a x b
void vector3d_cross(struct vector3d *a, struct vector3d *b, struct vector3d *c);

//calcuate vector scalar-product  n = s x a
void vector3d_scale(float s, struct vector3d *a, struct vector3d *b);

//calcuate vector sum   c = a + b
void vector3d_add(struct vector3d *a, struct vector3d *b, struct vector3d *c);

//copy a to b
void vector3d_copy(struct vector3d *a, struct vector3d *b);

#endif
