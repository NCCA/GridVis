#ifndef VEC3X8_H_
#define VEC3X8_H_

// SIMD version of a Vec3 class that batches 8 vec3 so we can use SIMD operations
// This is minimal for this demo and can be improved to a full featured version if
// needed

#ifdef __x86_64__
#ifdef _MSC_VER
// visual C++ only sets AVX2 flag :(
#ifndef __AVX2__
#error Set the compile option:  /arch:AVX2   in project settings -> code generation -> enhanced instruction set
#endif
#else
#if !defined(__AVX2__) || !defined(__FMA__) || !defined(__F16C__)
#error add the following to your compiler flags: -mavx2 -mfma -mf16c
#endif
#endif

#include <immintrin.h>
#else
#include "sse2neon.h"
#endif
#include <cstdint>
#include <ngl/Vec3.h>
#if defined(_MSC_VER)
#define ALIGNED(x) __declspec(align(x))
#else
#if defined(__GNUC__)
#define ALIGNED(x) __attribute__((aligned(x)))
#define __vectorcall
#endif

#endif
class Vec3x8
{
public:
  Vec3x8();
  Vec3x8(ngl::Vec3 _v[8]);
  Vec3x8(const Vec3x8 &) = default;
  Vec3x8(__m256 _x, __m256 _y, __m256 _z);
  Vec3x8(float _v); // set everything to single value
  Vec3x8(const ngl::Vec3 &_v);
  void __vectorcall set(__m256 _x, __m256 _y, __m256 _z);

  ngl::Vec3 operator[](size_t _i) const;
  void fillArray(ngl::Vec3 io_v[8]) const;
  Vec3x8 operator*(const Vec3x8 &_r) const;
  Vec3x8 operator*(float _r) const;
  Vec3x8 __vectorcall operator*(__m256 _r) const;

  Vec3x8 __vectorcall operator+(const Vec3x8 &_r) const;
  Vec3x8 __vectorcall reflect(const ngl::Vec3 &_r) const;
  __m256 __vectorcall dot(const Vec3x8 &_r) const;
  void __vectorcall operator+=(const Vec3x8 &_r);
  void clamp(float _min, float _max);
  // clamp to +/- this value in the simd register
  void __vectorcall clamp(__m256 _value);

  __m256 __vectorcall x() const { return m_x; }
  __m256 __vectorcall y() const { return m_y; }
  __m256 __vectorcall z() const { return m_z; }

private:
#pragma pack(push, 1)

  __m256 m_x; // float[8]
  __m256 m_y; // float[8]
  __m256 m_z; // float[8]
#pragma pack(pop)
};

Vec3x8 operator*(float _r, const Vec3x8 &_rhs);

#endif
