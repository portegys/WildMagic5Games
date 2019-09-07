// SpanTracker.
#include "SpanTracker.h"
#include "Castle.h"

// Integrator noise.
float SpanTracker::INTEGRATOR_SIGNAL_NOISE_RATIO = 2.0f;

// Logfile name.
char *SpanTracker::LOGFILE_NAME = "spantracker.log";

SpanTracker::SpanTracker(Castle *castle, string guiPath)
{
   mCastle  = castle;
   mGuiPath = guiPath;
   NodePtr marker = new0 Node();
   castle->mScene->AttachChild(marker);
   start             = new Tracker(startColor, castle->mDLight, marker);
   end               = new Tracker(endColor, castle->mDLight, marker);
   mDeviceRotation   = Matrix3f::IDENTITY;
   mDevicePosition   = Vector3f::ZERO;
   mIntegrator       = new Integrator(0.0f, 0.0f, 0.0f);
   mIntegrator_noisy = new Integrator(0.0f, 0.0f, 0.0f, true, INTEGRATOR_SIGNAL_NOISE_RATIO);
   mHelping          = false;
   mEnabled          = true;
   mLogging          = false;
   mWidth            = mHeight = guiWidth = guiHeight = -1;
   MediaPathManager::registerPath(guiPath + "resource/");
   guiFrame = new GUIFrame();
   guiFrame->GUIPanel::loadXMLSettings("SpanTrackerGUILayout.xml");
   startTargetIsTracked  = false;
   startTargetTrackCheck = (GUICheckBox *)guiFrame->getWidgetByCallbackString("Start");
   startTargetTrackCheck->setAlphaFadeScale(1000.0);
   endTargetIsTracked  = false;
   endTargetTrackCheck = (GUICheckBox *)guiFrame->getWidgetByCallbackString("End");
   endTargetTrackCheck->setAlphaFadeScale(1000.0);
   devicePositionText = (GUITextBox *)guiFrame->getWidgetByCallbackString("DevicePosition");
   targetPositionText = (GUITextBox *)guiFrame->getWidgetByCallbackString("TargetPosition");
   targetDistanceText = (GUITextBox *)guiFrame->getWidgetByCallbackString("TargetDistance");
   handler.client     = this;
   guiFrame->setGUIEventListener(&handler);
}


// Destructor.
SpanTracker::~SpanTracker()
{
   delete start;
   delete end;
   delete mIntegrator;
   delete mIntegrator_noisy;
}


// Toggle help mode.
bool SpanTracker::ToggleHelp()
{
   mHelping = !mHelping;
   return(mHelping);
}


// Toggle enablement.
bool SpanTracker::ToggleEnable()
{
   mEnabled = !mEnabled;
   if (mEnabled)
   {
      start->ShowMarker();
      end->ShowMarker();
   }
   else
   {
      start->HideMarker();
      end->HideMarker();
   }
   mCastle->mScene->Update();
   mCastle->mCuller.ComputeVisibleSet(mCastle->mScene);
   return(mEnabled);
}


// Toggle logging.
bool SpanTracker::ToggleLogging()
{
   time_t    rawtime;
   struct tm *timeinfo;

   time(&rawtime);
   timeinfo = localtime(&rawtime);
   FILE *fp = fopen(LOGFILE_NAME, "a");
   mLogging = !mLogging;
   if (mLogging)
   {
      mIntegrator->logfile       = LOGFILE_NAME;
      mIntegrator_noisy->logfile = LOGFILE_NAME;
      fprintf(fp, "Logging start: %s", asctime(timeinfo));
   }
   else
   {
      mIntegrator->logfile       = NULL;
      mIntegrator_noisy->logfile = NULL;
      fprintf(fp, "Logging stop: %s", asctime(timeinfo));
   }
   fclose(fp);
   return(mLogging);
}


// Integrator signal-to-noise ratio.
float SpanTracker::SignalNoiseRatioDecrease()
{
   float ratio = mIntegrator_noisy->signal_noise_decrease();

   if (mLogging)
   {
      FILE *fp = fopen(LOGFILE_NAME, "a");
      fprintf(fp, "Signal-to-noise ratio=%f\n", ratio);
      fclose(fp);
   }
   return(ratio);
}


