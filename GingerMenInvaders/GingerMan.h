// Gingerbread man.

#ifndef GINGER_MAN_H
#define GINGER_MAN_H

#include "Wm5WindowApplication3.h"
#include "ObjMtl/ObjLoader.h"
#include "GingerMenTerrain.h"
#include "Cannon.h"
#include "CannonBalls.h"
#include "GameState.h"
#include "gettime.h"
using namespace Wm5;

class GingerMan
{
public:

   friend class GingerMother;

   // Scaling factor.
   static float SizeScale;

   // Speed parameters.
   static float MinSpeed, MaxSpeed;

   // Rotation parameters.
   static float MinRotateWait, MaxRotateWait;
   static float MinRotateDelta, MaxRotateDelta;

   // Bomb parameters.
   static int   BombFrequency;
   static float BombRange;

   // Constructor/destructor.
   GingerMan(vector<ObjLoader::Float3>& vertexPositions,
             vector<ObjLoader::Float3>& vertexNormals,
             vector<vector<int> >& vertexIndices,
             vector<MtlLoader::Material> materials,
             float scale, Vector3f position, GingerMenTerrain *terrain,
             Cannon **cannons, CannonBalls *cannonBalls, Light *light);
   ~GingerMan();

   // Update.
   // Return true if man touches terrain.
   bool Update(float speedFactor);

   // Get components.
   NodePtr GetNode() { return(m_node); }
   TriMeshPtr GetBody() { return(m_body); }

   // Position.
   Vector3f GetPosition();
   void SetPosition(Vector3f position);

   // Rotation.
   float GetRotation() { return(m_rotate); }
   void SetRotation(float angle);

   // Speed.
   float GetSpeed() { return(m_speed); }
   void SetSpeed(float speed)
   {
      m_speed = m_nextSpeed = speed;
   }


   // Bounding boxes.
   enum { BOUNDING_BOX_SLICES = 8 };
   vector<Box3f> boundingBoxes;

private:

   vector<TriMesh *> m_meshes;
   TriMesh           *m_body;
   GingerMenTerrain  *m_terrain;
   Cannon            **m_cannons;
   CannonBalls       *m_cannonBalls;
   float             m_scale;
   Light             *m_light;
   NodePtr           m_node;
   float             m_boundRadius;
   Vector3f          m_nextPosition;
   float             m_speed, m_nextSpeed;
   float             m_rotate, m_nextRotate;
   float             m_rotateDelta;
   TIME              m_rotateTimer;
   TIME              m_rotateWait;
   TIME              m_bombTimer;
};
#endif
