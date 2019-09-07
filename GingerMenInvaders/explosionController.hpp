// Explosions controller.

#ifndef EXPLOSIONS_H
#define EXPLOSIONS_H

#include "Wm5WindowApplication3.h"
#include "RigidBall.h"
#include "explosion.hpp"
#include "gettime.h"
#include <vector>
using namespace Wm5;
using namespace std;

class ExplosionController
{
public:

   // Default duration of explosion (unit steps).
   static const int DefaultDuration;

   // Default number of particles in explosion.
   static const int DefaultParticles;

   // Constructor/destructor.
   ExplosionController(Node *scene);
   ~ExplosionController();

   // Add an explosion.
   void Add(Node *objects, Vector3f& location,
            int particles = DefaultParticles, int duration = DefaultDuration);

   // Update and draw explosions.
   void Update(float step);

private:

   Node                 *m_objects;
   vector<cExplosion *> m_explosions;
};
#endif
