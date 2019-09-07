#include "SMSound.h"

void ERRCHECK(FMOD_RESULT result)
{
   if (result != FMOD_OK)
   {
      printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
      exit(1);
   }
}


const FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f };
const FMOD_VECTOR forward = { 0.0f, 0.0f, 1.0f };
const FMOD_VECTOR up = { 0.0f, 1.0f, 0.0f };
const FMOD_VECTOR listener = { 0.0f, 0.0f, 0.0f };

void SMSInitSound(char *explosionPath, char *firePath,
                  char *windPath, char *attackPath)
{
   FMOD_RESULT result;

   // Create the sound system.
   if (!soundSystem)
   {
      DEBUG_MSG("Creating soundSystem...\n");
      result = FMOD_System_Create(&soundSystem);

      ERRCHECK(result);
      result = FMOD_System_Init(soundSystem,
                                10,
                                FMOD_INIT_NORMAL,
                                0);

      DEBUG_MSG("Created soundSystem.\n");
   }

   // Create the explosion sound.
   if (!explosionSound)
   {
      DEBUG_MSG("Creating explosionSound...\n");
      result = FMOD_System_CreateSound(soundSystem,
                                       explosionPath,
                                       FMOD_SOFTWARE | FMOD_3D,
                                       0,
                                       &explosionSound);

      ERRCHECK(result);

      result = FMOD_Sound_SetMode(explosionSound,
                                  FMOD_LOOP_OFF);

      ERRCHECK(result);

      DEBUG_MSG("Created explosionSound.\n");
   }

   // Create the attack sound.
   if (!attackSound)
   {
      DEBUG_MSG("Creating attackSound...\n");
      result = FMOD_System_CreateSound(soundSystem,
                                       attackPath,
                                       FMOD_SOFTWARE | FMOD_3D,
                                       0,
                                       &attackSound);

      ERRCHECK(result);

      result = FMOD_Sound_SetMode(attackSound,
                                  FMOD_LOOP_OFF);

      ERRCHECK(result);

      DEBUG_MSG("Created attackSound.\n");
   }

   // Create the firing sound.
   if (!fireSound)
   {
      DEBUG_MSG("Creating fireSound...\n");
      result = FMOD_System_CreateSound(soundSystem,
                                       firePath,
                                       FMOD_SOFTWARE | FMOD_3D,
                                       0,
                                       &fireSound);

      ERRCHECK(result);

      result = FMOD_Sound_SetMode(fireSound,
                                  FMOD_LOOP_OFF);

      ERRCHECK(result);

      DEBUG_MSG("Created fireSound.\n");
   }

   if (!windSound)
   {
      DEBUG_MSG("Creating windSound...\n");
      result = FMOD_System_CreateSound(soundSystem,
                                       windPath,
                                       FMOD_SOFTWARE | FMOD_3D,
                                       0,
                                       &windSound);

      ERRCHECK(result);

      result = FMOD_Sound_SetMode(windSound,
                                  FMOD_LOOP_NORMAL);

      ERRCHECK(result);

      DEBUG_MSG("Created windSound.\n");
   }

   // Place listener at the origin. All distances should be
   // calculated from the coordinates of the camera view, NOT
   // absolute world coordinates.
   result = FMOD_System_Set3DListenerAttributes(soundSystem,
                                                0,
                                                &listener,
                                                &velocity,
                                                &forward,
                                                &up);
   ERRCHECK(result);
}


