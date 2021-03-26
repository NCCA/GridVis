#version 410 core

layout(location=0) in vec2 xz;
uniform mat4 MVP;
// this sample buffer is defalted to 0 if you need more set in client program for the id.
uniform samplerBuffer yPosSampler;

void main()
{
  // read the data from the 1D array using gl_VertexID as the index.
  float ypos=texelFetch(yPosSampler,gl_VertexID).r;
  gl_Position = MVP * vec4(xz.x,ypos,xz.y,1);
}