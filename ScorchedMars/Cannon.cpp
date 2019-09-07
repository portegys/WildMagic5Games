// Scorched Mars cannon.

#include "Cannon.h"

// Scaling factor.
float Cannon::SizeScale = 1.0f;

// Hover height.
float Cannon::HeightAboveTerrain = 20.0f;

// Maximum viewing range.
float Cannon::MaxViewRange = 500.0f;

// Maximum viewing angle (degrees).
float Cannon::MaxViewAngle = 60.0f;

// Minimum/maximum directional movement persistence (ms).
int Cannon::MinMovePersistence = 3000;
int Cannon::MaxMovePersistence = 10000;

// Automatic firing rate (ms).
int Cannon::AutoFireRate = 1000;

//----------------------------------------------------------------------------
Cannon::Cannon(Float3 color, Node *scene,
               ScorchedMarsTerrain *terrain, Camera *camera, Light *light)
{
   m_color      = color;
   m_spkScene   = scene;
   m_spkTerrain = terrain;
   m_spkCamera  = camera;
   m_light      = light;
#ifdef NEVER
   m_baseNode = new0 Node();
   m_baseBody = new0 RigidCylinder(10.0f * SizeScale, 100.0f * SizeScale, false, color, light);
   m_baseNode->AttachChild(m_baseBody->Mesh());
#else
   m_baseNode = new0 Node();
#endif
   m_swivelNode = new0 Node();
#ifdef NEVER
   m_swivelNode->Local.SetTranslate(Vector3f(0.0f, 0.0f, 55.0f * SizeScale));
#endif
   m_baseNode->AttachChild(m_swivelNode);
   m_elevationNode = new0 Node();
   m_swivelNode->AttachChild(m_elevationNode);
   m_turretBody = new0 RigidBall(10.0f * SizeScale, color, light);
   m_elevationNode->AttachChild(m_turretBody->Mesh());
   m_barrelNode = new0 Node();
   m_barrelNode->LocalTransform.SetTranslate(Vector3f(0.0f, 0.0f, 10.0f * SizeScale));
   m_elevationNode->AttachChild(m_barrelNode);
   m_barrelBody = new0 RigidCylinder(2.0f * SizeScale, 20.0f * SizeScale, false, color, light);
   m_barrelNode->AttachChild(m_barrelBody->Mesh());
   m_swivel     = m_elevation = m_charge = 0.0f;
   fireTimer    = 0;
   moveTimer    = 0;
   targetSwivel = 0.0f;
}


//----------------------------------------------------------------------------
Cannon::~Cannon()
{
#ifdef NEVER
   delete0(m_baseBody);
#endif
   delete0(m_turretBody);
   delete0(m_barrelBody);
}


// Get ID.
// This will be the mesh of the turret, since this
// can be used for picking.
string Cannon::GetName()
{
   return(m_turretBody->Mesh()->GetName());
}


// Set color.
void Cannon::SetColor(Float3 color, Light *light)
{
   m_color = color;
   m_light = light;
#ifdef NEVER
   m_baseBody->SetColor(color, light);
#endif
   m_turretBody->SetColor(color, light);
   m_barrelBody->SetColor(color, light);
}


// Get radius (of turret).
float Cannon::GetRadius()
{
   return(GetTurretBody()->GetRadius());
}


// Get position.
Vector3f Cannon::GetPosition()
{
   return(m_baseNode->LocalTransform.GetTranslate());
}


// Set position.
void Cannon::SetPosition(Vector3f position)
{
   m_baseNode->LocalTransform.SetTranslate(position);
   m_baseNode->Update();
}


// Move forward.
void Cannon::MoveForward(float distance)
{
   Vector3f aim = GetAimingVector();

   aim.Z() = 0.0f;
   aim.Normalize();
   Vector3f position = GetPosition();
   position     = position + (aim * distance);
   position.Z() = m_spkTerrain->GetHeight(position.X(), position.Y()) +
                  HeightAboveTerrain;
   SetPosition(position);
}


// Move backward.
void Cannon::MoveBackward(float distance)
{
   MoveForward(-distance);
}


