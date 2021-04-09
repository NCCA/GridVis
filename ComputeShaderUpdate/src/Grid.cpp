#include "Grid.h"
#include <ngl/Random.h>
#include <ngl/Util.h>
#include <ngl/NGLStream.h>
#include <ngl/VAOFactory.h>
#include <ngl/MultiBufferVAO.h>
#include <ngl/SimpleVAO.h>
#include <ngl/ShaderLib.h>

unsigned int roundTo(unsigned int value, unsigned int roundTo)
{
    return (value + (roundTo - 1)) & ~(roundTo - 1);
}

Grid::Grid(uint32_t _w, uint32_t _h,size_t _numParticles) 
{
    m_width=_w;
    m_height=_h;

    m_numParticles=roundTo(_numParticles,128);
    initGrid();
    // Going to use a non NGL buffer see if it is quicker
    glGenVertexArrays(1, &m_svao);
    glBindVertexArray(m_svao);
    // Bind the Shader Storage Buffers we created earlier.
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
  ngl::ShaderLib::setUniform("dt",_dt);

  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo[0]);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ssbo[1]);
  
  glDispatchCompute(m_numParticles / 128, 1, 1);
  glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void Grid::initGrid()
{
  std::unique_ptr<ngl::Vec4 []> m_posAccel = std::make_unique<ngl::Vec4 []>(m_numParticles);
  std::unique_ptr<ngl::Vec4 []> m_dirSpeed = std::make_unique<ngl::Vec4 []>(m_numParticles);
  
  for(size_t i=0; i<m_numParticles; ++i)
  {
    m_posAccel[i].m_x=ngl::Random::randomNumber(m_width/2.0f);
    m_posAccel[i].m_z=ngl::Random::randomNumber(m_height/2.0f);
    m_posAccel[i].m_y=0.0f; // just in case!
    m_posAccel[i].m_w= ngl::Random::randomPositiveNumber(5) + 0.5f;
    m_dirSpeed[i] = ngl::Random::getRandomVec3()*2.0f;  
    m_dirSpeed[i].m_y=0.0f; // this needs to be done as reflect is 3d
    m_dirSpeed[i].m_w=ngl::Random::randomPositiveNumber(5)+0.5f;  
  }

  glGenBuffers(2, &m_ssbo[0]);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo[0]);
  glBufferData(GL_SHADER_STORAGE_BUFFER, m_numParticles*sizeof(ngl::Vec4), &m_posAccel[0].m_x, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo[1]);
  glBufferData(GL_SHADER_STORAGE_BUFFER, m_numParticles * sizeof(ngl::Vec4), &m_dirSpeed[0].m_x, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}