float SpanTracker::SignalNoiseRatioIncrease()
{
   float ratio = mIntegrator_noisy->signal_noise_increase();

   if (mLogging)
   {
      FILE *fp = fopen(LOGFILE_NAME, "a");
      fprintf(fp, "Signal-to-noise ratio=%f\n", ratio);
      fclose(fp);
   }
   return(ratio);
}


// Rotation rate change.
void SpanTracker::RotationRateChange(float rate)
{
   if (mLogging)
   {
      FILE *fp = fopen(LOGFILE_NAME, "a");
      fprintf(fp, "Rotation rate=%f\n", rate);
      fclose(fp);
   }
}


// Movement rate change.
void SpanTracker::MovementRateChange(float rate)
{
   if (mLogging)
   {
      FILE *fp = fopen(LOGFILE_NAME, "a");
      fprintf(fp, "Movement rate=%f\n", rate);
      fclose(fp);
   }
}


// Draw.
void SpanTracker::Draw(int width, int height)
{
   if ((width != mWidth) || (height != mHeight))
   {
      mWidth    = width;
      mHeight   = height;
      guiWidth  = width / GUI_WIDTH_DIVISOR;
      guiHeight = height / GUI_HEIGHT_DIVISOR;
      guiFrame->setDimensions((float)guiWidth, (float)guiHeight);
      guiFrame->forceUpdate(true);
   }
   if (mHelping)
   {
      ShowHelp();
      return;
   }
   if (mEnabled)
   {
      DrawCrosshairs(10.0f);
      DrawInstrument();
   }
}


// Draw crosshairs.
void SpanTracker::DrawCrosshairs(GLfloat radius)
{
   GLfloat   a, ad, x, y;
   const int sides = 20;

   enter2Dmode(mWidth, mHeight);
   glColor3f(1.0f, 0.0f, 0.5f);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   glTranslatef(mWidth / 2.0f, mHeight / 2.0f, 0.0f);
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
   glLineWidth(1.0f);

   /*
    * glBegin(GL_LINES);
    * glVertex2f(radius, 0.0f);
    * glVertex2f(-radius, 0.0f);
    * glVertex2f(0.0f, radius);
    * glVertex2f(0.0f, -radius);
    */
   glEnd();
   glPopMatrix();
   glColor3f(0.0f, 0.0f, 0.0f);
   exit2Dmode();
}


// Draw instrument.
void SpanTracker::DrawInstrument()
{
   glViewport(0, 0, guiWidth, guiHeight);
   enter2Dmode(guiWidth, guiHeight);
   counter.markFrameStart();
   guiFrame->render(counter.getFrameInterval());
   counter.markFrameEnd();
   exit2Dmode();
   glViewport(0, 0, mWidth, mHeight);
}


// Show help screen.
void SpanTracker::ShowHelp()
{
   int    v = 20;
   int    s = 20;
   char   buf[100];
   Float4 black(0.0f, 0.0f, 0.0f, 1.0f);

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   mCastle->mRenderer->Draw(20, v, black, "Help:");
   v += (2 * s);
   sprintf(buf, "T/t: +/- movement rate (current value=%f)", mCastle->mTrnSpeed);
   mCastle->mRenderer->Draw(20, v, black, buf);
   v += s;
   sprintf(buf, "R/r: +/- rotation rate (current value=%f)", mCastle->mRotSpeed);
   mCastle->mRenderer->Draw(20, v, black, buf);
   v += s;
   sprintf(buf, "S/s: +/- integrator signal-to-noise ratio (current value=%f)", mIntegrator_noisy->signal_noise_ratio);
   mCastle->mRenderer->Draw(20, v, black, buf);
   v += s;
   mCastle->mRenderer->Draw(20, v, black, "a: perform automatic measurement");
   v += s;
   if (mLogging)
   {
      sprintf(buf, "l: toggle logging to file %s (currently logging)", LOGFILE_NAME);
   }
   else
   {
      sprintf(buf, "l: toggle logging to file %s (currently not logging)", LOGFILE_NAME);
   }
   mCastle->mRenderer->Draw(20, v, black, buf);
   v += (2 * s);
   mCastle->mRenderer->Draw(20, v, black, "Hit any key to return");
}


// GUI 2D mode.
void SpanTracker::enter2Dmode(int width, int height)
{
   glDisable(GL_BLEND);
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_LIGHTING);

   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   gluOrtho2D(0, width, 0, height);

   // Invert the y axis, down is positive.
   glScalef(1, -1, 1);

   // Move the origin from the bottom left corner to the upper left corner.
   glTranslatef(0.0f, -(GLfloat)height, 0.0f);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
}


