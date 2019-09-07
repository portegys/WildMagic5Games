#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

//allocate matrix (m x n)
struct matrix *matrix_alloc(int m, int n)
{
   struct matrix *mx;

   mx    = (struct matrix *)malloc(sizeof(struct matrix));
   mx->r = m;
   mx->c = n;
   int j = m * n;
   mx->e = (float *)malloc(j * sizeof(float));
   int i;
   for (i = 0; i < j; i++)
   {
      mx->e[i] = 0.0f;
   }
   return(mx);
}


void matrix_free(struct matrix *mx)
{
   free(mx->e);
   free(mx);
}


//allocate identity matrix (n)
struct matrix *matrix_identity_alloc(int n)
{
   struct matrix *mx = matrix_alloc(n, n);
   int           i;

   for (i = 0; i < n; i++)
   {
      mx->e[i * n + i] = 1.0f;
   }
   return(mx);
}


//transpose matrix A (m x n)  to  B (n x m)
void matrix_transpose(struct matrix *A, struct matrix *B)
{
   int i, j;
   int m = A->r;
   int n = A->c;

   B->r = A->c;
   B->c = A->r;
   for (i = 0; i < m; i++)
   {
      for (j = 0; j < n; j++)
      {
         B->e[j * m + i] = A->e[i * n + j];
      }
   }
}


//multiply matrix A (m x p) by  B(p x n) , put result in C (m x n)
void matrix_multiply(struct matrix *A, struct matrix *B, struct matrix *C)
{
   int i, j, k;
   int m = A->r;
   int n = B->c;
   int p = A->c;

   C->r = m;
   C->c = n;
   for (i = 0; i < m; i++)              //each row in A
   {
      for (j = 0; j < n; j++)           //each column in B
      {
         C->e[i * n + j] = 0;
         for (k = 0; k < p; k++)         //each element in row A & column B
         {
            C->e[i * n + j] += A->e[i * p + k] * B->e[k * n + j];
         }
      }
   }
}


//copy matrix A to B
void matrix_copy(struct matrix *A, struct matrix *B)
{
   int i, j;
   int m = A->r;
   int n = A->c;

   B->r = m;
   B->c = n;
   for (i = 0; i < m; i++)
   {
      for (j = 0; j < n; j++)
      {
         B->e[i * n + j] = A->e[i * n + j];
      }
   }
}


//print matrix
void matrix_print(struct matrix *A)
{
   int i, j;
   int m = A->r;
   int n = A->c;

   for (i = 0; i < m; i++)
   {
      printf("{");
      for (j = 0; j < n; j++)
      {
         if (j > 0)
         {
            printf(",");
         }
         printf("%f", (double)(A->e[i * n + j]));
      }
      printf("},\n");
   }
}
