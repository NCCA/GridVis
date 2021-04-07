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
      m_numParticles=roundTo((unsigned int)_numParticles,8)/8;
      //m_posX=std::make_unique<AlignedArray<f256,32>>(m_numParticles);
      m_posX.reset(m_numParticles);
      m_posZ.reset(m_numParticles);
      m_dirX.reset(m_numParticles);
      m_dirZ.reset(m_numParticles);
      m_acceleration.reset(m_numParticles);
      m_maxspeed.reset(m_numParticles);
      initGrid();
      createTextureBuffer();
      // Going to use a non NGL buffer see if it is quicker
      glGenVertexArrays(1, &m_svao);

  }


  void Grid::draw() const
  {
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_BUFFER,m_posXBufferID);
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture(GL_TEXTURE_BUFFER,m_posZBufferID);

    glActiveTexture( GL_TEXTURE2 );
    glBindTexture(GL_TEXTURE_BUFFER,m_dirXBufferID);
    glActiveTexture( GL_TEXTURE3 );
    glBindTexture(GL_TEXTURE_BUFFER,m_dirZBufferID);
    glBindVertexArray(m_svao);
    glDrawArrays(GL_POINTS,0,m_numParticles*8);      
  }

  void Grid::checkForReset(size_t _i) noexcept
  {
    const f256 zero = zero8f();
    auto cmp=cmplteq8f(m_maxspeed[_i],zero);
    auto mask=movemask8f(cmp);
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
      float x[8];
      float z[8];
      float dirx[8];
      float dirz[8];
      float speed[8];
      float acceleration[8];
      store8f(x,m_posX[_i]);
      store8f(z,m_posZ[_i]);
      store8f(dirx,m_dirX[_i]);
      store8f(dirz,m_dirZ[_i]);
      store8f(speed,m_maxspeed[_i]);
      store8f(acceleration,m_acceleration[_i]);

      std::bitset<8> setLanes(mask);
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
    m_posX[_i]=load8f(x);
    m_posZ[_i]=load8f(z);
    m_dirX[_i]=load8f(dirx);
    m_dirZ[_i]=load8f(dirz);
    m_maxspeed[_i]=load8f(speed);
    m_acceleration[_i]=load8f(acceleration);
  }
}

