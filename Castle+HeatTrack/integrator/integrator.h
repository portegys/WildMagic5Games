#ifndef INTEGRATOR__H
#define INTEGRATOR__H

#include "matrix.h"
#include "vector3d.h"

// Current velocity, position, and rotation
extern struct vector3d *vel_t;
extern struct vector3d *pos_t;
extern struct matrix   *Cbi;

extern "C"
{
	void init_integrator();
	void integrate_movement(struct vector3d *gyr, struct vector3d *acc, struct vector3d *average_stat_acc, int dtime);
}

#endif
