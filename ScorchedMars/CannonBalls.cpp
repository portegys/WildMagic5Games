// Scorched Mars cannonball projectile controller.

#include "CannonBalls.h"

// Maximumu "age" of cannonball: -1.0 = eternal.
const float CannonBalls::MaxAge = 100.0f;

// Terrain collision proximity.
const float CannonBalls::TerrainCollisionProximity = 1.0f;

// Affect of wind on trajectory.
const float CannonBalls::WindFactor = 0.01f;

// Constructor.
CannonBalls::CannonBalls(Node *objects, ScorchedMarsTerrainPtr terrain,
                         Vector3f *wind, ExplosionController *explosions,
                         Camera *camera)
{
   m_objects    = objects;
   m_terrain    = terrain;
   m_wind       = wind;
   m_explosions = explosions;
   m_spkCamera  = camera;
}


// Destructor.
CannonBalls::~CannonBalls()
{
   int i, j;

   for (i = 0, j = m_cannonBalls.size(); i < j; i++)
   {
      delete0(m_cannonBalls[i]);
   }
   m_cannonBalls.clear();
}


// Add a cannonball.
void CannonBalls::Add(RigidBall *cannonBall)
{
   CannonBallState *state = new0 CannonBallState(cannonBall);

   m_cannonBalls.push_back(state);
   m_objects->AttachChild(state->m_node);
   state->m_node->Update();
}


// Update cannonballs.
void CannonBalls::Update(float simTime, float simDelta)
{
   int             i, j;
   CannonBallState *state;
   bool            update, deadball;

   vector<CannonBallState *> work;
   Vector3f position, cameraDist;
   float    height;

   update = deadball = false;
   for (i = 0, j = m_cannonBalls.size(); i < j; i++)
   {
      // "Age" cannonball.
      state = m_cannonBalls[i];
      if (state->m_state == CannonBallState::FLYING)
      {
         state->m_time += simDelta;
         if ((state->m_time > MaxAge) && (MaxAge >= 0.0f))
         {
            state->m_state = CannonBallState::DEAD;
         }
      }
      switch (state->m_state)
      {
      case CannonBallState::FLYING:
         state->m_ball->SetLinearMomentum(state->m_ball->GetLinearMomentum() + (*m_wind * WindFactor));
         state->m_ball->Update(simTime, simDelta);
         state->m_node->LocalTransform.SetTranslate(state->m_ball->GetPosition());
         update = true;

         // Check for collisions with terrain.
         position = state->m_ball->GetPosition();
         height   = position.Z() - m_terrain->GetHeight(position.X(), position.Y());
         if (height <= TerrainCollisionProximity)
         {
            // Explode cannonball.
            m_explosions->Add(m_objects, position);
            state->m_state = CannonBallState::DEAD;
            cameraDist     = (position - m_spkCamera->GetPosition()) / 100.0f;
            SMSPlaySound(SMSExplosionSound, cameraDist, (float *)&Vector3f::ZERO);
         }
         break;

      case CannonBallState::DEAD:
         deadball = true;
         break;
      }
   }

   // Remove dead cannonballs.
   if (deadball)
   {
      work.clear();
      for (i = 0, j = m_cannonBalls.size(); i < j; i++)
      {
         state = m_cannonBalls[i];
         if (state->m_state == CannonBallState::DEAD)
         {
            m_objects->DetachChild(state->m_node);
            delete0(state);
            update = true;
         }
         else
         {
            work.push_back(state);
         }
      }
      m_cannonBalls.clear();
      for (i = 0, j = work.size(); i < j; i++)
      {
         m_cannonBalls.push_back(work[i]);
      }
   }
   if (update)
   {
      m_objects->Update();
   }
}


// Bounding sphere collides with a cannonball?
// Explode colliding cannonball and return its color.
bool CannonBalls::Collides(Vector3f position, float radius, Float3& color)
{
   int             i, j;
   CannonBallState *state;
   bool            ret;

   vector<CannonBallState *> work;
   float    distance;
   Vector3f cameraDist;

   ret = false;
   for (i = 0, j = m_cannonBalls.size(); i < j; i++)
   {
      state = m_cannonBalls[i];
      if (state->m_state == CannonBallState::FLYING)
      {
         distance = (position - state->m_ball->GetPosition()).Length();
         if (distance < (radius + state->m_ball->GetRadius()))
         {
            // Explode cannonball.
            color = state->m_ball->GetColor();
            Vector3f ballPosition = state->m_ball->GetPosition();
            m_explosions->Add(m_objects, ballPosition);
            state->m_state = CannonBallState::DEAD;
            cameraDist     = (state->m_ball->GetPosition() - m_spkCamera->GetPosition()) / 100.0f;
            SMSPlaySound(SMSExplosionSound, cameraDist, (float *)&Vector3f::ZERO);
            ret = true;
         }
      }
   }

   // Remove dead cannonballs.
   if (ret)
   {
      work.clear();
      for (i = 0, j = m_cannonBalls.size(); i < j; i++)
      {
         state = m_cannonBalls[i];
         if (state->m_state == CannonBallState::DEAD)
         {
            m_objects->DetachChild(state->m_node);
            delete0(state);
         }
         else
         {
            work.push_back(state);
         }
      }
      m_cannonBalls.clear();
      for (i = 0, j = work.size(); i < j; i++)
      {
         m_cannonBalls.push_back(work[i]);
      }
      m_objects->Update();
   }
   return(ret);
}


// Clear cannonballs.
void CannonBalls::Clear()
{
   int             i, j;
   CannonBallState *state;

   for (i = 0, j = m_cannonBalls.size(); i < j; i++)
   {
      state = m_cannonBalls[i];
      m_objects->DetachChild(state->m_node);
      delete0(state);
   }
   m_cannonBalls.clear();
   m_objects->Update();
}
