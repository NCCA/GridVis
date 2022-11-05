#ifndef VEC3X4_H_
#define VEC3X4_H_

// SIMD version of a Vec3 class that batches 4 vec3 so we can use SIMD operations
// This is minimal for this demo and can be improved to a full featured version if
// needed
#if defined(__x86_64__)
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
#elif defined(__arm64__)
#include "sse2neon.h"
#else
#error "Intrinsics not supported on this platform"
#endif

#include <cstdint>

#include <ngl/Vec3.h>
#if defined(_MSC_VER)
#define ALIGNED(x) __declspec(align(x))
#else
#if defined(__GNUC__)
#define ALIGNED(x) __attribute__((aligned(x)))
#define __vectorcall ""
#endif
#endif
class Vec3x4
{
public:
  Vec3x4();
  Vec3x4(ngl::Vec3 _v[8]);
  Vec3x4(const Vec3x4 &) = default;
  Vec3x4(__m128 _x, __m128 _y, __m128 _z);
  Vec3x4(float _v); // set everything to single value
  Vec3x4(const ngl::Vec3 &_v);
  void set(__m128 _x, __m128 _y, __m128 _z);

  ngl::Vec3 operator[](size_t _i) const;
  Vec3x4 operator*(const Vec3x4 &_r) const;
  Vec3x4 operator*(float _r) const;
  Vec3x4 operator*(__m128 _r) const;

  Vec3x4 operator+(const Vec3x4 &_r) const;
  Vec3x4 reflect(const ngl::Vec3 &_r) const;
  __m128 dot(const Vec3x4 &_r) const;
  void operator+=(const Vec3x4 &_r);
  void clamp(float _min, float _max);
  void __vectorcall clamp(__m128 _value);
  void fillArray(ngl::Vec3 io_v[8]) const;

  __m128 x() const { return m_x; }
  __m128 y() const { return m_y; }
  __m128 z() const { return m_z; }

private:
  __m128 m_x; // float[4]
  __m128 m_y; // float[4]
  __m128 m_z; // float[4]
};

Vec3x4 operator*(float _r, const Vec3x4 &_rhs);

#endif
