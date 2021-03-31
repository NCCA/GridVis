#include "Vec3x8.h"

Vec3x8::Vec3x8()
{
  m_x=_mm256_setzero_ps();
  m_y=_mm256_setzero_ps();
  m_z=_mm256_setzero_ps();
}

Vec3x8::Vec3x8(float _v)
{
  m_x=_mm256_set1_ps(_v);
  m_y=_mm256_set1_ps(_v);
  m_z=_mm256_set1_ps(_v);
}

Vec3x8::Vec3x8(__m256 _x, __m256 _y, __m256 _z)
{
  m_x=_x;
  m_y=_y;
  m_z=_z;
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

ngl::Vec3  Vec3x8::operator[](size_t _i) const
{
  float x[8];
  float y[8];
  float z[8];
  _mm256_store_ps(static_cast<float *>(x), m_x);
  _mm256_store_ps(static_cast<float *>(y), m_y);
  _mm256_store_ps(static_cast<float *>(z), m_z);
  return ngl::Vec3(x[_i],y[_i],z[_i]);
}


Vec3x8  Vec3x8::operator*(const Vec3x8 &_r) const 
{

  auto x=_mm256_mul_ps(m_x,_r.m_x);
  auto y=_mm256_mul_ps(m_y,_r.m_y);
  auto z=_mm256_mul_ps(m_z,_r.m_z);

  return Vec3x8(x,y,z);
}

Vec3x8  Vec3x8::operator*(float _r) const 
{
  auto scalar=_mm256_set1_ps(_r);
  auto x=_mm256_mul_ps(m_x,scalar);
  auto y=_mm256_mul_ps(m_y,scalar);
  auto z=_mm256_mul_ps(m_z,scalar);

  return Vec3x8(x,y,z);
}

Vec3x8  operator*(float _r,const Vec3x8 &_rhs)
{
  auto scalar=_mm256_set1_ps(_r);
  auto x=_mm256_mul_ps(scalar,_rhs.x());
  auto y=_mm256_mul_ps(scalar,_rhs.y());
  auto z=_mm256_mul_ps(scalar,_rhs.z());
  return Vec3x8(x,y,z);
}


void Vec3x8::clamp(float _min, float _max)
{
  auto min=_mm256_set1_ps(_min);
  auto max=_mm256_set1_ps(_max);
  
  m_x=_mm256_max_ps(m_x,min);
  m_y=_mm256_max_ps(m_y,min);
  m_z=_mm256_max_ps(m_z,min);
  
  m_x=_mm256_min_ps(m_x,max);
  m_y=_mm256_min_ps(m_y,max);
  m_z=_mm256_min_ps(m_z,max);

}


Vec3x8  Vec3x8::operator+(const Vec3x8 &_r) const 
{
  auto x=_mm256_add_ps(m_x,_r.m_x);
  auto y=_mm256_add_ps(m_y,_r.m_y);
  auto z=_mm256_add_ps(m_z,_r.m_z);

  return Vec3x8(x,y,z);
}

void  Vec3x8::operator+=(const Vec3x8 &_r)
{
  m_x=_mm256_add_ps(m_x,_r.m_x);
  m_y=_mm256_add_ps(m_y,_r.m_y);
  m_z=_mm256_add_ps(m_z,_r.m_z);
}
