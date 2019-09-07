// Rigid block.

#ifndef RIGID_BLOCK_H
#define RIGID_BLOCK_H

#include "Wm5WindowApplication3.h"
using namespace Wm5;

class RigidBlock : public RigidBodyf
{
public:
   RigidBlock(Vector3f dimensions, Float3 color, Light *light);

   TriMeshPtr& Mesh() { return(m_spkMesh); }
   Vector3f GetDimensions() const { return(m_dimensions); }
   Float3 GetColor() { return(m_color); }
   void SetColor(Float3 color, Light *light);

   bool Moved;

   // get rigid body state
   float GetMass() { return(mMass); }
   float GetInvMass() { return(mInvMass); }
   Matrix3<float> GetBodyInertia() { return(mInertia); }
   Matrix3<float> GetBodyInvInertia() { return(mInvInertia); }
   Vector3<float> GetPosition() { return(mPosition); }
   Quaternion<float> GetQOrientation() { return(mQuatOrient); }
   Vector3<float> GetLinearMomentum() { return(mLinearMomentum); }
   Vector3<float> GetAngularMomentum() { return(mAngularMomentum); }
   Matrix3<float> GetROrientation() { return(mRotOrient); }
   Vector3<float> GetLinearVelocity() { return(mLinearVelocity); }
   Vector3<float> GetAngularVelocity() { return(mAngularVelocity); }
   Transform GetWorldTransform() { return(m_spkMesh->GetWorldTransform()); }

   // Some extra setters.
   void SetInvMass(float mass) { mInvMass = mass; }
   void SetBodyInvInertia(Matrix3<float>& m) { mInvInertia = m; }

private:
   TriMeshPtr  m_spkMesh;
   Vector3f    m_dimensions;
   Float3      m_color;
   MaterialPtr m_spkMaterial;
};
#endif