void SpanTracker::exit2Dmode()
{
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
}


// Mouse event callback.
bool SpanTracker::OnMouseClick(int button, int state, int x, int y)
{
   if (!mEnabled)
   {
      return(false);
   }
   if (button != WindowApplication::MOUSE_LEFT_BUTTON)
   {
      return(false);
   }
   if ((x < 0) || (x > guiWidth) || (y < (mHeight - guiHeight)))
   {
      return(false);
   }
   y -= (mHeight - guiHeight);
   MouseEvent event = MouseEvent(MB_BUTTON1, x, y, guiHeight - y);
   guiFrame->checkMouseEvents(event, (state == WindowApplication::MOUSE_DOWN) ? ME_CLICKED : ME_RELEASED);
   return(true);
}


// GUI event handler.
void SpanTracker::EventsHandler::actionPerformed(GUIEvent& evt)
{
   const std::string& callbackString   = evt.getCallbackString();
   GUIRectangle       *sourceRectangle = evt.getEventSource();
   int                widgetType       = sourceRectangle->getWidgetType();
   GUICheckBox        *checkbox;
   GUIButton          *button;

   switch (widgetType)
   {
   case WT_CHECK_BOX:
      checkbox = (GUICheckBox *)sourceRectangle;

      if (callbackString == "Start")
      {
         if (client->startTargetIsTracked)
         {
            client->startTargetTrackCheck->setChecked(true);
         }
         else
         {
            if (client->startTargetTrackCheck->isChecked())
            {
               if (client->TrackStartTarget())
               {
                  client->startTargetIsTracked = true;
                  client->UpdateDevicePosition();
               }
               else
               {
                  client->startTargetTrackCheck->setChecked(false);
               }
            }
         }
      }
      else if (callbackString == "End")
      {
         if (client->startTargetIsTracked)
         {
            if (client->endTargetIsTracked)
            {
               client->endTargetTrackCheck->setChecked(true);
            }
            else
            {
               if (client->endTargetTrackCheck->isChecked())
               {
                  if (client->TrackEndTarget())
                  {
                     client->endTargetIsTracked = true;
                     APoint position = client->GetTargetPosition();
                     memset(client->positionBuf, 0, BUFSIZ);
                     sprintf(client->positionBuf, "%.2f %.2f %.2f", position.X(), position.Y(), position.Z());
                     int len = strlen(client->positionBuf);
                     for (int i = len; i < 26; i++)
                     {
                        client->positionBuf[i] = ' ';
                     }
                     client->targetPositionText->setLabelString(client->positionBuf);
                     float distance = client->GetDistance();
                     memset(client->distanceBuf, 0, BUFSIZ);
                     sprintf(client->distanceBuf, "%.2f", distance);
                     len = strlen(client->distanceBuf);
                     for (int i = len; i < 16; i++)
                     {
                        client->distanceBuf[i] = ' ';
                     }
                     client->targetDistanceText->setLabelString(client->distanceBuf);
                  }
                  else
                  {
                     client->endTargetTrackCheck->setChecked(false);
                  }
               }
            }
         }
         else
         {
            client->endTargetTrackCheck->setChecked(false);
         }
      }
      break;

   case WT_BUTTON:
      button = (GUIButton *)sourceRectangle;

      if (callbackString == "Reset")
      {
         if (button->isClicked())
         {
            client->Reset();
         }
      }
      break;
   }
}


// Track start target.
bool SpanTracker::TrackStartTarget()
{
   APoint targetPosition;
   float  distance = PickTarget(0, targetPosition);

   if (distance < 0.0f)
   {
      if (mLogging)
      {
         FILE *fp = fopen(LOGFILE_NAME, "a");
         fprintf(fp, "Cannot start tracking\n");
         fclose(fp);
      }
      return(false);
   }
   start->Track(targetPosition, mCastle->mCamera->GetPosition());
   mDeviceRotation = Matrix3f::IDENTITY;
   mDevicePosition = Vector3f(0.0f, 0.0f, -distance);
   mIntegrator->init(0.0f, 0.0f, -distance);
   mIntegrator_noisy->init(0.0f, 0.0f, -distance);
   mCastle->mScene->Update();
   mCastle->mCuller.ComputeVisibleSet(mCastle->mScene);
   return(true);
}


