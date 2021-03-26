#ifndef PARTICLE_H_
#define PARTICLE_H_
#include <ngl/Vec3.h>

struct Particle
{
    Particle()=default;
    Particle(ngl::Vec3 _pos, ngl::Vec3 _dir) : pos{_pos},dir{_dir}{}
    ngl::Vec3 pos;
    ngl::Vec3 dir;

};

#endif