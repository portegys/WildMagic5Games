// Explosions controller.

#include "explosionController.hpp"

// Duration of explosion (unit steps).
const int ExplosionController::DefaultDuration = 10;

// Default number of particles in explosion.
const int ExplosionController::DefaultParticles = 500;

// Constructor.
ExplosionController::ExplosionController(Node *objects)
{
   m_objects = objects;
}


// Destructor.
ExplosionController::~ExplosionController()
{
   int i, j;

   for (i = 0, j = m_explosions.size(); i < j; i++)
   {
      delete0(m_explosions[i]);
   }
   m_explosions.clear();
}


// Add an explosion.
void ExplosionController::Add(Node *objects, Vector3f& location,
                              int particles, int duration)
{
   cExplosion *explosion =
      new0    cExplosion(objects, location, particles, duration);

   m_explosions.push_back(explosion);
}


// Update explosions.
void ExplosionController::Update(float step)
{
   int i, j;

   vector<cExplosion *> work;

   for (i = 0, j = m_explosions.size(); i < j; i++)
   {
      if (m_explosions[i]->GetNumLiveParticles() > 0)
      {
         m_explosions[i]->Update(step);
      }
   }

   // Remove completed explosions.
   work.clear();
   for (i = 0, j = m_explosions.size(); i < j; i++)
   {
      if (m_explosions[i]->GetNumLiveParticles() > 0)
      {
         work.push_back(m_explosions[i]);
      }
      else
      {
         delete0(m_explosions[i]);
      }
   }
   m_explosions.clear();
   for (i = 0, j = work.size(); i < j; i++)
   {
      m_explosions.push_back(work[i]);
   }
}
