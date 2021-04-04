#include "Grid.h"
#include <ngl/Random.h>
#include <ngl/Util.h>
#include <ngl/NGLStream.h>
#include <ngl/VAOFactory.h>
#include <ngl/MultiBufferVAO.h>
#include <ngl/SimpleVAO.h>
#include <tbb/parallel_for.h>


Grid::Grid(uint32_t _w, uint32_t _h,size_t _numParticles) noexcept
{
    m_width=_w;
    m_height=_h;
    m_numParticles=_numParticles;
    m_pos.resize(m_numParticles);
    m_dir.resize(m_numParticles);
    m_acceleration.resize(m_numParticles);
    m_maxspeed.resize(m_numParticles);
    initGrid();
    createTextureBuffer();
    // Going to use a non NGL buffer see if it is quicker
    glGenVertexArrays(1, &m_svao);
}


void Grid::draw() const noexcept
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
    auto dir=m_dir[i]*ngl::Vec3(m_acceleration[i],0.0f,m_acceleration[i])*_dt;
    m_dir[i].clamp(m_maxspeed[i]);
    m_pos[i] +=dir;
    if(m_maxspeed[i] <=0.0f)
    {
      resetParticle(i);
    }
    
    float xsize=m_width/2.0f;
    float zsize=m_height/2.0f;
    // Now check against the bounds of the grid and reflect if needed this is quite brute force but works
    // left plane
    if(m_pos[i].m_x <= -xsize)
    {
      m_dir[i]= m_dir[i].reflect({1.0f,0.0f,0.0f});
      m_maxspeed[i]-=0.1f;
    }
    // right plane
    else if(m_pos[i].m_x >= xsize)
    {
      m_dir[i]= m_dir[i].reflect({-1.0f,0.0f,0.0f});
      m_maxspeed[i]-=0.1f;
    }
    // top plane
    if(m_pos[i].m_z >= zsize)
    {
      m_dir[i]= m_dir[i].reflect({0.0f,0.0f,1.0f});
      m_maxspeed[i]-=0.1f;
    }
    // bottom plane
    else if(m_pos[i].m_z <= -zsize)
    {
      m_dir[i]= m_dir[i].reflect({0.0f,0.0f,-1.0f});
      m_maxspeed[i]-=0.1f;
    }

}

void Grid::update(float _dt) noexcept
{

tbb::parallel_for( tbb::blocked_range<int>(0,m_numParticles),
                       [&](tbb::blocked_range<int> r)
                       {
                          for (int i=r.begin(); i<r.end(); ++i)
                          {
                             updateParticle(i,_dt);
                          }
                       });
  // finally load into the texture buffer
  updateTextureBuffer();
}

void Grid::resetParticle(size_t i) noexcept
{
    m_pos[i].m_x=ngl::Random::randomNumber(m_width/2.0f);
    m_pos[i].m_z=ngl::Random::randomNumber(m_height/2.0f);
    m_pos[i].m_y=0.0f; // just in case!
    m_dir[i] = ngl::Random::getRandomVec3()*2.0f;  
    m_dir[i].m_y=0.0f; // this needs to be done as reflect is 3d
    m_maxspeed[i]=ngl::Random::randomPositiveNumber(5)+0.1f;  
    m_acceleration[i]=ngl::Random::randomPositiveNumber(5)+0.1f;
}

void Grid::initGrid() noexcept
{
  for(size_t i=0; i<m_numParticles; ++i)
  {
    resetParticle(i);
  }
}



void Grid::createTextureBuffer() noexcept
{
  // now generate a buffer and copy this data to it
  // we will update this every frame with new values.

  glGenBuffers(2,&m_tbo[0]);

  glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[0]);
  glBufferData(GL_TEXTURE_BUFFER, m_pos.size()*sizeof(ngl::Vec3), 0, GL_DYNAMIC_DRAW);
  // This buffer is now going to be associated with a texture
  // this will be read in the shader and the index will be from the vertexID
  glGenTextures(1, &m_posBufferID);
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture(GL_TEXTURE_BUFFER,m_posBufferID);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, m_tbo[0]);

  
  glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[1]);
  glBufferData(GL_TEXTURE_BUFFER, m_dir.size()*sizeof(ngl::Vec3), 0, GL_DYNAMIC_DRAW);
  // This buffer is now going to be associated with a texture
  // this will be read in the shader and the index will be from the vertexID
  glGenTextures(1, &m_dirBufferID);
  glActiveTexture( GL_TEXTURE1 );
  glBindTexture(GL_TEXTURE_BUFFER,m_dirBufferID);

  glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, m_tbo[1]);

}


void Grid::updateTextureBuffer() noexcept
{
  // update this buffer by copying the data to it
  glActiveTexture( GL_TEXTURE0 );
  glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[0]);
  glBufferData(GL_TEXTURE_BUFFER, m_pos.size()*sizeof(ngl::Vec3), &m_pos[0].m_x, GL_DYNAMIC_DRAW);
  // update this buffer by copying the data to it
  glActiveTexture( GL_TEXTURE1 );
  glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[1]);
  glBufferData(GL_TEXTURE_BUFFER, m_dir.size()*sizeof(ngl::Vec3), &m_dir[0].m_x, GL_DYNAMIC_DRAW);

}  