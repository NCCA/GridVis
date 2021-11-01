  #include "Grid.h"
  #include <ngl/Random.h>
  #include <ngl/Util.h>
  #include <ngl/NGLStream.h>
  #include <ngl/VAOFactory.h>
  #include <ngl/MultiBufferVAO.h>
  #include <ngl/SimpleVAO.h>
  #include <tbb/parallel_for.h>
  #include <bitset>

  unsigned int roundTo(unsigned int value, unsigned int roundTo)
  {
      return (value + (roundTo - 1)) & ~(roundTo - 1);
  }

  #if defined(__x86_64__)
    constexpr int LaneSize=8;
  #elif defined(__arm64__)
    constexpr int LaneSize=4;
  #endif

  Grid::Grid(uint32_t _w, uint32_t _h,size_t _numParticles) 
  {
      m_width=_w;
      m_height=_h;
      m_numParticles=roundTo(_numParticles,LaneSize)/LaneSize;
      m_pos.resize(m_numParticles);
      m_dir.resize(m_numParticles);
      m_acceleration.resize(m_numParticles);
      m_maxspeed.resize(m_numParticles);
      initGrid();
      createTextureBuffer();
      // Going to use a non NGL buffer see if it is quicker
      glGenVertexArrays(1, &m_svao);
      #if defined(__x86_64__)
        std::cout<<"size of Vec3x8 "<<sizeof(Vec3x8)<<" size of buffer "<<sizeof(Vec3x8)*m_numParticles<<'\n';
      #elif defined(__arm64__)
        std::cout<<"size of Vec3x4 "<<sizeof(Vec3x4)<<" size of buffer "<<sizeof(Vec3x4)*m_numParticles<<'\n';
      #endif
      m_posBuffer.resize(m_numParticles*LaneSize);
      m_dirBuffer.resize(m_numParticles*LaneSize);

  }


  void Grid::draw() const
  {
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_BUFFER,m_posBufferID);
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture(GL_TEXTURE_BUFFER,m_dirBufferID);
    glBindVertexArray(m_svao);
    glDrawArrays(GL_POINTS,0,m_numParticles*LaneSize);      
  }

  void Grid::checkForReset(size_t _i) noexcept
  {
  
    #if defined(__x86_64__)
      const __m256 zero = _mm256_setzero_ps();
      auto cmp=_mm256_cmp_ps(m_maxspeed[_i],zero,_CMP_LE_OS);
      auto mask=_mm256_movemask_ps(cmp);
    #elif defined(__arm64__)
      const __m128 zero = _mm_setzero_ps();
      auto cmp=_mm_cmple_ps(m_maxspeed[_i],zero);
      auto mask=_mm_movemask_ps(cmp);
    #endif

    if(mask ==0)
      return ; // not to update so return

    else if(mask == 0xFF)
    {
      resetParticle(_i);
    }
    else
    {
      // we need to go to non SIMD mode here as we need to access the individual float[] elements
      // first we get the particle properties
      // If we had avx512 we could use _mm256_movemask_ps and create a new Vec3x8 and mask with the old to update.
      auto xValues=m_pos[_i].x();
      auto zValues=m_pos[_i].z();
      auto xDirValues=m_dir[_i].x();
      auto zDirValues=m_dir[_i].z();
      float x[LaneSize];
      float z[LaneSize];
      float dirx[LaneSize];
      float dirz[LaneSize];
      #if defined(__x86_64__)
        _mm256_store_ps(static_cast<float *>(x), xValues);
        _mm256_store_ps(static_cast<float *>(z), zValues);
        _mm256_store_ps(static_cast<float *>(dirx), xDirValues);
        _mm256_store_ps(static_cast<float *>(dirz), zDirValues);
      #elif defined(__arm64__)
        _mm_store_ps(static_cast<float *>(x), xValues);
        _mm_store_ps(static_cast<float *>(z), zValues);
        _mm_store_ps(static_cast<float *>(dirx), xDirValues);
        _mm_store_ps(static_cast<float *>(dirz), zDirValues);
      #endif
      float speed[LaneSize];
      float acceleration[LaneSize];
      #if defined( __x86_64__)
        _mm256_store_ps(static_cast<float *>(speed), m_maxspeed[_i]);
        _mm256_store_ps(static_cast<float *>(acceleration), m_acceleration[_i]);
      #elif defined(__arm64__)
        _mm_store_ps(static_cast<float *>(speed), m_maxspeed[_i]);
        _mm_store_ps(static_cast<float *>(acceleration), m_acceleration[_i]);
      #endif
      std::bitset<LaneSize> setLanes(mask);
      for(size_t i=0; i<setLanes.size(); ++i)
      {
        if(setLanes[i])
        {
          x[i]=ngl::Random::randomNumber(m_width/2.0f);
          z[i]=ngl::Random::randomNumber(m_height/2.0f);
          dirx[i]=ngl::Random::randomNumber(2.0);
          dirz[i]=ngl::Random::randomNumber(2.0f);
          speed[i]=ngl::Random::randomPositiveNumber(5.0f)+0.1f;
          acceleration[i]=ngl::Random::randomPositiveNumber(5.0f)+0.1f;
        }
      }
    // now reset with modified values
    #if defined(__x86_64__)
      auto newX=_mm256_load_ps(x);
      auto newZ=_mm256_load_ps(z);
      m_pos[_i].set(newX,zero,newZ);
      m_maxspeed[_i]=_mm256_load_ps(speed);
      m_acceleration[_i]=_mm256_load_ps(acceleration);
    #elif defined(__arm64__)
      auto newX=_mm_load_ps(dirx);
      auto newZ=_mm_load_ps(dirz);
      m_dir[_i].set(newX,zero,newZ);
      m_maxspeed[_i]=_mm_load_ps(speed);
      m_acceleration[_i]=_mm_load_ps(acceleration);
    #endif

  }
}

