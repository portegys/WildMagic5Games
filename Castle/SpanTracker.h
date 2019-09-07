// SpanTracker.

#ifndef SPANTRACKER_H
#define SPANTRACKER_H

#include "Wm5WindowApplication3.h"
#include "TimeUtils.h"
#include "EasyGL.h"
#include "gettime.h"
#include "RigidBall.h"
#include "integrator/integrator.h"
#include <GL/glu.h>
using namespace Wm5;
using namespace std;

class Castle;

class SpanTracker
{
   friend class Castle;

public:

   // Constructor.
   SpanTracker(Castle *castle, string guiPath);

   // Destructor.
   ~SpanTracker();

   // Toggle help mode.
   bool ToggleHelp();

   bool IsHelping() { return(mHelping); }

   // Toggle enable.
   bool ToggleEnable();

   bool IsEnabled() { return(mEnabled); }

   // Toggle logging.
   bool ToggleLogging();

   bool IsLogging() { return(mLogging); }

   // Rotation rate change.
   void RotationRateChange(float rate);

   // Movement rate change.
   void MovementRateChange(float rate);

   // Draw.
   void Draw(int width, int height);

   // Mouse.
   bool OnMouseClick(int button, int state, int x, int y);

   // Track targets.
   bool TrackStartTarget();
   bool TrackEndTarget();

   // Update device position.
   void UpdateDevicePosition();

   // Get target position and spanning distance.
   APoint GetTargetPosition();
   float GetDistance();

   // Integrate device movement.
   void MoveForward();
   void MoveBackward();
   void TurnLeft();
   void TurnRight();
   void LookUp();
   void LookDown();
   void AdjustVerticalDistance(float distance);

   static float INTEGRATOR_SIGNAL_NOISE_RATIO;
   Integrator   *mIntegrator;
   Integrator   *mIntegrator_noisy;

   // Change integrator signal-to-noise ratio.
   float SignalNoiseRatioDecrease();
   float SignalNoiseRatioIncrease();

   // Reset device.
   void Reset();

private:

   // Drawing.
   void DrawCrosshairs(GLfloat radius);
   void DrawInstrument();
   void ShowHelp();
   void enter2Dmode(int width, int height);
   void exit2Dmode();

   // Configuration.
   Castle      *mCastle;
   bool        mHelping;
   bool        mEnabled;
   static char *LOGFILE_NAME;
   bool        mLogging;
   int         mWidth, mHeight;

   // GUI.
   string mGuiPath;
   enum { GUI_WIDTH_DIVISOR=3, GUI_HEIGHT_DIVISOR=3 };
   int guiWidth, guiHeight;
   class EventsHandler : public GUIEventListener
   {
public:
      virtual void actionPerformed(GUIEvent& evt);

      SpanTracker *client;
   };
   EventsHandler handler;
   FPSCounter    counter;
   GUIFrame      *guiFrame;
   bool          startTargetIsTracked;
   GUICheckBox   *startTargetTrackCheck;
   bool          endTargetIsTracked;
   GUICheckBox   *endTargetTrackCheck;
   GUITextBox    *devicePositionText;
   GUITextBox    *targetPositionText;
   GUITextBox    *targetDistanceText;

   // Device rotation and position.
   Matrix3f mDeviceRotation;
   Vector3f mDevicePosition;

   // Get rotation vectors from matrix.
   Vector3f getForwardVector(Matrix3f& rotation);
   Vector3f getUpVector(Matrix3f& rotation);
   Vector3f getRightVector(Matrix3f& rotation);

   // Set rotation vectors into matrix.
   void setForwardVector(Matrix3f& rotation, Vector3f& forward);
   void setUpVector(Matrix3f& rotation, Vector3f& up);
   void setRightVector(Matrix3f& rotation, Vector3f& right);

   // Pick target point.
   float PickTarget(int direction, APoint& target);

   // Maximum tracking range.
   const float MAX_RANGE = 100.0f;

   // Marker colors.
   const Float3 startColor = Float3(1.0f, 0.0f, 0.0f);
   const Float3 endColor   = Float3(0.0f, 0.0f, 1.0f);

   // Target marker.
   class Marker
   {
public:

      // Marker radius.
      const float radius = 0.1f;

      // Constructor.
      Marker(Float3 color, Light *light, Node *parent)
      {
         m_parent = parent;
         m_node   = new0 Node();
         m_ball   = new0 RigidBall(radius, color, light);
         m_node->LocalTransform.SetTranslate(m_ball->GetPosition());
         m_node->AttachChild(m_ball->Mesh());
         attached = false;
      }


      // Destructor.
      ~Marker()
      {
         delete0(m_ball);
      }


      // Set position.
      void SetPosition(APoint position)
      {
         m_node->LocalTransform.SetTranslate(position);
      }


      // Attach to parent node.
      void Attach()
      {
         if (!attached)
         {
            m_parent->AttachChild(m_node);
            attached = true;
         }
         m_parent->Update();
      }


      // Detach from parent node.
      void Detach()
      {
         if (attached)
         {
            m_parent->DetachChild(m_node);
            attached = false;
         }
         m_parent->Update();
      }


      NodePtr   m_parent;
      NodePtr   m_node;
      bool      attached;
      RigidBall *m_ball;
   };

   // Tracker.
   class Tracker
   {
public:
      bool   tracking;
      APoint targetPosition;
      APoint devicePosition;
      Marker *targetMarker;

      Tracker(Float3 color, Light *light, Node *parent)
      {
         tracking     = false;
         targetMarker = new Marker(color, light, parent);
      }


      ~Tracker()
      {
         delete targetMarker;
      }


      void Track(APoint targetPosition, APoint devicePosition)
      {
         this->targetPosition = targetPosition;
         this->devicePosition = devicePosition;
         targetMarker->SetPosition(targetPosition);
         targetMarker->Attach();
         tracking = true;
      }


      void Untrack()
      {
         this->targetPosition = Vector3f::ZERO;
         this->devicePosition = Vector3f::ZERO;
         targetMarker->SetPosition(Vector3f::ZERO);
         targetMarker->Detach();
         tracking = false;
      }


      void ShowMarker()
      {
         if (tracking) { targetMarker->Attach(); }
      }


      void HideMarker()
      {
         if (tracking) { targetMarker->Detach(); }
      }
   };
   enum MARKER_DIRECTION
   {
      FORWARD    =0,
      UP         =1,
      DOWN       =2,
      LEFT       =3,
      RIGHT      =4,
      NUM_MARKERS=5
   };
   Tracker *start;
   Tracker *end;
   char    positionBuf[BUFSIZ];
   char    distanceBuf[BUFSIZ];
};
#endif
