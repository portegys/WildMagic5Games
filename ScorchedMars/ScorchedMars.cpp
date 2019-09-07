// Scorched Mars Battle Game.
// Based loosely on Scorched Earth.

#include "ScorchedMars.h"
#include "TerrainEffect.h"

WM5_WINDOW_APPLICATION(ScorchedMars);

// Cannon parameters:
// Rotation increment (degrees).
const float ScorchedMars::fCannonRotationIncrement = 1.0f;
// Movement increment.
const float ScorchedMars::fCannonMovementIncrement = 2.0f;
// Maximum/minimum charge and increment.
const float ScorchedMars::fMaxCannonCharge       = 100.0f;
const float ScorchedMars::fMinCannonCharge       = 10.0f;
const float ScorchedMars::fCannonChargeIncrement = 10.0f;
// Placement dispersion.
const float ScorchedMars::fCannonDispersion = 500.0f;

// Wind parameters.
const float ScorchedMars::fMaxWindSpeed   = 1.0f;
const float ScorchedMars::fWindChangeProb = 0.1f;
const float ScorchedMars::fMaxWindAlpha   = 0.5f;

//----------------------------------------------------------------------------
ScorchedMars::ScorchedMars()
   :
     WindowApplication3("ScorchedMars", 0, 0, 1024, 1024,
                        Float4(0.5f, 0.0f, 1.0f, 1.0f))
{
   // Access resources.
   ResourcePath = WM5Path + "Projects/";
   Environment::InsertDirectory(ResourcePath);

   m_state = SPLASH;
   memset(m_name, 0, NAME_SIZE);
   m_name[0]            = '_';
   m_HeightAboveTerrain = 20.0f;
   m_windVector         = Vector3f(0.0f, 0.0f, 0.0f);
   m_windTimer          = 0;
   m_currentCannon      = 0;
   memset(m_errorMsg, 0, ERROR_MSG_SIZE);
#ifdef NETWORK
   network = NULL;
   memset(m_masterHost, 0, HOST_NAME_SIZE);
   m_masterHost[0] = '_';
#endif
}


// "Main".
int ScorchedMars::Main(int argc, char **argv)
{
#ifdef NETWORK
   // Initialize networking for slave.
   if (argc == 2)
   {
      network = new0 Network();
      if (!network->initSlave(argv[1]))
      {
         // Fall back to master mode.
         if (!network->initMaster())
         {
            fprintf(stderr, "Cannot initialize networking: %s", network->statusMessage);
            return(1);
         }
      }
      m_currentCannon = network->myIndex;
      m_state         = RUN;
   }
#endif
   return(WindowApplication3::Main(argc, argv));
}


//----------------------------------------------------------------------------
bool ScorchedMars::OnInitialize()
{
   if (!WindowApplication3::OnInitialize())
   {
      return(false);
   }

   // Seed random numbers.
   Mathf::SymmetricRandom((int)time(0));

   // Set up the camera.
   // Position the camera in the middle of page[0][0].
   // Orient it to look diagonally across the terrain pages.
   mCamera->SetFrustum(60.0f, GetAspectRatio(), 1.0f, 1500.0f);
   APoint  camPosition(64.0f, 64.0f, m_HeightAboveTerrain);
   AVector camDVector(Mathf::INV_SQRT_2, Mathf::INV_SQRT_2, 0.0f);
   AVector camUVector(0.0f, 0.0f, 1.0f);
   AVector camRVector = camDVector.Cross(camUVector);
   mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

   // Create scene.
   m_Scene = new0 Node();
   CreateSkyDome();
   CreateTerrain();
   CreateLight();
   CreateObjects();

   // Initial update of objects.
   m_Scene->Update();

   // Initial culling of scene.
   m_Culler.SetCamera(mCamera);
   m_Culler.ComputeVisibleSet(m_Scene);

   // Initialize camera motion.
   InitializeCameraMotion(1.0f, 0.01f);
   MoveForward();

   // Initialize sound.
   char expPath[256];
   char firePath[256];
   char windPath[256];
   strncpy(expPath, (char *)Environment::GetPathR("Data/Sounds/explosion.wav").c_str(), 255);
   strncpy(firePath, (char *)Environment::GetPathR("Data/Sounds/fire.wav").c_str(), 255);
   strncpy(windPath, (char *)Environment::GetPathR("Data/Sounds/wind.wav").c_str(), 255);
   SMSInitSound(expPath, firePath, windPath);
   SMSStartWind();

   // Set the target frame rate.
   m_frameRate.setTarget(TARGET_FRAME_RATE);

   // Rig for wire frame view.
   mWireState = new0 WireState();
   mRenderer->SetOverrideWireState(mWireState);

   // Set the simulation time.
   m_simTime  = 0.0f;
   m_simDelta = 0.1f;

   // Load the splash bitmap.
   if (!glbmp_LoadBitmap(Environment::GetPathR("Data/Images/scorched-mars-logo.bmp").c_str(), 0, &m_bitmap))
   {
      fprintf(stderr, "Cannot load splash file scorched-mars-logo.bmp\n");
      return(false);
   }
   m_splashStart = gettime();

   return(true);
}


#ifdef NETWORK
// Initialize networking.
void ScorchedMars::InitNetwork()
{
   network = new0 Network();
   if (m_masterHost[0] != 0)
   {
      // Slave.
      if (!network->initSlave(m_masterHost))
      {
         // Fall back to master mode.
         if (!network->initMaster())
         {
            fprintf(stderr, "Cannot initialize networking: %s", network->statusMessage);
            sprintf(m_errorMsg, "Cannot initialize networking: %s", network->statusMessage);
            m_state = ERR;
            return;
         }
      }
   }
   else
   {
      // Master.
      if (!network->initMaster())
      {
         fprintf(stderr, "initMaster failed: %s", network->statusMessage);
         sprintf(m_errorMsg, "initMaster failed: %s", network->statusMessage);
         m_state = ERR;
         return;
      }
   }
   m_currentCannon = network->myIndex;
   m_cannonNodes[m_currentCannon]->AttachChild(m_cannons[m_currentCannon]->GetBaseNode());
   m_cannonNodes[m_currentCannon]->Update();
}


#endif

//----------------------------------------------------------------------------
void ScorchedMars::OnTerminate()
{
#ifdef NETWORK
   TerminateNetwork();
   if (network != NULL)
   {
      delete0(network);
      network = NULL;
   }
#endif

   for (int i = 0; i < NUM_CANNONS; i++)
   {
      if (m_cannons[i] != NULL)
      {
         delete0(m_cannons[i]);
      }
      m_cannonNodes[i] = 0;
   }
   delete0(m_cannonBalls);
   delete0(m_explosions);
   m_Scene   = 0;
   m_SkyDome = 0;
   m_Terrain = 0;
   m_Light   = 0;
   m_objects = 0;
   WindowApplication3::OnTerminate();

   // Free sounds.
   SMSCleanUp();
}