void Grid::checkForBounds(size_t _i) noexcept
{
  float xsize=m_width/2.0f;
  float zsize=m_height/2.0f;
  #if defined(__x86_64__)
    const __m256 zero = _mm256_setzero_ps();
    const __m256 minusXSize=_mm256_set1_ps(-xsize);
    const __m256 plusXSize=_mm256_set1_ps(xsize);
    const __m256 minusZSize=_mm256_set1_ps(-zsize);
    const __m256 plusZSize=_mm256_set1_ps(zsize);
  #elif defined(__arm64__)
    const __m128 zero = _mm_setzero_ps();
    const __m128 minusXSize=_mm_set1_ps(-xsize);
    const __m128 plusXSize=_mm_set1_ps(xsize);
    const __m128 minusZSize=_mm_set1_ps(-zsize);
    const __m128 plusZSize=_mm_set1_ps(zsize);
  #endif
  float xValues[LaneSize];
  float zValues[LaneSize];
  float speed[LaneSize];
  #if defined(__x86_64__)
    // grab the values from dir and speed
    _mm256_store_ps(static_cast<float *>(xValues), m_dir[_i].x());
    _mm256_store_ps(static_cast<float *>(zValues), m_dir[_i].z());
    _mm256_store_ps(static_cast<float *>(speed), m_maxspeed[_i]);

    auto cmp=_mm256_cmp_ps(m_pos[_i].x(),minusXSize,_CMP_LE_OS);
    auto mask=_mm256_movemask_ps(cmp);
  #elif defined(__arm64__)
    // grab the values from dir and speed
    _mm_store_ps(static_cast<float *>(xValues), m_dir[_i].x());
    _mm_store_ps(static_cast<float *>(zValues), m_dir[_i].z());
    _mm_store_ps(static_cast<float *>(speed), m_maxspeed[_i]);

    auto cmp=_mm_cmple_ps(m_pos[_i].x(),minusXSize);
    auto mask=_mm_movemask_ps(cmp);
  #endif
  if(mask !=0)
  {
  std::bitset<LaneSize> setLanes(mask);
   for(size_t i=0; i<setLanes.size(); ++i)
   {
     if(setLanes[i])
     {
       float dot=1.0f*xValues[i]+0.0f*zValues[i];
        // hard coded reflect value
        // m_x-2.0f*d*_n.m_x, m_y-2.0f*d*_n.m_y, m_z-2.0f*d*_n.m_z
       xValues[i]=xValues[i]-2.0f*dot*1.0f; //dir.m_x;
       zValues[i]=zValues[i]-2.0f*dot*0.0f;  //dir.m_z;
       speed[i]-=0.1f;
     }
   }
   #if defined(__x86_64__)
    m_dir[_i].set(_mm256_load_ps(xValues),zero,_mm256_load_ps(zValues));
    m_maxspeed[_i]=_mm256_load_ps(speed);
  #elif defined(__arm64__)
    m_dir[_i].set(_mm_load_ps(xValues),zero,_mm_load_ps(zValues));
    m_maxspeed[_i]=_mm_load_ps(speed);
  #endif

  }
  #if defined(__x86_64__)
    cmp=_mm256_cmp_ps(m_pos[_i].x(),plusXSize,_CMP_GE_OS);
    mask=_mm256_movemask_ps(cmp);
  #elif defined(__arm64__)
    cmp=_mm_cmpge_ps(m_pos[_i].x(),plusXSize);
    mask=_mm_movemask_ps(cmp);
  #endif

  if(mask !=0)
  {
    std::bitset<LaneSize> setLanes(mask);
   for(size_t i=0; i<setLanes.size(); ++i)
   {
     if(setLanes[i])
     {
       float dot=-1.0f*xValues[i]+0.0f*zValues[i];
        // hard coded reflect value
        // m_x-2.0f*d*_n.m_x, m_y-2.0f*d*_n.m_y, m_z-2.0f*d*_n.m_z
       xValues[i]=xValues[i]-2.0f*dot*-1.0f; //dir.m_x;
       zValues[i]=zValues[i]-2.0f*dot*0.0f;  //dir.m_z;
     }
   }
   #if defined(__x86_64__)
    m_dir[_i].set(_mm256_load_ps(xValues),zero,_mm256_load_ps(zValues));
    m_maxspeed[_i]=_mm256_load_ps(speed);
   #elif defined(__arm64__)
    m_dir[_i].set(_mm_load_ps(xValues),zero,_mm_load_ps(zValues));
    m_maxspeed[_i]=_mm_load_ps(speed);
   #endif
  }


  // // top plane
  // if(m_pos[i].m_z >= zsize)
  #if defined(__x86_64__)
    cmp=_mm256_cmp_ps(m_pos[_i].z(),plusZSize,_CMP_GE_OS);
    mask=_mm256_movemask_ps(cmp);
  #elif defined(__arm64__)
    cmp=_mm_cmpge_ps(m_pos[_i].z(),plusZSize);
    mask=_mm_movemask_ps(cmp);
  #endif

  if(mask !=0)
  {
  std::bitset<LaneSize> setLanes(mask);
   for(size_t i=0; i<setLanes.size(); ++i)
   {
     if(setLanes[i])
     {
      float dot=0.0f*xValues[i]+-1.0f*zValues[i];
        // hard coded reflect value
        // m_x-2.0f*d*_n.m_x, m_y-2.0f*d*_n.m_y, m_z-2.0f*d*_n.m_z
       xValues[i]=xValues[i]-2.0f*dot*0.0f; //dir.m_x;
       zValues[i]=zValues[i]-2.0f*dot*-1.0f;  //dir.m_z;
       speed[i]-=0.1f;

     }
   }
   #if defined(__x86_64__)
    m_dir[_i].set(_mm256_load_ps(xValues),zero,_mm256_load_ps(zValues));
    m_maxspeed[_i]=_mm256_load_ps(speed);
  #elif defined(__arm64__)
    m_dir[_i].set(_mm_load_ps(xValues),zero,_mm_load_ps(zValues));
    m_maxspeed[_i]=_mm_load_ps(speed);

  #endif

  }
  #if defined(__x86_64__)
    cmp=_mm256_cmp_ps(m_pos[_i].z(),minusZSize,_CMP_LE_OS);
    mask=_mm256_movemask_ps(cmp);
  #elif defined(__arm64__)
    cmp=_mm_cmple_ps(m_pos[_i].z(),minusZSize);
    mask=_mm_movemask_ps(cmp);
  #endif
  if(mask !=0)
  {
  std::bitset<LaneSize> setLanes(mask);
   for(size_t i=0; i<setLanes.size(); ++i)
   {
     if(setLanes[i])
     {
      float dot=0.0f*xValues[i]+-1.0f*zValues[i];
        // hard coded reflect value
        // m_x-2.0f*d*_n.m_x, m_y-2.0f*d*_n.m_y, m_z-2.0f*d*_n.m_z
       xValues[i]=xValues[i]-2.0f*dot*0.0f; //dir.m_x;
       zValues[i]=zValues[i]-2.0f*dot*-1.0f;  //dir.m_z;

       speed[i]-=0.1f;
     }
   }
   #if defined(__x86_64__)
    m_dir[_i].set(_mm256_load_ps(xValues),zero,_mm256_load_ps(zValues));
    m_maxspeed[_i]=_mm256_load_ps(speed);
  #elif defined(__arm64__)
    m_dir[_i].set(_mm_load_ps(xValues),zero,_mm_load_ps(zValues));
    m_maxspeed[_i]=_mm_load_ps(speed);
  #endif

  }
}


