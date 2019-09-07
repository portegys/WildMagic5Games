/*
 *  SMSound.h
 *
 *	Sound library.
 *
 *  Created by Damien Sorresso on 4/26/06.
 *  Copyright 2006. All rights reserved.
 *
 *	In order to use this library, you must link against the fmod libraries.
 *	You must also have the fmod headers in your include search paths.
 *	This library can be used in either a C or C++ program.
 */

#ifndef _SMSOUND
#define _SMSOUND

#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fmod.h"
#include "fmod_errors.h"
//#define SMS_DEBUG 1

// Debug symbol.
#ifdef SMS_DEBUG
#define IFDEBUG(x)          x
#define DEBUG_INFO(x, y)    fprintf(stderr, x, y)
#define DEBUG_MSG(x)        fprintf(stderr, x)
#else
#define IFDEBUG(x)          /* no op */
#define DEBUG_INFO(x, y)    /* no op */
#define DEBUG_MSG(x)        /* no op */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

static FMOD_SYSTEM  *soundSystem    = NULL;
static FMOD_SOUND   *explosionSound = NULL;
static FMOD_SOUND   *fireSound      = NULL;
static FMOD_SOUND   *windSound      = NULL;
static FMOD_SOUND   *attackSound    = NULL;
static FMOD_CHANNEL *channel1       = 0;
static FMOD_CHANNEL *channel2       = 0;
static FMOD_CHANNEL *channel3       = 0;
static FMOD_CHANNEL *windChannel    = 0;

// This is how you tell what sound to play.
typedef enum _SMSSound
{
   SMSFireSound,
   SMSExplosionSound,
   SMSWindSound,
   SMSAttackSound
} SMSSound;

// Error checking.
void ERRCHECK(FMOD_RESULT result);

// Public functions.

// Initialize everything by calling SMSInitSound(...) with paths to the
// explosion and firing sounds. The fmod sound system will be initialized,
// sounds created, etc ...
void SMSInitSound(char *explosionPath, char *firePath,
                  char *windPath, char *attackPath);

// Play `sound' from distance `WMDistanceVec' and `WMVelocityVec'. Note that
// you can pass in WildMagic Vector3f's cast as floats for the last two
// parameters. For this limited purpose, the cast is safe.
// When passing in `WMDistanceVec', this is the distance FROM THE LISTENER
// to the sound. In the case of the game, the listener is the current
// position of the camera, and the sound's is ... well, wherever it is.
// In any case,
//		distance = soundsource - cameraposition.
// Velocity will probably be zero in all cases unless the source is moving.
// It's up to you to correctly calculate velocity.
void SMSPlaySound(SMSSound sound, float *WMDistanceVec, float *WMVelocityVec);
void SMSStartWind();
void SMSStopWind();

// Frees up the memory from the sounds.
void SMSCleanUp();

// Helpers. Yeah you could call them if you wanted, but why would you want to?
void playExplosion(FMOD_VECTOR distance, FMOD_VECTOR velocity);
void playFire(FMOD_VECTOR distance, FMOD_VECTOR velocity);
void playAttack(FMOD_VECTOR distance, FMOD_VECTOR velocity);

#ifdef __cplusplus
}
#endif
#endif
