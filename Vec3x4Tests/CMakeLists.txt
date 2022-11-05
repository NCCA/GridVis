cmake_minimum_required(VERSION 3.12)
#-------------------------------------------------------------------------------------------
# I'm going to use vcpk in most cases for our install of 3rd party libs
# this is going to check the environment variable for CMAKE_TOOLCHAIN_FILE and this must point to where
# vcpkg.cmake is in the University this is set in your .bash_profile to
# export CMAKE_TOOLCHAIN_FILE=/public/devel/2020/vcpkg/scripts/buildsystems/vcpkg.cmake
# to build see the NGL instructions 
# Windows :- mkdir build; cd build ; cmake -DCMAKE_PREFIX_PATH=~/NGL/ .. ; cmake --build . 
# Linux / Mac mkdir build; cd build; cmake -DCMAKE_PREFIX_PATH~/NGL/ .. ; make
#-------------------------------------------------------------------------------------------
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
   set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})
endif()

# Name of the project
project(Vec3x4Build)
# This is the name of the Exe change this and it will change everywhere
set(TargetName Vec3x4)
# This will include the file NGLConfig.cmake, you need to add the location to this either using
# -DCMAKE_PREFIX_PATH=~/NGL or as a system environment variable. 
find_package(NGL CONFIG REQUIRED)
# use C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
# Set the name of the executable we want to build
add_executable(${TargetName})
find_package(GTest CONFIG REQUIRED)
# bug in gtest cmake may cause issues so this is set
include(GoogleTest)
enable_testing()
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   target_compile_options(${TargetName} PRIVATE -msse -msse2 -msse3 -mavx2 -mfma -mf16c)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   target_compile_options(${TargetName} PRIVATE /arch:AVX2)
endif()

# Add NGL include path
include_directories(include $ENV{HOME}/NGL/include)
target_sources(${TargetName} PRIVATE ${PROJECT_SOURCE_DIR}/main.cpp  
			${PROJECT_SOURCE_DIR}/Vec3x4.h             
            ${PROJECT_SOURCE_DIR}/Vec3x4.cpp             
)
add_compile_definitions(GL_SILENCE_DEPRECATION)
target_link_libraries(${TargetName} PRIVATE  NGL GTest::gtest )

