//***************************************************************************//
//* File Name: simp_particle.hpp                                            *//
//*    Author: Chris McBride chris_a_mcbride@hotmail.com                    *//
//* Date Made: 04/06/02                                                     *//
//* File Desc: Class declaration for a base                                 *//
//*            particle class for use in any particle system                *//
//* Rev. Date: xx/xx/02                                                     *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//
#ifndef __PARTICLE_HPP__
#define __PARTICLE_HPP__

#include "Wm5WindowApplication3.h"
using namespace Wm5;

class cParticle
{
public:

   void GetColors3f(float *colorArray) const;
   void SetColors3f(const float *colorArray);

   void GetColors4f(float *colorArray) const;
   void SetColors4f(const float *colorArray);

   void      GetPosition(float posArray[3]);
   Vector3f& GetPosition() { return(itsPosition); }

   void SetVelocity(Vector3f& vel) { itsVelocity = vel; }
   void SetVelocity(float x, float y, float z);

   void SetPosition(Vector3f& pos) { itsPosition = pos; }
   void SetPosition(float x, float y, float z);

   void UpdatePosition(float step, Vector3f& grav);
   void UpdateVelocity(Vector3f& vel);
   void UpdateLife(float step);

protected:                                        // allow only derived classes direct access to protected members
   cParticle();
   virtual ~cParticle() {       }

   Vector3f itsPosition;                          // Position of the particle
   Vector3f itsVelocity;                          // Initial x, y, and z movement
   float    r, g, b, a;                           // Color of the particle
   float    itsSize;                              // Size of the particle (may change over time)
   float    itsWeight;                            // Weight of particle (may change as size changes)
   float    itsLife;                              // Time for the particle to be 'alive'
   float    itsFade;                              // How fast the particle dies

   friend class cParticleEngine;
   friend class cExplosion;
};
#endif
