// Integrator.

#include "integrator.h"
#include "PSI.h"
#include <iostream>

float Integrator::DEFAULT_SIGNAL_NOISE_RATIO = 0.0f;
float Integrator::SIGNAL_NOISE_RATIO_DELTA   = 0.5f;

Integrator::Integrator(float x, float y, float z, bool noisy, float signal_noise_ratio)
{
   this->noisy = noisy;
   this->signal_noise_ratio = signal_noise_ratio;
   logfile   = NULL;
   vel_t     = 0;
   pos_t     = 0;
   Cbi       = 0;
   psi       = 0;
   Psi       = 0;
   acc_t     = 0;
   mwork3x3  = 0;
   mwork3x1a = 0;
   mwork3x1b = 0;
   init(x, y, z);
}


void Integrator::init(float x, float y, float z)
{
   clear();
   vel_t     = vector3d_alloc(0.0f, 0.0f, 0.0f);
   pos_t     = vector3d_alloc(x, y, z);
   Cbi       = matrix_identity_alloc(3);
   psi       = matrix_alloc(1, 3);
   Psi       = matrix_alloc(3, 3);
   acc_t     = vector3d_alloc(0.0f, 0.0f, 0.0f);
   mwork3x3  = matrix_alloc(3, 3);
   mwork3x1a = matrix_alloc(3, 1);
   mwork3x1b = matrix_alloc(3, 1);
}


Integrator::~Integrator()
{
   clear();
}


void Integrator::clear()
{
   if (vel_t != 0)
   {
      vector3d_free(vel_t);
   }
   if (pos_t != 0)
   {
      vector3d_free(pos_t);
   }
   if (Cbi != 0)
   {
      matrix_free(Cbi);
   }
   if (psi != 0)
   {
      matrix_free(psi);
   }
   if (Psi != 0)
   {
      matrix_free(Psi);
   }
   if (acc_t != 0)
   {
      vector3d_free(acc_t);
   }
   if (mwork3x3 != 0)
   {
      matrix_free(mwork3x3);
   }
   if (mwork3x1a != 0)
   {
      matrix_free(mwork3x1a);
   }
   if (mwork3x1b != 0)
   {
      matrix_free(mwork3x1b);
   }
}


void Integrator::integrate_movement(struct vector3d *gyr, struct vector3d *acc,
                                    struct vector3d *average_stat_acc, float dtime)
{
   struct vector3d gyr_work;
   struct vector3d acc_work;

   gyr_work.x = gyr->x;
   gyr_work.y = gyr->y;
   gyr_work.z = gyr->z;
   acc_work.x = acc->x;
   acc_work.y = acc->y;
   acc_work.z = acc->z;

   if (noisy && (signal_noise_ratio > 0.0f))
   {
      default_random_engine generator;
      generator.seed((unsigned long)rand());
      gyr_work.x = add_noise(gyr->x, generator);
      gyr_work.x = add_noise(gyr->y, generator);
      gyr_work.x = add_noise(gyr->z, generator);
      acc_work.x = add_noise(acc->x, generator);
      acc_work.y = add_noise(acc->y, generator);
      acc_work.z = add_noise(acc->z, generator);
   }

   FILE *logfp = NULL;
   if (logfile != NULL)
   {
      logfp = fopen(logfile, "a");
      fprintf(logfp, "interator: gyr=%f %f %f, acc=%f %f %f\n", gyr_work.x, gyr_work.y, gyr_work.z, acc_work.x, acc_work.y, acc_work.z);
   }

   //psix=gyrX*dtime;
   //psiy=gyrY*dtime;
   //psiz=gyrZ*dtime;
   float psix = gyr_work.x * dtime;
   float psiy = gyr_work.y * dtime;
   float psiz = gyr_work.z * dtime;

   //psi=[psix;psiy;psiz];
   psi->e[0] = psix;
   psi->e[1] = psiy;
   psi->e[2] = psiz;

   //% CALCULATES ORIENTATION
   //Psi = PSI(psi);
   PSI(psi, Psi);

   //Cbi=Psi*Cbi;
   if (logfp != NULL)
   {
      fprintf(logfp, "Cbi:\n");
      log_matrix(Cbi, logfp);
      fprintf(logfp, "->\n");
   }
   matrix_multiply(Psi, Cbi, mwork3x3);
   matrix_copy(mwork3x3, Cbi);
   if (logfp != NULL)
   {
      log_matrix(Cbi, logfp);
   }

