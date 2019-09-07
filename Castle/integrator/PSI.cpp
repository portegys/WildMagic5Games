#include <stdlib.h>
#include <math.h>
#include "PSI.h"

//function [new_Psi] = PSI(psi)
void PSI(struct matrix *psi, struct matrix *Psi)
{
   int i;

   //first-time allocation of static matrices.
   //restricts to serial execution!
   static struct matrix *psi_cross;
   static struct matrix *eye;
   static struct matrix *psit;
   static struct matrix *psix;
   static int           first = 1;

   if (first == 1)
   {
      first     = 0;
      psi_cross = matrix_alloc(3, 3);
      eye       = matrix_identity_alloc(3);
      psit      = matrix_alloc(3, 1);
      psix      = matrix_alloc(3, 3);
   }

   //%define psi_cross
   //psi=double(psi);
   //psi_cross=double([0,-psi(3),psi(2);psi(3),0,-psi(1);-psi(2),psi(1),0]);
   psi_cross->e[1] = -psi->e[2];
   psi_cross->e[2] = psi->e[1];
   psi_cross->e[3] = psi->e[2];
   psi_cross->e[5] = -psi->e[0];
   psi_cross->e[6] = -psi->e[1];
   psi_cross->e[7] = psi->e[0];

   //%if the absolute value of psi is too small, psi=eye(3)
   float dx      = psi->e[0];
   float dy      = psi->e[1];
   float dz      = psi->e[2];
   float normpsi = (float)sqrt((dx * dx) + (dy * dy) + (dz * dz));
   for (i = 0; i < 3; i++)
   {
      eye->e[i * 3 + i] = 1.0f;
   }
   //if norm(double(psi))<1e-8
   if (normpsi < 1e-8)
   {
      //new_Psi=eye(3);
      matrix_copy(eye, Psi);
   }
   //else
   else
   {
      //%else calculate new_psi
      //new_Psi=cos(norm(psi))*eye(3)+(1-cos(norm(psi)))*(psi*psi')/((norm(psi))^2)-sin(norm(psi))*psi_cross/(norm(psi));
      int i;
      matrix_transpose(psi, psit);

      //A=cos(norm(psi))*eye(3) +
      float c = (float)cos(normpsi);
      for (i = 0; i < 9; i++)
      {
         eye->e[i] *= c;
      }

      //B=(1 - cos(norm(psi)))*(psi*psit) / ((norm(psi)) ^ 2) -
      c = (1.0f - c) / (normpsi * normpsi);
      matrix_multiply(psit, psi, psix);
      for (i = 0; i < 9; i++)
      {
         psix->e[i] *= c;
      }

      //C=sin(norm(psi))*psi_cross / (norm(psi))
      float s = (float)sin(normpsi) / normpsi;
      for (i = 0; i < 9; i++)
      {
         psi_cross->e[i] *= s;
      }

      // Psi=A+B-C
      for (i = 0; i < 9; i++)
      {
         Psi->e[i] = eye->e[i] + psix->e[i] - psi_cross->e[i];
      }
   }
}
