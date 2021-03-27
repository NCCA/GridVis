#version 400 core

layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inDir;
uniform mat4 MVP;
flat out vec3 dir;

void main()
{
    gl_Position=MVP*vec4(inPos,1.0);
    dir=vec3(MVP*vec4(inDir,0));

}