#ifdef NETWORK
// Terminate networking.
void ScorchedMars::TerminateNetwork()
{
   if ((network != NULL) && !network->terminated)
   {
      if (network->master)
      {
         // Pick a new master and notify.
         if (!network->exitNotify(Network::QUIT))
         {
            fprintf(stderr, "exitNotify failed: %s\n", network->statusMessage);
            exit(1);
         }

         // Synch new master.
         network->masterPayload.size = sizeof(struct GAME_STATE);
         assert(network->masterPayload.size <= MAX_MASTER_PAYLOAD);
         memcpy(network->masterPayload.data, &m_gameState, sizeof(struct GAME_STATE));
         if (!network->sendMaster())
         {
            fprintf(stderr, "sendMaster failed: %s\n", network->statusMessage);
            exit(1);
         }
      }
      else                                        // slave.
      {
         if (!network->exitNotify(Network::QUIT))
         {
            fprintf(stderr, "exitNotify failed: %s\n", network->statusMessage);
            exit(1);
         }
      }
   }
}


#endif

//----------------------------------------------------------------------------
void ScorchedMars::OnIdle()
{
   int    i;
   TIME   currentTime;
   Float3 ballColor;

#ifdef WIN32
   static TIME lastTime = 0;
#endif

   MeasureTime();

   // Get current time.
   currentTime = gettime();

   if (mRenderer->PreDraw())
   {
      mRenderer->ClearBuffers();
      switch (m_state)
      {
      case RUN:
      case STATUS:
      case HELP:
         // Vary wind.
         if ((currentTime - m_windTimer) >= 1000)
         {
            m_windTimer = currentTime;
            if (Mathf::UnitRandom() < fWindChangeProb)
            {
               float alpha = Mathf::IntervalRandom(0.0f, fMaxWindAlpha);
               m_windVector = (m_windVector * (1.0f - alpha)) +
                              (alpha * Vector3f(
                                  Mathf::IntervalRandom(-fMaxWindSpeed, fMaxWindSpeed),
                                  Mathf::IntervalRandom(-fMaxWindSpeed, fMaxWindSpeed),
                                  0.0f));
               if (m_windVector.Length() > fMaxWindSpeed)
               {
                  m_windVector.Normalize();
                  m_windVector *= fMaxWindSpeed;
               }
            }
         }

#ifdef NETWORK
         DoNetwork();
#else
         // Run "NPC" cannons.
         for (i = 0; i < NUM_CANNONS; i++)
         {
            if ((m_cannons[i] != NULL) && (i != m_currentCannon))
            {
               RigidBall *cannonball = m_cannons[i]->DoAI(m_cannons[m_currentCannon], fCannonMovementIncrement, m_frameRate.speedFactor);
               if (cannonball != NULL)
               {
                  m_cannonBalls->Add(cannonball);
               }
            }
         }
#endif

         // Update fired cannonballs.
         m_cannonBalls->Update(m_simTime, m_simDelta * m_frameRate.speedFactor);

         // Check for cannonball collisions with cannons.
#ifdef NETWORK
         for (i = 0; i < NUM_CANNONS; i++)
         {
            if (network->currentPlayers[i])
            {
               if (m_cannonBalls->Collides(m_cannons[i]->GetPosition(),
                                           m_cannons[i]->GetRadius(), ballColor))
               {
                  for (int j = 0; j < NUM_CANNONS; j++)
                  {
                     if (network->currentPlayers[j] &&
                         (m_gameState.cannons[j].color == ballColor))
                     {
                        m_gameState.cannons[j].score++;
                        break;
                     }
                  }

                  if (i == m_currentCannon)
                  {
                     TerminateNetwork();
                     m_state = DIE;
                  }
               }
            }
         }
#else
         for (i = 0; i < NUM_CANNONS; i++)
         {
            if (m_cannons[i] != NULL)
            {
               if (m_cannonBalls->Collides(m_cannons[i]->GetPosition(),
                                           m_cannons[i]->GetRadius(), ballColor))
               {
                  if (m_cannonNodes[i]->GetNumChildren() == 1)
                  {
                     m_cannonNodes[i]->DetachChild(m_cannons[i]->GetBaseNode());
                     m_cannonNodes[i]->Update();
                  }
                  delete0(m_cannons[i]);
                  m_cannons[i] = NULL;
               }
            }
         }
         if (m_cannons[m_currentCannon] == NULL)
         {
            m_state = DIE;
         }
#endif

         switch (m_state)
         {
         case RUN:
            {
               // Link camera to cannon.
               SetCannonView(m_cannons[m_currentCannon]);

               // The sky dome moves with the camera.
               APoint camPosition = mCamera->GetPosition();
               APoint skyPosition(camPosition[0], camPosition[1], 0.0f);
               m_SkyDome->LocalTransform.SetTranslate(skyPosition);
               m_SkyDome->Update();

               // Update the active terrain pages.
               m_Terrain->OnCameraMotion();

               // Update explosions.
               m_explosions->Update(m_frameRate.speedFactor);

               // Draw scene.
               Spatial::CullingMode cullingMode = m_cannonNodes[m_currentCannon]->Culling;
#ifdef WIN32
               m_cannonNodes[m_currentCannon]->Culling = Spatial::CullingMode::CULL_ALWAYS;
#else
               m_cannonNodes[m_currentCannon]->Culling = (Spatial::CullingMode)1;
#endif
               m_Culler.ComputeVisibleSet(m_Scene);
               mRenderer->Draw(m_Culler.GetVisibleSet());
               m_cannonNodes[m_currentCannon]->Culling = cullingMode;

               // Draw GUI.
               DrawGUI();

#ifdef WIN32
#ifndef LOOP_AROUND
               // Use Windows "quick" key detection.
               if ((currentTime - lastTime) >= KEY_INPUT_REPEAT_DELAY)
               {
                  lastTime = currentTime;
                  lastTime = currentTime;
                  if (GetAsyncKeyState(VK_UP))
                  {
                     OnSpecialKeyDown(KEY_UP_ARROW, 0, 0);
                  }
                  if (GetAsyncKeyState(VK_DOWN))
                  {
                     OnSpecialKeyDown(KEY_DOWN_ARROW, 0, 0);
                  }
                  if (GetAsyncKeyState(VK_LEFT))
                  {
                     OnSpecialKeyDown(KEY_LEFT_ARROW, 0, 0);
                  }
                  if (GetAsyncKeyState(VK_RIGHT))
                  {
                     OnSpecialKeyDown(KEY_RIGHT_ARROW, 0, 0);
                  }
                  if (GetAsyncKeyState(VK_PRIOR))
                  {
                     OnSpecialKeyDown(KEY_PAGE_UP, 0, 0);
                  }
                  if (GetAsyncKeyState(VK_NEXT))
                  {
                     OnSpecialKeyDown(KEY_PAGE_DOWN, 0, 0);
                  }
               }
#endif
#endif

               // next simulation time
               m_simTime += m_simDelta;
            }
            break;

         case STATUS:
            ShowStatus();
            break;

         case HELP:
            ShowHelp();
            break;

         case DIE:
            ShowDeath();
            break;
         }
         break;

      case SPLASH:
         ShowSplash();
         break;

      case INIT:
         ShowInit();
         break;

      case ERR:
         ShowError();
         break;

      case DIE:
         ShowDeath();
         break;

      case QUIT:
         OnTerminate();
         break;
      }
      mRenderer->PostDraw();
      mRenderer->DisplayColorBuffer();
   }

   // Count frames.
   UpdateFrameCount();

   // Set frame-rate independence speed factor.
   m_frameRate.update();
}