   //% CALCULATES ACCELERATION in global coordinates
   //acc_t=Cbi'*[accX;accY;accZ]-average_stat_acc;
   matrix_transpose(Cbi, mwork3x3);
   mwork3x1a->e[0] = acc_work.x;
   mwork3x1a->e[1] = acc_work.y;
   mwork3x1a->e[2] = acc_work.z;
   matrix_multiply(mwork3x3, mwork3x1a, mwork3x1b);
   acc_t->x = mwork3x1b->e[0] - average_stat_acc->x;
   acc_t->y = mwork3x1b->e[1] - average_stat_acc->y;
   acc_t->z = mwork3x1b->e[2] - average_stat_acc->z;
   if (logfp != NULL)
   {
      fprintf(logfp, "rotated acc=%f %f %f\n", acc_t->x, acc_t->y, acc_t->z);
   }

   //% CALCULATES POSITION
   //pos_t = pos_t + vel_t * dtime;
   if (logfp != NULL)
   {
      fprintf(logfp, "pos=%f %f %f ->", pos_t->x, pos_t->y, pos_t->z);
   }
   pos_t->x = pos_t->x + (vel_t->x + (acc_t->x * dtime * 0.5f)) * dtime;
   pos_t->y = pos_t->y + (vel_t->y + (acc_t->y * dtime * 0.5f)) * dtime;
   pos_t->z = pos_t->z + (vel_t->z + (acc_t->z * dtime * 0.5f)) * dtime;
   if (logfp != NULL)
   {
      fprintf(logfp, " %f %f %f, length=%f\n", pos_t->x, pos_t->y, pos_t->z, vector3d_length(pos_t));
   }

   //% CALCULATES VELOCITY
   //vel_t = vel_t*eta + acc_t * dtime;
   if (logfp != NULL)
   {
      fprintf(logfp, "vel=%f %f %f ->", vel_t->x, vel_t->y, vel_t->z);
   }
   vel_t->x = vel_t->x * eta + acc_t->x * dtime;
   vel_t->y = vel_t->y * eta + acc_t->y * dtime;
   vel_t->z = vel_t->z * eta + acc_t->z * dtime;
   if (logfp != NULL)
   {
      fprintf(logfp, " %f %f %f\n", vel_t->x, vel_t->y, vel_t->z);
      fclose(logfp);
   }
}


// Add noise to signal.
// Formula: signal-to-noise ratio = signal mean / signal stdev
float Integrator::add_noise(float signal, default_random_engine& generator)
{
   if (signal == 0.0f)
   {
      return(0.0f);
   }
   float sign = 1.0f;
   if (signal < 0.0f)
   {
      sign = -1.0f;
   }

   float u = signal * sign;
   normal_distribution<float> signal_dist(u, u / signal_noise_ratio);
   return(signal_dist(generator) * sign);
}


// Get forward vector.
void Integrator::get_forward_vector(struct vector3d *forward)
{
   forward->x = Cbi->e[6];
   forward->y = Cbi->e[7];
   forward->z = Cbi->e[8];
}


// Get up vector.
void Integrator::get_up_vector(struct vector3d *up)
{
   up->x = Cbi->e[3];
   up->y = Cbi->e[4];
   up->z = Cbi->e[5];
}


// Get right vector.
void Integrator::get_right_vector(struct vector3d *right)
{
   right->x = Cbi->e[0];
   right->y = Cbi->e[1];
   right->z = Cbi->e[2];
}


// Get target distance.
float Integrator::get_target_distance(float device_to_target_distance)
{
   struct vector3d forward;

   get_forward_vector(&forward);
   vector3d_normalize(&forward);
   vector3d_scale(device_to_target_distance, &forward, &forward);
   struct vector3d target;
   vector3d_add(pos_t, &forward, &target);
   return(vector3d_length(&target));
}


float Integrator::signal_noise_decrease()
{
   signal_noise_ratio -= SIGNAL_NOISE_RATIO_DELTA;
   if (signal_noise_ratio < 0.0f)
   {
      signal_noise_ratio = 0.0f;
   }
   return(signal_noise_ratio);
}


float Integrator::signal_noise_increase()
{
   signal_noise_ratio += SIGNAL_NOISE_RATIO_DELTA;
   return(signal_noise_ratio);
}


//log matrix
void Integrator::log_matrix(struct matrix *A, FILE *logfp)
{
   int i, j;
   int m = A->r;
   int n = A->c;

   for (i = 0; i < m; i++)
   {
      fprintf(logfp, "{");
      for (j = 0; j < n; j++)
      {
         if (j > 0)
         {
            fprintf(logfp, ",");
         }
         fprintf(logfp, "%f", (double)(A->e[i * n + j]));
      }
      fprintf(logfp, "},\n");
   }
}


//eta = 1.0; % NEEDS TUNING (Should be <1, perhapse 0.95)
float Integrator::eta = 1.0f;   //% NEEDS TUNING (Should be <1, perhapse 0.95)