// Track end target.
bool SpanTracker::TrackEndTarget()
{
   APoint targetPosition;
   float  distance = PickTarget(0, targetPosition);

   if (distance < 0.0f)
   {
      if (mLogging)
      {
         FILE *fp = fopen(LOGFILE_NAME, "a");
         fprintf(fp, "Cannot end tracking\n");
         fclose(fp);
      }
      return(false);
   }
   end->Track(targetPosition, mCastle->mCamera->GetPosition());
   if (mLogging)
   {
      FILE *fp = fopen(LOGFILE_NAME, "a");
      fprintf(fp, "integrated distance=%f, noisy distance=%f\n",
              mIntegrator->get_target_distance(distance), mIntegrator_noisy->get_target_distance(distance));
      fclose(fp);
   }
   mCastle->mScene->Update();
   mCastle->mCuller.ComputeVisibleSet(mCastle->mScene);
   return(true);
}


// Update device position.
void SpanTracker::UpdateDevicePosition()
{
   memset(positionBuf, 0, BUFSIZ);
   for (int i = 0; i < 26; i++)
   {
      positionBuf[i] = ' ';
   }
   if (start->tracking)
   {
      APoint pos = mCastle->mCamera->GetPosition();
      pos.X() = pos.X() - start->targetPosition.X();
      pos.Y() = pos.Y() - start->targetPosition.Y();
      pos.Z() = pos.Z() - start->targetPosition.Z();
      memset(positionBuf, 0, BUFSIZ);
      sprintf(positionBuf, "%.2f %.2f %.2f", pos.X(), pos.Y(), pos.Z());
      int len = strlen(positionBuf);
      for (int i = len; i < 26; i++)
      {
         positionBuf[i] = ' ';
      }
   }
   devicePositionText->setLabelString(positionBuf);
}


// Get target position.
APoint SpanTracker::GetTargetPosition()
{
   APoint result;

   result.X() = end->targetPosition.X() - start->targetPosition.X();
   result.Y() = end->targetPosition.Y() - start->targetPosition.Y();
   result.Z() = end->targetPosition.Z() - start->targetPosition.Z();
   return(result);
}


// Get spanning distance between endpoints.
float SpanTracker::GetDistance()
{
   return((end->targetPosition - start->targetPosition).Length());
}


// Get rotation vectors from matrix.
Vector3f SpanTracker::getForwardVector(Matrix3f& rotation)
{
   Vector3f forward;

   forward[0] = rotation[2][0];
   forward[1] = rotation[2][1];
   forward[2] = rotation[2][2];
   return(forward);
}


Vector3f SpanTracker::getUpVector(Matrix3f& rotation)
{
   Vector3f up;

   up[0] = rotation[1][0];
   up[1] = rotation[1][1];
   up[2] = rotation[1][2];
   return(up);
}


Vector3f SpanTracker::getRightVector(Matrix3f& rotation)
{
   Vector3f right;

   right[0] = rotation[0][0];
   right[1] = rotation[0][1];
   right[2] = rotation[0][2];
   return(right);
}


// Set rotation vectors into matrix.
void SpanTracker::setForwardVector(Matrix3f& rotation, Vector3f& forward)
{
   rotation[2][0] = forward[0];
   rotation[2][1] = forward[1];
   rotation[2][2] = forward[2];
}


void SpanTracker::setUpVector(Matrix3f& rotation, Vector3f& up)
{
   rotation[1][0] = up[0];
   rotation[1][1] = up[1];
   rotation[1][2] = up[2];
}


void SpanTracker::setRightVector(Matrix3f& rotation, Vector3f& right)
{
   rotation[0][0] = right[0];
   rotation[0][1] = right[1];
   rotation[0][2] = right[2];
}


// Pick target point.
float SpanTracker::PickTarget(int direction, APoint& target)
{
   APoint  pos = mCastle->mCamera->GetPosition();
   AVector dir;

   switch (direction)
   {
   case FORWARD:
      dir = mCastle->mCamera->GetDVector();
      break;

   case UP:
      dir = mCastle->mCamera->GetUVector();
      break;

   case DOWN:
      dir = -mCastle->mCamera->GetUVector();
      break;

   case LEFT:
      dir = -mCastle->mCamera->GetRVector();
      break;

   case RIGHT:
      dir = mCastle->mCamera->GetRVector();
      break;
   }
   mCastle->mPicker.Execute(mCastle->mScene, pos, dir, 0.0f, Mathf::MAX_REAL);
   if (mCastle->mPicker.Records.size() > 0)
   {
      const PickRecord& record = mCastle->mPicker.GetClosestNonnegative();
      TriMesh           *mesh  = DynamicCast<TriMesh>(record.Intersected);
      APoint            tri[3];
      mesh->GetWorldTriangle(record.Triangle, tri);
      target = record.Bary[0] * tri[0] + record.Bary[1] * tri[1] +
               record.Bary[2] * tri[2];
      return((mCastle->mCamera->GetPosition() - target).Length());
   }
   return(-1.0f);
}


