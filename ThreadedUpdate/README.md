# TexelFetch

This demo put both position and direction values in a TextureBuffer and updates the values each frame.

A VAO is created but no data / buffers are associated with it so when draw is called the vertex shader is executed without vertex attributes being passed.

The Vertex shader uses texelFetch to read the data from the bound texture buffers containing float32f values.

```
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
```

## Command line Options.

```
-w [int] The width of the grid default 150
-h [int] The Height of the grid default 150
-n [int] The number of particles default 2000
```