//----------------------------------------------------------------------------
bool ScorchedMars::OnKeyDown(unsigned char ucKey, int iX, int iY)
{
   float       charge;
   static int  index   = 0;
   static bool gotName = false;

   if ((m_state == SPLASH) || (m_state == DIE))
   {
      return(true);
   }

   if (m_state == INIT)
   {
      // Grab m_name and optional m_masterHost.
      if (!gotName)
      {
         if (ucKey != KEY_RETURN)
         {
            if (ucKey == KEY_BACKSPACE)
            {
               m_name[index] = 0;
               if (index > 0)
               {
                  index--;
               }
               m_name[index] = '_';
            }
            else
            {
               if (index < NAME_SIZE - 1)
               {
                  m_name[index] = ucKey;
                  index++;
                  if (index < NAME_SIZE - 1)
                  {
                     m_name[index] = '_';
                  }
               }
            }
         }
         else
         {
            gotName = true;
            for (int i = NAME_SIZE - 2; i >= 0; i--)
            {
               if (m_name[i] == '_')
               {
                  m_name[i] = 0;
               }
               else
               {
                  if (m_name[i] != 0)
                  {
                     break;
                  }
               }
            }
#ifdef NETWORK
            index = 0;
#else
            m_state = RUN;
#endif
         }
#ifdef NETWORK
      }
      else
      {
         if (ucKey != KEY_RETURN)
         {
            if (ucKey == KEY_BACKSPACE)
            {
               m_masterHost[index] = 0;
               if (index > 0)
               {
                  index--;
               }
               m_masterHost[index] = '_';
            }
            else
            {
               if (index < HOST_NAME_SIZE - 1)
               {
                  m_masterHost[index] = ucKey;
                  index++;
                  if (index < HOST_NAME_SIZE - 1)
                  {
                     m_masterHost[index] = '_';
                  }
               }
            }
         }
         else
         {
            for (int i = HOST_NAME_SIZE - 2; i >= 0; i--)
            {
               if (m_masterHost[i] == '_')
               {
                  m_masterHost[i] = 0;
               }
               else
               {
                  if (m_masterHost[i] != 0)
                  {
                     break;
                  }
               }
            }
            InitNetwork();
            m_state = RUN;
         }
#endif
      }
      return(true);
   }

   if (m_state != RUN)
   {
      m_state = RUN;
      return(true);
   }

   if (m_cannons[m_currentCannon] == NULL)
   {
      return(true);
   }
   switch (ucKey)
   {
   case 'g':
   case 'G':
      if (m_cannons[m_currentCannon] == NULL)
      {
         return(true);
      }
      charge  = m_cannons[m_currentCannon]->GetCharge();
      charge += fCannonChargeIncrement;
      if (charge > fMaxCannonCharge)
      {
         charge = fMaxCannonCharge;
      }
      m_cannons[m_currentCannon]->SetCharge(charge);
      return(true);

   case 'f':
   case 'F':
      if (m_cannons[m_currentCannon] == NULL)
      {
         return(true);
      }
      charge  = m_cannons[m_currentCannon]->GetCharge();
      charge -= fCannonChargeIncrement;
      if (charge < fMinCannonCharge)
      {
         charge = fMinCannonCharge;
      }
      m_cannons[m_currentCannon]->SetCharge(charge);
      return(true);

   case ' ':
      if (m_cannons[m_currentCannon] == NULL)
      {
         return(true);
      }
#ifdef NETWORK
      if (!m_gameState.cannons[m_currentCannon].firing)
      {
         m_gameState.cannons[m_currentCannon].firing = true;
#endif
      m_cannonBalls->Add(m_cannons[m_currentCannon]->Fire());
#ifdef NETWORK
   }
#endif
      return(true);

   case 'h':
   case 'H':
      m_state = HELP;
      return(true);

   case 's':
   case 'S':
      m_state = STATUS;
      return(true);

   case 'w':
   case 'W':
      mWireState->Enabled = !mWireState->Enabled;
      return(true);
   }

   return(false);
}


//---------------------------------------------------------------------------//
// Special Keyboard mappings.
//---------------------------------------------------------------------------//
bool ScorchedMars::OnSpecialKeyDown(int iKey, int iX, int iY)
{
   float swivel, elevation;

   if (m_cannons[m_currentCannon] == NULL)
   {
      return(true);
   }
   if (iKey == KEY_UP_ARROW)
   {
      m_cannons[m_currentCannon]->MoveForward(fCannonMovementIncrement * m_frameRate.speedFactor);
      return(true);
   }
   else if (iKey == KEY_DOWN_ARROW)
   {
      m_cannons[m_currentCannon]->MoveBackward(fCannonMovementIncrement * m_frameRate.speedFactor);
      return(true);
   }
   else if (iKey == KEY_LEFT_ARROW)
   {
      if (m_cannons[m_currentCannon] == NULL)
      {
         return(true);
      }
      swivel = m_cannons[m_currentCannon]->GetSwivel();
      m_cannons[m_currentCannon]->SetSwivel(swivel + (fCannonRotationIncrement * m_frameRate.speedFactor));
      return(true);
   }
   else if (iKey == KEY_RIGHT_ARROW)
   {
      if (m_cannons[m_currentCannon] == NULL)
      {
         return(true);
      }
      swivel = m_cannons[m_currentCannon]->GetSwivel();
      m_cannons[m_currentCannon]->SetSwivel(swivel - (fCannonRotationIncrement * m_frameRate.speedFactor));
      return(true);
   }
   else if (iKey == KEY_PAGE_UP)
   {
      if (m_cannons[m_currentCannon] == NULL)
      {
         return(true);
      }
      elevation = m_cannons[m_currentCannon]->GetElevation();
      m_cannons[m_currentCannon]->SetElevation(elevation - (fCannonRotationIncrement * m_frameRate.speedFactor));
      return(true);
   }
   else if (iKey == KEY_PAGE_DOWN)
   {
      if (m_cannons[m_currentCannon] == NULL)
      {
         return(true);
      }
      elevation = m_cannons[m_currentCannon]->GetElevation();
      m_cannons[m_currentCannon]->SetElevation(elevation + (fCannonRotationIncrement * m_frameRate.speedFactor));
      return(true);
   }

   // See if WindowApplication has any default keyboard mappings to process
   return(WindowApplication3::OnSpecialKeyDown(iKey, iX, iY));
}


