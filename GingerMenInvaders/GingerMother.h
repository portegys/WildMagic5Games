// Gingerbread men controller.

#ifndef GINGER_MOTHER_H
#define GINGER_MOTHER_H

#include "Wm5WindowApplication3.h"
#include "GingerMan.h"
#include "Cannon.h"
#include "CannonBalls.h"
#include "explosionController.hpp"
#include "gettime.h"
#include "GameState.h"
#include <vector>
using namespace Wm5;

class GingerMother
{
public:

   // Gingerbread man launch time parameters (seconds).
   static float MinLaunchTime, MaxLaunchTime;

   // Gingerbread men dispersion.
   static float Dispersion;

   // Initial height.
   static float InitialHeightAboveTerrain;

   // Scaling factor.
   static float SizeScale;

   // Speed parameter.
   static float Speed;

   // Constructor/destructor.
   GingerMother(Vector3f position, Node *scene,
                GingerMenTerrain *terrain, Cannon **cannons,
                CannonBalls *cannonBalls, struct GAME_STATE *gameState,
                Camera *camera, Light *light);
   ~GingerMother();

   // Update.
   // Return:
   // -1 if a gingerbread man touches terrain.
   // 0 if active or un-launched men.
   // 1 if men exhausted.
   int Update(float speedFactor);

   // Get base node.
   NodePtr GetBaseNode() { return(m_baseNode); }

   // Position.
   Vector3f GetPosition();
   void SetPosition(Vector3f position);

   // Gingerbread men count.
   int GetCount();

#ifdef NETWORK
   // Set the network.
   void SetNetwork(Network *network);

   // Update game state.
   void UpdateGameState();

   // Synchronize game state.
   void SynchGameState();
#endif

private:

   // Create a gingerbread man.
   GingerMan *CreateGingerMan(float scale);

   Vector3f                    m_position;
   Node                        *m_scene;
   GingerMenTerrain            *m_terrain;
   Cannon                      **m_cannons;
   CannonBalls                 *m_cannonBalls;
   Light                       *m_light;
   struct GAME_STATE           *m_gameState;
   Camera                      *m_Camera;
   GingerMan                   *m_gingerMen[NUM_GINGER_MEN];
   bool                        m_launched[NUM_GINGER_MEN];
   TIME                        m_launchTime[NUM_GINGER_MEN];
   GingerMan                   *m_gingerMother;
   bool                        m_motherLaunched;
   vector<ObjLoader::Float3>   m_vertexPositions;
   vector<ObjLoader::Float3>   m_vertexNormals;
   vector<vector<int> >        m_vertexIndices;
   vector<MtlLoader::Material> m_materials;
   NodePtr                     m_baseNode;
   NodePtr                     m_gingerMenNodes[NUM_GINGER_MEN];
   NodePtr                     m_gingerMotherNode;
#ifdef NETWORK
   Network *m_network;
#endif
};
#endif