void SMSPlaySound(SMSSound sound, float *WMDistanceVec, float *WMVelocityVec)
{
   float       *pos;
   FMOD_VECTOR dist, vel;

   pos    = WMDistanceVec;                        //
   dist.x = *pos;                                 // Extract 3 `float's from whatever is passed in as the distance.
   // This could be a float array of size 3, a `FMOD_VECTOR',
   pos++;                                         // or even a `Vector3f'.
   dist.y = *pos;                                 // Any vector structure which stores its components at
   // the beginning of its memory is good for this conversion.
   pos++;                                         //
   dist.z = *pos;                                 //

   pos   = WMVelocityVec;                         //
   vel.x = *pos;                                  //
   //
   pos++;                                         // Same as above, only for velocity.
   vel.y = *pos;                                  //
   //
   pos++;                                         //
   vel.z = *pos;                                  //

   IFDEBUG(fprintf(stderr, "Got coordinates: dist.x = %f, dist.y = %f, dist.z = %f\n",
                   dist.x, dist.y, dist.z);
           )
   IFDEBUG(fprintf(stderr, "Got coordinates: vel.x = %f, vel.y = %f, vel.z = %f\n",
                   vel.x, vel.y, vel.z);
           )

   if (sound == SMSFireSound)
   {
      playFire(dist, vel);
   }
   else if (sound == SMSExplosionSound)
   {
      playExplosion(dist, vel);
   }
   else if (sound == SMSAttackSound)
   {
      playAttack(dist, vel);
   }
}


void SMSStartWind()
{
   FMOD_RESULT result;

   FMOD_VECTOR distance = { 0.0f, 0.0f, 0.0f };
   FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f };

   DEBUG_MSG("Starting windSound...\n");
   result = FMOD_System_PlaySound(soundSystem,
                                  FMOD_CHANNEL_FREE,
                                  windSound,
                                  1,
                                  &windChannel);

   ERRCHECK(result);

   result = FMOD_Channel_Set3DAttributes(windChannel,
                                         &distance,
                                         &velocity);

   ERRCHECK(result);

   result = FMOD_Channel_SetPaused(windChannel, 0);

   ERRCHECK(result);
}


void SMSStopWind()
{
   FMOD_RESULT result;

   DEBUG_MSG("Stopping windSound...\n");

   result = FMOD_Channel_SetPaused(windChannel, 1);

   ERRCHECK(result);
}


void SMSCleanUp()
{
   SMSStopWind();
   FMOD_Sound_Release(fireSound);
   FMOD_Sound_Release(explosionSound);
   FMOD_Sound_Release(windSound);
   FMOD_Sound_Release(attackSound);

   FMOD_System_Close(soundSystem);
   FMOD_System_Release(soundSystem);
}


void playExplosion(FMOD_VECTOR distance, FMOD_VECTOR velocity)
{
   FMOD_RESULT result;

   DEBUG_MSG("Playing explosionSound...\n");
   // Pump the sound through the channel, but leave it paused.
   result = FMOD_System_PlaySound(soundSystem,
                                  FMOD_CHANNEL_FREE,
                                  explosionSound,
                                  1,
                                  &channel1);

   ERRCHECK(result);

   // Tell our sound where it's supposed to play from.
   result = FMOD_Channel_Set3DAttributes(channel1,
                                         &distance,
                                         &velocity);

   ERRCHECK(result);

   // Play the sound.
   result = FMOD_Channel_SetPaused(channel1, 0);

   ERRCHECK(result);
}


void playFire(FMOD_VECTOR distance, FMOD_VECTOR velocity)
{
   FMOD_RESULT result;

   DEBUG_MSG("Playing fireSound...\n");
   result = FMOD_System_PlaySound(soundSystem,
                                  FMOD_CHANNEL_FREE,
                                  fireSound,
                                  1,
                                  &channel2);

   ERRCHECK(result);

   result = FMOD_Channel_Set3DAttributes(channel2,
                                         &distance,
                                         &velocity);

   ERRCHECK(result);

   result = FMOD_Channel_SetPaused(channel2, 0);

   ERRCHECK(result);
}


void playAttack(FMOD_VECTOR distance, FMOD_VECTOR velocity)
{
   FMOD_RESULT result;

   DEBUG_MSG("Playing attackSound...\n");
   result = FMOD_System_PlaySound(soundSystem,
                                  FMOD_CHANNEL_FREE,
                                  attackSound,
                                  1,
                                  &channel3);

   ERRCHECK(result);

   result = FMOD_Channel_Set3DAttributes(channel3,
                                         &distance,
                                         &velocity);

   ERRCHECK(result);

   result = FMOD_Channel_SetPaused(channel3, 0);

   ERRCHECK(result);
}