//----------------------------------------------------------------------------
void ScorchedMars::CreateSkyDome()
{
#ifdef NEVER
   // Load and initialize the sky dome.  It follows the camera.
   std::string skyMeshName = Environment::GetPathR("Data/Wmvf/SkyDomePNT2.wmvf");
   Visual::PrimitiveType type;
   VertexFormat *vformat;
   VertexBuffer *vbuffer;
   IndexBuffer *ibuffer;
   Visual::LoadWMVF(skyMeshName, type, vformat, vbuffer, ibuffer);
   m_SkyDome = new0 TriMesh(vformat, vbuffer, ibuffer);
   m_Scene->AttachChild(m_SkyDome);

   APoint skyPosition = mCamera->GetPosition();
   skyPosition[2] = 0.0f;
   m_SkyDome->LocalTransform.SetTranslate(skyPosition);
   m_SkyDome->LocalTransform.SetUniformScale(mCamera->GetDMax());

   Texture2DEffect *skyEffect = new0 Texture2DEffect(
      Shader::SF_LINEAR_LINEAR, Shader::SC_REPEAT, Shader::SC_REPEAT);
   std::string skyTextureName = Environment::GetPathR("Data/Wmtf/SkyDome.wmtf");
   Texture2D *skyTexture      = Texture2D::LoadWMTF(skyTextureName);
   skyTexture->GenerateMipmaps();
   m_SkyDome->SetEffectInstance(skyEffect->CreateInstance(skyTexture));
#else
   // Color the sky "Martian"
   VertexFormat *vformat = VertexFormat::Create(2,
                                                VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
                                                VertexFormat::AU_COLOR, VertexFormat::AT_FLOAT3, 0);
   m_SkyDome = StandardMesh(vformat).Sphere(24, 24, 1.0f);
   Float3 kSkyColor(224.0f / 255.0f, 196.0f / 255.0f, 172.0f / 255.0f);
   VertexBufferAccessor vba(m_SkyDome);
   for (int i = 0; i < 4; ++i)
   {
      vba.Color<Float3>(0, i) = kSkyColor;
   }
   VertexColor3Effect *vceffect = new0 VertexColor3Effect();
   vceffect->GetWireState(0, 0)->Enabled = true;
   m_SkyDome->SetEffectInstance(vceffect->CreateInstance());
   m_Scene->AttachChild(m_SkyDome);
   APoint skyPosition = mCamera->GetPosition();
   skyPosition[2] = 0.0f;
   m_SkyDome->LocalTransform.SetTranslate(skyPosition);
   m_SkyDome->LocalTransform.SetUniformScale(mCamera->GetDMax());
#endif
}


//----------------------------------------------------------------------------
void ScorchedMars::CreateTerrain()
{
   // Load the height field and create the terrain.
   VertexFormat *vformat = VertexFormat::Create(3,
                                                VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
                                                VertexFormat::AU_TEXCOORD, VertexFormat::AT_FLOAT2, 0,
                                                VertexFormat::AU_TEXCOORD, VertexFormat::AT_FLOAT2, 1);

   // For lower-resolution terrain, change the paths to Height64/Color64 or
   // Height32/Color32.
   std::string heightName = ResourcePath + "Data/Terrain/MarsHeight32/height";
   std::string colorName  = ResourcePath + "Data/Terrain/MarsImage32/image";

   m_Terrain = new0 ScorchedMarsTerrain(heightName, vformat, mCamera);
   m_Scene->AttachChild(m_Terrain);

   // The effect that is shared across all pages.
   std::string effectFile =
      Environment::GetPathR("Shaders/BaseMulDetailFogExpSqr.wmfx");
   TerrainEffect *terrainEffect = new0 TerrainEffect(effectFile);

   std::string detailName   = Environment::GetPathR("Data/Terrain/Detail.wmtf");
   Texture2D *detailTexture = Texture2D::LoadWMTF(detailName);
   detailTexture->GenerateMipmaps();

   ShaderFloat *fogColorDensity = new0 ShaderFloat(1);
   (*fogColorDensity)[0] = 224.0f / 255.0f;
   (*fogColorDensity)[1] = 196.0f / 255.0f;
   (*fogColorDensity)[2] = 172.0f / 255.0f;
   (*fogColorDensity)[3] = 0.0015f;

   // Attach an effect to each page.  Preload all resources to video memory.
   // This will avoid frame rate stalls when new terrain pages are
   // encountered as the camera moves.
   const int numRows = m_Terrain->GetRowQuantity();
   const int numCols = m_Terrain->GetColQuantity();
   for (int r = 0; r < numRows; ++r)
   {
      for (int c = 0; c < numCols; ++c)
      {
         TerrainPage *page = m_Terrain->GetPage(r, c);

         char suffix[32];
         sprintf(suffix, ".%d.%d.wmtf", r, c);
         std::string colorTextureName = colorName + std::string(suffix);
         Texture2D *colorTexture      = Texture2D::LoadWMTF(colorTextureName);
         colorTexture->GenerateMipmaps();

         VisualEffectInstance *instance = terrainEffect->CreateInstance(
            colorTexture, detailTexture, fogColorDensity);

         page->SetEffectInstance(instance);

         mRenderer->Bind(page->GetVertexBuffer());
         mRenderer->Bind(page->GetVertexFormat());
         mRenderer->Bind(page->GetIndexBuffer());
         mRenderer->Bind(colorTexture);
      }
   }
}


//----------------------------------------------------------------------------
void ScorchedMars::CreateLight()
{
   // Create a new directional light
   m_Light           = new Light(Light::LT_DIRECTIONAL);
   m_Light->Ambient  = Float4(1.0f, 1.0f, 1.0f, 1.0f);
   m_Light->Diffuse  = Float4(0.75f, 0.75f, 0.75f, 1.0f);
   m_Light->Specular = Float4(0.25f, 0.25f, 0.25f, 1.0f);
   m_Light->SetDirection(AVector(-1.0f, -1.0f, -1.0f));
}