// Integrate device movement.
void SpanTracker::MoveForward()
{
   // Integrate acceleration for 1/2 distance, then decelerate for 1/2.
   // acceleration = 2 * distance / time ^ 2
   struct vector3d gyr;

   gyr.x = gyr.y = gyr.z = 0.0f;
   Vector3f f = getForwardVector(mDeviceRotation);
   f.Y() = 0.0f;
   f.Normalize();
   f *= mCastle->mTrnSpeed;
   Vector3f        v = mDeviceRotation * f;
   Vector3f        a = v / 0.25f;
   struct vector3d acc;
   acc.x = a.X();
   acc.y = a.Y();
   acc.z = a.Z();
   struct vector3d average_stat_acc;
   average_stat_acc.x = average_stat_acc.y = average_stat_acc.z = 0.0f;
   float dtime = 0.5f;
   mIntegrator->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   mIntegrator_noisy->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   acc.x = -acc.x;
   acc.y = -acc.y;
   acc.z = -acc.z;
   mIntegrator->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   mIntegrator_noisy->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   Vector3f p = mDevicePosition;
   mDevicePosition += f;
   if (mLogging)
   {
      FILE *fp = fopen(LOGFILE_NAME, "a");
      fprintf(fp, "forward: speed=%f pos=%f %f %f -> %f %f %f, length=%f\n", p.X(), p.Y(), p.Z(),
              mDevicePosition.X(), mDevicePosition.Y(), mDevicePosition.Z(), mDevicePosition.Length());
      fclose(fp);
   }
}


void SpanTracker::MoveBackward()
{
   struct vector3d gyr;

   gyr.x = gyr.y = gyr.z = 0.0f;
   Vector3f f = -getForwardVector(mDeviceRotation);
   f.Y() = 0.0f;
   f.Normalize();
   f *= mCastle->mTrnSpeed;
   Vector3f        v = mDeviceRotation * f;
   Vector3f        a = v / 0.25f;
   struct vector3d acc;
   acc.x = a.X();
   acc.y = a.Y();
   acc.z = a.Z();
   struct vector3d average_stat_acc;
   average_stat_acc.x = average_stat_acc.y = average_stat_acc.z = 0.0f;
   float dtime = 0.5f;
   mIntegrator->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   mIntegrator_noisy->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   acc.x = -acc.x;
   acc.y = -acc.y;
   acc.z = -acc.z;
   mIntegrator->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   mIntegrator_noisy->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   Vector3f p = mDevicePosition;
   mDevicePosition += f;
   if (mLogging)
   {
      FILE *fp = fopen(LOGFILE_NAME, "a");
      fprintf(fp, "backward: speed=%f pos=%f %f %f -> %f %f %f, length=%f\n", p.X(), p.Y(), p.Z(),
              mDevicePosition.X(), mDevicePosition.Y(), mDevicePosition.Z(), mDevicePosition.Length());
      fclose(fp);
   }
}


