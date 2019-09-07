#include "ScorchedMarsTerrain.h"
using namespace Wm5;

ScorchedMarsTerrain::ScorchedMarsTerrain(const std::string& heightName, VertexFormat *vformat,
                                         Camera *camera, int mode)
   :
     Terrain(heightName, vformat, camera, mode)
{
}


// Get height.
float ScorchedMarsTerrain::GetHeight(float x, float y) const
{
   APoint p         = WorldTransform.Inverse() * APoint(x, y, 0.0f);
   float  length    = mSpacing * (float)(mSize - 1);
   float  invLength = 1.0f / length;
   int    nCol      = (int)Mathf::Floor(p.X() * invLength);
   int    nRow      = (int)Mathf::Floor(p.Y() * invLength);

   int cminO = nCol - mNumCols / 2;
   int cminP = cminO % mNumCols;

   if (cminP < 0)
   {
      cminP += mNumCols;
   }

   int rminO = nRow - mNumRows / 2;
   int rminP = rminO % mNumRows;
   if (rminP < 0)
   {
      rminP += mNumRows;
   }

   int rO = rminO, rP = rminP;
   for (int row = 0; row < mNumRows; ++row)
   {
      int cO = cminO, cP = cminP;
      for (int col = 0; col < mNumCols; ++col)
      {
         TerrainPage *page     = mPages[rP][cP];
         Float2      oldOrigin = page->GetOrigin();
         Float2      newOrigin(cO * length, rO * length);
         APoint      pageTrn(
            newOrigin[0] - oldOrigin[0],
            newOrigin[1] - oldOrigin[1],
            page->LocalTransform.GetTranslate().Z());
         page->LocalTransform.SetTranslate(pageTrn);

         ++cO;
         if (++cP == mNumCols)
         {
            cP = 0;
         }
      }

      ++rO;
      if (++rP == mNumRows)
      {
         rP = 0;
      }
   }

   TerrainPage *page = GetCurrentPage(x, y);
   x -= page->LocalTransform.GetTranslate().X();
   y -= page->LocalTransform.GetTranslate().Y();
   return(page->GetHeight(x, y));
}