void Grid::updateParticle(size_t _i, float _dt) noexcept
{
  #if defined(__x86_64__)
    const __m256 zeros=_mm256_setzero_ps();
    auto dir=m_dir[_i]*Vec3x8(m_acceleration[_i],zeros,m_acceleration[_i])*_dt;
  #elif defined(__arm64__)
    const __m128 zeros=_mm_setzero_ps();
    auto dir=m_dir[_i]*Vec3x4(m_acceleration[_i],zeros,m_acceleration[_i])*_dt;

  #endif

  dir.clamp(m_maxspeed[_i]); 
  m_pos[_i] +=dir;
  // __m256 sub=_mm256_set1_ps(0.01f);
  // m_maxspeed[i]=_mm256_sub_ps(m_maxspeed[i],sub);
  checkForReset(_i);    
  checkForBounds(_i);
}

  void Grid::update(float _dt)
  {

  tbb::parallel_for( tbb::blocked_range<int>(0,m_numParticles),
                         [&](tbb::blocked_range<int> r)
                         {
                            for (int i=r.begin(); i<r.end(); ++i)
                            {
                               updateParticle(i,_dt);
                            }
                         });

    // for(size_t i=0; i<m_numParticles; ++i)
    //   updateParticle(i,_dt);
  }

  void Grid::resetParticle(size_t i)
  {
    auto randm256 = [](float range)
    {
      float v[LaneSize];
      for(int i=0; i<LaneSize; ++i)
      {
        v[i]=ngl::Random::randomNumber(range);
      }
      #if defined(__x86_64__)
        return _mm256_loadu_ps(&v[0]);
      #elif defined(__arm64__)
        return _mm_loadu_ps(&v[0]);
      #endif
    };

    auto randPositivem256 = [](float range)
    {
      float v[LaneSize];
      for(int i=0; i<LaneSize; ++i)
      {
        v[i]=ngl::Random::randomPositiveNumber(range);
      }
      #if defined(__x86_64__)
        return _mm256_loadu_ps(&v[0]);
      #elif defined(__arm64__)
        return _mm_loadu_ps(&v[0]);
      #endif    
      };

      auto x=randm256(m_width/2.0f);
      #if defined(__x86_64__)
        auto y=_mm256_setzero_ps();
      #elif defined(__arm64__)
        auto y=_mm_setzero_ps();
      #endif
      auto z=randm256(m_height/2.0f);
      m_pos[i].set(x,y,z);

      x=randm256(2.0f);
      z=randm256(2.0f);
      m_dir[i].set(x,y,z);

      m_maxspeed[i]=randPositivem256(5);  
      m_acceleration[i]=randPositivem256(5);  
  }

  void Grid::initGrid()
  {
    for(size_t i=0; i<m_numParticles; ++i)
    {
      resetParticle(i);
    }
  }



  void Grid::createTextureBuffer()
  {
    // now generate a buffer and copy this data to it
    // we will update this every frame with new values.

    glGenBuffers(2,&m_tbo[0]);

    glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[0]);
    glBufferData(GL_TEXTURE_BUFFER, m_numParticles*LaneSize*sizeof(ngl::Vec3), 0, GL_DYNAMIC_DRAW);
    // This buffer is now going to be associated with a texture
    // this will be read in the shader and the index will be from the vertexID
    glGenTextures(1, &m_posBufferID);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_BUFFER,m_posBufferID);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, m_tbo[0]);

    
    glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[1]);
    glBufferData(GL_TEXTURE_BUFFER, m_numParticles*LaneSize*sizeof(ngl::Vec3), 0, GL_DYNAMIC_DRAW);
    // This buffer is now going to be associated with a texture
    // this will be read in the shader and the index will be from the vertexID
    glGenTextures(1, &m_dirBufferID);
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture(GL_TEXTURE_BUFFER,m_dirBufferID);

    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, m_tbo[1]);

  }


  void Grid::updateTextureBuffer()
  {
    glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[0]);
    ngl::Vec3 *posBuffer=(ngl::Vec3 *)glMapBuffer(GL_TEXTURE_BUFFER,GL_READ_WRITE);
    
    size_t index=0;
    for(size_t i=0; i<m_numParticles*LaneSize; i+=LaneSize)
    {
      m_pos[index++].fillArray(posBuffer);
      posBuffer+=LaneSize;
    }
    glUnmapBuffer(GL_TEXTURE_BUFFER);

    glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[1]);

    ngl::Vec3 *dirBuffer=(ngl::Vec3 *)glMapBuffer(GL_TEXTURE_BUFFER,GL_READ_WRITE);

    index=0;
    for(size_t i=0; i<m_numParticles*LaneSize; i+=LaneSize)
    {
      m_dir[index++].fillArray(dirBuffer);
      dirBuffer+=LaneSize;
    }
    glUnmapBuffer(GL_TEXTURE_BUFFER);
  }  