//----------------------------------------------------------------------------
void ScorchedMars::CreateObjects()
{
   // Create parent of objects.
   m_objects = new0 Node();
   m_Scene->AttachChild(m_objects);

   // Create cannons.
   for (int i = 0; i < NUM_CANNONS; i++)
   {
      m_cannons[i] = new0 Cannon(
         Float3(Mathf::UnitRandom(), Mathf::UnitRandom(), 0.0f),
         m_Scene, m_Terrain, mCamera, m_Light);
      m_cannons[i]->SetSwivel(Mathf::IntervalRandom(0.0f, 180.0f));
      m_cannons[i]->SetElevation(90.0f);
      m_cannons[i]->SetCharge(fMaxCannonCharge);
      m_cannonNodes[i] = new0 Node();
      float x = Mathf::SymmetricRandom() * fCannonDispersion;
      float y = Mathf::SymmetricRandom() * fCannonDispersion;
#ifndef NETWORK
      if (i == m_currentCannon)
      {
         y += 1000.0f;
      }
#endif
      float z = m_Terrain->GetHeight(x, y) + Cannon::HeightAboveTerrain;
      m_cannons[i]->SetPosition(Vector3f(x, y, z));
      m_objects->AttachChild(m_cannonNodes[i]);
#ifdef NETWORK
      m_gameState.cannons[i].color     = m_cannons[i]->GetColor();
      m_gameState.cannons[i].position  = m_cannons[i]->GetPosition();
      m_gameState.cannons[i].swivel    = m_cannons[i]->GetSwivel();
      m_gameState.cannons[i].elevation = m_cannons[i]->GetElevation();
      m_gameState.cannons[i].charge    = m_cannons[i]->GetCharge();
      m_gameState.cannons[i].firing    = false;
      m_gameState.cannons[i].score     = 0;
      memset(m_gameState.cannons[i].name, 0, NAME_SIZE);
      if ((network != NULL) && (m_currentCannon == i))
#endif
      m_cannonNodes[i]->AttachChild(m_cannons[i]->GetBaseNode());
   }

   // Create explosions controller.
   m_explosions = new0 ExplosionController(m_objects);

   // Create cannonball controller.
   m_cannonBalls = new0 CannonBalls(m_objects, m_Terrain,
                                    &m_windVector, m_explosions, mCamera);
}


#ifdef NETWORK

/*
 *
 * Networking function.
 *
 * The normal message exchange:
 *
 * Slave                   Master
 *
 * sendSlave()  -----
 |
 |                -----> getSlave()
 |
 | ------ sendMaster()
 |
 | getMaster() <-----
 |
 | sendSlave() sends the state of a slave to the master.
 | getSlave() get the slaves' states.
 | sendMaster() sends the state of the master to the slaves.
 | getMaster() gets the master's state.
 |
 | Special flags in the network object:
 |
 | Master view:
 | masterSynch - master needs to re-synch global state of slaves.
 | slaveSynch - slave requesting re-synch.
 | Notes:
 | 1. If masterSynch is set after getSlave() call, then
 | application should load global state before calling
 | sendMaster().
 | 2. Application can force a global re-synch by setting
 | the masterSynch flag and loading the global state before
 | sendMaster() call.
 |
 | Slave view:
 | masterSynch - master is re-synching all slaves.
 | slaveSynch - this slave is requesting a re-synch.
 | newMaster - master has passed mastership to this slave.
 | Notes:
 | 1. If masterSynch is set after getMaster() call, application
 | should expect global state information in message.
 | 2. Application can set slaveSynch flag to request a global
 | re-synch of all slaves by master.
 |
 */
