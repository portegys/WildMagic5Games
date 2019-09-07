#ifndef SCORCHEDMARS_TERRAIN_H
#define SCORCHEDMARS_TERRAIN_H

#include "Wm5Terrain.h"

namespace Wm5
{
class ScorchedMarsTerrain : public Terrain
{
public:
   ScorchedMarsTerrain(const std::string& heightName, VertexFormat *vformat,
                       Camera *camera, int mode = FileIO::FM_DEFAULT_READ);

   float GetHeight(float x, float y) const;
};

typedef Pointer0<ScorchedMarsTerrain>   ScorchedMarsTerrainPtr;
}

#endif
