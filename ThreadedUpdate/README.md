# Threaded Update

This demo is an extension of the Texel Fetch demo with an effort to use threads to update the particles. As this is an [Embarrassingly parallel](https://en.wikipedia.org/wiki/Embarrassingly_parallel) algorithm it is easy to thread.

It would be easy to do this using OpenMP parallel for loops however not supported under mac so I have used a simple thread pool. 

We create a std::vector<std::thread> on construction of the grid class with the number of cores we have free (8 on my mac). We then partition the work load into chunks and have a thread for each update as follows.

```
void Grid::update(float _dt)
{
  // implement an OpenMP paralell for loop (as mac doesn support omp)
  for(int t = 0;t<m_nthreads;t++)
  {
    m_threadPool[t] = std::thread(std::bind(
        [&](const int bi, const int ei, const int t)
        {
          // loop over all items
          for(int i = bi;i<ei;i++)
          {
            // inner loop
            {
              updateParticle(i,_dt);
            }
          }
        },t*m_numParticles/m_nthreads,(t+1)==m_nthreads?m_numParticles:(t+1)*m_numParticles/m_nthreads,t));
    }
    std::for_each(m_threadPool.begin(),m_threadPool.end(),[](std::thread& x){x.join();});

  updateTextureBuffer();
}

```


## Command line Options.

```
-w [int] The width of the grid default 150
-h [int] The Height of the grid default 150
-n [int] The number of particles default 2000
```