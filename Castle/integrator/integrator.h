#ifndef INTEGRATOR__H
#define INTEGRATOR__H

#include "matrix.h"
#include "vector3d.h"
#include <stdio.h>
#include <random>
using namespace std;

class Integrator
{
public:

   // Current velocity, position, and rotation
   struct vector3d *vel_t;
   struct vector3d *pos_t;
   struct matrix   *Cbi;

   Integrator(float x, float y, float z, bool noisy = false, float signal_noise_ratio = DEFAULT_SIGNAL_NOISE_RATIO);
   void init(float x, float y, float z);

   ~Integrator();
   void clear();

   void integrate_movement(struct vector3d *gyr, struct vector3d *acc, struct vector3d *average_stat_acc, float dtime);
   float get_target_distance(float device_to_target_distance);
   void get_forward_vector(struct vector3d *forward);
   void get_up_vector(struct vector3d *up);
   void get_right_vector(struct vector3d *right);

   bool  noisy;
   float signal_noise_ratio;
   float add_noise(float signal, default_random_engine& generator);

   static float DEFAULT_SIGNAL_NOISE_RATIO;
   static float SIGNAL_NOISE_RATIO_DELTA;
   float signal_noise_decrease();
   float signal_noise_increase();

   char *logfile;
   void log_matrix(struct matrix *A, FILE *fp);

   // Work.
   static float    eta;
   struct matrix   *psi;
   struct matrix   *Psi;
   struct vector3d *acc_t;
   struct matrix   *mwork3x3;
   struct matrix   *mwork3x1a;
   struct matrix   *mwork3x1b;
};
#endif
