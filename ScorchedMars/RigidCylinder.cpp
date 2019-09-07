// Rigid cylinder.

#include "RigidCylinder.h"

//----------------------------------------------------------------------------
RigidCylinder::RigidCylinder(float radius, float height,
                             bool closed, Float3 color, Light *light)
{
   Moved = false;

   // Create a cylinder using the StandardMesh class.
   m_radius = radius;
   m_height = height;
   m_closed = closed;
   VertexFormat *vformat = VertexFormat::Create(2,
                                                VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
                                                VertexFormat::AU_NORMAL, VertexFormat::AT_FLOAT3, 0);
   m_spkMesh = StandardMesh(vformat).Cylinder(24, 24, radius, height, !closed);

   // Set the color.
   SetColor(color, light);
}


// Set the cylinder color.
void RigidCylinder::SetColor(Float3 color, Light *light)
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
