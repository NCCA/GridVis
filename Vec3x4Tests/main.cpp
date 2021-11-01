#include <gtest/gtest.h>
#include <ngl/Vec3.h>
#include "Vec3x4.h"

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}


TEST(VEC3x4,defaultCtor)
{
    Vec3x4 v;
    for(int i=0; i<4; ++i)
    {
        auto a=v[i];
        EXPECT_TRUE(a==ngl::Vec3(0.0f,0.0f,0.0f));
    }
}

TEST(VEC3x4,arrayCtor)
{
   ngl::Vec3 a[4]={{1,2,3},{4,5,6},{7,8,9},{10,11,12}};
    Vec3x4 v(a);
    for(int i=0; i<4; ++i)
    {
        auto test=v[i];
        EXPECT_TRUE(test==a[i]);
    }
}

TEST(VEC3x4,floatCtor)
{
    Vec3x4 v(0.25f);
    for(int i=0; i<4; ++i)
    {
        auto test=v[i];
        EXPECT_FLOAT_EQ(test.m_x,0.25f);
        EXPECT_FLOAT_EQ(test.m_y,0.25f);
        EXPECT_FLOAT_EQ(test.m_z,0.25f);
    }
}



TEST(VEC3x4,mult)
{
   ngl::Vec3 a[4]={{1,2,3},{4,5,6},{7,8,9},{10,11,12}};
    Vec3x4 v(a);
    Vec3x4 mult(0.5f);
    auto res=v*mult;
    for(int i=0; i<4; ++i)
    {
        auto test=res[i];
        EXPECT_TRUE(test==a[i]*0.5f);
    }
}


TEST(VEC3x4,multScalar)
{
   ngl::Vec3 a[4]={{1,2,3},{4,5,6},{7,8,9},{10,11,12}};
    Vec3x4 v(a);
    
    auto res=v*0.5f;
    for(int i=0; i<4; ++i)
    {
        auto test=res[i];
        EXPECT_TRUE(test==a[i]*0.5f);
    }
}

TEST(VEC3x4,multScalarVec)
{

    Vec3x4 a(1.0f);
    auto values=_mm_setr_ps(0.1f,0.2f,0.3f,0.4f);

    auto res=a*values;
    float inc=0.1f;
    for(int i=0; i<4; ++i)
    {
      EXPECT_FLOAT_EQ(res[i].m_x,inc);
      EXPECT_FLOAT_EQ(res[i].m_y,inc);
      EXPECT_FLOAT_EQ(res[i].m_z,inc);
      inc+=0.1f;
    }



}



TEST(VEC3x4,add)
{
   ngl::Vec3 a[4]={{1,2,3},{4,5,6},{7,8,9},{10,11,12}};
    Vec3x4 v(a);
    Vec3x4 x(a);
    auto res=v+x;
    for(int i=0; i<4; ++i)
    {
        auto test=res[i];
        EXPECT_TRUE(test==(a[i]+a[i]));
    }
}

TEST(Vec3x4,dot)
{
  Vec3x4 a(ngl::Vec3(1.0f,2.0f,3.0f));
  Vec3x4 b(ngl::Vec3(4.0f,5.0f,6.0f));
  auto res=a.dot(b);
  // copy the array of 8 floats back should all be 32.0f
  float dots[4];
  _mm_store_ps(static_cast<float *>(dots), res);
  for(int i=0; i<4; ++i)
  {
    EXPECT_FLOAT_EQ(dots[i],32.0f);
  }

}


TEST(Vec3x4,reflect)
{
  Vec3x4 a(ngl::Vec3(0.1f,0.2f,2.0f));
  ngl::Vec3 na(0.1f,0.2f,2.0f);
  auto refngl=na.reflect(ngl::Vec3(0.0f,1.0f,0.0f));

  auto ref=a.reflect(ngl::Vec3(0.0f,1.0f,0.0f));

  for(int i=0; i<4; ++i)
  {
    auto test=ref[i];
    EXPECT_FLOAT_EQ(refngl.m_x,test.m_x);
    EXPECT_FLOAT_EQ(refngl.m_y,test.m_y);
    EXPECT_FLOAT_EQ(refngl.m_z,test.m_z);
  }
}

TEST(VEC3x4,plusEquals)
{
   ngl::Vec3 a[4]={{1,2,3},{4,5,6},{7,8,9},{10,11,12}};
    Vec3x4 v(a);
    Vec3x4 x(a);

    
    v+=x;
    for(int i=0; i<4; ++i)
    {
        auto test=v[i];
        EXPECT_TRUE(test==(a[i]+a[i]));
    }
}

TEST(VEC3x4,clamp)
{
    {
        Vec3x4 lower(0.1f);
        lower.clamp(0.2f,1.0f);
        for(int i=0; i<4; ++i)
        {
            auto test=lower[i];
            EXPECT_FLOAT_EQ(test.m_x,0.2f);
            EXPECT_FLOAT_EQ(test.m_y,0.2f);
            EXPECT_FLOAT_EQ(test.m_z,0.2f);
        }
    }
    {
        Vec3x4 upper(-2.1f);
        upper.clamp(-0.8f,0.8f);
        for(int i=0; i<4; ++i)
        {
            auto test=upper[i];
            EXPECT_FLOAT_EQ(test.m_x,-0.8f);
            EXPECT_FLOAT_EQ(test.m_y,-0.8f);
            EXPECT_FLOAT_EQ(test.m_z,-0.8f);
        }
    }

}
