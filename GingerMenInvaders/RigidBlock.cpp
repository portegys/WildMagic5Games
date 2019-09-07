// Rigid block.

#include "RigidBlock.h"

//----------------------------------------------------------------------------
RigidBlock::RigidBlock(Vector3f dimensions, Float3 color, Light *light)
{
   Moved = false;

   // Create a block using the StandardMesh class.
   m_dimensions = dimensions;
   VertexFormat *vformat = VertexFormat::Create(2,
                                                VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
                                                VertexFormat::AU_NORMAL, VertexFormat::AT_FLOAT3, 0);
   m_spkMesh = StandardMesh(vformat).Box(dimensions.X(), dimensions.Y(), dimensions.Z());

   // Set the color.
   SetColor(color, light);
}


// Set the block color.
void RigidBlock::SetColor(Float3 color, Light *light)
{
   m_color                 = color;
   m_spkMaterial           = new0 Material;
   m_spkMaterial->Diffuse  = Float4(m_color[0], m_color[1], m_color[2], 1.0f);
   m_spkMaterial->Emissive = Float4(m_color[0] * 0.1f, m_color[1] * 0.1f, m_color[2] * 0.1f, 1.0f);
   m_spkMaterial->Specular = Float4(0.9f, 0.9f, 0.9f, 1.0f);
   LightDirPerVerEffect *effect   = new0 LightDirPerVerEffect();
   VisualEffectInstance *instance = effect->CreateInstance(light, m_spkMaterial);
   m_spkMesh->SetEffectInstance(instance);
}