// Set cannon swivel angle in degrees.
void Cannon::SetSwivel(float angle)
{
   m_swivel = angle;
   Quaternionf quat(Vector3f(0.0f, 0.0f, 1.0f), angle * Mathf::DEG_TO_RAD);
   Matrix3f    rot;
   quat.ToRotationMatrix(rot);
   m_swivelNode->LocalTransform.SetRotate(rot);
   m_swivelNode->Update();
}


// Get cannon swivel angle in degrees.
float Cannon::GetSwivel()
{
   return(m_swivel);
}


// Set cannon elevation angle in degrees.
void Cannon::SetElevation(float angle)
{
   m_elevation = angle;
   Quaternionf quat(Vector3f(1.0f, 0.0f, 0.0f), angle * Mathf::DEG_TO_RAD);
   Matrix3f    rot;
   quat.ToRotationMatrix(rot);
   m_elevationNode->LocalTransform.SetRotate(rot);
   m_elevationNode->Update();
}


// Get cannon elevation angle in degrees.
float Cannon::GetElevation()
{
   return(m_elevation);
}


// Get normalized barrel aiming vector.
Vector3f Cannon::GetAimingVector()
{
   Quaternionf quat = Quaternionf(GetRotate());
   Vector3f    aim  = quat.Rotate(Vector3f(0.0f, 0.0f, 1.0f));

   aim.Normalize();
   return(aim);
}


// Get cannon rotation.
Matrix3f Cannon::GetRotate()
{
   return(m_barrelNode->WorldTransform.GetRotate());
}


// Fire cannon: return cannon ball.
RigidBall *Cannon::Fire()
{
   RigidBall   *ball;
   APoint      kPos;
   Vector3f    kLinVel;
   float       fMass, fRadius, f;
   Matrix3f    kInertia;
   const float fDensityConstant = 1.0f;

   fRadius   = 1.0f;
   ball      = new0 RigidBall(fRadius, m_color, m_light);
   fMass     = 4.0f / 3.0f * Mathf::PI * (fRadius * fRadius * fRadius) * fDensityConstant;
   kPos      = m_barrelNode->WorldTransform.GetTranslate();
   kLinVel   = GetAimingVector();
   kPos.X() += kLinVel.X() * 10.0f;                       // position at end of barrel
   kPos.Y() += kLinVel.Y() * 10.0f;
   kPos.Z() += kLinVel.Z() * 10.0f;
   kLinVel  *= m_charge;
   ball->SetMass(fMass);
   f        = (2.0f / 5.0f) * fMass * fRadius * fRadius;
   kInertia = kInertia.MakeDiagonal(f, f, f);
   ball->SetBodyInertia(kInertia);
   ball->SetPosition(kPos);
   ball->SetLinearVelocity(kLinVel);
   ball->mForce  = Force;
   ball->mTorque = Torque;

   // Play firing sound.
   Vector3f dist = m_baseNode->WorldTransform.GetTranslate() - m_spkCamera->GetPosition();
   SMSPlaySound(SMSFireSound, (float *)&dist, (float *)&Vector3f::ZERO);

   return(ball);
}


