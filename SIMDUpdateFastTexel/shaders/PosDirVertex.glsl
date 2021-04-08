#version 400 core

uniform mat4 MVP;
out vec3 dir;
uniform samplerBuffer posXSampler;
uniform samplerBuffer posZSampler;
uniform samplerBuffer dirXSampler;
uniform samplerBuffer dirZSampler;
void main()
{
    vec3 inPos=vec3(texelFetch(posXSampler,gl_VertexID).x, 0.0,texelFetch(posZSampler,gl_VertexID).x) ;
    vec3 inDir=vec3(texelFetch(dirXSampler,gl_VertexID).x, 0.0,texelFetch(dirZSampler,gl_VertexID).x) ;
    gl_Position=MVP*vec4(inPos,1.0);
    dir=vec3(MVP*vec4(inDir,0));
}
