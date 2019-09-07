//***************************************************************************//
//* File Name: explosion.cpp                                                *//
//*    Author: Chris McBride chris_a_mcbride@hotmail.com                    *//
//* Date Made: 04/06/02                                                     *//
//* File Desc: Class implementation for particle system class               *//
//*            used to simulate explosions                                  *//
//* Rev. Date: 04/21/02                                                     *//
//* Rev. Desc: random tweaks to update and constructors                     *//
//*                                                                         *//
//***************************************************************************//

#include "explosion.hpp"

// Particle size.
const float cExplosion::ParticleSize = 5.0f;

// Particle velocity parameters.
const float cExplosion::MaxParticleVelocity = 10.0f;
const float cExplosion::MinParticleVelocity = 0.0f;

// Constructor that spawns particles from a given location
cExplosion::cExplosion(Node *objects, Vector3f& location,
                       int particle_count, int duration)
{
   m_objects    = objects;
   itsLocation  = location;
   numParticles = numLiveParticles = particle_count;
   AllocateVertexArrays();
   SetupExplosion(duration);
}


void cExplosion::SetupExplosion(int duration)
{
   cParticle *shard;

   particles.resize(numParticles);
   for (int i = 0; i < numParticles; i++)
   {
      shard = new cParticle();

      SetRandomVelocity(shard->itsVelocity.X());
      SetRandomVelocity(shard->itsVelocity.Y());
      SetRandomVelocity(shard->itsVelocity.Z());

      shard->SetPosition(itsLocation);
      shard->itsSize = ParticleSize;
      shard->itsLife = Mathf::UnitRandom();
      shard->itsFade = shard->itsLife / (float)duration;
      particles[i]   = shard;
   }

   m_vformat = VertexFormat::Create(2,
                                    VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
                                    VertexFormat::AU_TEXCOORD, VertexFormat::AT_FLOAT2, 0);
   m_vstride       = m_vformat->GetStride();
   m_vbuffer       = new0 VertexBuffer(4 * numParticles, m_vstride);
   m_positionSizes = new1<Float4>(numParticles);
   Vector3f position;
   for (int i = 0; i < numParticles; ++i)
   {
      shard    = particles[i];
      position = shard->GetPosition();
      m_positionSizes[i][0] = position.X();
      m_positionSizes[i][1] = position.Y();
      m_positionSizes[i][2] = position.Z();
      m_positionSizes[i][3] = ParticleSize;
   }
   m_particles = new0 Particles(m_vformat, m_vbuffer, sizeof(int),
                                m_positionSizes, 1.0f);
   m_particles->SetNumActive(numParticles);

   // Create texture with transparency.
   const int        xsize = 32, ysize = 32;
   m_texture = new0 Texture2D(Texture::TF_A8R8G8B8, xsize,
                              ysize, 1);
   unsigned char *data = (unsigned char *)m_texture->GetData(0);

   float factor = 1.0f / (xsize * xsize + ysize * ysize);
   for (int y = 0, i = 0; y < ysize; ++y)
   {
      for (int x = 0; x < xsize; ++x)
      {
         data[i++] = 255;
         data[i++] = 255;
         data[i++] = 255;

         // Semitransparent within a disk, dropping off to zero outside the
         // disk.
         int   dx    = 2 * x - xsize;
         int   dy    = 2 * y - ysize;
         float value = factor * (dx * dx + dy * dy);
         if (value < 0.125f)
         {
            value = Mathf::Cos(4.0f * Mathf::PI * value);
         }
         else
         {
            value = 0.0f;
         }
         data[i++] = (unsigned char)(255.0f * value);
      }
   }

   Texture2DEffect *effect = new0 Texture2DEffect(Shader::SF_LINEAR);
   effect->GetAlphaState(0, 0)->BlendEnabled = true;
   m_particles->SetEffectInstance(effect->CreateInstance(m_texture));

   m_particlesNode = new0 Node;
   m_particlesNode->AttachChild(m_particles);
   m_objects->AttachChild(m_particlesNode);
   m_particlesNode->Update();
}


// Assign completely random velocity to particle
void cExplosion::SetRandomVelocity(float& velocity)
{
   velocity = Mathf::SymmetricRandom() *
              Mathf::IntervalRandom(MinParticleVelocity, MaxParticleVelocity);
}


// Assign random velocity with some random offset from base velocity
void cExplosion::SetVelocityWithRange(float& velocity, float& refValue)
{
   velocity = Mathf::SymmetricRandom() + refValue;
}


// Update only live particles in the system
void cExplosion::UpdateParticlesSpurt(float step)
{
   int      active = 0;
   Vector3f position;

   for (int i = 0; i < numParticles; i++)
   {
      if (particles[i]->itsLife > 0.0f)
      {
         particles[i]->UpdatePosition(step, gravity);
         particles[i]->UpdateLife(step);
         position = particles[i]->GetPosition();
         m_positionSizes[i][0] = position.X();
         m_positionSizes[i][1] = position.Y();
         m_positionSizes[i][2] = position.Z();
         active++;
      }
   }
   m_particles->SetNumActive(active);
   m_particles->Update();
}


void cExplosion::UpdateParticlesContinuous(float step)
{
   Vector3f position;

   for (int i = 0; i < numParticles; i++)
   {
      if (particles[i]->itsLife <= 0.0f)
      {
         particles[i]->SetPosition(itsLocation);
         particles[i]->itsLife = 1.0f;

         SetRandomVelocity(particles[i]->itsVelocity.X());
         SetRandomVelocity(particles[i]->itsVelocity.Y());
         SetRandomVelocity(particles[i]->itsVelocity.Z());
      }
      else
      {
         particles[i]->UpdatePosition(step, gravity);
         particles[i]->UpdateLife(step);
      }
      position = particles[i]->GetPosition();
      m_positionSizes[i][0] = position.X();
      m_positionSizes[i][1] = position.Y();
      m_positionSizes[i][2] = position.Z();
   }
   m_particles->SetNumActive(numParticles);
   m_particles->Update();
}


// Update the particle system
void cExplosion::Update(float step)
{
   UpdateParticlesSpurt(step);
   CreateParticles();
}


// Function to reuse allocated memory for new particles
void cExplosion::Reset()
{
   Vector3f position;

   for (int i = 0; i < numParticles; i++)
   {
      particles[i]->SetPosition(itsLocation);
      particles[i]->itsSize = ParticleSize;
      particles[i]->itsLife = Mathf::UnitRandom();
      particles[i]->itsFade = 0.04f;

      SetRandomVelocity(particles[i]->itsVelocity.X());
      SetRandomVelocity(particles[i]->itsVelocity.Y());
      SetRandomVelocity(particles[i]->itsVelocity.Z());

      position = particles[i]->GetPosition();
      m_positionSizes[i][0] = position.X();
      m_positionSizes[i][1] = position.Y();
      m_positionSizes[i][2] = position.Z();
      m_positionSizes[i][3] = ParticleSize;
   }
   numLiveParticles = numParticles;
   m_particles->SetNumActive(numParticles);

   m_particles->Update();
}


// The destructor
cExplosion::~cExplosion()
{
   for (int i = 0; i < numParticles; i++)
   {
      delete particles[i];
   }
   particles.clear();

   DeallocateVertexArrays();

   m_objects->DetachChild(m_particlesNode);
}