void ScorchedMars::DoNetwork()
{
   int i;

   m_currentCannon = network->myIndex;
   if (network->master)
   {
      if (!network->getSlave())
      {
         fprintf(stderr, "getSlave failed: %s\n", network->statusMessage);
         sprintf(m_errorMsg, "getSlave failed: %s", network->statusMessage);
         m_state = ERR;
         return;
      }
      if (network->masterSynch)
      {
         // New player, etc.: clear flying cannonballs.
         m_cannonBalls->Clear();
      }
      for (i = 0; i < NUM_CANNONS; i++)
      {
         if (i == m_currentCannon)
         {
            continue;
         }
         if (network->currentPlayers[i])
         {
            memcpy(&m_gameState.cannons[i], network->slavePayloads[i].data, sizeof(struct GAME_STATE::CANNON_STATE));
            m_cannons[i]->SetColor(m_gameState.cannons[i].color, m_Light);
            m_cannons[i]->SetPosition(m_gameState.cannons[i].position);
            if (m_cannonNodes[i]->GetNumChildren() == 0)
            {
               m_cannonNodes[i]->AttachChild(m_cannons[i]->GetBaseNode());
            }
            m_cannons[i]->SetSwivel(m_gameState.cannons[i].swivel);
            m_cannons[i]->SetElevation(m_gameState.cannons[i].elevation);
            m_cannons[i]->SetCharge(m_gameState.cannons[i].charge);
            if (m_gameState.cannons[i].firing)
            {
               m_cannonBalls->Add(m_cannons[i]->Fire());
            }
            m_cannonNodes[i]->Update();
         }
         else
         {
            if (m_cannonNodes[i]->GetNumChildren() == 1)
            {
               m_cannonNodes[i]->DetachChild(m_cannons[i]->GetBaseNode());
               m_cannonNodes[i]->Update();
            }
         }
      }

      // Synchronize slaves.
      m_gameState.windVector = m_windVector;
      m_gameState.cannons[m_currentCannon].color     = m_cannons[m_currentCannon]->GetColor();
      m_gameState.cannons[m_currentCannon].position  = m_cannons[m_currentCannon]->GetPosition();
      m_gameState.cannons[m_currentCannon].swivel    = m_cannons[m_currentCannon]->GetSwivel();
      m_gameState.cannons[m_currentCannon].elevation = m_cannons[m_currentCannon]->GetElevation();
      m_gameState.cannons[m_currentCannon].charge    = m_cannons[m_currentCannon]->GetCharge();
      strncpy(m_gameState.cannons[m_currentCannon].name, m_name, NAME_SIZE - 1);
      m_gameState.cannons[m_currentCannon].name[NAME_SIZE - 1] = '\0';
      network->masterPayload.size = sizeof(struct GAME_STATE);
      assert(network->masterPayload.size <= MAX_MASTER_PAYLOAD);
      memcpy(network->masterPayload.data, &m_gameState, sizeof(struct GAME_STATE));
      if (!network->sendMaster())
      {
         fprintf(stderr, "sendMaster failed: %s\n", network->statusMessage);
         sprintf(m_errorMsg, "sendMaster failed: %s", network->statusMessage);
         m_state = ERR;
         return;
      }
   }
   else                                           // slave.
   {
      // Send slave state to master.
      m_gameState.cannons[m_currentCannon].color     = m_cannons[m_currentCannon]->GetColor();
      m_gameState.cannons[m_currentCannon].position  = m_cannons[m_currentCannon]->GetPosition();
      m_gameState.cannons[m_currentCannon].swivel    = m_cannons[m_currentCannon]->GetSwivel();
      m_gameState.cannons[m_currentCannon].elevation = m_cannons[m_currentCannon]->GetElevation();
      m_gameState.cannons[m_currentCannon].charge    = m_cannons[m_currentCannon]->GetCharge();
      strncpy(m_gameState.cannons[m_currentCannon].name, m_name, NAME_SIZE - 1);
      m_gameState.cannons[m_currentCannon].name[NAME_SIZE - 1] = '\0';
      memcpy(network->slavePayloads[m_currentCannon].data, &m_gameState.cannons[m_currentCannon],
             sizeof(struct GAME_STATE::CANNON_STATE));
      network->slavePayloads[m_currentCannon].size = sizeof(struct GAME_STATE::CANNON_STATE);
      if (!network->sendSlave())
      {
         fprintf(stderr, "sendSlave failed: %s\n", network->statusMessage);
         sprintf(m_errorMsg, "sendSlave failed: %s", network->statusMessage);
         m_state = ERR;
         return;
      }

      // Get master message.
      if (!network->getMaster())
      {
         fprintf(stderr, "getMaster failed: %s\n", network->statusMessage);
         sprintf(m_errorMsg, "getMaster failed: %s", network->statusMessage);
         m_state = ERR;
         return;
      }
      if (network->masterSynch)
      {
         // New player, etc.: clear flying cannonballs.
         m_cannonBalls->Clear();
      }
      if (network->masterTimeouts == 0)
      {
         // Copy wind and new cannon states.
         m_windVector = m_gameState.windVector;
         memcpy(&m_gameState, network->masterPayload.data, sizeof(struct GAME_STATE));
         for (i = 0; i < NUM_CANNONS; i++)
         {
            if (network->currentPlayers[i])
            {
               m_cannons[i]->SetColor(m_gameState.cannons[i].color, m_Light);
               m_cannons[i]->SetPosition(m_gameState.cannons[i].position);
               if (m_cannonNodes[i]->GetNumChildren() == 0)
               {
                  m_cannonNodes[i]->AttachChild(m_cannons[i]->GetBaseNode());
               }
               m_cannons[i]->SetSwivel(m_gameState.cannons[i].swivel);
               m_cannons[i]->SetElevation(m_gameState.cannons[i].elevation);
               m_cannons[i]->SetCharge(m_gameState.cannons[i].charge);
               if ((i != m_currentCannon) && m_gameState.cannons[i].firing)
               {
                  m_cannonBalls->Add(m_cannons[i]->Fire());
               }
               m_cannonNodes[i]->Update();
            }
            else
            {
               if (m_cannonNodes[i]->GetNumChildren() == 1)
               {
                  m_cannonNodes[i]->DetachChild(m_cannons[i]->GetBaseNode());
                  m_cannonNodes[i]->Update();
               }
            }
         }
      }
      if (network->newMaster)
      {
         // Slaves are waiting for master message.
         if (!network->sendMaster())
         {
            fprintf(stderr, "sendMaster failed: %s\n", network->statusMessage);
            sprintf(m_errorMsg, "sendMaster failed: %s", network->statusMessage);
            m_state = ERR;
            return;
         }
      }
   }

   // Clear firing state.
   m_gameState.cannons[m_currentCannon].firing = false;
}


#endif

//----------------------------------------------------------------------------
void ScorchedMars::MoveForward()
{
   WindowApplication3::MoveForward();

   APoint camPosition = mCamera->GetPosition();
   float height       = m_Terrain->GetHeight(camPosition[0], camPosition[1]);
   camPosition[2] = height + m_HeightAboveTerrain;
   mCamera->SetPosition(camPosition);
}


//----------------------------------------------------------------------------
void ScorchedMars::MoveBackward()
{
   WindowApplication3::MoveBackward();

   APoint camPosition = mCamera->GetPosition();
   float height       = m_Terrain->GetHeight(camPosition[0], camPosition[1]);
   camPosition[2] = height + m_HeightAboveTerrain;
   mCamera->SetPosition(camPosition);
}


//----------------------------------------------------------------------------
void ScorchedMars::MoveDown()
{
   if (m_HeightAboveTerrain >= mTrnSpeed)
   {
      m_HeightAboveTerrain -= mTrnSpeed;
   }

   APoint camPosition = mCamera->GetPosition();
   float height       = m_Terrain->GetHeight(camPosition[0], camPosition[1]);
   camPosition[2] = height + m_HeightAboveTerrain;
   mCamera->SetPosition(camPosition);
}


//----------------------------------------------------------------------------
void ScorchedMars::MoveUp()
{
   m_HeightAboveTerrain += mTrnSpeed;

   APoint camPosition = mCamera->GetPosition();
   float height       = m_Terrain->GetHeight(camPosition[0], camPosition[1]);
   camPosition[2] = height + m_HeightAboveTerrain;
   mCamera->SetPosition(camPosition);
}


// Set camera view to cannon.
void ScorchedMars::SetCannonView(Cannon *cannon)
{
#ifdef NEVER
   // Third-person view with level camera.
   Vector3f aim  = cannon->GetAimingVector();
   Vector3f loc  = cannon->GetPosition();
   Vector3f cDir = aim;
   cDir.Z() = 0.0f;
   cDir.Normalize();
   Vector3f cLoc = loc - (cDir * 30.0f);
   cLoc.Z() = cLoc.Z() + 20.0f;
   Vector3f cUp(0.0f, 0.0f, 1.0f);
   Vector3f cRight = cDir.Cross(cUp);
   mCamera->SetFrame(cLoc, cDir, cUp, cRight);
#endif
   // First-person view with camera looking down cannon barrel.
   Quaternionf quat  = Quaternionf(cannon->GetRotate());
   Vector3f kDVector = quat.Rotate(Vector3f(0.0f, 0.0f, 1.0f));
   kDVector.Normalize();
   Vector3f kUVector = quat.Rotate(Vector3f(0.0f, 1.0f, 0.0f));
   kUVector.Normalize();
   Vector3f kRVector = quat.Rotate(Vector3f(-1.0f, 0.0f, 0.0f));
   kRVector.Normalize();
   mCamera->SetAxes(kDVector, kUVector, kRVector);
#ifdef NEVER
   // Over barrel view.
   Vector3f kLoc = quat.Rotate(Vector3f(0.0f, 5.0f, 2.0f));
   kLoc = kLoc + cannon->GetPosition();
   mCamera->SetLocation(kLoc);
#endif
   mCamera->SetPosition(cannon->GetPosition());
}


