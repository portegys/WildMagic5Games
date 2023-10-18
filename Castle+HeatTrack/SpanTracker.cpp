// SpanTracker.
#include "SpanTracker.h"
#include "Castle.h"

SpanTracker::SpanTracker(Castle *castle, string guiPath)
{
   mCastle  = castle;
   mGuiPath = guiPath;
   NodePtr marker = new0 Node();
   castle->mScene->AttachChild(marker);
   start = new Tracker(startColor, castle->mDLight, marker);
   end   = new Tracker(endColor, castle->mDLight, marker);
   for (int i = 0; i < NUM_MARKERS; i++)
   {
      hot[i]  = new Tracker(hotColor, castle->mDLight, marker);
      warm[i] = new Tracker(warmColor, castle->mDLight, marker);
   }
   init_integrator();
   mEnabled = true;
   mWidth   = mHeight = guiWidth = guiHeight = -1;
   MediaPathManager::registerPath(guiPath + "resource/");
   guiFrame = new GUIFrame();
   guiFrame->GUIPanel::loadXMLSettings("SpanTrackerGUILayout.xml");
   startTargetIsTracked  = false;
   startTargetTrackCheck = (GUICheckBox *)guiFrame->getWidgetByCallbackString("Start");
   startTargetTrackCheck->setAlphaFadeScale(1000.0);
   endTargetIsTracked  = false;
   endTargetTrackCheck = (GUICheckBox *)guiFrame->getWidgetByCallbackString("End");
   endTargetTrackCheck->setAlphaFadeScale(1000.0);
   heatTrackCheck = (GUICheckBox *)guiFrame->getWidgetByCallbackString("Heat");
   heatTrackCheck->setAlphaFadeScale(1000.0);
   heatIsTracked      = false;
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
   for (int i = 0; i < NUM_MARKERS; i++)
   {
      delete hot[i];
      delete warm[i];
   }
}


// Toggle enablement.
bool SpanTracker::ToggleEnable()
{
   mEnabled = !mEnabled;
   if (mEnabled)
   {
      start->ShowMarker();
      end->ShowMarker();
      for (int i = 0; i < NUM_MARKERS; i++)
      {
         hot[i]->ShowMarker();
         warm[i]->ShowMarker();
      }
   }
   else
   {
      start->HideMarker();
      end->HideMarker();
      for (int i = 0; i < NUM_MARKERS; i++)
      {
         hot[i]->HideMarker();
         warm[i]->HideMarker();
      }
   }
   mCastle->mScene->Update();
   mCastle->mCuller.ComputeVisibleSet(mCastle->mScene);
   return(mEnabled);
}


