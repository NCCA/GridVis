# CPUOnly

This is the base demo to show a simple particle on grid moving, in this case each particle will be given an initial position on the grid and direction. If it reaches the edge of the grid it will bounce using the ngl::Vec3 reflect function.


The visualisation uses the Geometry shader to draw a line based on the current particle position and the direction of the particle.

```
-w [int] The width of the grid default 150
-h [int] The Height of the grid default 150
-n [int] The number of particles default 2000
```