#ifndef VEC3X8_H_
#define VEC3X8_H_

// SIMD version of a Vec3 class that batches 8 vec3 so we can use SIMD operations
// This is minimal for this demo and can be improved to a full featured version if 
// needed

#ifdef _MSC_VER
// visual C++ only sets AVX2 flag :(
# ifndef __AVX2__
#  error Set the compile option:  /arch:AVX2   in project settings -> code generation -> enhanced instruction set
# endif
#else
# if !defined(__AVX2__) || !defined(__FMA__) || !defined(__F16C__)
#  error add the following to your compiler flags: -mavx2 -mfma -mf16c
# endif
#endif


#include <immintrin.h>
#include <cstdint>
#include <ngl/Vec3.h>
#if defined(_MSC_VER)
#define ALIGNED(x) __declspec(align(x))
#else
#if defined(__GNUC__)
#define ALIGNED(x) __attribute__ ((aligned(x)))
#endif
#endif
class Vec3x8
{
  public :
    Vec3x8();
    Vec3x8(ngl::Vec3 _v[8]);
    Vec3x8(const Vec3x8 &)=default;
    ngl::Vec3  operator[](size_t i) const ;


  private :

    __m256 m_x; // float[8]
    __m256 m_y; // float[8]
    __m256 m_z; // float[8]

};

#endif
