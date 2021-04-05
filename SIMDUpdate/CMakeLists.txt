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
project(SIMDUpdateBuild)
# This is the name of the Exe change this and it will change everywhere
set(TargetName SIMDUpdate)
# This will include the file NGLConfig.cmake, you need to add the location to this either using
# -DCMAKE_PREFIX_PATH=~/NGL or as a system environment variable. 
find_package(NGL CONFIG REQUIRED)
# Instruct CMake to run moc automatically when needed (Qt projects only)
set(CMAKE_AUTOMOC ON)
# find Qt libs
find_package(Qt5Widgets)
find_package(Qt5Concurrent)
# use C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
# Set the name of the executable we want to build
add_executable(${TargetName})
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   target_compile_options(${TargetName} PRIVATE -msse -msse2 -msse3 -mavx2 -mfma -mf16c)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   target_compile_options(${TargetName} PRIVATE /arch:AVX2)
endif()

# Add NGL include path
include_directories(include $ENV{HOME}/NGL/include)
target_sources(${TargetName} PRIVATE ${PROJECT_SOURCE_DIR}/src/main.cpp  
			${PROJECT_SOURCE_DIR}/src/NGLScene.cpp  
			${PROJECT_SOURCE_DIR}/src/Grid.cpp  
            ${PROJECT_SOURCE_DIR}/src/Vec3x8.cpp   
			${PROJECT_SOURCE_DIR}/include/NGLScene.h  
            ${PROJECT_SOURCE_DIR}/include/Grid.h             
            ${PROJECT_SOURCE_DIR}/include/Vec3x8.h             
)
add_compile_definitions(GL_SILENCE_DEPRECATION)
target_link_libraries(${TargetName} PRIVATE  NGL Qt5::Widgets)

add_custom_target(${TargetName}CopyShaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:${TargetName}>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/fonts
    $<TARGET_FILE_DIR:${TargetName}>/fonts
)