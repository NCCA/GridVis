# TBB Threaded Update

This demo is an extension of the Texel Fetch demo with an effort to use threads to update the particles. As this is an [Embarrassingly parallel](https://en.wikipedia.org/wiki/Embarrassingly_parallel) algorithm it is easy to thread.

In one of the previous examples I used standard C++ 11 threading to create a thread pool and execute a parallel for loop. Intel's Threading Building Blocks [github](https://github.com/oneapi-src/oneTBB) has a nice built in paralled_for construct that does this for us.  You can install tbb using vcpkg for ease.

The loop then becomes

```
tbb::parallel_for( tbb::blocked_range<int>(0,m_numParticles),
                       [&](tbb::blocked_range<int> r)
                       {
                          for (int i=r.begin(); i<r.end(); ++i)
                          {
                             updateParticle(i,_dt);
                          }
                       });
```


## Command line Options.

```
-w [int] The width of the grid default 150
-h [int] The Height of the grid default 150
-n [int] The number of particles default 2000
```