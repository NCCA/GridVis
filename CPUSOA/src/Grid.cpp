#include "Grid.h"
#include <ngl/Random.h>
#include <ngl/Util.h>
#include <ngl/NGLStream.h>
#include <ngl/VAOFactory.h>
#include <ngl/MultiBufferVAO.h>

Grid::Grid(uint32_t _w, uint32_t _h,size_t _numParticles) 
{
    m_width=_w;
    m_height=_h;
    m_numParticles=_numParticles;
    m_pos.resize(m_numParticles);
    m_dir.resize(m_numParticles);
    m_acceleration.resize(m_numParticles);
    m_maxspeed.resize(m_numParticles);
    initGrid();
    m_vao=ngl::VAOFactory::createVAO(ngl::multiBufferVAO,GL_POINTS);
  }

void Grid::draw() const
{
  
  m_vao->bind();

  // in this case we are going to set our data as the vertices above
  m_vao->setData(ngl::MultiBufferVAO::VertexData(m_pos.size()*sizeof(ngl::Vec3),m_pos[0].m_x));
  // now we set the attribute pointer to be 0 (as this matches vertIn in our shader)
  m_vao->setVertexAttributePointer(0,3,GL_FLOAT,0,0);
  m_vao->setData(ngl::MultiBufferVAO::VertexData(m_dir.size()*sizeof(ngl::Vec3),m_dir[0].m_x));
  m_vao->setVertexAttributePointer(1,3,GL_FLOAT,0,0);
  m_vao->setNumIndices(m_numParticles);

 // now unbind
  m_vao->draw();
  m_vao->unbind();

}
void Grid::update(float _dt)
{
  for(size_t i=0; i<m_numParticles; ++i)
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
}

void Grid::resetParticle(size_t i)
{
    m_pos[i].m_x=ngl::Random::randomNumber(m_width/2.0f);
    m_pos[i].m_z=ngl::Random::randomNumber(m_height/2.0f);
    m_pos[i].m_y=0.0f; // just in case!
    m_dir[i] = ngl::Random::getRandomVec3()*2.0f;  
    m_dir[i].m_y=0.0f; // this needs to be done as reflect is 3d
    m_maxspeed[i]=ngl::Random::randomPositiveNumber(5)+0.1f;  
    m_acceleration[i]=ngl::Random::randomPositiveNumber(5)+0.1f;
}

void Grid::initGrid()
{
  for(size_t i=0; i<m_numParticles; ++i)
  {
    resetParticle(i);
  }
}