void SpanTracker::TurnLeft()
{
   Vector3f v(0.0f, mCastle->mRotSpeed, 0.0f);

   v = mDeviceRotation * v;
   struct vector3d gyr;
   gyr.x = v.X();
   gyr.y = v.Y();
   gyr.z = v.Z();
   struct vector3d acc;
   acc.x = acc.y = acc.z = 0.0f;
   struct vector3d average_stat_acc;
   average_stat_acc.x = average_stat_acc.y = average_stat_acc.z = 0.0f;
   float dtime = 1.0f;
   mIntegrator->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   mIntegrator_noisy->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   Quaternionf quat(Vector3f(0.0f, 1.0f, 0.0f), mCastle->mRotSpeed);
   Matrix3f    rot;
   quat.ToRotationMatrix(rot);
   Vector3f f = getForwardVector(mDeviceRotation);
   Vector3f u = getUpVector(mDeviceRotation);
   Vector3f r = getRightVector(mDeviceRotation);
   mDeviceRotation = mDeviceRotation * rot;
   if (mLogging)
   {
      FILE     *fp = fopen(LOGFILE_NAME, "a");
      Vector3f f2  = getForwardVector(mDeviceRotation);
      Vector3f u2  = getUpVector(mDeviceRotation);
      Vector3f r2  = getRightVector(mDeviceRotation);
      fprintf(fp, "left: speed=%f\n      f=%f %f %f -> %f %f %f\n      u=%f %f %f -> %f %f %f\n      r=%f %f %f -> %f %f %f\n",
              f.X(), f.Y(), f.Z(), f2.X(), f2.Y(), f2.Z(),
              u.X(), u.Y(), u.Z(), u2.X(), u2.Y(), u2.Z(),
              r.X(), r.Y(), r.Z(), r2.X(), r2.Y(), r2.Z());
      fclose(fp);
   }
}


void SpanTracker::TurnRight()
{
   Vector3f v(0.0f, -mCastle->mRotSpeed, 0.0f);

   v = mDeviceRotation * v;
   struct vector3d gyr;
   gyr.x = v.X();
   gyr.y = v.Y();
   gyr.z = v.Z();
   struct vector3d acc;
   acc.x = acc.y = acc.z = 0.0f;
   struct vector3d average_stat_acc;
   average_stat_acc.x = average_stat_acc.y = average_stat_acc.z = 0.0f;
   float dtime = 1.0f;
   mIntegrator->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   mIntegrator_noisy->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   Quaternionf quat(Vector3f(0.0f, -1.0f, 0.0f), mCastle->mRotSpeed);
   Matrix3f    rot;
   quat.ToRotationMatrix(rot);
   Vector3f f = getForwardVector(mDeviceRotation);
   Vector3f u = getUpVector(mDeviceRotation);
   Vector3f r = getRightVector(mDeviceRotation);
   mDeviceRotation = mDeviceRotation * rot;
   if (mLogging)
   {
      FILE     *fp = fopen(LOGFILE_NAME, "a");
      Vector3f f2  = getForwardVector(mDeviceRotation);
      Vector3f u2  = getUpVector(mDeviceRotation);
      Vector3f r2  = getRightVector(mDeviceRotation);
      fprintf(fp, "right: speed=%f\n      f=%f %f %f -> %f %f %f\n      u=%f %f %f -> %f %f %f\n      r=%f %f %f -> %f %f %f\n",
              f.X(), f.Y(), f.Z(), f2.X(), f2.Y(), f2.Z(),
              u.X(), u.Y(), u.Z(), u2.X(), u2.Y(), u2.Z(),
              r.X(), r.Y(), r.Z(), r2.X(), r2.Y(), r2.Z());
      fclose(fp);
   }
}


void SpanTracker::LookUp()
{
   Vector3f v(mCastle->mRotSpeed, 0.0f, 0.0f);

   v = mDeviceRotation * v;
   struct vector3d gyr;
   gyr.x = v.X();
   gyr.y = v.Y();
   gyr.z = v.Z();
   struct vector3d acc;
   acc.x = acc.y = acc.z = 0.0f;
   struct vector3d average_stat_acc;
   average_stat_acc.x = average_stat_acc.y = average_stat_acc.z = 0.0f;
   float dtime = 1.0f;
   mIntegrator->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   mIntegrator_noisy->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   Quaternionf quat(Vector3f(1.0f, 0.0f, 0.0f), mCastle->mRotSpeed);
   Matrix3f    rot;
   quat.ToRotationMatrix(rot);
   Vector3f f = getForwardVector(mDeviceRotation);
   Vector3f u = getUpVector(mDeviceRotation);
   Vector3f r = getRightVector(mDeviceRotation);
   mDeviceRotation = mDeviceRotation * rot;
   if (mLogging)
   {
      FILE     *fp = fopen(LOGFILE_NAME, "a");
      Vector3f f2  = getForwardVector(mDeviceRotation);
      Vector3f u2  = getUpVector(mDeviceRotation);
      Vector3f r2  = getRightVector(mDeviceRotation);
      fprintf(fp, "up: speed=%f\n      f=%f %f %f -> %f %f %f\n      u=%f %f %f -> %f %f %f\n      r=%f %f %f -> %f %f %f\n",
              f.X(), f.Y(), f.Z(), f2.X(), f2.Y(), f2.Z(),
              u.X(), u.Y(), u.Z(), u2.X(), u2.Y(), u2.Z(),
              r.X(), r.Y(), r.Z(), r2.X(), r2.Y(), r2.Z());
      fclose(fp);
   }
}


