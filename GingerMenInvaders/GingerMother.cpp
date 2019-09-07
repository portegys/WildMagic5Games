// Gingerbread man controller.

#include "GingerMother.h"

// Gingerbread man launch time parameters (seconds).
float GingerMother::MinLaunchTime = 0;
float GingerMother::MaxLaunchTime = 120;

// Gingerbread men dispersion.
float GingerMother::Dispersion = 300.0f;

// Initial height.
float GingerMother::InitialHeightAboveTerrain = 500.0f;

// Scaling factor.
float GingerMother::SizeScale = 25.0f;

// Speed parameter.
float GingerMother::Speed = 0.5f;

// Constructor.
GingerMother::GingerMother(Vector3f position, Node *scene,
                           GingerMenTerrain *terrain, Cannon **cannons, CannonBalls *cannonBalls,
                           struct GAME_STATE *gameState, Camera *camera, Light *light)
{
   m_position    = position;
   m_scene       = scene;
   m_terrain     = terrain;
   m_cannons     = cannons;
   m_cannonBalls = cannonBalls;
   m_gameState   = gameState;
   m_light       = light;
#ifdef NETWORK
   m_network = NULL;
#endif

   // Load gingerman meshes, normals and materials.
   ObjLoader loader(Environment::GetDirectory(0) + "Data/Models/", "gingerman.obj");
   m_materials = loader.GetMaterials();

   // Access the vertices and normals.
   const vector<ObjLoader::Float3> positions = loader.GetPositions();
   m_vertexPositions.resize(positions.size());
   for (int i = 0, j = (int)positions.size(); i < j; i++)
   {
      m_vertexPositions[i] = positions[i];
   }
   int numVertices = m_vertexPositions.size();
   const vector<ObjLoader::Float3> normalsRaw = loader.GetNormals();
   m_vertexNormals.resize(numVertices);

   // Align normal indices with vertex indices.
   const vector<ObjLoader::Group> groups = loader.GetGroups();
   assert(groups.size() == 1);
   ObjLoader::Group group     = groups[0];
   int              numMeshes = group.Meshes.size();
   assert(numMeshes > 0);
   for (int i = 0; i < numMeshes; i++)
   {
      ObjLoader::Mesh         mesh  = group.Meshes[i];
      vector<ObjLoader::Face> faces = mesh.Faces;
      int numFaces = faces.size();
      for (int j = 0; j < numFaces; j++)
      {
         ObjLoader::Face           face     = faces[j];
         vector<ObjLoader::Vertex> vertices = face.Vertices;
         for (int k = 0; k < 3; k++)
         {
            m_vertexNormals[vertices[k].PosIndex] = normalsRaw[vertices[k].NorIndex];
         }
      }
   }

   // Create vertex indices.
   m_vertexIndices.resize(numMeshes);
   for (int i = 0; i < numMeshes; i++)
   {
      ObjLoader::Mesh         mesh  = group.Meshes[i];
      vector<ObjLoader::Face> faces = mesh.Faces;
      int numFaces   = faces.size();
      int numIndices = faces.size() * 3;
      m_vertexIndices[i].resize(numIndices);
      int idx = 0;
      for (int j = 0; j < numFaces; j++)
      {
         ObjLoader::Face           face     = faces[j];
         vector<ObjLoader::Vertex> vertices = face.Vertices;
         for (int k = 0; k < 3; k++)
         {
            m_vertexIndices[i][idx] = vertices[k].PosIndex;
            idx++;
         }
      }
   }

   // Create nodes.
   m_baseNode = new0 Node();
   for (int i = 0; i < NUM_GINGER_MEN; i++)
   {
      m_gingerMen[i]      = NULL;
      m_gingerMenNodes[i] = new0 Node();
      m_baseNode->AttachChild(m_gingerMenNodes[i]);
      m_launched[i]   = false;
      m_launchTime[i] =
         (TIME) Mathf::IntervalRandom((float)MinLaunchTime, (float)MaxLaunchTime + 0.99f);
   }
   m_gingerMother     = NULL;
   m_gingerMotherNode = new0 Node();
   m_baseNode->AttachChild(m_gingerMotherNode);
   m_motherLaunched = false;
   m_Camera         = camera;
}


// Destructor.
GingerMother::~GingerMother()
{
   for (int i = 0; i < NUM_GINGER_MEN; i++)
   {
      if (m_gingerMen[i] != NULL)
      {
         delete0(m_gingerMen[i]);
      }
      m_gingerMenNodes[i] = 0;
   }
   if (m_gingerMother != NULL)
   {
      delete0(m_gingerMother);
   }
   m_gingerMother = 0;
}


