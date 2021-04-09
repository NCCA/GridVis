#include "Grid.h"
#include <ngl/Random.h>
#include <ngl/Util.h>
#include <ngl/NGLStream.h>
#include <ngl/VAOFactory.h>
#include <ngl/MultiBufferVAO.h>
#include <ngl/SimpleVAO.h>
#include <ngl/ShaderLib.h>

Grid::Grid(uint32_t _w, uint32_t _h,size_t _numParticles) 
{
    m_width=_w;
    m_height=_h;
    m_numParticles=_numParticles;
    m_posAccel.resize(m_numParticles);
    m_dirSpeed.resize(m_numParticles);
    initGrid();
    createTextureBuffer();
    // Going to use a non NGL buffer see if it is quicker
    glGenVertexArrays(1, &m_svao);
    glBindVertexArray(m_svao);
    glBindBuffer(GL_ARRAY_BUFFER, m_ssbo[0]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, m_ssbo[1]);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

}


void Grid::draw() const
{
  glBindVertexArray(m_svao);
  glDrawArrays(GL_POINTS,0,m_numParticles);      
}
void Grid::update(float _dt)
{
  ngl::ShaderLib::use("ComputeShader");
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo[0]);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ssbo[1]);
  
  glDispatchCompute(m_numParticles / 128, 1, 1);
  glMemoryBarrier(GL_ALL_BARRIER_BITS);
//  glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);


/*  for(size_t i=0; i<m_numParticles; ++i)
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
  updateTextureBuffer();*/
}

void Grid::resetParticle(size_t i)
{
    m_posAccel[i].m_x=ngl::Random::randomNumber(m_width/2.0f);
    m_posAccel[i].m_z=ngl::Random::randomNumber(m_height/2.0f);
    m_posAccel[i].m_y=0.0f; // just in case!
    m_posAccel[i].m_w= ngl::Random::randomPositiveNumber(5) + 0.1f;
    m_dirSpeed[i] = ngl::Random::getRandomVec3()*2.0f;  
    m_dirSpeed[i].m_y=0.0f; // this needs to be done as reflect is 3d
    m_dirSpeed[i].m_w=ngl::Random::randomPositiveNumber(5)+0.1f;  
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


  glGenBuffers(2, &m_ssbo[0]);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo[0]);
  glBufferData(GL_SHADER_STORAGE_BUFFER, m_posAccel.size()*sizeof(ngl::Vec4), &m_posAccel[0].m_x, GL_DYNAMIC_DRAW);
  //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo[0]);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo[1]);
  glBufferData(GL_SHADER_STORAGE_BUFFER, m_dirSpeed.size() * sizeof(ngl::Vec4), &m_dirSpeed[0].m_x, GL_DYNAMIC_DRAW);
  //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo[1]);



  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  /*
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
  */
}


void Grid::updateTextureBuffer()
{
  /*
  // update this buffer by copying the data to it
  glActiveTexture( GL_TEXTURE0 );
  glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[0]);
  glBufferData(GL_TEXTURE_BUFFER, m_pos.size()*sizeof(ngl::Vec3), &m_pos[0].m_x, GL_DYNAMIC_DRAW);
  // update this buffer by copying the data to it
  glActiveTexture( GL_TEXTURE1 );
  glBindBuffer(GL_TEXTURE_BUFFER, m_tbo[1]);
  glBufferData(GL_TEXTURE_BUFFER, m_dir.size()*sizeof(ngl::Vec3), &m_dir[0].m_x, GL_DYNAMIC_DRAW);
  */
}  