void SpanTracker::LookDown()
{
   Vector3f v(-mCastle->mRotSpeed, 0.0f, 0.0f);

   v = mDeviceRotation * v;
   struct vector3d gyr;
   gyr.x = v.X();
   gyr.y = v.Y();
   gyr.z = v.Z();
   struct vector3d acc;
   acc.x = acc.y = acc.z = 0.0f;
   struct vector3d average_stat_acc;
   average_stat_acc.x = average_stat_acc.y = average_stat_acc.z = 0.0f;
   float dtime = 1.0f;
   mIntegrator->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   mIntegrator_noisy->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   Quaternionf quat(Vector3f(-1.0f, 0.0f, 0.0f), mCastle->mRotSpeed);
   Matrix3f    rot;
   quat.ToRotationMatrix(rot);
   Vector3f f = getForwardVector(mDeviceRotation);
   Vector3f u = getUpVector(mDeviceRotation);
   Vector3f r = getRightVector(mDeviceRotation);
   mDeviceRotation = mDeviceRotation * rot;
   if (mLogging)
   {
      FILE     *fp = fopen(LOGFILE_NAME, "a");
      Vector3f f2  = getForwardVector(mDeviceRotation);
      Vector3f u2  = getUpVector(mDeviceRotation);
      Vector3f r2  = getRightVector(mDeviceRotation);
      fprintf(fp, "down: speed=%f\n      f=%f %f %f -> %f %f %f\n      u=%f %f %f -> %f %f %f\n      r=%f %f %f -> %f %f %f\n",
              f.X(), f.Y(), f.Z(), f2.X(), f2.Y(), f2.Z(),
              u.X(), u.Y(), u.Z(), u2.X(), u2.Y(), u2.Z(),
              r.X(), r.Y(), r.Z(), r2.X(), r2.Y(), r2.Z());
      fclose(fp);
   }
}


void SpanTracker::AdjustVerticalDistance(float distance)
{
   struct vector3d gyr;

   gyr.x = gyr.y = gyr.z = 0.0f;
   Vector3f u(0.0f, distance, 0.0f);
   u = mDeviceRotation * u;
   Vector3f        a = u / 0.25f;
   struct vector3d acc;
   acc.x = a.X();
   acc.y = a.Y();
   acc.z = a.Z();
   struct vector3d average_stat_acc;
   average_stat_acc.x = average_stat_acc.y = average_stat_acc.z = 0.0f;
   float dtime = 0.5f;
   mIntegrator->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   mIntegrator_noisy->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   acc.x = -a.X();
   acc.y = -a.Y();
   acc.z = -a.Z();
   mIntegrator->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   mIntegrator_noisy->integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
   Vector3f p = mDevicePosition;
   mDevicePosition += Vector3f(0.0f, distance, 0.0f);
   if (mLogging)
   {
      FILE *fp = fopen(LOGFILE_NAME, "a");
      fprintf(fp, "vertical: pos=%f %f %f -> %f %f %f, length=%f\n", p.X(), p.Y(), p.Z(),
              mDevicePosition.X(), mDevicePosition.Y(), mDevicePosition.Z(), mDevicePosition.Length());
      fclose(fp);
   }
}


void SpanTracker::Reset()
{
   mDeviceRotation = Matrix3f::IDENTITY;
   mDevicePosition = Vector3f::ZERO;
   mIntegrator->init(0.0f, 0.0f, 0.0f);
   mIntegrator_noisy->init(0.0f, 0.0f, 0.0f);
   startTargetIsTracked = false;
   startTargetTrackCheck->setChecked(false);
   endTargetIsTracked = false;
   endTargetTrackCheck->setChecked(false);
   memset(positionBuf, 0, BUFSIZ);
   devicePositionText->setLabelString("                          ");
   targetPositionText->setLabelString("                          ");
   memset(distanceBuf, 0, BUFSIZ);
   targetDistanceText->setLabelString("        ");
   start->Untrack();
   end->Untrack();
   mCastle->mScene->Update();
   mCastle->mCuller.ComputeVisibleSet(mCastle->mScene);
}