// Update.
// Return:
// -1 if a gingerbread man touches terrain.
// 0 if active or un-launched men.
// 1 if men exhausted.
int GingerMother::Update(float speedFactor)
{
   int      i, j;
   Float3   ballColor;
   TIME     t;
   Vector3f cameraDist;

   // Collision detection.
   for (i = 0; i < NUM_GINGER_MEN; i++)
   {
      if (m_gingerMen[i] != NULL)
      {
         if (m_gingerMen[i]->Update(speedFactor))
         {
            // Player loses.
            return(-1);
         }
         if (m_cannonBalls->Collides(m_gingerMen[i], ballColor))
         {
            if (m_gingerMenNodes[i]->GetNumChildren() == 1)
            {
               m_gingerMenNodes[i]->DetachChild(m_gingerMen[i]->GetNode());
               m_gingerMenNodes[i]->Update();
            }
            delete0(m_gingerMen[i]);
            m_gingerMen[i] = NULL;

            // Increment score of destroying cannon.
            for (j = 0; j < NUM_CANNONS; j++)
            {
               if (m_gameState->cannons[j].color == ballColor)
               {
                  m_gameState->cannons[j].score++;
                  break;
               }
            }
         }
      }
   }
   if (m_gingerMother != NULL)
   {
      if (m_gingerMother->Update(speedFactor))
      {
         // Player loses.
         return(-1);
      }
      if (m_cannonBalls->Collides(m_gingerMother, ballColor))
      {
         if (m_gingerMotherNode->GetNumChildren() == 1)
         {
            m_gingerMotherNode->DetachChild(m_gingerMother->GetNode());
            m_gingerMotherNode->Update();
         }
         delete0(m_gingerMother);
         m_gingerMother = NULL;

         // Increment score of destroying cannon.
         for (j = 0; j < NUM_CANNONS; j++)
         {
            if (m_gameState->cannons[j].color == ballColor)
            {
               m_gameState->cannons[j].score++;
               break;
            }
         }
      }
   }

   // All gingerbread men dead?
   for (i = 0; i < NUM_GINGER_MEN; i++)
   {
      if (!m_launched[i] || (m_gingerMen[i] != NULL))
      {
         break;
      }
   }
   if (i == NUM_GINGER_MEN)
   {
      if (m_motherLaunched && (m_gingerMother == NULL))
      {
         // Player wins.
         return(1);
      }
   }

   // Launch gingerbread men.
#ifdef NETWORK
   if ((m_network != NULL) && m_network->master)
   {
#endif
   t = gettime() / 1000;
   for (i = 0; i < NUM_GINGER_MEN; i++)
   {
      // Time to launch?
      if (!m_launched[i] && (m_launchTime[i] <= t))
      {
         m_launched[i]  = true;
         m_gingerMen[i] = CreateGingerMan(GingerMan::SizeScale);
         m_gingerMenNodes[i]->AttachChild(m_gingerMen[i]->GetNode());
         m_gingerMenNodes[i]->Update();
         cameraDist = (m_gingerMen[i]->GetPosition() - m_Camera->GetPosition()) / 100.0f;
         SMSPlaySound(SMSAttackSound, cameraDist, (float *)&Vector3f::ZERO);
      }
   }

   // Time to launch mother?
   for (i = 0; i < NUM_GINGER_MEN; i++)
   {
      if (!m_launched[i] || (m_gingerMen[i] != NULL))
      {
         break;
      }
   }
   if (i == NUM_GINGER_MEN)
   {
      if (!m_motherLaunched)
      {
         // Launch mother.
         m_motherLaunched = true;
         m_gingerMother   = CreateGingerMan(SizeScale);
         m_gingerMother->SetSpeed(Speed);
         m_gingerMotherNode->AttachChild(m_gingerMother->GetNode());
         m_gingerMotherNode->Update();
         cameraDist = (m_gingerMother->GetPosition() - m_Camera->GetPosition()) / 100.0f;
         SMSPlaySound(SMSAttackSound, cameraDist, (float *)&Vector3f::ZERO);
      }
   }
#ifdef NETWORK
}
#endif





   return(0);
}


// Create a gingerbread man.
GingerMan *GingerMother::CreateGingerMan(float scale)
{
   GingerMan *gingerMan;

   float x = m_position.X() + Mathf::SymmetricRandom() * Dispersion;
   float y = m_position.Y() + Mathf::SymmetricRandom() * Dispersion;
   float z = m_terrain->GetHeight(x, y) + InitialHeightAboveTerrain;

   gingerMan = new0 GingerMan(m_vertexPositions, m_vertexNormals, m_vertexIndices,
                              m_materials, scale, Vector3f(x, y, z),
                              m_terrain, m_cannons, m_cannonBalls, m_light);
   return(gingerMan);
}


// Gingerbread men count.
int GingerMother::GetCount()
{
   int count = 1;

   for (int i = 0; i < NUM_GINGER_MEN; i++)
   {
      if (!m_launched[i] || m_gingerMen[i])
      {
         count++;
      }
   }
   return(count);
}


