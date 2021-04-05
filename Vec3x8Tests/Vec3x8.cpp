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

Vec3x8::Vec3x8(const ngl::Vec3  &_v)
{
  m_x=_mm256_set1_ps(_v.m_x);
  m_y=_mm256_set1_ps(_v.m_y);
  m_z=_mm256_set1_ps(_v.m_z);
}


Vec3x8::Vec3x8(__m256 _x, __m256 _y, __m256 _z)
{
  m_x=_x;
  m_y=_y;
  m_z=_z;
}

void Vec3x8::set(__m256 _x, __m256 _y, __m256 _z)
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

__m256 Vec3x8::dot(const Vec3x8 & _r) const 
{
  // 	return m_x * _v.m_x + m_y * _v.m_y + m_z * _v.m_z;
  auto x=_mm256_mul_ps(m_x,_r.m_x);
  auto y=_mm256_mul_ps(m_y,_r.m_y);
  auto z=_mm256_mul_ps(m_z,_r.m_z);
  auto ret=_mm256_add_ps(x,y);
  return _mm256_add_ps(ret,z);
}

Vec3x8  Vec3x8::operator*(__m256 _r) const 
{
  auto x=_mm256_mul_ps(m_x,_r);
  auto y=_mm256_mul_ps(m_y,_r);
  auto z=_mm256_mul_ps(m_z,_r);
  return Vec3x8(x,y,z);
}


Vec3x8 Vec3x8::reflect(const ngl::Vec3 & _r) const 
{
 //  I - 2.0 * dot(N, I) * N
  auto d=dot(Vec3x8(_r));
  //  return Vec3( m_x-2.0f*d*_n.m_x, m_y-2.0f*d*_n.m_y, m_z-2.0f*d*_n.m_z);

  d=_mm256_mul_ps(_mm256_set1_ps(2.0f),d);
  auto inx = _mm256_set1_ps(_r.m_x);
  auto iny = _mm256_set1_ps(_r.m_y);
  auto inz = _mm256_set1_ps(_r.m_z);
 
  auto x=_mm256_fnmadd_ps(d,inx,m_x);
  auto y=_mm256_fnmadd_ps(d,iny,m_y);
  auto z=_mm256_fnmadd_ps(d,inz,m_z);
  return Vec3x8(x,y,z);
 
}
void  Vec3x8::operator+=(const Vec3x8 &_r)
{
  m_x=_mm256_add_ps(m_x,_r.m_x);
  m_y=_mm256_add_ps(m_y,_r.m_y);
  m_z=_mm256_add_ps(m_z,_r.m_z);
}
