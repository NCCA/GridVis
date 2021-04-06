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

  Grid::Grid(uint32_t _w, uint32_t _h,size_t _numParticles) 
  {
      m_width=_w;
      m_height=_h;
      m_numParticles=roundTo(_numParticles,8)/8;
      m_pos.resize(m_numParticles);
      m_dir.resize(m_numParticles);
      m_acceleration.resize(m_numParticles);
      m_maxspeed.resize(m_numParticles);
      initGrid();
      createTextureBuffer();
      // Going to use a non NGL buffer see if it is quicker
      glGenVertexArrays(1, &m_svao);
      std::cout<<"size of Vec3x8 "<<sizeof(Vec3x8)<<" size of buffer "<<sizeof(Vec3x8)*m_numParticles<<'\n';
      m_posBuffer.resize(m_numParticles*8);
      m_dirBuffer.resize(m_numParticles*8);

  }


  void Grid::draw() const
  {
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_BUFFER,m_posBufferID);
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture(GL_TEXTURE_BUFFER,m_dirBufferID);
    glBindVertexArray(m_svao);
    glDrawArrays(GL_POINTS,0,m_numParticles);      
  }

  void Grid::checkForReset(size_t _i) noexcept
  {
    const __m256 zero = _mm256_setzero_ps();
    auto cmp=_mm256_cmp_ps(m_maxspeed[_i],zero,_CMP_LE_OS);
    auto mask=_mm256_movemask_ps(cmp);
    if(mask ==0)
      return ; // not to update so return

    else if(mask == 0xFF)
    {
      resetParticle(_i);
    }
    else
    {
      // we need to go to non SIMD mode here
      // first we get the particle properties
      auto xValues=m_pos[_i].x();
      auto zValues=m_pos[_i].z();
      auto xDirValues=m_dir[_i].x();
      auto zDirValues=m_dir[_i].z();
      float x[8];
      float z[8];
      float dirx[8];
      float dirz[8];
      _mm256_store_ps(static_cast<float *>(x), xValues);
      _mm256_store_ps(static_cast<float *>(z), zValues);
      _mm256_store_ps(static_cast<float *>(dirx), xDirValues);
      _mm256_store_ps(static_cast<float *>(dirz), zDirValues);
      float speed[8];
      float acceleration[8];
      _mm256_store_ps(static_cast<float *>(speed), m_maxspeed[_i]);
      _mm256_store_ps(static_cast<float *>(acceleration), m_acceleration[_i]);

      std::bitset<8> setLanes(mask);
      for(size_t i=0; i<setLanes.size(); ++i)
      {
        if(setLanes[i])
        {
          x[i]=ngl::Random::randomNumber(m_width/2.0f);
          z[i]=ngl::Random::randomNumber(m_height/2.0f);
          dirx[i]=ngl::Random::randomPositiveNumber(2.0)+0.1f;
          dirz[i]=ngl::Random::randomPositiveNumber(2.0f)+0.1f;
          speed[i]=ngl::Random::randomPositiveNumber(5.0f)+0.1f;
          acceleration[i]=ngl::Random::randomPositiveNumber(5.0f)+0.1f;
        }
      }
    // now reset with modified values
    auto newX=_mm256_load_ps(x);
    auto newZ=_mm256_load_ps(z);
    m_pos[_i].set(newX,zero,newZ);
    newX=_mm256_load_ps(dirx);
    newZ=_mm256_load_ps(dirz);
    m_dir[_i].set(newX,zero,newZ);
    m_maxspeed[_i]=_mm256_load_ps(speed);
    m_acceleration[_i]=_mm256_load_ps(acceleration);
  }
}

