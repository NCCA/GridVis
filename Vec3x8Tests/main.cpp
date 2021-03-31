#include <gtest/gtest.h>
#include <ngl/Vec3.h>
#include "Vec3x8.h"

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}


TEST(VEC3x8,defaultCtor)
{
    Vec3x8 v;
    for(int i=0; i<8; ++i)
    {
        auto a=v[i];
        EXPECT_TRUE(a==ngl::Vec3(0.0f,0.0f,0.0f));
    }
}

TEST(VEC3x8,arrayCtor)
{
   ngl::Vec3 a[8]={{1,2,3},{4,5,6},{7,8,9},{10,11,12},
              {13,14,15},{16,17,18},{19,20,21},{22,23,24}};
    Vec3x8 v(a);
    for(int i=0; i<8; ++i)
    {
        auto test=v[i];
        EXPECT_TRUE(test==a[i]);
    }
}

TEST(VEC3x8,floatCtor)
{
    Vec3x8 v(0.25f);
    for(int i=0; i<8; ++i)
    {
        auto test=v[i];
        EXPECT_FLOAT_EQ(test.m_x,0.25);
        EXPECT_FLOAT_EQ(test.m_y,0.25);
        EXPECT_FLOAT_EQ(test.m_z,0.25);
    }
}



TEST(VEC3x8,mult)
{
   ngl::Vec3 a[8]={{1,2,3},{4,5,6},{7,8,9},{10,11,12},
              {13,14,15},{16,17,18},{19,20,21},{22,23,24}};
    Vec3x8 v(a);
    Vec3x8 mult(0.5f);
    auto res=v*mult;
    for(int i=0; i<8; ++i)
    {
        auto test=res[i];
        EXPECT_TRUE(test==a[i]*0.5f);
    }
}


TEST(VEC3x8,multScalar)
{
   ngl::Vec3 a[8]={{1,2,3},{4,5,6},{7,8,9},{10,11,12},
              {13,14,15},{16,17,18},{19,20,21},{22,23,24}};
    Vec3x8 v(a);
    
    auto res=v*0.5f;
    for(int i=0; i<8; ++i)
    {
        auto test=res[i];
        EXPECT_TRUE(test==a[i]*0.5f);
    }
}

TEST(VEC3x8,multScalarVec)
{

    Vec3x8 a(1.0f);
    __m256 values=_mm256_setr_ps(0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f);

    auto res=a*values;
    float inc=0.1f;
    for(int i=0; i<8; ++i)
    {
      EXPECT_FLOAT_EQ(res[i].m_x,inc);
      EXPECT_FLOAT_EQ(res[i].m_y,inc);
      EXPECT_FLOAT_EQ(res[i].m_z,inc);
      inc+=0.1f;
    }



}



TEST(VEC3x8,add)
{
   ngl::Vec3 a[8]={{1,2,3},{4,5,6},{7,8,9},{10,11,12},
              {13,14,15},{16,17,18},{19,20,21},{22,23,24}};
    Vec3x8 v(a);
    Vec3x8 x(a);

    
    auto res=v+x;
    for(int i=0; i<8; ++i)
    {
        auto test=res[i];
        EXPECT_TRUE(test==(a[i]+a[i]));
    }
}

TEST(Vec3x8,dot)
{
  Vec3x8 a(ngl::Vec3(1.0f,2.0f,3.0f));
  Vec3x8 b(ngl::Vec3(4.0f,5.0f,6.0f));
  auto res=a.dot(b);
  // copy the array of 8 floats back should all be 32.0f
  float dots[8];
  _mm256_store_ps(static_cast<float *>(dots), res);
  for(int i=0; i<8; ++i)
  {
    EXPECT_FLOAT_EQ(dots[i],32.0f);
  }

}


TEST(Vec3x8,reflect)
{
  Vec3x8 a(ngl::Vec3(0.1f,0.2f,2.0f));
  ngl::Vec3 na(0.1f,0.2f,2.0f);
  auto refngl=na.reflect(ngl::Vec3(0.0f,1.0f,0.0f));

  auto ref=a.reflect(ngl::Vec3(0.0f,1.0f,0.0f));

  for(int i=0; i<8; ++i)
  {
    auto test=ref[i];
    EXPECT_FLOAT_EQ(refngl.m_x,test.m_x);
    EXPECT_FLOAT_EQ(refngl.m_y,test.m_y);
    EXPECT_FLOAT_EQ(refngl.m_z,test.m_z);
  }

}