// Draw splash screen.
void ScorchedMars::ShowSplash()
{
   int i, x, y;
   GLfloat w, h, r, g, b, to2, fade;
   GLfloat saveClear[4];
   TIME t;

   enter2Dmode();
   glGetFloatv(GL_COLOR_CLEAR_VALUE, saveClear);
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   w = (GLfloat)GetWidth();
   h = (GLfloat)GetHeight();
   glTranslatef((w / 2.0f) - (m_bitmap.width / 2.0f),
                (h / 2.0f) - (m_bitmap.height / 2.0f), 0.0f);
   t     = gettime() - m_splashStart;
   to2   = (GLfloat)(SPLASH_TIMEOUT / 2);
   fade  = Mathf::FAbs(to2 - (GLfloat)t);
   fade /= to2;
   fade  = 1.0f - fade;
   glBegin(GL_POINTS);
   for (y = 0; y < m_bitmap.height; y++)
   {
      for (x = 0; x < m_bitmap.width; x++)
      {
         i = ((y * m_bitmap.width) + x) * 3;
         r = fade * (GLfloat)(m_bitmap.rgb_data[i] / 255);
         g = fade * (GLfloat)(m_bitmap.rgb_data[i + 1] / 255);
         b = fade * (GLfloat)(m_bitmap.rgb_data[i + 2] / 255);
         glColor3f(r, g, b);
         glVertex3f((GLfloat)x, (GLfloat)(m_bitmap.height - y), 0.0f);
      }
   }
   glEnd();
   glPopMatrix();
   glClearColor(saveClear[0], saveClear[1], saveClear[2], saveClear[3]);
   exit2Dmode();

   // Splash finished?
   if (t >= SPLASH_TIMEOUT)
   {
      // Free the bitmap data.
      glbmp_FreeBitmap(&m_bitmap);
      m_state = INIT;
   }
}


// Draw init screen.
void ScorchedMars::ShowInit()
{
   char nbuf[NAME_SIZE + 10];

   sprintf(nbuf, "Name: %s", m_name);
   mRenderer->Draw(20, (GetHeight() / 2) - 10,
                   Float4(1.0f, 1.0f, 1.0f, 1.0f), nbuf);
#ifdef NETWORK
   char hbuf[HOST_NAME_SIZE + 20];
   sprintf(hbuf, "Connect to: %s", m_masterHost);
   mRenderer->Draw(20, (GetHeight() / 2) + 10,
                   Float4(1.0f, 1.0f, 1.0f, 1.0f), hbuf);
#endif
}


// Draw status screen.
void ScorchedMars::ShowStatus()
{
   Float4 white(1.0f, 1.0f, 1.0f, 1.0f);

   mRenderer->Draw(GetWidth() - 20, 20, white, "S");
   mRenderer->Draw(GetWidth() - 20, 40, white, "T");
   mRenderer->Draw(GetWidth() - 20, 60, white, "A");
   mRenderer->Draw(GetWidth() - 20, 80, white, "T");
   mRenderer->Draw(GetWidth() - 20, 100, white, "U");
   mRenderer->Draw(GetWidth() - 20, 120, white, "S");
   mRenderer->Draw(GetWidth() - 20, 140, white, "");
   mRenderer->Draw(GetWidth() - 20, 160, white, "S");
   mRenderer->Draw(GetWidth() - 20, 180, white, "C");
   mRenderer->Draw(GetWidth() - 20, 200, white, "R");
   mRenderer->Draw(GetWidth() - 20, 220, white, "E");
   mRenderer->Draw(GetWidth() - 20, 240, white, "E");
   mRenderer->Draw(GetWidth() - 20, 260, white, "N");

   int posCounter = 0;
   int posTop     = 100;
   mRenderer->Draw(150, posTop + (posCounter * 20), white, "Name");
   mRenderer->Draw(300, posTop + (posCounter * 20), white, "Kills");
   posCounter++;
   mRenderer->Draw(150, posTop + (posCounter * 20), white, "- - - - - - - - - - - - - - - - - - - - - - - ");
   posCounter++;

#ifdef NETWORK
   char buf[NAME_SIZE];
   for (int i = 0; i < NUM_CANNONS; i++)
   {
      if (network->currentPlayers[i])
      {
         float *cp = m_gameState.cannons[i].color;
         Float4 color(cp[0], cp[1], cp[2], 1.0f);
         sprintf(buf, "%s", m_gameState.cannons[i].name);
         mRenderer->Draw(150, posTop + (posCounter * 20), color, buf);
         sprintf(buf, "%d", m_gameState.cannons[i].score);
         mRenderer->Draw(300, posTop + (posCounter * 20), color, buf);
         posCounter++;
      }
   }
#endif
}


// Draw help screen.
void ScorchedMars::ShowHelp()
{
   Float4 white(1.0f, 1.0f, 1.0f, 1.0f);

   mRenderer->Draw(GetWidth() - 20, 20, white, "H");
   mRenderer->Draw(GetWidth() - 20, 40, white, "E");
   mRenderer->Draw(GetWidth() - 20, 60, white, "L");
   mRenderer->Draw(GetWidth() - 20, 80, white, "P");
   mRenderer->Draw(GetWidth() - 20, 100, white, "");
   mRenderer->Draw(GetWidth() - 20, 120, white, "S");
   mRenderer->Draw(GetWidth() - 20, 140, white, "C");
   mRenderer->Draw(GetWidth() - 20, 160, white, "R");
   mRenderer->Draw(GetWidth() - 20, 180, white, "E");
   mRenderer->Draw(GetWidth() - 20, 200, white, "E");
   mRenderer->Draw(GetWidth() - 20, 220, white, "N");

   mRenderer->Draw(150, 60, white, "Up ........ Rotate cannon left (aim more to the left)");
   mRenderer->Draw(150, 90, white, "Down ...... Rotate cannon right (aim more to the right)");
   mRenderer->Draw(150, 120, white, "Left ...... Rotate cannon left (aim more to the left)");
   mRenderer->Draw(150, 150, white, "Right ..... Rotate cannon right (aim more to the right)");
   mRenderer->Draw(150, 180, white, "PgUp....... Raise the cannon (aim more upward)");
   mRenderer->Draw(150, 210, white, "PgDown .... Lower the cannon (aim more downward)");
   mRenderer->Draw(150, 240, white, "SPACEBAR .. Fire a cannon ball");
   mRenderer->Draw(150, 270, white, "G ......... Increase shot power");
   mRenderer->Draw(150, 300, white, "F ......... Decrease shot power");
   mRenderer->Draw(150, 330, white, "H ......... Open help screen (this one you're looking at)");
   mRenderer->Draw(150, 360, white, "S ......... Status screen to see who's in game and scores");
   mRenderer->Draw(150, 390, white, "ESC ....... Quit");
}


