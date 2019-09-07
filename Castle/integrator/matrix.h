#ifndef MATRIX_H
#define MATRIX_H

// matrix structure
struct matrix
{
   int   r;   // rows
   int   c;   // columns
   float *e;  // elements
};

//allocate matrix (m x n)
struct matrix *matrix_alloc(int m, int n);
void matrix_free(struct matrix *);

//allocate identity matrix (n)
struct matrix *matrix_identity_alloc(int n);

//transpose matrix A (m x n)  to  B (n x m)
void matrix_transpose(struct matrix *A, struct matrix *B);

//multiply matrix A (m x p) by  B(p x n) , put result in C (m x n)
void matrix_multiply(struct matrix *A, struct matrix *B, struct matrix *C);

//copy matrix A to B
void matrix_copy(struct matrix *A, struct matrix *B);

//print matrix
void matrix_print(struct matrix *A);

#endif
