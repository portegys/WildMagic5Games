// Cannon.

#ifndef CANNON_H
#define CANNON_H

#include "Wm5WindowApplication3.h"
#include "GingerMenTerrain.h"
#include "RigidBlock.h"
#include "RigidBall.h"
#include "RigidCylinder.h"
#include "SMSound.h"
#include "gettime.h"
using namespace Wm5;
using namespace std;

class Cannon
{
public:

   // Scaling factor.
   static float SizeScale;

   // Hover height.
   static float HeightAboveTerrain;

   // Maximum viewing range.
   static float MaxViewRange;

   // Maximum viewing angle (degrees).
   static float MaxViewAngle;

   // Minimum/maximum directional movement persistence (ms).
   static int MinMovePersistence;
   static int MaxMovePersistence;

   // Automatic firing rate (ms).
   static int AutoFireRate;

   // Constructor/destructor.
   Cannon(Float3 color, Node *scene,
          GingerMenTerrain *terrain, Camera *camera, Light *light);
   ~Cannon();

   // Properties.
   string GetName();

   Float3 GetColor() { return(m_color); }
   void SetColor(Float3 color, Light *light);
   float GetRadius();

   // Get components.
   NodePtr GetBaseNode() { return(m_baseNode); }
   RigidCylinder *GetBaseBody() { return(m_baseBody); }
   NodePtr GetSwivelNode() { return(m_swivelNode); }
   NodePtr GetElevationNode() { return(m_elevationNode); }
   RigidBall *GetTurretBody() { return(m_turretBody); }
   NodePtr GetBarrelNode() { return(m_barrelNode); }
   RigidCylinder *GetBarrelBody() { return(m_barrelBody); }

   // Cannon position.
   Vector3f GetPosition();
   void SetPosition(Vector3f position);
   void MoveForward(float distance);
   void MoveBackward(float distance);

   // Aim (rotate) cannon (angles are in degrees).
   void SetSwivel(float angle);
   float GetSwivel();
   void SetElevation(float angle);
   float GetElevation();
   Vector3f GetAimingVector();
   Matrix3f GetRotate();

   // Charge cannon.
   void SetCharge(float charge) { m_charge = charge; }
   float GetCharge() { return(m_charge); }

   // Fire cannon: return cannonball.
   RigidBall *Fire();

   // Do AI for autonomous cannon.
   // Possibly returns fired cannonball.
   RigidBall *DoAI(Cannon *opponent, float movementIncrement, float speedFactor);

   // Is other cannon visible?
   bool IsVisible(Cannon *, float& range, Vector3f& axis, float& angle);

   // Get name of nearest object in given direction.
   string DoPick(Vector3f direction);

   // External forces and torques for cannonballs.
   static Vector3f Force(float fTime, float fMass, const Vector3f& rkPos,
                         const Quaternionf& rkQOrient, const Vector3f& rkLinMom,
                         const Vector3f& rkAngMom, const Matrix3f& rkOrient,
                         const Vector3f& rkLinVel, const Vector3f& rkAngVel);

   static Vector3f Torque(float fTime, float fMass, const Vector3f& rkPos,
                          const Quaternionf& rkQOrient, const Vector3f& rkLinMom,
                          const Vector3f& rkAngMom, const Matrix3f& rkOrient,
                          const Vector3f& rkLinVel, const Vector3f& rkAngVel);

private:
   Node             *m_spkScene;
   GingerMenTerrain *m_spkTerrain;
   Camera           *m_spkCamera;
   NodePtr          m_baseNode;
   RigidCylinder    *m_baseBody;
   NodePtr          m_swivelNode;
   NodePtr          m_elevationNode;
   RigidBall        *m_turretBody;
   NodePtr          m_barrelNode;
   RigidCylinder    *m_barrelBody;
   Float3           m_color;
   Light            *m_light;
   float            m_swivel;
   float            m_elevation;
   float            m_charge;
   TIME             fireTimer;
   TIME             moveTimer;
   float            targetSwivel;
};
#endif
