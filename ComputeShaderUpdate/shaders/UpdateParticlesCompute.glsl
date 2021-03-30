#version 430 core

// Process particles in blocks of 128
layout (local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer PositionBuffer
{
  vec4 positions[];
};
layout (std430, binding = 1) buffer VelocityBuffer
{
  vec3 velocities[];
};
layout (binding = 2) buffer AttractorBuffer
{
  vec3 attractors[];
};

// Delta time
uniform float dt;


highp float rand(vec2 co)
{
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}


vec3 calcForceFor (vec3 forcePoint, vec3 pos)
{
  // Force:
  float gauss = 10000.0;
  float e = 2.71828183;
  float k_weak = 1.0;
  vec3 dir = forcePoint - pos.xyz;
  float g = pow (e, -pow(length(dir), 2) / gauss);
  vec3 f = normalize(dir) * k_weak * (1+ mod(rand(dir.xy), 10) - mod(rand(dir.yz), 10)) / 10.0 * g;
  return f;
}

void main()
{
  uint index = gl_GlobalInvocationID.x;

  int i;
  float newDT = dt * 100.0;


  vec3 forcePoint = vec3(0);

  for (i = 0; i < attractors.length(); i++)
  {
    forcePoint += attractors[i];
  }

  // Read the current position and velocity from the buffers
  vec3 vel = velocities[index];
  vec3 pos = positions[index].xyz;
  float newW = positions[index].w;

  float k_v = 1.5;

  vec3 f = calcForceFor(forcePoint, pos) + rand(pos.xz)/100.0;

  // Velocity:
  vec3 v = normalize(vel.xyz + (f * newDT)) * k_v;

  v += (forcePoint-pos) * 0.005;

  // Pos:
  vec3 s = pos + v * newDT;

  newW -= 0.0001f * newDT;

  // If the particle expires, reset it
  if (newW <= 0)
  {
    s  = -s + rand(s.xy)*40.0 -rand(s.yz)*40.0;
    newW = 0.99f;
  }

  positions[index].w = newW;
  // Store the new position and velocity back into the buffers
  positions[index].xyz = s;
  velocities[index] = v;

}
