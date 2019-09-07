//***************************************************************************//
//* File Name: simp_particle.cpp                                            *//
//*    Author: Chris McBride chris_a_mcbride@hotmail.com                    *//
//* Date Made: 04/06/02                                                     *//
//* File Desc: Class implementation for a base                              *//
//*            particle class for use in any particle system                *//
//* Rev. Date: xx/xx/02                                                     *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//

#include "particle.hpp"

// Default particle constructor.
cParticle::cParticle()
{
   itsVelocity = Vector3f::ZERO;
   itsPosition = Vector3f::ZERO;

   itsSize   = 1.0f;
   itsWeight = 1.0f;
   itsLife   = 1.0f;
   itsFade   = 0.1f;

   //r = rand()%1000 * 0.001;
   //g = rand()%1000 * 0.001;
   //b = rand()%1000 * 0.001;
   a = itsLife;
}


// Update the particles attributes
void cParticle::UpdatePosition(float step, Vector3f& grav)
{
   itsVelocity += (itsWeight * grav * step);
   itsPosition += (itsVelocity * step);
}


void cParticle::UpdateLife(float step)
{
   itsLife -= (itsFade * step);                   // Should I be using alpha for both?
   a       -= (itsFade * step);                   // Probably not.. if life fades, some
   // particles will not need blending
}


void cParticle::UpdateVelocity(Vector3f& vel)
{
   itsVelocity += vel;
}


// Return the position of the particle into an array
// (use for glVertex3fv calls)
void cParticle::GetPosition(float posArray[3])
{
   posArray[0] = itsPosition.X();
   posArray[1] = itsPosition.Y();
   posArray[2] = itsPosition.Z();
}


// Set the particles velocity by component
void cParticle::SetVelocity(float x, float y, float z)
{
   itsVelocity.X() = x;
   itsVelocity.Y() = y;
   itsVelocity.Z() = z;
}


// Set the particles position by component
void cParticle::SetPosition(float x, float y, float z)
{
   itsPosition.X() = x;
   itsPosition.Y() = y;
   itsPosition.Z() = z;
}


// Return an rgb color for the particle
void cParticle::GetColors3f(float *colorArray) const
{
   colorArray[0] = r;
   colorArray[1] = g;
   colorArray[2] = b;
}


// Set the particles rgb color
void cParticle::SetColors3f(const float *colorArray)
{
   r = colorArray[0];
   g = colorArray[1];
   b = colorArray[2];
}


// Return an rgba color for the particle
void cParticle::GetColors4f(float *colorArray) const
{
   colorArray[0] = r;
   colorArray[1] = g;
   colorArray[2] = b;
   colorArray[3] = a;
}


// Set the rgba color for the particle
void cParticle::SetColors4f(const float *colorArray)
{
   r = colorArray[0];
   g = colorArray[1];
   b = colorArray[2];
   a = colorArray[3];
}
