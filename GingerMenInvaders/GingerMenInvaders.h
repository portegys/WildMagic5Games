// Gingerbread Men Invaders Game.
// Based loosely on Space Invaders.

#ifndef GINGER_MEN_INVADERS_H
#define GINGER_MEN_INVADERS_H

#include "Wm5WindowApplication3.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include "GingerMother.h"
#include "glbmp.h"
#include "frameRate.hpp"
#ifdef NETWORK
#include "network.hpp"
#endif
#include "SMSound.h"
using namespace Wm5;

class GingerMenInvaders : public WindowApplication3
{
   WM5_DECLARE_INITIALIZE;
   WM5_DECLARE_TERMINATE;

public:

   GingerMenInvaders();

   virtual bool OnInitialize();
   virtual void OnTerminate();
   virtual void OnIdle();

   // Key input.
   virtual bool OnKeyDown(unsigned char ucKey, int iX, int iY);
   virtual bool OnSpecialKeyDown(int iKey, int iX, int iY);

   // Cannon parameters:
   // Rotation increment (degrees).
   static const float fCannonRotationIncrement;
   // Movement increment.
   static const float fCannonMovementIncrement;
   // Maximum/minimum charge and increment.
   static const float fMaxCannonCharge;
   static const float fMinCannonCharge;
   static const float fCannonChargeIncrement;
   // Placement dispersion.
   static const float fCannonDispersion;

   // Wind parameters:
   static const float fMaxWindSpeed;
   // Probability of changing wind - checked per second.
   static const float fWindChangeProb;
   // Maximum effect of wind speed change.
   static const float fMaxWindAlpha;

protected:

   // Resource path.
   std::string ResourcePath;

   void CreateSkyDome();
   void CreateTerrain();
   void CreateLight();
   void CreateObjects();

   // "main".
   virtual int Main(int, char **);

   // Turret-based motion.
   virtual void MoveForward();
   virtual void MoveBackward();
   virtual void MoveUp();
   virtual void MoveDown();
   void SetCannonView(Cannon *);

   // Game state.
   typedef enum
   {
      SPLASH = 0,
      INIT   = 1,
      STATUS = 2,
      ERR    = 3,
      HELP   = 4,
      RUN    = 5,
      WIN    = 6,
      DIE    = 7,
      QUIT   = 8
   }
   STATE;
   STATE m_state;

   // Player name.
   enum { NAME_SIZE = 16 };
   char m_name[NAME_SIZE];

   // Splash screen.
   void CreateSplash();
   void ShowSplash();

   Node    *m_splashScene;
   Node    *m_splashNode;
   TriMesh *m_splashQuad;
   TIME    m_splashStart;
   enum { SPLASH_TIMEOUT = 5000 };

   // Init screen.
   void ShowInit();

   // Status screen.
   void ShowStatus();

   // Help screen.
   void ShowHelp();

   // Error screen.
   void ShowError();

   enum { ERROR_MSG_SIZE = 100 };
   char m_errorMsg[ERROR_MSG_SIZE];

   // End of game screen.
   void ShowEnd(char *message);

   // Draw user interface displays.
   void DrawGUI();
   void DrawGauge(GLfloat x, GLfloat y,
                  GLfloat radius, Vector3f& needle);
   void DrawCrosshairs(GLfloat radius);
   void ShowNumCannons(GLfloat x, GLfloat y);

   // 2D drawing mode.
   void enter2Dmode();
   void exit2Dmode();

   // Scene, sky, terrain, and light.
   NodePtr             m_Scene;
   TriMeshPtr          m_SkyDome;
   GingerMenTerrainPtr m_Terrain;
   LightPtr            m_Light;
   float               m_HeightAboveTerrain;
   Culler              m_Culler;
   Float4              m_FogColor;
   WireStatePtr        mWireState;

   // Objects.
   NodePtr             m_objects;
   CannonBalls         *m_cannonBalls;
   ExplosionController *m_explosions;
#ifdef NETWORK
   enum { NUM_CANNONS = MAX_PLAYERS };
#else
   enum { NUM_CANNONS = 1 };
#endif
   Cannon       *m_cannons[NUM_CANNONS];
   Node         *m_cannonNodes[NUM_CANNONS];
   int          m_currentCannon;
   GingerMother *m_gingerMother;

   // Wind vector.
   Vector3f m_windVector;
   TIME     m_windTimer;

   // Simulated clock.
   float m_simTime, m_simDelta;

   // Frame rate.
   enum { TARGET_FRAME_RATE = 30 };
   FrameRate m_frameRate;

#ifdef NETWORK
   // Networking.
   Network *network;
   void InitNetwork();
   void DoNetwork();
   void TerminateNetwork();

   char m_masterHost[HOST_NAME_SIZE];
#endif

   // Game state.
   struct GAME_STATE m_gameState;

#ifdef WIN32
   // Keyboard input repeat delay (ms).
   enum { KEY_INPUT_REPEAT_DELAY = 0 };
#endif
};

WM5_REGISTER_INITIALIZE(GingerMenInvaders);
WM5_REGISTER_TERMINATE(GingerMenInvaders);

#endif
