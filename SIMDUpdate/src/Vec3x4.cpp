#include "Vec3x4.h"

Vec3x4::Vec3x4()
{
  m_x=_mm_setzero_ps();
  m_y=_mm_setzero_ps();
  m_z=_mm_setzero_ps();
}

Vec3x4::Vec3x4(float _v)
{
  m_x=_mm_set1_ps(_v);
  m_y=_mm_set1_ps(_v);
  m_z=_mm_set1_ps(_v);
}

Vec3x4::Vec3x4(const ngl::Vec3  &_v)
{
  m_x=_mm_set1_ps(_v.m_x);
  m_y=_mm_set1_ps(_v.m_y);
  m_z=_mm_set1_ps(_v.m_z);
}


Vec3x4::Vec3x4(__m128 _x, __m128 _y, __m128 _z)
{
  m_x=_x;
  m_y=_y;
  m_z=_z;
}

void Vec3x4::set(__m128 _x, __m128 _y, __m128 _z)
{
  m_x=_x;
  m_y=_y;
  m_z=_z;
}

Vec3x4::Vec3x4(ngl::Vec3 _v[8])
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
  m_x=_mm_load_ps(x);
  m_y=_mm_load_ps(y);
  m_z=_mm_load_ps(z);
  
}

ngl::Vec3  Vec3x4::operator[](size_t _i) const
{
  float x[4];
  float y[4];
  float z[4];
  _mm_store_ps(static_cast<float *>(x), m_x);
  _mm_store_ps(static_cast<float *>(y), m_y);
  _mm_store_ps(static_cast<float *>(z), m_z);
  return ngl::Vec3(x[_i],y[_i],z[_i]);
}


Vec3x4  Vec3x4::operator*(const Vec3x4 &_r) const 
{

  auto x=_mm_mul_ps(m_x,_r.m_x);
  auto y=_mm_mul_ps(m_y,_r.m_y);
  auto z=_mm_mul_ps(m_z,_r.m_z);

  return Vec3x4(x,y,z);
}

Vec3x4  Vec3x4::operator*(float _r) const 
{
  auto scalar=_mm_set1_ps(_r);
  auto x=_mm_mul_ps(m_x,scalar);
  auto y=_mm_mul_ps(m_y,scalar);
  auto z=_mm_mul_ps(m_z,scalar);

  return Vec3x4(x,y,z);
}

Vec3x4  operator*(float _r,const Vec3x4 &_rhs)
{
  auto scalar=_mm_set1_ps(_r);
  auto x=_mm_mul_ps(scalar,_rhs.x());
  auto y=_mm_mul_ps(scalar,_rhs.y());
  auto z=_mm_mul_ps(scalar,_rhs.z());
  return Vec3x4(x,y,z);
}


void Vec3x4::clamp(float _min, float _max)
{
  auto min=_mm_set1_ps(_min);
  auto max=_mm_set1_ps(_max);
  
  m_x=_mm_max_ps(m_x,min);
  m_y=_mm_max_ps(m_y,min);
  m_z=_mm_max_ps(m_z,min);
  
  m_x=_mm_min_ps(m_x,max);
  m_y=_mm_min_ps(m_y,max);
  m_z=_mm_min_ps(m_z,max);

}


Vec3x4  Vec3x4::operator+(const Vec3x4 &_r) const 
{
  auto x=_mm_add_ps(m_x,_r.m_x);
  auto y=_mm_add_ps(m_y,_r.m_y);
  auto z=_mm_add_ps(m_z,_r.m_z);
  return Vec3x4(x,y,z);
}

__m128 Vec3x4::dot(const Vec3x4 & _r) const 
{
  // 	return m_x * _v.m_x + m_y * _v.m_y + m_z * _v.m_z;
  auto x=_mm_mul_ps(m_x,_r.m_x);
  auto y=_mm_mul_ps(m_y,_r.m_y);
  auto z=_mm_mul_ps(m_z,_r.m_z);
  auto ret=_mm_add_ps(x,y);
  return _mm_add_ps(ret,z);
}

Vec3x4  Vec3x4::operator*(__m128 _r) const 
{
  auto x=_mm_mul_ps(m_x,_r);
  auto y=_mm_mul_ps(m_y,_r);
  auto z=_mm_mul_ps(m_z,_r);
  return Vec3x4(x,y,z);
}


Vec3x4 Vec3x4::reflect(const ngl::Vec3 & _r) const 
{
 //  I - 2.0 * dot(N, I) * N
  auto d=dot(Vec3x4(_r));
  //  return Vec3( m_x-2.0f*d*_n.m_x, m_y-2.0f*d*_n.m_y, m_z-2.0f*d*_n.m_z);

  d=  _mm_mul_ps(_mm_set1_ps(2.0f),d);
  // negate 
  d= _mm_sub_ps(_mm_set1_ps(0.0f), d);

  auto inx = _mm_set1_ps(_r.m_x);
  auto iny = _mm_set1_ps(_r.m_y);
  auto inz = _mm_set1_ps(_r.m_z);
  // no fnmadd in neon.  dst[i+31:i] := -(a[i+31:i] * b[i+31:i]) + c[i+31:i]
  // auto x=_mm_fnmadd_ps(d,inx,m_x);
  // auto y=_mm_fnmadd_ps(d,iny,m_y);
  // auto z=_mm_fnmadd_ps(d,inz,m_z);
  auto x=_mm_mul_ps(d,inx);
  auto y=_mm_mul_ps(d,iny);
  auto z=_mm_mul_ps(d,inz);
  x=_mm_add_ps(x,m_x);
  y=_mm_add_ps(y,m_y);
  z=_mm_add_ps(z,m_z);
  
  return Vec3x4(x,y,z);
 
}
void  Vec3x4::operator+=(const Vec3x4 &_r)
{
  m_x=_mm_add_ps(m_x,_r.m_x);
  m_y=_mm_add_ps(m_y,_r.m_y);
  m_z=_mm_add_ps(m_z,_r.m_z);
}

void Vec3x4::clamp(__m128 _range)
{
  auto min = _mm_sub_ps(_mm_set1_ps(0.0f), _range);
  m_x=_mm_max_ps(m_x,min);
  m_y=_mm_max_ps(m_y,min);
  m_z=_mm_max_ps(m_z,min);
  
  m_x=_mm_min_ps(m_x,_range);
  m_y=_mm_min_ps(m_y,_range);
  m_z=_mm_min_ps(m_z,_range);

}

void Vec3x4::fillArray(ngl::Vec3 io_v[4]) const
{
  float x[4];
  float y[4];
  float z[4];
  _mm_store_ps(static_cast<float *>(x), m_x);
  _mm_store_ps(static_cast<float *>(y), m_y);
  _mm_store_ps(static_cast<float *>(z), m_z);
  for(int i=0; i<4; ++i)
    io_v[i].set(x[i],y[i],z[i]);
}

