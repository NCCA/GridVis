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

TEST(VEC3x8,plusEquals)
{
   ngl::Vec3 a[8]={{1,2,3},{4,5,6},{7,8,9},{10,11,12},
              {13,14,15},{16,17,18},{19,20,21},{22,23,24}};
    Vec3x8 v(a);
    Vec3x8 x(a);

    
    v+=x;
    for(int i=0; i<8; ++i)
    {
        auto test=v[i];
        EXPECT_TRUE(test==(a[i]+a[i]));
    }
}

TEST(VEC3x8,clamp)
{
    {
        Vec3x8 lower(0.1f);
        lower.clamp(0.2f,1.0f);
        for(int i=0; i<8; ++i)
        {
            auto test=lower[i];
            EXPECT_FLOAT_EQ(test.m_x,0.2f);
            EXPECT_FLOAT_EQ(test.m_y,0.2f);
            EXPECT_FLOAT_EQ(test.m_z,0.2f);
        }
    }
    {
        Vec3x8 upper(-2.1f);
        upper.clamp(-0.8f,0.8f);
        for(int i=0; i<8; ++i)
        {
            auto test=upper[i];
            EXPECT_FLOAT_EQ(test.m_x,-0.8f);
            EXPECT_FLOAT_EQ(test.m_y,-0.8f);
            EXPECT_FLOAT_EQ(test.m_z,-0.8f);
        }
    }

}