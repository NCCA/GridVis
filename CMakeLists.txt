cmake_minimum_required(VERSION 3.12)
#-------------------------------------------------------------------------------------------
# I'm going to use vcpk in most cases for our install of 3rd party libs
# this is going to check the environment variable for CMAKE_TOOLCHAIN_FILE and this must point to where
# vcpkg.cmake is in the University this is set in your .bash_profile to
# export CMAKE_TOOLCHAIN_FILE=/public/devel/2020/vcpkg/scripts/buildsystems/vcpkg.cmake
#-------------------------------------------------------------------------------------------
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
   set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})
endif()

project(GridVisBuildALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/CPUOnly/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/CPUSOA/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/TexelFetch/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/ThreadedUpdate/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/TBBThreadedUpdate/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/Vec3x4Tests/ )
if(${CMAKE_SYSTEM_NAME} MATCHES  "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES  "Windows")

   add_subdirectory(${PROJECT_SOURCE_DIR}/ComputeShaderUpdate/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/SIMDUpdate/ )
   add_subdirectory(${PROJECT_SOURCE_DIR}/SIMDUpdateFastTexel/ )
else()
   message("excluding Computer shader and SIMD demos on Mac")
endif()

