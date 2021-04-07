# GridVis

A series of demos showing a simple grid based simulation and how it can be visualised.

The simplest demo uses the CPU to do all of the particle updates and draws using brute force, the other demos improve on this using modern GPU techniques (most of which will only work with OpenGL 4.3 and above so not for mac)


All demos take the following command line options 

```
-w [int] The width of the grid default 150
-h [int] The Height of the grid default 150
-n [int] The number of particles default 2000
```

## Analysis of programs
These are the approx times (Mac Book pro 2013) for 100000 (which may get rounded to nearest 8 for SIMD)

CPUOnly Draw 16500 uS Update 6600uS  
CPU SOA Draw 240uS Update 9150uS  
Texel Fetch Draw 56uS Buffer Update 300uS Sim Update 9800uS 
Threaded Update Draw 56uS Buffer Update 300uS Sim Update 2820uS
TBB Update Draw 56uS Buffer Update 300uS Sim Update 2800uS 
SIMD Update Draw 62uS Buffer Update 1800uS Sim Update 930uS   

## [CPUOnly](CPUOnly/)

This is the simplest implementation and it uses an array of structures for the data layout. It's not particularly quick and the draw function takes longer than the update (this is due to copying the data to a suitable format for OpenGL).

## [CPUSOA](CPUSOA)





## References

https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object
https://www.khronos.org/opengl/wiki/Image_Load_Store
https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/texelFetch.xhtml