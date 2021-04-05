#include "Grid.h"
#include <ngl/Random.h>
#include <ngl/Util.h>
#include <ngl/NGLStream.h>
#include <ngl/VAOFactory.h>
#include <ngl/MultiBufferVAO.h>
#include <ngl/SimpleVAO.h>
#include <tbb/parallel_for.h>


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


void Grid::updateParticle(size_t i, float _dt) noexcept
{
    const __m256 zeros=_mm256_setzero_ps();
    auto dir=m_dir[i]*Vec3x8(m_acceleration[i],zeros,m_acceleration[i])*_dt;
    m_dir[i].clamp(0,2.0f); //zeros,m_maxspeed[i]);
    m_pos[i] +=dir;
    // if(m_maxspeed[i] <=0.0f)
    // {
    //   resetParticle(i);
    // }
    
    float xsize=m_width/2.0f;
    float zsize=m_height/2.0f;
    // // Now check against the bounds of the grid and reflect if needed this is quite brute force but works
    // // left plane
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

    //m_pos[i].m_x=ngl::Random::randomNumber(m_width/2.0f);
    //m_pos[i].m_z=ngl::Random::randomNumber(m_height/2.0f);
    //m_pos[i].m_y=0.0f; // just in case!
    auto x=randm256(m_width/2.0f);
    auto y=_mm256_setzero_ps();
    auto z=randm256(m_width/2.0f);
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

  std::vector<ngl::Vec3> pos(m_numParticles*8);
  std::vector<ngl::Vec3> dir(m_numParticles*8);
  size_t index=0;
  for(size_t i=0; i<m_numParticles*8; i+=8)
  {
  
    m_pos[index].fillArray(&pos[i]);
    m_dir[index++].fillArray(&dir[i]);

  }


  // update this buffer by copying the data to it
  glActiveTexture( GL_TEXTURE0 );
  glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[0]);
  glBufferData(GL_TEXTURE_BUFFER, pos.size()*sizeof(ngl::Vec3), &pos[0].m_x, GL_DYNAMIC_DRAW);
  // update this buffer by copying the data to it
  glActiveTexture( GL_TEXTURE1 );
  glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[1]);
  glBufferData(GL_TEXTURE_BUFFER, dir.size()*sizeof(ngl::Vec3), &dir[0].m_x, GL_DYNAMIC_DRAW);

}  