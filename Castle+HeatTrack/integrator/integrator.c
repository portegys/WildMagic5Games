// Integrator.

#include "PSI.h"

// Current velocity, position, and rotation
struct vector3d *vel_t;
struct vector3d *pos_t;
struct matrix   *Cbi;

// Work.
static struct matrix   *psi;
static struct matrix   *Psi;
static struct vector3d *acc_g;
static struct vector3d *acc_t;
static struct matrix   *mwork3x3;
static struct matrix   *mwork3x1a;
static struct matrix   *mwork3x1b;

//eta = 0.2; % NEEDS TUNING (Should be <1, perhapse 0.95)
static float eta = 0.2f;   //% NEEDS TUNING (Should be <1, perhapse 0.95)

void init_integrator()
{
	if (vel_t != 0) vector3d_free(vel_t);
		vel_t = vector3d_alloc(0.0f, 0.0f, 0.0f);
		if (pos_t != 0) vector3d_free(pos_t);
		pos_t = vector3d_alloc(0.0f, 0.0f, 0.0f);
		if (Cbi != 0) matrix_free(Cbi);
		Cbi = matrix_identity_alloc(3);
		if (psi != 0) matrix_free(psi);
		psi = matrix_alloc(1, 3);
		if (Psi != 0) matrix_free(Psi);
		Psi = matrix_alloc(3, 3);
		if (acc_g != 0) vector3d_free(acc_g);
		acc_g = vector3d_alloc(0.0f, 0.0f, 0.0f);
		if (acc_t != 0) vector3d_free(acc_t);
		acc_t = vector3d_alloc(0.0f, 0.0f, 0.0f);
		if (mwork3x3 != 0) matrix_free(mwork3x3);
		mwork3x3 = matrix_alloc(3, 3);
		if (mwork3x1a != 0) matrix_free(mwork3x1a);
		mwork3x1a = matrix_alloc(3, 1);
		if (mwork3x1b != 0) matrix_free(mwork3x1b);
		mwork3x1b = matrix_alloc(3, 1);
}

void integrate_movement(struct vector3d *gyr, struct vector3d *acc, struct vector3d *average_stat_acc, int dtime)
{
   //psix=gyrX*dtime;
   //psiy=gyrY*dtime;
   //psiz=gyrZ*dtime;
   float psix = gyr->x * (float)dtime;
   float psiy = gyr->y * (float)dtime;
   float psiz = gyr->z * (float)dtime;

   //psi=[psix;psiy;psiz];
   psi->e[0] = psix;
   psi->e[1] = psiy;
   psi->e[2] = psiz;

   //% CALCULATES ORIENTATION
   //Psi = PSI(psi);
   PSI(psi, Psi);

   //Cbi=Psi*Cbi;
   matrix_multiply(Psi, Cbi, mwork3x3);
   matrix_copy(mwork3x3, Cbi);

   //rot{t} = Cbi; % for debugging only (note that this is a cell array)

   //% CALCULATES ACCELERATION in global coordinates
   //acc_g=Cbi'*[accX;accY;accZ]-average_stat_acc;
   matrix_transpose(Cbi, mwork3x3);
   mwork3x1a->e[0] = acc->x;
   mwork3x1a->e[1] = acc->y;
   mwork3x1a->e[2] = acc->z;
   matrix_multiply(mwork3x3, mwork3x1a, mwork3x1b);
   acc_g->x = mwork3x1b->e[0] - average_stat_acc->x;
   acc_g->y = mwork3x1b->e[1] - average_stat_acc->y;
   acc_g->z = mwork3x1b->e[2] - average_stat_acc->z;

   //acc_t = acc_g*9.81; % convert from G to m/s/s
   acc_t->x = acc_g->x * 9.81f;      //% convert from G to m/s/s
   acc_t->y = acc_g->y * 9.81f;
   acc_t->z = acc_g->z * 9.81f;

   //% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   //% THIS SECTION APPLIES A BAND-PASS FILTER TO THE SIGNAL.
   //% !INCOMPLETE!
   //% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   //% CALCULATES VELOCITY
   //vel_t = vel_t*eta + acc_t * dtime;
   vel_t->x = vel_t->x * eta + acc_t->x * (float)dtime;
   vel_t->y = vel_t->y * eta + acc_t->y * (float)dtime;
   vel_t->z = vel_t->z * eta + acc_t->z * (float)dtime;

   //% CALCULATES POSITION
   //pos_t = pos_t + vel_t * dtime;
   pos_t->x = pos_t->x + vel_t->x * (float)dtime;
   pos_t->y = pos_t->y + vel_t->y * (float)dtime;
   pos_t->z = pos_t->z + vel_t->z * (float)dtime;
}
