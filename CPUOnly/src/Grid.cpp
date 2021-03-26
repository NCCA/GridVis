#include "Grid.h"
#include <ngl/Random.h>
#include <ngl/Util.h>
#include <ngl/NGLStream.h>
#include <ngl/VAOFactory.h>
#include <ngl/SimpleVAO.h>

Grid::Grid(uint32_t _w, uint32_t _h,size_t _numParticles) 
{
    m_width=_w;
    m_height=_h;
    m_particles.resize(_numParticles);
    initGrid();
    m_vao=ngl::VAOFactory::createVAO(ngl::simpleVAO,GL_POINTS);
  }

void Grid::draw() const
{
  std::vector<ngl::Vec3> parts;
  for(auto p : m_particles)
  {
    parts.push_back(p.pos);
  }
  // create a vao as a series of GL_TRIANGLES
  m_vao->bind();

  // in this case we are going to set our data as the vertices above
  m_vao->setData(ngl::SimpleVAO::VertexData(parts.size()*sizeof(ngl::Vec3),parts[0].m_x));
  // now we set the attribute pointer to be 0 (as this matches vertIn in our shader)

  m_vao->setVertexAttributePointer(0,3,GL_FLOAT,0,0);
  m_vao->setNumIndices(parts.size());

 // now unbind
  m_vao->draw();
  m_vao->unbind();

}
void Grid::update(float _dt)
{
  for(auto &p : m_particles)
  {
    p.pos +=p.dir*_dt;
    float xsize=m_width/2.0f;
    float zsize=m_height/2.0f;
    // Now check against the bounds of the grid and reflect if needed this is quite brute force but works
    // left plane
    if(p.pos.m_x <= -xsize)
    {
      p.dir= p.dir.reflect({1.0f,0.0f,0.0f});
    }
    // right plane
    else if(p.pos.m_x >= xsize)
    {
      p.dir= p.dir.reflect({-1.0f,0.0f,0.0f});
    }
    // top plane
    if(p.pos.m_z >= zsize)
    {
      p.dir= p.dir.reflect({0.0f,0.0f,1.0f});

    }
    // bottom plane
    else if(p.pos.m_z <= -zsize)
    {
      p.dir= p.dir.reflect({0.0f,0.0f,-1.0f});

    }
  }
}

void Grid::initGrid()
{
  // we assume that 0,0,0 is the origin and the w/h are half extents
  // we are going to view from Y looking down so only set the .x and z values 
  for(auto &p : m_particles)
  {
    p.pos.m_x=ngl::Random::randomNumber(m_width/2.0f);
    p.pos.m_z=ngl::Random::randomNumber(m_height/2.0f);
    p.pos.m_y=0.0f; // just in case!
    p.dir = ngl::Random::getRandomNormalizedVec3();  
    p.dir.m_y=0.0f; // this needs to be done as reflect is 3d
  }
}
