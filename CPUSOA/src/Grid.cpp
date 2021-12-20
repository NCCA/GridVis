#include "Grid.h"
#include <ngl/Random.h>
#include <ngl/Util.h>
#include <ngl/NGLStream.h>
#include <ngl/VAOFactory.h>
#include <ngl/MultiBufferVAO.h>
#include <ngl/SimpleVAO.h>


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
    m_vao=ngl::vaoFactoryCast<ngl::MultiBufferVAO>(   ngl::VAOFactory::createVAO(ngl::multiBufferVAO,GL_POINTS));
    m_vao->bind();
    m_vao->setData(ngl::MultiBufferVAO::VertexData(m_pos.size()*sizeof(ngl::Vec3),m_pos[0].m_x));
    m_vao->setVertexAttributePointer(0,3,GL_FLOAT,0,0);
    m_vao->setData(ngl::MultiBufferVAO::VertexData(m_dir.size()*sizeof(ngl::Vec3),m_dir[0].m_x));
    m_vao->setVertexAttributePointer(1,3,GL_FLOAT,0,0);
    m_vao->setNumIndices(m_numParticles);
    m_vao->unbind();



    // Going to use a non NGL buffer see if it is quicker
    
    glGenVertexArrays(1, &m_svao);
    glBindVertexArray(m_svao);
    glGenBuffers(1, &m_vboID);
    // now bind this to the VBO buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
    // allocate the buffer data we need two lots of vec3 one for pos one for dir use dynamic as we update
    // per frame and it may be quicker
    glBufferData(GL_ARRAY_BUFFER, (m_numParticles*2)*sizeof(ngl::Vec3), 0, GL_DYNAMIC_DRAW);
    // As we are using glBufferSubData later we can set these now and it will be the same.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    // The dir vec3 is going to be put at the end of the pos block
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<float *>(m_numParticles*sizeof(ngl::Vec3)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Grid::toggleDrawMode(DrawMode _mode)
{
  m_drawMode=_mode;
}

void Grid::draw() const
{
  if (m_drawMode == DrawMode::MULTIBUFFER)
  {
    m_vao->bind();
    // going to get a pointer to the data and update using memcpy
    auto ptr=m_vao->mapBuffer(0,GL_READ_WRITE);
    memcpy(ptr,&m_pos[0].m_x,m_pos.size()*sizeof(ngl::Vec3));
    m_vao->unmapBuffer();

    ptr=m_vao->mapBuffer(1,GL_READ_WRITE);
    memcpy(ptr,&m_dir[0].m_x,m_dir.size()*sizeof(ngl::Vec3));
    m_vao->unmapBuffer();

  // now unbind
    m_vao->draw();
    m_vao->unbind();
  }
  else
  {
      glBindVertexArray(m_svao);
      // bind the buffer to copy the data
      glBindBuffer(GL_ARRAY_BUFFER,m_vboID);
      // copy the pos data
      glBufferSubData(GL_ARRAY_BUFFER,0,m_numParticles*sizeof(ngl::Vec3),&m_pos[0].m_x);
      // concatinate the dir data
      glBufferSubData(GL_ARRAY_BUFFER,m_numParticles*sizeof(ngl::Vec3),m_numParticles*sizeof(ngl::Vec3),&m_dir[0].m_x);      
      // draw
      glDrawArrays(GL_POINTS,0,m_numParticles);      
      glBindVertexArray(0);
  }
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