void Grid::checkForBounds(size_t _i) noexcept
{
  float xsize=m_width/2.0f;
  float zsize=m_height/2.0f;

  const __m256 minusXSize=_mm256_set1_ps(-xsize);
  const __m256 plusXSize=_mm256_set1_ps(xsize);
  const __m256 minusZSize=_mm256_set1_ps(-zsize);
  const __m256 plusZSize=_mm256_set1_ps(zsize);

  auto cmp=_mm256_cmp_ps(m_pos[_i].x(),minusXSize,_CMP_LE_OS);
  auto mask=_mm256_movemask_ps(cmp);
  if(mask !=0)
  {
    std::cout<<"Need -x reflect \n";
  }

  cmp=_mm256_cmp_ps(m_pos[_i].x(),plusXSize,_CMP_GE_OS);
  mask=_mm256_movemask_ps(cmp);
  if(mask !=0)
  {
    std::cout<<"Need x reflect\n";
  }


  // // top plane
  // if(m_pos[i].m_z >= zsize)
  cmp=_mm256_cmp_ps(m_pos[_i].z(),plusZSize,_CMP_GE_OS);
  mask=_mm256_movemask_ps(cmp);
  if(mask !=0)
  {
    std::cout<<"Need Z reflect\n";
  }

  cmp=_mm256_cmp_ps(m_pos[_i].z(),minusZSize,_CMP_LE_OS);
  mask=_mm256_movemask_ps(cmp);
  if(mask !=0)
  {
    std::cout<<"Need -Z reflect\n";
  }




  // Now check against the bounds of the grid and reflect if needed this is quite brute force but works
  // left plane
  
      // if(m_pos[i].m_x <= -xsize)
      // {
      //   m_dir[i]= m_dir[i].reflect({1.0f,0.0f,0.0f});
      //   m_maxspeed[i]-=0.1f;
      // }
      // // right plane
      // else if(m_pos[i].m_x >= xsize)
      // {
      //   m_dir[i]= m_dir[i].reflect({-1.0f,0.0f,0.0f});
      //   m_maxspeed[i]-=0.1f;
      // }
      // // top plane
      // if(m_pos[i].m_z >= zsize)
      // {
      //   m_dir[i]= m_dir[i].reflect({0.0f,0.0f,1.0f});
      //   m_maxspeed[i]-=0.1f;
      // }
      // // bottom plane
      // else if(m_pos[i].m_z <= -zsize)
      // {
      //   m_dir[i]= m_dir[i].reflect({0.0f,0.0f,-1.0f});
      //   m_maxspeed[i]-=0.1f;
      // }

}



  void Grid::updateParticle(size_t i, float _dt) noexcept
  {
      const __m256 zeros=_mm256_setzero_ps();
      auto dir=m_dir[i]*Vec3x8(m_acceleration[i],zeros,m_acceleration[i])*_dt;
      m_dir[i].clamp(m_maxspeed[i]); 
      m_pos[i] +=dir;
      __m256 sub=_mm256_set1_ps(0.01f);
      m_maxspeed[i]=_mm256_sub_ps(m_maxspeed[i],sub);
      checkForReset(i);    
      checkForBounds(i);

  
  }

  void Grid::update(float _dt)
  {

  // tbb::parallel_for( tbb::blocked_range<int>(0,m_numParticles),
  //                        [&](tbb::blocked_range<int> r)
  //                        {
  //                           for (int i=r.begin(); i<r.end(); ++i)
  //                           {
  //                              updateParticle(i,_dt);
  //                           }
  //                        });

    for(size_t i=0; i<m_numParticles; ++i)
      updateParticle(i,_dt);

    updateTextureBuffer();
  }

  void Grid::resetParticle(size_t i)
  {
    auto randm256 = [](float range)
    {
      float v[8];
      for(int i=0; i<8; ++i)
      {
        v[i]=ngl::Random::randomNumber(range);
      }
      return _mm256_loadu_ps(&v[0]);
    };

    auto randPositivem256 = [](float range)
    {
      float v[8];
      for(int i=0; i<8; ++i)
      {
        v[i]=ngl::Random::randomPositiveNumber(range);
      }
      return _mm256_loadu_ps(&v[0]);
    };

      auto x=randm256(m_width/2.0f);
      auto y=_mm256_setzero_ps();
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
    glBufferData(GL_TEXTURE_BUFFER, m_pos.size()*sizeof(Vec3x8), 0, GL_DYNAMIC_DRAW);
    // This buffer is now going to be associated with a texture
    // this will be read in the shader and the index will be from the vertexID
    glGenTextures(1, &m_posBufferID);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_BUFFER,m_posBufferID);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, m_tbo[0]);

    
    glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[1]);
    glBufferData(GL_TEXTURE_BUFFER, m_dir.size()*sizeof(Vec3x8), 0, GL_DYNAMIC_DRAW);
    // This buffer is now going to be associated with a texture
    // this will be read in the shader and the index will be from the vertexID
    glGenTextures(1, &m_dirBufferID);
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture(GL_TEXTURE_BUFFER,m_dirBufferID);

    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, m_tbo[1]);

  }


  void Grid::updateTextureBuffer()
  {

    size_t index=0;
    for(size_t i=0; i<m_numParticles*8; i+=8)
    {
      m_pos[index].fillArray(&m_posBuffer[i]);
      m_dir[index++].fillArray(&m_dirBuffer[i]);
    }


    // update this buffer by copying the data to it
    glActiveTexture( GL_TEXTURE0 );
    glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[0]);
    glBufferData(GL_TEXTURE_BUFFER, m_posBuffer.size()*sizeof(ngl::Vec3), &m_posBuffer[0].m_x, GL_DYNAMIC_DRAW);
    // update this buffer by copying the data to it
    glActiveTexture( GL_TEXTURE1 );
    glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[1]);
    glBufferData(GL_TEXTURE_BUFFER, m_dirBuffer.size()*sizeof(ngl::Vec3), &m_dirBuffer[0].m_x, GL_DYNAMIC_DRAW);

  }  