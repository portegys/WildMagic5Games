//***************************************************************************//
//* File Name: frameRate.hpp                                                *//
//*    Author: Chris McBride chris_a_mcbride@hotmail.com                    *//
//* Date Made: 04/06/02                                                     *//
//* File Desc: Frame rate counter for any project, windows specific.        *//
//* Rev. Date: 11/26/02                                                     *//
//* Rev. Desc: Added frame rate independence and UNIX functionality (TEP)   *//
//*                                                                         *//
//***************************************************************************//
#ifndef __FRAMERATE_HPP__
#define __FRAMERATE_HPP__

#include <time.h>
#include "gettime.h"

class FrameRate
{
public:

   int                targetFPS;                  // Target frames per second (FPS).
   float              FPS;                        // Current FPS.
   float              speedFactor;                // Frame rate independence speed factor.
   static const float MaxSpeedFactor;
   static const int   DefaultTargetFPS;

   FrameRate()
   {
      this->targetFPS = DefaultTargetFPS;
      FPS             = (float)targetFPS;
      speedFactor     = 0.0f;
      frameCount      = 0;
      lastTime        = gettime();
   }


   // Set target frame rate.
   void setTarget(int fps)
   {
      targetFPS = fps;
   }


   // Update: call per frame.
   void update()
   {
      TIME currentTime, delta;

      // Count the frame.
      frameCount++;

      // Get the time delta.
      currentTime = gettime();
      delta       = (currentTime - lastTime) / 1000;

      // Has >= 1 second elapsed?
      if (delta >= 1)
      {
         // Calculate new values.
         FPS = (float)frameCount / (float)delta;
         if (FPS > 0.0f)
         {
            speedFactor = (float)targetFPS / FPS;
         }
         else
         {
            speedFactor = 0.0f;
         }
         if (speedFactor > MaxSpeedFactor) { speedFactor = MaxSpeedFactor; }
         frameCount = 0;
         lastTime   = currentTime;
      }
   }


   // Reset.
   void reset()
   {
      FPS         = (float)targetFPS;
      speedFactor = 1.0f;
      frameCount  = 0;
      lastTime    = gettime();
   }


private:

   int  frameCount;
   TIME lastTime;
};

// Default target frame rate.
const int FrameRate::DefaultTargetFPS = 30;

// Maximum speed factor.
const float FrameRate::MaxSpeedFactor = 5.0f;
#endif
