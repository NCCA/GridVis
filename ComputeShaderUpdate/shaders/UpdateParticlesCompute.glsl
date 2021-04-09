#version 430 core

// Process particles in blocks of 128
layout (local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer PositionBuffer
{
  vec4 positions[]; // w is Accel

};
layout (std430, binding = 1) buffer DirBuffer
{
  vec4 directions[]; // w is max speed
};

uniform float xsize;
uniform float zsize;
uniform float dt;

highp float rand(vec2 co)
{
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}



void main()
{
  uint index = gl_GlobalInvocationID.x;
  
  vec3 dir = directions[index].xyz * vec3(positions[index].w, 0.0f, positions[index].w) * dt;
  dir=clamp(dir,-directions[index].w,directions[index].w);
  
  positions[index].xyz = positions[index].xyz + dir;


  // Now check against the bounds of the grid and reflect if needed this is quite brute force but works
  // left plane
  if(positions[index].x <= -xsize)
  {
    directions[index].xyz= reflect(directions[index].xyz,vec3(1.0f,0.0f,0.0f));
    //directions[index].w-=0.1f;
  }
  // right plane
  else if(positions[index].x >= xsize)
  {
    directions[index].xyz= reflect(directions[index].xyz,vec3(-1.0f,0.0f,0.0f));
    //directions[index].w-=0.1f;
  }
  // // top plane
  if(positions[index].z >= zsize)
  {
    directions[index].xyz=reflect(directions[index].xyz,vec3(0.0f,0.0f,1.0f));
    //directions[index].w-=0.1f;
  }
  // bottom plane
  else if(positions[index].z <= -zsize)
  {
    directions[index].xyz= reflect(directions[index].xyz,vec3(0.0f,0.0f,-1.0f));
  //  directions[index].w-=0.1f;
  }


  // if(directions[index].w <= 0.0)
  // {
  //   directions[index].w=2.0*rand(vec2(xsize,zsize);
  // }


}
