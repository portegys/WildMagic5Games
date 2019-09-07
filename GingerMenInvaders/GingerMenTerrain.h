#ifndef GINGERMEN_TERRAIN_H
#define GINGERMEN_TERRAIN_H

#include "Wm5Terrain.h"

namespace Wm5
{
class GingerMenTerrain : public Terrain
{
public:
   GingerMenTerrain(const std::string& heightName, VertexFormat *vformat,
                    Camera *camera, int mode = FileIO::FM_DEFAULT_READ);

   float GetHeight(float x, float y) const;
};

typedef Pointer0<GingerMenTerrain>   GingerMenTerrainPtr;
}

#endif