void Grid::checkForBounds(size_t _i) noexcept
{

  float xsize=m_width/2.0f;
  float zsize=m_height/2.0f;
  const f256 zero = zero8f();

  const f256 minusXSize=splat8f(-xsize);
  const f256 plusXSize=splat8f(xsize);
  const f256 minusZSize=splat8f(-zsize);
  const f256 plusZSize=splat8f(zsize);

  float xValues[8];
  float zValues[8];
  float speed[8];
  // grab the values from dir and speed
  store8f(xValues, m_dirX[_i]);
  store8f(zValues, m_dirZ[_i]);
  store8f(speed, m_maxspeed[_i]);
  
  auto cmp=cmplteq8f(m_posX[_i],minusXSize);
  auto mask=movemask8f(cmp);
  if(mask !=0)
  {
  std::bitset<8> setLanes(mask);
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
   m_dirX[_i]=load8f(xValues);
   m_dirZ[_i]=load8f(zValues);
   m_maxspeed[_i]=load8f(speed);
  }

  cmp=cmpgteq8f(m_posX[_i],plusXSize);
  mask=movemask8f(cmp);
  if(mask !=0)
  {
   std::bitset<8> setLanes(mask);
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
   m_dirX[_i]=load8f(xValues);
   m_dirZ[_i]=load8f(zValues);
   m_maxspeed[_i]=load8f(speed);
  }
  

  // // top plane
  // if(m_pos[i].m_z >= zsize)
  cmp=cmpgteq8f(m_posZ[_i],plusZSize);
  mask=movemask8f(cmp);
  if(mask !=0)
  {
  std::bitset<8> setLanes(mask);
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
   m_dirX[_i]=load8f(xValues);
   m_dirZ[_i]=load8f(zValues);
   m_maxspeed[_i]=load8f(speed);
  }

  cmp=cmplteq8f(m_posZ[_i],minusZSize);
  mask=movemask8f(cmp);
  if(mask !=0)
  {
  std::bitset<8> setLanes(mask);
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
   m_dirX[_i]=load8f(xValues);
   m_dirZ[_i]=load8f(zValues);
   m_maxspeed[_i]=load8f(speed);
  }
}



  void Grid::updateParticle(size_t _i, float _dt) noexcept
  {
      const f256 zeros=zero8f();
      f256 dt=splat8f(_dt);
      // create xdir as accel * _dt
      f256 dirX=mul8f(m_dirX[_i],mul8f(m_acceleration[_i],dt));
      f256 dirZ=mul8f(m_dirZ[_i],mul8f(m_acceleration[_i],dt));
      auto clamp=[](f256 _v, f256 _range)
      {
        f256 c;
        auto min = sub8f(splat8f(0.0f), _range);
        c=max8f(_v,min);
        c=min8f(_v,_range);
        return c;
      };
      dirX=clamp(dirX,m_maxspeed[_i]);
      dirZ=clamp(dirZ,m_maxspeed[_i]);
      m_posX[_i]=add8f(m_posX[_i],dirX); 
      m_posZ[_i]=add8f(m_posZ[_i],dirZ); 

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

  }

  void Grid::resetParticle(size_t _i)
  {

    auto randm256 = [](float range)
    {
      float v[8];
      for(int i=0; i<8; ++i)
      {
        v[i]=ngl::Random::randomNumber(range);
      }
      return loadu8f(&v[0]);
    };

    auto randPositivem256 = [](float range)
    {
      float v[8];
      for(int i=0; i<8; ++i)
      {
        v[i]=ngl::Random::randomPositiveNumber(range);
      }
      return loadu8f(&v[0]);
    };
    
    m_posX[_i]=randm256(m_width/2.0f);
    m_posZ[_i]=randm256(m_height/2.0f);
    m_dirX[_i]=randm256(2.0f);
    m_dirZ[_i]=randm256(2.0f);

    m_maxspeed[_i]=randPositivem256(5);  
    m_acceleration[_i]=randPositivem256(5);  
  
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
    // going to have 4 buffers one for each of the params x,z,dirX,dirZ
    glGenBuffers(4,&m_tbo[0]);

    glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[0]);
    glBufferData(GL_TEXTURE_BUFFER, m_numParticles*sizeof(f256), 0, GL_DYNAMIC_DRAW);
    // This buffer is now going to be associated with a texture
    // this will be read in the shader and the index will be from the vertexID
    glGenTextures(1, &m_posXBufferID);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_BUFFER,m_posXBufferID);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, m_tbo[0]);


    glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[1]);
    glBufferData(GL_TEXTURE_BUFFER, m_numParticles*sizeof(f256), 0, GL_DYNAMIC_DRAW);
    // This buffer is now going to be associated with a texture
    // this will be read in the shader and the index will be from the vertexID
    glGenTextures(1, &m_posZBufferID);
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture(GL_TEXTURE_BUFFER,m_posZBufferID);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, m_tbo[1]);


    glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[2]);
    glBufferData(GL_TEXTURE_BUFFER, m_numParticles*sizeof(f256), 0, GL_DYNAMIC_DRAW);
    // This buffer is now going to be associated with a texture
    // this will be read in the shader and the index will be from the vertexID
    glGenTextures(1, &m_dirXBufferID);
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture(GL_TEXTURE_BUFFER,m_dirXBufferID);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, m_tbo[2]);

    glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[3]);
    glBufferData(GL_TEXTURE_BUFFER, m_numParticles*sizeof(f256), 0, GL_DYNAMIC_DRAW);
    // This buffer is now going to be associated with a texture
    // this will be read in the shader and the index will be from the vertexID
    glGenTextures(1, &m_dirZBufferID);
    glActiveTexture( GL_TEXTURE3 );
    glBindTexture(GL_TEXTURE_BUFFER,m_dirZBufferID);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, m_tbo[3]);


  }


  void Grid::updateTextureBuffer()
  {
    // update this buffer by copying the data to it
    glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[0]);
    glBufferData(GL_TEXTURE_BUFFER, m_numParticles*sizeof(f256), &m_posX[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[1]);
    glBufferData(GL_TEXTURE_BUFFER, m_numParticles*sizeof(f256), &m_posZ[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[2]);
    glBufferData(GL_TEXTURE_BUFFER, m_numParticles*sizeof(f256), &m_dirX[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[3]);
    glBufferData(GL_TEXTURE_BUFFER, m_numParticles*sizeof(f256), &m_dirZ[0], GL_DYNAMIC_DRAW);


}  