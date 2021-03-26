# GridVis

A series of demos showing a simple grid based simulation and how it can be visualised.

The simplest demo uses the CPU to do all of the particle updates and draws using brute force, the other demos improve on this using modern GPU techniques (most of which will only work with OpenGL 4.3 and above so not for mac)


All demos take the following command line options 

```
-w [int] The width of the grid default 150
-h [int] The Height of the grid default 150
-n [int] The number of particles default 2000
```


## References

https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object
https://www.khronos.org/opengl/wiki/Image_Load_Store
https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/texelFetch.xhtml