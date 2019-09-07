// Gingerbread man.

#include "GingerMan.h"

// Scaling factor.
float GingerMan::SizeScale = 5.0f;

// Speed parameters.
float GingerMan::MinSpeed = 0.1f;
float GingerMan::MaxSpeed = 1.0f;

// Rotation parameters.
float GingerMan::MinRotateWait  = 3000.0f;
float GingerMan::MaxRotateWait  = 6000.0f;
float GingerMan::MinRotateDelta = 1.0f;
float GingerMan::MaxRotateDelta = 5.0f;

// Bomb parameters.
int GingerMan::  BombFrequency = 5;
float GingerMan::BombRange     = 100.0f;

// Constructor.
GingerMan::GingerMan(vector<ObjLoader::Float3>& vertexPositions,
                     vector<ObjLoader::Float3>& vertexNormals,
                     vector<vector<int> >& vertexIndices,
                     vector<MtlLoader::Material> materials,
                     float scale, Vector3f position, GingerMenTerrain *terrain,
                     Cannon **cannons, CannonBalls *cannonBalls, Light *light)
{
   int i, j, x, z;

   m_terrain     = terrain;
   m_cannons     = cannons;
   m_cannonBalls = cannonBalls;
   m_scale       = scale;
   m_light       = light;

   // Create node meshes and bounding boxes.
   m_node = new0 Node();
   VertexFormat  *vformat = VertexFormat::Create(2,
                                                 VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
                                                 VertexFormat::AU_NORMAL, VertexFormat::AT_FLOAT3, 0);
   int                  vstride     = vformat->GetStride();
   int                  numVertices = vertexPositions.size();
   VertexBuffer         *vbuffer    = new0 VertexBuffer(numVertices, vstride);
   VertexBufferAccessor vba(vformat, vbuffer);
   float                xmin, xmax, ymin, ymax, zmin, zmax;
   for (i = 0; i < numVertices; i++)
   {
      Vector3f v;
      v.X() = vertexPositions[i].x * scale;
      v.Y() = vertexPositions[i].y * scale;
      v.Z() = vertexPositions[i].z * scale;
      if ((i == 0) || (v.X() < xmin))
      {
         xmin = v.X();
      }
      if ((i == 0) || (v.Y() < ymin))
      {
         ymin = v.Y();
      }
      if ((i == 0) || (v.Z() < zmin))
      {
         zmin = v.Z();
      }
      if ((i == 0) || (v.X() > xmax))
      {
         xmax = v.X();
      }
      if ((i == 0) || (v.Y() > ymax))
      {
         ymax = v.Y();
      }
      if ((i == 0) || (v.Z() > zmax))
      {
         zmax = v.Z();
      }
      vba.Position<Vector3f>(i) = v;
      v.X() = vertexNormals[i].x;
      v.Y() = vertexNormals[i].y;
      v.Z() = vertexNormals[i].z;
      vba.Normal<Vector3f>(i) = v;
   }

   bool validBox[BOUNDING_BOX_SLICES][BOUNDING_BOX_SLICES];
   for (x = 0; x < BOUNDING_BOX_SLICES; x++)
   {
      for (z = 0; z < BOUNDING_BOX_SLICES; z++)
      {
         validBox[x][z] = false;
      }
   }
   float xdelta = (xmax - xmin) / (float)BOUNDING_BOX_SLICES;
   float zdelta = (zmax - zmin) / (float)BOUNDING_BOX_SLICES;
   for (i = 0; i < numVertices; i++)
   {
      x = (int)(((vertexPositions[i].x * scale) - xmin) / xdelta);
      if (x < 0)
      {
         x = 0;
      }
      if (x == BOUNDING_BOX_SLICES)
      {
         x--;
      }
      z = (int)(((vertexPositions[i].z * scale) - zmin) / zdelta);
      if (z < 0)
      {
         z = 0;
      }
      if (z == BOUNDING_BOX_SLICES)
      {
         z--;
      }
      validBox[x][z] = true;
   }
   for (x = 0; x < BOUNDING_BOX_SLICES; x++)
   {
      for (z = 0; z < BOUNDING_BOX_SLICES; z++)
      {
         if (validBox[x][z])
         {
            Vector3f center;
            center.X() = (xdelta * ((float)x + 0.5f)) + xmin;
            center.Y() = (ymax + ymin) / 2.0f;
            center.Z() = (zdelta * ((float)z + 0.5f)) + zmin;
            Box3f box(center, Vector3f::UNIT_X, Vector3f::UNIT_Y, Vector3f::UNIT_Z,
                      xdelta, (ymax - ymin), zdelta);
            boundingBoxes.push_back(box);
         }
      }
   }

   int numMeshes = vertexIndices.size();
   m_meshes.resize(numMeshes);
   for (i = 0; i < numMeshes; i++)
   {
      int         numIndices = vertexIndices[i].size();
      IndexBuffer *ibuffer   = new0 IndexBuffer(numIndices, sizeof(int));
      int         *indices   = (int *)ibuffer->GetData();
      for (j = 0; j < numIndices; j++)
      {
         indices[j] = vertexIndices[i][j];
      }
      m_meshes[i] = new0 TriMesh(vformat, vbuffer, ibuffer);
   }
   m_body = m_meshes[0];
   m_node->AttachChild(m_meshes[0]);
   for (i = 1, j = (int)m_meshes.size(); i < j; i++)
   {
      m_node->AttachChild(m_meshes[i]);
      if (m_body->GetNumVertices() < m_meshes[i]->GetNumVertices())
      {
         m_body = m_meshes[i];
      }
   }

   // Lighting.
   for (i = 0, j = (int)m_meshes.size(); i < j; i++)
   {
      Material *material = new0 Material;
      Float4   diffuse(0.0f, 0.0f, 0.0f, 1.0f);
      Float4   specular(0.0f, 0.0f, 0.0f, 1.0f);
      for (j = 0; j < 3; j++)
      {
         diffuse[j]  = materials[i].DiffuseColor[j];
         specular[j] = materials[i].SpecularColor[j];
      }
      material->Diffuse  = diffuse;
      material->Specular = specular;
      LightDirPerVerEffect *effect   = new0 LightDirPerVerEffect();
      VisualEffectInstance *instance = effect->CreateInstance(light, material);
      m_meshes[i]->SetEffectInstance(instance);
   }

   // Position.
   m_nextPosition = position;
   m_speed        = m_nextSpeed = Mathf::IntervalRandom(MinSpeed, MaxSpeed);
   m_rotateDelta  = Mathf::IntervalRandom(MinRotateDelta, MaxRotateDelta);
   if (Mathf::SymmetricRandom() < 0.0f)
   {
      m_rotateDelta = -m_rotateDelta;
   }
   m_nextRotate  = m_rotateDelta;
   m_rotateTimer = gettime();
   m_rotateWait  = (int)Mathf::IntervalRandom(MinRotateWait, MaxRotateWait);
   Update(0.0f);
   m_boundRadius = m_body->GetModelBound().GetRadius();

   // Set bomb timer.
   m_bombTimer = gettime();
}


