#version 400 core

uniform mat4 MVP;
flat out vec3 dir;
uniform samplerBuffer posSampler;
uniform samplerBuffer dirSampler;

void main()
{
    vec3 inPos=texelFetch(posSampler,gl_VertexID).xyz;
    vec3 inDir=texelFetch(dirSampler,gl_VertexID).xyz;
    gl_Position=MVP*vec4(inPos,1.0);
    dir=vec3(MVP*vec4(inDir,0));
}
