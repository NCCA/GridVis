#version 430 core

// Process particles in blocks of 128
layout (local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer PositionBuffer
{
  vec4 positions[]; // w is Accel

};
layout (std430, binding = 1) buffer DirBuffer
{
  vec4 direction[]; // w is max speed
};



highp float rand(vec2 co)
{
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}



void main()
{
  uint index = gl_GlobalInvocationID.x;
  float dt = 0.01;
  vec3 dir = direction[index].xyz * vec3(positions[index].w, 0.0f, positions[index].w) * dt;
  dir=clamp(dir,-direction[index].w,direction[index].w);
  
  positions[index].xyz = positions[index].xyz + dir;
  positions[index].w = positions[index].w;
  direction[index]= direction[index];

//  positions[index].w = newW;
  // Store the new position and velocity back into the buffers
//  positions[index].xyz = s;
//  velocities[index] = v;

}
