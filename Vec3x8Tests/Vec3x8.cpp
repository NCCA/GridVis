#include "Vec3x8.h"

Vec3x8::Vec3x8()
{
  m_x=_mm256_setzero_ps();
  m_y=_mm256_setzero_ps();
  m_z=_mm256_setzero_ps();
}

Vec3x8::Vec3x8(ngl::Vec3 _v[8])
{
  float x[8];
  float y[8];
  float z[8];

  for(int i=0; i<8; ++i)
  {
    x[i]=_v[i].m_x;
    y[i]=_v[i].m_y;
    z[i]=_v[i].m_z;
  }
  m_x=_mm256_load_ps(x);
  m_y=_mm256_load_ps(y);
  m_z=_mm256_load_ps(z);
  
}



ngl::Vec3  Vec3x8::operator[](size_t i) const
{
  float x[8];
  float y[8];
  float z[8];
  _mm256_store_ps(static_cast<float *>(x), m_x);
  _mm256_store_ps(static_cast<float *>(y), m_y);
  _mm256_store_ps(static_cast<float *>(z), m_z);
  return ngl::Vec3(x[i],y[i],z[i]);
}

