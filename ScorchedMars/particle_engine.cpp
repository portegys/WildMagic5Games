//***************************************************************************//
//* File Name: simp_particle_engine.cpp                                     *//
//*    Author: Chris McBride chris_a_mcbride@hotmail.com                    *//
//* Date Made: 04/06/02                                                     *//
//* File Desc: Class implementationfor a base particle engine               *//
//* Rev. Date: 04/16/02                                                     *//
//* Rev. Desc: Moved drawing & building code into base class, put in        *//
//*            billboarding, went to glDrawElements instead of glDrawArrays *//
//***************************************************************************//

#include "particle_engine.hpp"

cParticleEngine::cParticleEngine()
   : numParticles(0),
     numLiveParticles(0),
     m_deviation(1, 1, 1),
     gravity(0, 0, 0),
     pIndices(NULL),
     pVertices(NULL),
     pTexCoords(NULL)
{
}


// Allocate space for per-vertex particle info
void cParticleEngine::AllocateVertexArrays()
{
   pVertices  = new float[numParticles * 12];     // 3 components per vertex, 4 vertices per quad
   pTexCoords = new float[numParticles * 8];      // 2 tex coords per vertex, 4 vertices per quad
   pIndices   = new GLuint[numParticles * 4];     // 4 indices needed to draw 4 vertices from vertex array
   //pColors    = new float[numParticles*16];
}


// Deallocate all space for per-vertex particle info
void cParticleEngine::DeallocateVertexArrays()
{
   delete [] pIndices;
   delete [] pVertices;
   delete [] pTexCoords;
   //delete [] pColors;
}


// Create representation of all particles in the system
void cParticleEngine::CreateParticles()
{
   float mat[16] = { 0 };

   glGetFloatv(GL_MODELVIEW_MATRIX, mat);
   static int NextIndex = 0;
   numLiveParticles = numParticles;

   for (int k = 0; k < numParticles; k++)
   {
      if (particles[k]->itsLife > 0.0f)
      {
         BuildParticle(pVertices + (k * 12), k, mat);

         pIndices[NextIndex]     = NextIndex;
         pIndices[NextIndex + 1] = NextIndex + 1;
         pIndices[NextIndex + 2] = NextIndex + 2;
         pIndices[NextIndex + 3] = NextIndex + 3;

         NextIndex += 4;
      }
      else
      {
         --numLiveParticles;
      }
   }

   NextIndex = 0;
}


// Use particles single position to build a quad
void cParticleEngine::BuildParticle(float *vertexArray, int p, float *mat)
{
   Vector3f right(mat[0], mat[4], mat[8]);
   Vector3f up(mat[1], mat[5], mat[9]);

   right.Normalize();
   up.Normalize();

   right *= particles[p]->itsSize;
   up    *= particles[p]->itsSize;

   // Bottom Left Corner
   Vector3f temp;
   temp           = particles[p]->itsPosition + (-right - up);
   vertexArray[0] = temp.X();
   vertexArray[1] = temp.Y();
   vertexArray[2] = temp.Z();

   // Bottom Right Corner
   temp           = particles[p]->itsPosition + (right - up);
   vertexArray[3] = temp.X();
   vertexArray[4] = temp.Y();
   vertexArray[5] = temp.Z();

   // Top Right Corner
   temp           = particles[p]->itsPosition + (right + up);
   vertexArray[6] = temp.X();
   vertexArray[7] = temp.Y();
   vertexArray[8] = temp.Z();

   // Top Left Corner
   temp            = particles[p]->itsPosition + (up - right);
   vertexArray[9]  = temp.X();
   vertexArray[10] = temp.Y();
   vertexArray[11] = temp.Z();
}


/*void cParticleEngine::BuildParticleColors(float* colorArray, int p)
 * {
 * particles[p]->GetColors4f(colorArray);
 * particles[p]->GetColors4f(colorArray+4);
 * particles[p]->GetColors4f(colorArray+8);
 * particles[p]->GetColors4f(colorArray+12);
 * }*/
void cParticleEngine::UpdateParticlesSpurt(float step)
{
}


void cParticleEngine::UpdateParticlesContinuous(float step)
{
}


void cParticleEngine::SetNumParticles(int num)
{
   numParticles = numLiveParticles = num;
}


// Set the force acting upon the particles at the system level
void cParticleEngine::SetGravity(float xi, float yi, float zi)
{
   gravity.X() = xi;
   gravity.Y() = yi;
   gravity.Z() = zi;
}


void cParticleEngine::SetLocation(float xi, float yi, float zi)
{
   itsLocation.X() = xi;
   itsLocation.Y() = yi;
   itsLocation.Z() = zi;

   for (int x = 0; x < numParticles; x++)
   {
      particles[x]->SetPosition(xi, yi, zi);
   }
}


void cParticleEngine::SetDeviation(float xDev, float yDev, float zDev)
{
   m_deviation.X() = xDev;
   m_deviation.Y() = yDev;
   m_deviation.Z() = zDev;
}
