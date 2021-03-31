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