// Do AI for autonomous cannon.
RigidBall *Cannon::DoAI(Cannon *opponent, float movementIncrement, float speedFactor)
{
   float       range, angle, yaw, pitch, roll, swivel;
   Vector3f    aim, level, axis;
   Matrix3f    rotation;
   RigidBall   *cannonball;
   TIME        t;
   const float rotMaxDelta = 5.0f;

   cannonball = NULL;
   t          = gettime();
   if (IsVisible(opponent, range, axis, angle))
   {
      // Aiming at opponent?
      if (Mathf::FAbs(angle * Mathf::RAD_TO_DEG) < 4.0f)
      {
         // Fire at opponent.
         if ((int)(t - fireTimer) >= AutoFireRate)
         {
            fireTimer  = t;
            cannonball = Fire();
         }
      }
      else
      {
         // Aim at opponent.
         rotation = rotation.MakeRotation(axis, angle);
         rotation.ExtractEulerXYZ(yaw, pitch, roll);
         roll *= Mathf::RAD_TO_DEG;
         if (roll > rotMaxDelta)
         {
            roll = rotMaxDelta;
         }
         if (roll < -rotMaxDelta)
         {
            roll = -rotMaxDelta;
         }
         yaw *= Mathf::RAD_TO_DEG;
         if (yaw > rotMaxDelta)
         {
            yaw = rotMaxDelta;
         }
         if (yaw < -rotMaxDelta)
         {
            yaw = -rotMaxDelta;
         }
         if (Mathf::FAbs(roll) > Mathf::FAbs(yaw))
         {
            SetSwivel((roll * speedFactor) + GetSwivel());
         }
         else
         {
            SetElevation(GetElevation() - (yaw * speedFactor));
         }
      }

      // Move toward opponent at high speed.
      MoveForward(movementIncrement * speedFactor * 1.5f);
      moveTimer = t + MinMovePersistence;
   }
   else
   {
      // Return to level aim.
      SetElevation(90.0f);

      // Time to change direction?
      if (t > moveTimer)
      {
         moveTimer    = t + (TIME) Mathf::IntervalRandom((float)MinMovePersistence, (float)MaxMovePersistence);
         targetSwivel = Mathf::IntervalRandom(0.0f, 180.0f);
      }
      swivel = targetSwivel - GetSwivel();
      if (swivel > rotMaxDelta)
      {
         swivel = rotMaxDelta;
      }
      if (swivel < -rotMaxDelta)
      {
         swivel = -rotMaxDelta;
      }
      SetSwivel(GetSwivel() + (swivel * speedFactor));

      // Move slowly.
      MoveForward(movementIncrement * speedFactor * 0.5f);
   }
   return(cannonball);
}


// Is other cannon visible?
bool Cannon::IsVisible(Cannon *cannon, float& range,
                       Vector3f& axis, float& angle)
{
   // Check range.
   Vector3f pv = GetPosition();
   Vector3f rv = cannon->GetPosition() - pv;

   range = rv.Length();
   if (range > MaxViewRange)
   {
      return(false);
   }

   // Check angle between current direction and cannon.
   Vector3f aim = GetAimingVector();
   Vector3f dir = rv;
   dir.Normalize();
   axis = aim.Cross(dir);
   axis.Normalize();
   angle = Mathf::ACos(aim.Dot(dir));
   if (Mathf::FAbs(angle * Mathf::RAD_TO_DEG) > MaxViewAngle)
   {
      return(false);
   }

   // Concealed by terrain?
   Vector3f pv2 = pv;
   for (int i = 1; (float)i < range; i++)
   {
      pv2 = pv2 + dir;
      if (m_spkTerrain->GetHeight(pv2.X(), pv2.Y()) > pv2.Z())
      {
         return(false);
      }
   }
   return(true);
}


// Get name of nearest object in given direction.
string Cannon::DoPick(Vector3f direction)
{
   Picker picker;
   string name = "";

   picker.Execute(m_spkScene, GetPosition(), direction, 0.0f, Mathf::MAX_REAL);
   if (picker.Records.size() > 0)
   {
      const PickRecord& record  = picker.GetClosestNonnegative();
      const Spatial     *object = record.Intersected;
      name = object->GetName();
   }
   return(name);
}


//----------------------------------------------------------------------------
Vector3f Cannon::Force(float, float fMass, const Vector3f&,
                       const Quaternionf&, const Vector3f&, const Vector3f&, const Matrix3f&,
                       const Vector3f&, const Vector3f&)
{
   const float    fGravityConstant  = 9.81f;      // m/sec/sec
   const Vector3f kGravityDirection = Vector3f(0.0f, 0.0f, -1.0f);

   return((fMass * fGravityConstant) * kGravityDirection);
}


//----------------------------------------------------------------------------
Vector3f Cannon::Torque(float, float, const Vector3f&,
                        const Quaternionf&, const Vector3f&, const Vector3f&, const Matrix3f&,
                        const Vector3f&, const Vector3f&)
{
   return(Vector3f::ZERO);
}