// Draw error screen.
void ScorchedMars::ShowError()
{
   mRenderer->Draw(8, 16, Float4(1.0f, 1.0f, 1.0f, 1.0f), m_errorMsg);
}


// Draw death screen.
void ScorchedMars::ShowDeath()
{
   const TIME deathTimeout = 3000;
   static TIME startTime   = 0;
   TIME t;

   t = gettime();
   if (startTime == 0)
   {
      startTime = t;
   }
   mRenderer->Draw((GetWidth() / 2) - 20, GetHeight() / 2,
                   Float4(1.0f, 1.0f, 1.0f, 1.0f), "You Lose");
   if ((t - startTime) >= deathTimeout)
   {
      m_state = QUIT;
   }
}


// Draw user interface displays.
void ScorchedMars::DrawGUI()
{
   Vector3f needle;
   const float radius = 30.0f;
   Float4 white(1.0f, 1.0f, 1.0f, 1.0f);
   Float4 black(0.0f, 0.0f, 0.0f, 1.0f);

   // Draw wind indicator.
   needle     = m_windVector;
   needle    *= (radius / fMaxWindSpeed);
   needle.X() = -needle.X();
   DrawGauge(GetWidth() - (radius * 1.1f), radius * 1.1f,
             radius, needle);
   mRenderer->Draw(GetWidth() - (int)(1.65f * radius), (int)(2.5f * radius),
                   black, "wind");

   // Draw heading indicator.
   AVector v = mCamera->GetDVector();
   needle.X() = v.X();
   needle.Y() = v.Y();
   needle.Z() = 0.0f;
   needle.Normalize();
   needle    *= radius;
   needle.X() = -needle.X();
   DrawGauge(GetWidth() - (3.0f * (radius * 1.1f)), radius * 1.1f,
             radius, needle);
   mRenderer->Draw(GetWidth() - (int)(3.75f * radius), (int)(2.5f * radius),
                   black, "dir");

   // Displays the number of cannons in the world.
   ShowNumCannons(8, 16);

   // Show frame rate.
   DrawFrameRate(8, GetHeight() - 8, white);

   // Show crosshairs.
   DrawCrosshairs(10.0f);

   mRenderer->Draw(GetWidth() - 100, GetHeight() - 10,
                   white, "'H' for help");
}


// Draw a gauge.
void ScorchedMars::DrawGauge(GLfloat cx, GLfloat cy,
                             GLfloat radius, Vector3f& needle)
{
   GLfloat a, ad, x, y;
   const int sides = 20;

   enter2Dmode();
   glColor3f(0.0f, 0.0f, 0.0f);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   glTranslatef(cx, cy, 0.0f);
   glLineWidth(2.0f);
   glBegin(GL_LINE_LOOP);
   a  = 0.0f;
   ad = 360.0f / sides;
   for (int i = 0; i < sides; i++, a += ad)
   {
      x  = Mathf::Cos(a * (Mathf::PI / 180.0f));
      y  = Mathf::Sin(a * (Mathf::PI / 180.0f));
      x *= radius;
      y *= radius;
      glVertex2f(x, y);
   }
   glEnd();
   glBegin(GL_LINES);
   glVertex2f(radius, 0.0f);
   glVertex2f(radius * 0.75f, 0.0f);
   glVertex2f(0.0f, radius);
   glVertex2f(0.0f, radius * 0.75f);
   glVertex2f(-radius, 0.0f);
   glVertex2f(-radius * 0.75f, 0.0f);
   glVertex2f(0.0f, -radius);
   glVertex2f(0.0f, -radius * 0.75f);
   glEnd();
   glBegin(GL_LINES);
   glVertex2f(0.0f, 0.0f);
   glVertex2f(needle.X(), needle.Y());
   glEnd();
   glLineWidth(1.0f);
   glPopMatrix();
   exit2Dmode();
}


// Draw crosshairs.
void ScorchedMars::DrawCrosshairs(GLfloat radius)
{
   GLfloat a, ad, x, y;
   const int sides = 20;

   enter2Dmode();
   glColor3f(0.0f, 0.0f, 0.0f);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   glTranslatef(GetWidth() / 2.0f, GetHeight() / 2.0f, 0.0f);
   glLineWidth(1.0f);
   glBegin(GL_LINE_LOOP);
   a  = 0.0f;
   ad = 360.0f / sides;
   for (int i = 0; i < sides; i++, a += ad)
   {
      x  = Mathf::Cos(a * (Mathf::PI / 180.0f));
      y  = Mathf::Sin(a * (Mathf::PI / 180.0f));
      x *= radius;
      y *= radius;
      glVertex2f(x, y);
   }
   glEnd();
   glBegin(GL_LINES);
   glVertex2f(radius, 0.0f);
   glVertex2f(-radius, 0.0f);
   glVertex2f(0.0f, radius);
   glVertex2f(0.0f, -radius);
   glEnd();
   glPopMatrix();
   exit2Dmode();
}


// Show number of cannons in world.
void ScorchedMars::ShowNumCannons(GLfloat x, GLfloat y)
{
   int i, j;
   const size_t uiSize = 256;
   char acMessage[uiSize];

#ifdef NETWORK
   for (i = j = 0; i < NUM_CANNONS; i++)
   {
      if (network->currentPlayers[i])
      {
         j++;
      }
   }
   sprintf(acMessage, "Cannons=%d", j);
#else
   for (i = j = 0; i < NUM_CANNONS; i++)
   {
      if (m_cannons[i] != NULL)
      {
         j++;
      }
   }
   sprintf(acMessage, "Cannons=%d", j);
#endif
   mRenderer->Draw((int)x, (int)y, Float4(0.0f, 0.0f, 0.0f, 1.0f), acMessage);
}


// GUI 2D mode.
void ScorchedMars::enter2Dmode()
{
   glDisable(GL_BLEND);
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_LIGHTING);

   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   gluOrtho2D(0, GetWidth(), 0, GetHeight());

   // Invert the y axis, down is positive.
   glScalef(1, -1, 1);

   // Move the origin from the bottom left corner to the upper left corner.
   glTranslatef(0.0f, -(GLfloat)GetHeight(), 0.0f);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
}


void ScorchedMars::exit2Dmode()
{
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
}
