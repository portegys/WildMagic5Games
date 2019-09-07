//***************************************************************************//
//* File Name: explosion.hpp                                                *//
//*    Author: Chris McBride chris_a_mcbride@hotmail.com                    *//
//* Date Made: 04/06/02                                                     *//
//* File Desc: Class declaration for particle system class                  *//
//*            used to simulate explosions                                  *//
//* Rev. Date: 04/21/02                                                     *//
//* Rev. Desc: random tweaks to update and constructors                     *//
//*                                                                         *//
//***************************************************************************//
#ifndef __EXPLOSION_HPP__
#define __EXPLOSION_HPP__

#include "particle_engine.hpp"

class cExplosion : public cParticleEngine
{
public:

   // Particle size.
   static const float ParticleSize;

   // Particle velocity parameters.
   static const float MaxParticleVelocity;
   static const float MinParticleVelocity;

   cExplosion(Node *objects, Vector3f& location,
              int particle_count, int duration);
   ~cExplosion();

   void Update(float step);
   void Reset();

private:
   void SetupExplosion(int duration);
   void UpdateParticlesSpurt(float step);
   void UpdateParticlesContinuous(float step);
   void SetRandomVelocity(float& oldVelocity);
   void SetVelocityWithRange(float& oldVelocity, float& refValue);

   void DrawParticles() {}
   Node         *m_objects;
   VertexFormat *m_vformat;
   int          m_vstride;
   VertexBuffer *m_vbuffer;
   Float4       *m_positionSizes;
   Particles    *m_particles;
   Node         *m_particlesNode;
   Texture2D    *m_texture;
};
#endif
