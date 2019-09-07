// Game state.

#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "Wm5WindowApplication3.h"
#ifdef NETWORK
#include "network.hpp"
#endif
using namespace Wm5;

// Name size.
enum
{
   NAME_SIZE = 16
};

// Number of cannons.
#ifdef NETWORK
enum
{
   NUM_CANNONS = MAX_PLAYERS
};
#else
enum
{
   NUM_CANNONS = 1
};
#endif

// Number of gingerbread men (not counting mother).
enum
{
   NUM_GINGER_MEN = 5
};

// Game state.
struct GAME_STATE
{
   // Cannon states.
   struct CANNON_STATE
   {
      Float3   color;
      Vector3f position;
      float    swivel;
      float    elevation;
      float    charge;
      bool     firing;
      int      score;
      char     name[NAME_SIZE];
   }
                           cannons[NUM_CANNONS];

   // Gingerbread men states.
   struct GINGER_MAN_STATE
   {
      bool     alive;
      float    nextSpeed;
      Vector3f nextPosition;
      float    nextRotate;
      bool     launched;
   }
                           gingerMen[NUM_GINGER_MEN];
   struct GINGER_MAN_STATE gingerMother;
   Vector3f                windVector;
};
#endif
