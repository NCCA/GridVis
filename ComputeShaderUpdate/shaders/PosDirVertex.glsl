#version 400 core

uniform mat4 MVP;
out vec3 dir;
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inDir;

void main()
{
//    vec3 inPos=texelFetch(posSampler,gl_VertexID).xyz;
//    vec3 inDir=texelFetch(dirSampler,gl_VertexID).xyz;
  

    gl_Position=MVP*vec4(inPos.xyz,1.0);
    dir=vec3(MVP*vec4(inDir.xyz,0));
}