#ifdef NETWORK
// Set the network.
void GingerMother::SetNetwork(Network *network)
{
   m_network = network;
}


// Update game state.
void GingerMother::UpdateGameState()
{
   for (int i = 0; i < NUM_GINGER_MEN; i++)
   {
      if (m_gingerMen[i] != NULL)
      {
         m_gameState->gingerMen[i].alive        = true;
         m_gameState->gingerMen[i].nextSpeed    = m_gingerMen[i]->m_nextSpeed;
         m_gameState->gingerMen[i].nextPosition = m_gingerMen[i]->m_nextPosition;
         m_gameState->gingerMen[i].nextRotate   = m_gingerMen[i]->m_nextRotate;
      }
      else
      {
         m_gameState->gingerMen[i].alive = false;
      }
      m_gameState->gingerMen[i].launched = m_launched[i];
   }
   if (m_gingerMother != NULL)
   {
      m_gameState->gingerMother.alive        = true;
      m_gameState->gingerMother.nextSpeed    = m_gingerMother->m_nextSpeed;
      m_gameState->gingerMother.nextPosition = m_gingerMother->m_nextPosition;
      m_gameState->gingerMother.nextRotate   = m_gingerMother->m_nextRotate;
   }
   else
   {
      m_gameState->gingerMother.alive = false;
   }
   m_gameState->gingerMother.launched = m_motherLaunched;
}


// Synchronize game state.
void GingerMother::SynchGameState()
{
   Vector3f cameraDist;

   for (int i = 0; i < NUM_GINGER_MEN; i++)
   {
      if ((m_gingerMen[i] != NULL) && !m_gameState->gingerMen[i].alive)
      {
         if (m_gingerMenNodes[i]->GetNumChildren() == 1)
         {
            m_gingerMenNodes[i]->DetachChild(m_gingerMen[i]->GetNode());
            m_gingerMenNodes[i]->Update();
         }
         delete0(m_gingerMen[i]);
         m_gingerMen[i] = NULL;
      }
      else if ((m_gingerMen[i] == NULL) && m_gameState->gingerMen[i].alive)
      {
         m_gingerMen[i] = CreateGingerMan(GingerMan::SizeScale);
         m_gingerMenNodes[i]->AttachChild(m_gingerMen[i]->GetNode());
         m_gingerMen[i]->m_nextSpeed    = m_gameState->gingerMen[i].nextSpeed;
         m_gingerMen[i]->m_nextPosition = m_gameState->gingerMen[i].nextPosition;
         m_gingerMen[i]->m_nextRotate   = m_gameState->gingerMen[i].nextRotate;
         m_gingerMenNodes[i]->Update();
         cameraDist = (m_gingerMen[i]->GetPosition() - m_Camera->GetPosition()) / 100.0f;
         SMSPlaySound(SMSAttackSound, cameraDist, (float *)&Vector3f::ZERO);
      }
      else if (m_gingerMen[i] != NULL)
      {
         m_gingerMen[i]->m_nextSpeed    = m_gameState->gingerMen[i].nextSpeed;
         m_gingerMen[i]->m_nextPosition = m_gameState->gingerMen[i].nextPosition;
         m_gingerMen[i]->m_nextRotate   = m_gameState->gingerMen[i].nextRotate;
      }
      m_launched[i] = m_gameState->gingerMen[i].launched;
   }
   if ((m_gingerMother != NULL) && !m_gameState->gingerMother.alive)
   {
      if (m_gingerMotherNode->GetNumChildren() == 1)
      {
         m_gingerMotherNode->DetachChild(m_gingerMother->GetNode());
         m_gingerMotherNode->Update();
      }
      delete0(m_gingerMother);
      m_gingerMother = NULL;
   }
   else if ((m_gingerMother == NULL) && m_gameState->gingerMother.alive)
   {
      m_gingerMother = CreateGingerMan(GingerMother::SizeScale);
      m_gingerMotherNode->AttachChild(m_gingerMother->GetNode());
      m_gingerMother->m_nextSpeed    = m_gameState->gingerMother.nextSpeed;
      m_gingerMother->m_nextPosition = m_gameState->gingerMother.nextPosition;
      m_gingerMother->m_nextRotate   = m_gameState->gingerMother.nextRotate;
      m_gingerMotherNode->Update();
      cameraDist = (m_gingerMother->GetPosition() - m_Camera->GetPosition()) / 100.0f;
      SMSPlaySound(SMSAttackSound, cameraDist, (float *)&Vector3f::ZERO);
   }
   else if (m_gingerMother != NULL)
   {
      m_gingerMother->m_nextSpeed    = m_gameState->gingerMother.nextSpeed;
      m_gingerMother->m_nextPosition = m_gameState->gingerMother.nextPosition;
      m_gingerMother->m_nextRotate   = m_gameState->gingerMother.nextRotate;
   }
   m_motherLaunched = m_gameState->gingerMother.launched;
}


#endif