// Destructor.
GingerMan::~GingerMan()
{
   m_node = 0;
}


// Update.
bool GingerMan::Update(float speedFactor)
{
   int         i, j;
   TIME        t;
   Vector3f    p, cp;
   RigidBall   *ball;
   float       mass, radius, d, e, f;
   Matrix3f    inertia;
   const float densityConstant = 1.0f;

   // Speed changed?
   m_speed = m_nextSpeed;

   // Vary rotation.
   if (m_rotate != m_nextRotate)
   {
      m_rotate = m_nextRotate;
      m_node->LocalTransform.SetRotate(Matrix3f(Vector3f::UNIT_Z, m_rotate * Mathf::DEG_TO_RAD));
   }
   t = gettime();
   if ((t - m_rotateTimer) >= m_rotateWait)
   {
      m_rotateTimer = t;
      m_rotateWait  = (int)Mathf::IntervalRandom(MinRotateWait, MaxRotateWait);
      m_rotateDelta = Mathf::IntervalRandom(MinRotateDelta, MaxRotateDelta);
      if (Mathf::SymmetricRandom() < 0.0f)
      {
         m_rotateDelta = -m_rotateDelta;
      }
   }
   m_nextRotate += (m_rotateDelta * speedFactor);

   // Descend.
   Vector3f position = m_nextPosition;
   m_node->LocalTransform.SetTranslate(position);
   m_nextPosition.Z() -= (m_speed * speedFactor);
   m_node->Update();

   // Drop bomb?
   if (t >= (m_bombTimer + (BombFrequency * 1000)))
   {
      // Find closest cannon on XY plane (terrain).
      p     = position;
      p.Z() = 0.0f;
      for (i = 0, j = -1; i < NUM_CANNONS; i++)
      {
         if (m_cannons[i] != NULL)
         {
            cp     = m_cannons[i]->GetPosition();
            cp.Z() = 0.0f;
            e      = (cp - p).Length();
            if (((j == -1) || (e < d)) && (e <= BombRange))
            {
               d = e;
               j = i;
            }
         }
         if (j >= 0)
         {
            // Drop bomb on cannon.
            p      = m_cannons[j]->GetPosition();
            p.Z()  = position.Z() - (m_boundRadius * m_scale);
            radius = 1.0f;
            ball   = new0 RigidBall(radius, Float3(1.0f, 0.5f, 0.0f), m_light);
            mass   = 4.0f / 3.0f * Mathf::PI * (radius * radius * radius) * densityConstant;
            ball->SetMass(mass);
            f       = (2.0f / 5.0f) * mass * radius * radius;
            inertia = inertia.MakeDiagonal(f, f, f);
            ball->SetBodyInertia(inertia);
            ball->SetPosition(p);
            ball->mForce  = Cannon::Force;
            ball->mTorque = Cannon::Torque;
            m_cannonBalls->Add(ball);

            // Reset timer.
            m_bombTimer = t;
         }
      }
   }

   // Touched terrain?
   if ((position.Z() - m_boundRadius) < m_terrain->GetHeight(position.X(), position.Y()))
   {
      return(true);
   }
   else
   {
      return(false);
   }
}


// Get position.
Vector3f GingerMan::GetPosition()
{
   return(m_node->LocalTransform.GetTranslate());
}


// Set position.
void GingerMan::SetPosition(Vector3f position)
{
   m_nextPosition = position;
   m_node->LocalTransform.SetTranslate(m_nextPosition);
   m_node->Update();
}


// Set rotation.
void GingerMan::SetRotation(float angle)
{
   m_rotate = m_nextRotate = angle;
   m_node->LocalTransform.SetRotate(Matrix3f(Vector3f::UNIT_Z, m_rotate * Mathf::DEG_TO_RAD));
   m_node->Update();
}
