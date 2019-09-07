// Cannonball projectile controller.

#include "CannonBalls.h"
#include "GingerMan.h"

// Maximumu "age" of cannonball: -1.0 = eternal.
const float CannonBalls::MaxAge = 100.0f;

// Terrain collision proximity.
const float CannonBalls::TerrainCollisionProximity = 1.0f;

// Affect of wind on trajectory.
const float CannonBalls::WindFactor = 0.01f;

// Constructor.
CannonBalls::CannonBalls(Node *objects, GingerMenTerrainPtr terrain,
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
   Vector3f        position, cameraDist;
   float           height;

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

   // Purge dead cannonballs.
   if (deadball)
   {
      update = Purge();
   }
   if (update)
   {
      m_objects->Update();
   }
}


// Cannonball collides with bounding sphere?
// Explode colliding cannonball and return its color.
bool CannonBalls::Collides(Vector3f position, float radius, Float3& color)
{
   int             i, j;
   CannonBallState *state;
   bool            result;
   float           distance;
   Vector3f        cameraDist;

   result = false;
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
            result = true;
         }
      }
   }

   // Purge dead cannonballs.
   if (result)
   {
      if (Purge())
      {
         m_objects->Update();
      }
   }

   return(result);
}


// Cannonball collides with body?
// Explode colliding cannonball and return its color.
bool CannonBalls::Collides(GingerMan *body, Float3& color)
{
   int             i, j, p, q;
   CannonBallState *state;
   bool            result;

   vector<CannonBallState *> work;
   float    distance1, distance2, distance3;
   Vector3f cameraDist;

   result = false;
   for (i = 0, j = m_cannonBalls.size(); i < j; i++)
   {
      state = m_cannonBalls[i];
      if (state->m_state == CannonBallState::FLYING)
      {
         Vector3f position = body->GetBody()->WorldTransform.GetTranslate();
         Bound    bound    = body->GetBody()->GetModelBound();
         Vector3f center   = bound.GetCenter();
         distance1 = (position - state->m_ball->GetPosition()).Length();
         distance2 = (position - state->m_ball->GetPreviousPosition()).Length();
         distance3 = bound.GetRadius() + center.Length() + state->m_ball->GetRadius();
         if ((distance1 < distance3) || (distance2 < distance3))
         {
            // Intersection with bounding boxes?
            Matrix3f rotate = body->GetBody()->WorldTransform.GetRotate().Inverse();
            Vector3f start  = state->m_ball->GetPreviousPosition() - position;
            start = rotate * start;
            Vector3f end = state->m_ball->GetPosition() - position;
            end = rotate * end;
            Segment3f segment(start, end);
            bool      intersection = false;
            for (p = 0, q = (int)body->boundingBoxes.size(); p < q; p++)
            {
               IntrSegment3Box3f intersector(segment, body->boundingBoxes[p], false);
               if (intersector.Test())
               {
                  intersection = true;
                  break;
               }
            }
            if (intersection)
            {
               // Explode cannonball.
               color = state->m_ball->GetColor();
               Vector3f ballPosition = state->m_ball->GetPosition();
               m_explosions->Add(m_objects, ballPosition);
               state->m_state = CannonBallState::DEAD;
               cameraDist     = (state->m_ball->GetPosition() - m_spkCamera->GetPosition()) / 100.0f;
               SMSPlaySound(SMSExplosionSound, cameraDist, (float *)&Vector3f::ZERO);
               result = true;
            }
         }
      }
   }

   // Purge dead cannonballs.
   if (result)
   {
      if (Purge())
      {
         m_objects->Update();
      }
   }

   return(result);
}


// Purge dead cannonballs.
bool CannonBalls::Purge()
{
   int             i, j;
   CannonBallState *state;
   bool            update;

   vector<CannonBallState *> work;

   update = false;
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

   return(update);
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
