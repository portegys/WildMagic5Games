// Scorched Mars cannonball projectile controller.

#ifndef CANNONBALLS_H
#define CANNONBALLS_H

#include "Wm5WindowApplication3.h"
#include "ScorchedMarsTerrain.h"
#include "RigidBall.h"
#include "explosionController.hpp"
#include "gettime.h"
#include "SMSound.h"
#include <vector>
using namespace Wm5;
using namespace std;

class CannonBalls
{
public:

   // Maximum "age" of cannonball: -1.0 = eternal.
   static const float MaxAge;

   // Terrain collision proximity.
   static const float TerrainCollisionProximity;

   // Affect of wind on trajectory.
   static const float WindFactor;

   // Constructor/destructor.
   CannonBalls(Node *objects, ScorchedMarsTerrainPtr,
               Vector3f *wind, ExplosionController *, Camera *camera);
   ~CannonBalls();

   // Cannonball state.
   class CannonBallState
   {
public:

      // Constructor.
      CannonBallState(RigidBall *cannonBall)
      {
         m_state = FLYING;
         m_time  = 0.0f;
         m_node  = new0 Node();
         m_ball  = cannonBall;
         m_node->LocalTransform.SetTranslate(m_ball->GetPosition());
         m_node->AttachChild(m_ball->Mesh());
      }


      // Destructor.
      ~CannonBallState()
      {
         if (m_ball != NULL) { delete0(m_ball); }
      }


      enum { FLYING, DEAD }
                m_state;
      float     m_time;
      NodePtr   m_node;
      RigidBall *m_ball;
   };

   // Add a cannonball.
   void Add(RigidBall *);

   // Update cannonball trajectories.
   void Update(float simTime, float simDelta);

   // Bounding sphere collides with a cannonball?
   // Explode colliding cannonball and return its color.
   bool Collides(Vector3f position, float radius, Float3& color);

   // Clear cannonballs.
   void Clear();

private:

   Node *m_objects;
   ScorchedMarsTerrainPtr    m_terrain;
   Vector3f                  *m_wind;
   ExplosionController       *m_explosions;
   Camera                    *m_spkCamera;
   vector<CannonBallState *> m_cannonBalls;
};
#endif