// Draw.
void SpanTracker::Draw(int width, int height)
{
   if (!mEnabled)
   {
      return;
   }
   if ((width != mWidth) || (height != mHeight))
   {
      mWidth    = width;
      mHeight   = height;
      guiWidth  = width / GUI_WIDTH_DIVISOR;
      guiHeight = height / GUI_HEIGHT_DIVISOR;
      guiFrame->setDimensions((float)guiWidth, (float)guiHeight);
      guiFrame->forceUpdate(true);
   }
   DrawCrosshairs(10.0f);
   DrawInstrument();
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
                     if (client->heatIsTracked)
                     {
                        float heatDistance = client->GetHeatTrackedDistance();
                        sprintf(client->distanceBuf, "%.2f (%.2f)", distance, heatDistance);
                     }
                     else
                     {
                        sprintf(client->distanceBuf, "%.2f", distance);
                     }
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
      else if (callbackString == "Heat")
      {
         if (!client->startTargetIsTracked && !client->endTargetIsTracked)
         {
            if (client->heatTrackCheck->isChecked())
            {
               client->heatIsTracked = true;
            }
            else
            {
               client->heatIsTracked = false;
            }
         }
         else
         {
            if (client->heatIsTracked)
            {
               client->heatTrackCheck->setChecked(true);
            }
            else
            {
               client->heatTrackCheck->setChecked(false);
            }
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

   if (!PickTarget(0, targetPosition))
   {
      return(false);
   }
   start->Track(targetPosition, mCastle->mCamera->GetPosition());
   mDeviceRotation = Matrix3f::IDENTITY;
   mDevicePosition = Vector3f(0.0f, 0.0f, -(start->targetPosition - start->devicePosition).Length());
   init_integrator();
   mCastle->mScene->Update();
   mCastle->mCuller.ComputeVisibleSet(mCastle->mScene);
   return(true);
}


// Track end target.
bool SpanTracker::TrackEndTarget()
{
   APoint targetPosition;

   if (!PickTarget(0, targetPosition))
   {
      return(false);
   }
   end->Track(targetPosition, mCastle->mCamera->GetPosition());
   mCastle->mScene->Update();
   mCastle->mCuller.ComputeVisibleSet(mCastle->mScene);
   return(true);
}


// Track movemment start.
bool SpanTracker::TrackMovementStart(int key)
{
   if (!startTargetIsTracked || endTargetIsTracked || !heatIsTracked)
   {
      return(false);
   }

   if (!((key == Wm5::WindowApplication::KEY_LEFT_ARROW) ||
         (key == Wm5::WindowApplication::KEY_RIGHT_ARROW) ||
         (key == Wm5::WindowApplication::KEY_UP_ARROW) ||
         (key == Wm5::WindowApplication::KEY_DOWN_ARROW) ||
         (key == Wm5::WindowApplication::KEY_PAGE_UP) ||
         (key == Wm5::WindowApplication::KEY_PAGE_DOWN) ||
         (key == Wm5::WindowApplication::KEY_INSERT) ||
         (key == Wm5::WindowApplication::KEY_DELETE)))
   {
      return(false);
   }

   // Set warm tracker marks.
   APoint targetPosition;
   bool   trackerSet = false;
   for (int i = 0; i < NUM_MARKERS; i++)
   {
      hot[i]->Untrack();
      warm[i]->Untrack();
   }
   for (int i = 0; i < NUM_MARKERS; i++)
   {
      if (PickTarget(i, targetPosition) && ((targetPosition - mCastle->mCamera->GetPosition()).Length() <= MAX_RANGE))
      {
         warm[i]->Track(targetPosition, mCastle->mCamera->GetPosition());
         trackerSet = true;
      }
   }
   mCastle->mScene->Update();
   return(trackerSet);
}


/*
 * Track movemment end.
 *
 * Rotation calculation:
 * In an actual device, the distances to the hot and warm points as well as the
 * proportional distance between them as detected by the camera are known.
 * This will yield a rotation angle.
 * The position of the hot point relative to the device orientation axes
 * determines an axis of rotation.
 * Together, the angle and axis define the updated device rotation.
 *
 * Movement calculation:
 * Distance displacement from warm to hot points along the device axes determine
 * updated device position.
 */
bool SpanTracker::TrackMovementEnd(int key)
{
   if (!startTargetIsTracked || endTargetIsTracked || !heatIsTracked)
   {
      return(false);
   }

   if (!((key == Wm5::WindowApplication::KEY_LEFT_ARROW) ||
         (key == Wm5::WindowApplication::KEY_RIGHT_ARROW) ||
         (key == Wm5::WindowApplication::KEY_UP_ARROW) ||
         (key == Wm5::WindowApplication::KEY_DOWN_ARROW) ||
         (key == Wm5::WindowApplication::KEY_PAGE_UP) ||
         (key == Wm5::WindowApplication::KEY_PAGE_DOWN) ||
         (key == Wm5::WindowApplication::KEY_INSERT) ||
         (key == Wm5::WindowApplication::KEY_DELETE)))
   {
      return(false);
   }

   // Set hot tracker marks.
   APoint targetPosition;
   bool   trackerSet = false;
   for (int i = 0; i < NUM_MARKERS; i++)
   {
      if (PickTarget(i, targetPosition) && ((targetPosition - mCastle->mCamera->GetPosition()).Length() <= MAX_RANGE))
      {
         hot[i]->Track(targetPosition, mCastle->mCamera->GetPosition());
         trackerSet = true;
      }
      else
      {
         hot[i]->Untrack();
      }
   }
   if (!trackerSet)
   {
      return(false);
   }

   // Update tracker rotation for verifiable rotation or movement.
   // Note 1: For actual device, camera will detect motion direction.
   // Note 2: For simplicity, use only forward tracking to update rotation.
   if (hot[FORWARD]->tracking && warm[FORWARD]->tracking &&
       (hot[UP]->tracking && warm[UP]->tracking ||
        hot[DOWN]->tracking && warm[DOWN]->tracking ||
        hot[LEFT]->tracking && warm[LEFT]->tracking ||
        hot[RIGHT]->tracking && warm[RIGHT]->tracking))
   {
      // Rotation?
      if ((key == Wm5::WindowApplication::KEY_LEFT_ARROW) ||
          (key == Wm5::WindowApplication::KEY_RIGHT_ARROW) ||
          (key == Wm5::WindowApplication::KEY_PAGE_UP) ||
          (key == Wm5::WindowApplication::KEY_PAGE_DOWN))
      {
         float    d2h   = (hot[FORWARD]->devicePosition - hot[FORWARD]->targetPosition).Length();
         float    d2w   = (warm[FORWARD]->devicePosition - warm[FORWARD]->targetPosition).Length();
         float    h2w   = (hot[FORWARD]->targetPosition - warm[FORWARD]->targetPosition).Length();
         float    angle = (h2w / (d2h + d2w)) * 180.0f;
         Vector3f axis;
         if (key == Wm5::WindowApplication::KEY_LEFT_ARROW)
         {
            axis = getUpVector(mDeviceRotation);
         }
         else if (key == Wm5::WindowApplication::KEY_RIGHT_ARROW)
         {
            axis = -getUpVector(mDeviceRotation);
         }
         else if (key == Wm5::WindowApplication::KEY_PAGE_UP)
         {
            axis = getRightVector(mDeviceRotation);
         }
         else           // KEY_PAGE_DOWN
         {
            axis = -getRightVector(mDeviceRotation);
         }
         Quaternionf quat(axis, angle * Mathf::DEG_TO_RAD);
         quat.ToRotationMatrix(mDeviceRotation);
      }

      // Left-right movement?
      else if ((key == Wm5::WindowApplication::KEY_INSERT) ||
               (key == Wm5::WindowApplication::KEY_DELETE))
      {
         Vector3f hotp     = hot[FORWARD]->targetPosition;
         Vector3f warmp    = warm[FORWARD]->targetPosition;
         float    vertical = hotp.Y() - warmp.Y();
         hotp.Y()  = 0.0f;
         warmp.Y() = 0.0f;
         float horizontal = (hotp - warmp).Length();
         if (key == Wm5::WindowApplication::KEY_INSERT)
         {
            // Right.
            mDevicePosition += (getRightVector(mDeviceRotation) * horizontal);
         }
         else
         {
            // Left.
            mDevicePosition -= (getRightVector(mDeviceRotation) * horizontal);
         }
         mDevicePosition.Y() += vertical;
      }

      // Forward-backward movement?
      else if ((key == Wm5::WindowApplication::KEY_UP_ARROW) ||
               (key == Wm5::WindowApplication::KEY_DOWN_ARROW))
      {
         int direction;
         if (hot[LEFT]->tracking && warm[LEFT]->tracking)
         {
            direction = LEFT;
         }
         else if (hot[RIGHT]->tracking && warm[RIGHT]->tracking)
         {
            direction = RIGHT;
         }
         else if (hot[DOWN]->tracking && warm[DOWN]->tracking)
         {
            direction = DOWN;
         }
         else
         {
            direction = UP;
         }
         Vector3f hotp     = hot[direction]->targetPosition;
         Vector3f warmp    = warm[direction]->targetPosition;
         float    vertical = hotp.Y() - warmp.Y();
         hotp.Y()  = 0.0f;
         warmp.Y() = 0.0f;
         float horizontal = (hotp - warmp).Length();
         if (key == Wm5::WindowApplication::KEY_UP_ARROW)
         {
            mDevicePosition += (getForwardVector(mDeviceRotation) * horizontal);
         }
         else
         {
            mDevicePosition -= (getForwardVector(mDeviceRotation) * horizontal);
         }
         mDevicePosition.Y() += vertical;
      }
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


// Get heat-tracked spanning distance between endpoints.
float SpanTracker::GetHeatTrackedDistance()
{
   float    length         = (end->targetPosition - end->devicePosition).Length();
   Vector3f targetPosition = mDevicePosition + (getForwardVector(mDeviceRotation) * length);

   return(targetPosition.Length());
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
bool SpanTracker::PickTarget(int direction, APoint& target)
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
      return(true);
   }
   return(false);
}

// Integrate device movement.
void SpanTracker::MoveForward()
{
	//APoint pos = mCamera->GetPosition();
	//pos += mTrnSpeed*mWorldAxis[0];
	//mCamera->SetPosition(pos);
	//a = 2s / t ^ 2
	struct vector3d gyr;
	gyr.x = gyr.y = gyr.z = 0.0f;
	struct vector3d acc;
	acc.x = acc.y = acc.z = 0.0f;
	struct vector3d average_stat_acc;
	average_stat_acc.x = average_stat_acc.y = average_stat_acc.z = 0.0f;
	int dtime = 1;
	integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
}

void SpanTracker::MoveBackward()
{
	//APoint pos = mCamera->GetPosition();
	//pos -= mTrnSpeed*mWorldAxis[0];
	//mCamera->SetPosition(pos);
//TODO: Keep a "compass" vector to keep track of forward direction. 
	//Use that scaled by move distance to be transformed by the ST orientation to yield the acc. components.
}

void SpanTracker::TurnLeft()
{
	struct vector3d gyr;
	gyr.x = 0.0f;
	gyr.y = mCastle->mRotSpeed;
	gyr.z = 0.0f;
	struct vector3d acc;
	acc.x = acc.y = acc.z = 0.0f;
	struct vector3d average_stat_acc;
	average_stat_acc.x = average_stat_acc.y = average_stat_acc.z = 0.0f;
	int dtime = 1;
	integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
	// TODO: For left/right turn, transform scaled vector by the ST orientation, yielding the gyr components.
}

void SpanTracker::TurnRight()
{
	struct vector3d gyr;
	gyr.x = 0.0f;
	gyr.y = -mCastle->mRotSpeed;
	gyr.z = 0.0f;
	struct vector3d acc;
	acc.x = acc.y = acc.z = 0.0f;
	struct vector3d average_stat_acc;
	average_stat_acc.x = average_stat_acc.y = average_stat_acc.z = 0.0f;
	int dtime = 1;
	integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
}

void SpanTracker::LookUp()
{
	struct vector3d gyr;
	gyr.x = 0.0f;
	gyr.y = 0.0f;
	gyr.z = mCastle->mRotSpeed;
	struct vector3d acc;
	acc.x = acc.y = acc.z = 0.0f;
	struct vector3d average_stat_acc;
	average_stat_acc.x = average_stat_acc.y = average_stat_acc.z = 0.0f;
	int dtime = 1;
	integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
}

void SpanTracker::LookDown()
{
	struct vector3d gyr;
	gyr.x = 0.0f;
	gyr.y = 0.0f;
	gyr.z = -mCastle->mRotSpeed;
	struct vector3d acc;
	acc.x = acc.y = acc.z = 0.0f;
	struct vector3d average_stat_acc;
	average_stat_acc.x = average_stat_acc.y = average_stat_acc.z = 0.0f;
	int dtime = 1;
	integrate_movement(&gyr, &acc, &average_stat_acc, dtime);
}

void SpanTracker::AdjustVerticalDistance(float distance)
{

}

void SpanTracker::Reset()
{
   mDeviceRotation      = Matrix3f::IDENTITY;
   mDevicePosition      = Vector3f::ZERO;
   init_integrator();
   startTargetIsTracked = false;
   startTargetTrackCheck->setChecked(false);
   endTargetIsTracked = false;
   endTargetTrackCheck->setChecked(false);
   heatIsTracked = false;
   heatTrackCheck->setChecked(false);
   memset(positionBuf, 0, BUFSIZ);
   devicePositionText->setLabelString("                          ");
   targetPositionText->setLabelString("                          ");
   memset(distanceBuf, 0, BUFSIZ);
   targetDistanceText->setLabelString("        ");
   start->Untrack();
   end->Untrack();
   for (int i = 0; i < NUM_MARKERS; i++)
   {
      hot[i]->Untrack();
      warm[i]->Untrack();
   }
   mCastle->mScene->Update();
   mCastle->mCuller.ComputeVisibleSet(mCastle->mScene);
}
