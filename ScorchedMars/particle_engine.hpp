//***************************************************************************//
//* File Name: simp_particle_engine.hpp                                     *//
//*    Author: Chris McBride chris_a_mcbride@hotmail.com                    *//
//* Date Made: 04/06/02                                                     *//
//* File Desc: Class declaration for a base particle engine                 *//
//* Rev. Date: 04/16/02                                                     *//
//* Rev. Desc: Moved drawing & building code into base class, put in        *//
//*            billboarding, went to glDrawElements instead of glDrawArrays *//
//***************************************************************************//
#ifndef __PARTICLE_ENGINE_HPP__
#define __PARTICLE_ENGINE_HPP__

#ifdef WIN32
#include <windows.h>
#endif
#include "particle.hpp"
#include <vector>
#include <GL/gl.h>
#include <GL/glu.h>
using namespace std;

class cParticleEngine
{
public:
   cParticleEngine();                             // Create and initialize particles for system
   virtual ~cParticleEngine()                     // Free all memory for the system
   {
   }


   virtual void DrawParticles() = 0;              // Cycle through particle collection & draw

   // Cycle through particle collection & update
   virtual void UpdateParticlesSpurt(float step);

   // Cycle through particle collection & update
   virtual void UpdateParticlesContinuous(float step);
   virtual void CreateParticles();

   // Build the particle from its position
   virtual void BuildParticle(float *vertexArray, int p, float *matrix);

   //virtual void BuildParticleColors(float* colorArray, int p);
   virtual void Go(float step) {       }
   virtual void Reset() {       }

   int GetNumLiveParticles() { return(numLiveParticles); }
   void KillParticles() { numLiveParticles = 0; }
   void SetDeviation(float xDev, float yDev, float zDev);

   void SetDeviation(Vector3f& dev) { m_deviation = dev; }
   void SetGravity(Vector3f& grav) { gravity = grav; }
   void SetGravity(float xi, float yi, float zi);

   void SetLocation(Vector3f loc) { itsLocation = loc; }
   void SetLocation(float xi, float yi, float zi);
   void SetNumParticles(int num);
   void AllocateVertexArrays();
   void DeallocateVertexArrays();

   void ResetEngine();

protected:
   int numParticles;                              // Number of particles in system
   int numLiveParticles;

   GLuint *pIndices;
   float  *pVertices;                             // Storage for particle vertices
   float  *pTexCoords;                            // Storage for particle texture coords
   //float*  pColors;                        // Storage for particle color

   Vector3f            m_deviation;               // Allowed deviation from emitter position
   Vector3f            gravity;                   // Force of the system on all particles
   Vector3f            itsLocation;               // Base location of the particles spawning
   vector<cParticle *> particles;                 // The collection of particles
};
#endif
