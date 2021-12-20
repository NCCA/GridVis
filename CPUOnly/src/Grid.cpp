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
    m_particles.resize(_numParticles);
    initGrid();
    m_vao=ngl::vaoFactoryCast<ngl::MultiBufferVAO>(   ngl::VAOFactory::createVAO(ngl::multiBufferVAO,GL_POINTS));
    m_vao->bind();
    m_vao->setData(ngl::MultiBufferVAO::VertexData(0,0)); // index 0 points
    m_vao->setData(ngl::MultiBufferVAO::VertexData(0,0)); // index 1 dirs
    m_vao->unbind();
    
  }

void Grid::draw() const
{
  std::vector<ngl::Vec3> parts;
  std::vector<ngl::Vec3> dirs;
  
  for(auto p : m_particles)
  {
    parts.push_back(p.pos);
    dirs.push_back(p.dir);
  }
  m_vao->bind();

  // in this case we are going to set our data as the vertices above
  m_vao->setData(0,ngl::MultiBufferVAO::VertexData(parts.size()*sizeof(ngl::Vec3),parts[0].m_x));
  // now we set the attribute pointer to be 0 (as this matches vertIn in our shader)
  m_vao->setVertexAttributePointer(0,3,GL_FLOAT,0,0);
  m_vao->setData(1,ngl::MultiBufferVAO::VertexData(dirs.size()*sizeof(ngl::Vec3),dirs[0].m_x));
  m_vao->setVertexAttributePointer(1,3,GL_FLOAT,0,0);
  m_vao->setNumIndices(parts.size());

 // now unbind
  m_vao->draw();
  m_vao->unbind();

}
void Grid::update(float _dt)
{
  for(auto &p : m_particles)
  {
    auto dir=p.dir*ngl::Vec3(p.acceleration,0.0f,p.acceleration)*_dt;
    dir.clamp(p.maxspeed);
    p.pos +=dir;
    if(p.maxspeed <=0.0f)
    {
      resetParticle(p);
    }
    
    float xsize=m_width/2.0f;
    float zsize=m_height/2.0f;
    // Now check against the bounds of the grid and reflect if needed this is quite brute force but works
    // left plane
    if(p.pos.m_x <= -xsize)
    {
      p.dir= p.dir.reflect({1.0f,0.0f,0.0f});
      p.maxspeed-=0.1f;
    }
    // right plane
    else if(p.pos.m_x >= xsize)
    {
      p.dir= p.dir.reflect({-1.0f,0.0f,0.0f});
      p.maxspeed-=0.1f;
    }
    // top plane
    if(p.pos.m_z >= zsize)
    {
      p.dir= p.dir.reflect({0.0f,0.0f,1.0f});
      p.maxspeed-=0.1f;
    }
    // bottom plane
    else if(p.pos.m_z <= -zsize)
    {
      p.dir= p.dir.reflect({0.0f,0.0f,-1.0f});
      p.maxspeed-=0.1f;
    }
  }
}

void Grid::resetParticle(Particle &io_p)
{
    io_p.pos.m_x=ngl::Random::randomNumber(m_width/2.0f);
    io_p.pos.m_z=ngl::Random::randomNumber(m_height/2.0f);
    io_p.pos.m_y=0.0f; // just in case!
    io_p.dir = ngl::Random::getRandomVec3()*5.0f;  
    io_p.dir.m_y=0.0f; // this needs to be done as reflect is 3d
    io_p.maxspeed=ngl::Random::randomPositiveNumber(5)+0.1f;  
    io_p.acceleration=ngl::Random::randomPositiveNumber(5)+0.1f;
}

void Grid::initGrid()
{
  // we assume that 0,0,0 is the origin and the w/h are half extents
  // we are going to view from Y looking down so only set the .x and z values 
  for(auto &p : m_particles)
  {
    resetParticle(p);
  }
}
