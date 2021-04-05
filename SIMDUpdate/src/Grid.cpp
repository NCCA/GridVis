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

  auto zero = _mm256_setzero_ps();
  __mmask8 mask=_mm256_cmp_ps_mask(m_maxspeed[_i],zero,_CMP_LE_OS);
  float res[8];
  // _mm256_store_ps(static_cast<float *>(res), mask);
  // for(int i=0; i<8; ++i)
  //   std::cout<<(int)res[i]<<' ';
  // std::cout<<'\n';


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
    auto newX =_mm256_mask_permute_ps(m_dir[_i].x(),mask,x,_CMP_LE_OS);
    auto newZ =_mm256_mask_permute_ps(m_dir[_i].z(),mask,z,_CMP_LE_OS);

    m_dir[_i].set(newX,y,newZ);
//  _mm256_mask_permute_ps

/*
//_mm256_maskstore_ps

  // We have 8 floats to check, split into 4's for speed as we need to check
  // All combinations
  __m128 hi=_mm256_castps256_ps128(m_maxspeed[_i]); // grab the high 4 floats
  __m128 lo=_mm256_castps256_ps128(_mm256_permute2f128_ps(m_maxspeed[_i],m_maxspeed[_i],1)); // grab the low 4 floats
  for(size_t e=0; e<2; ++e)
    {
      int32_t sw;
      if(e==0)
        sw=_mm_movemask_ps(_mm_cmple_ps(hi, zero));
      else
        sw=_mm_movemask_ps(_mm_cmple_ps(lo, zero));
      switch (sw)
      {
      case 0: // f f f f
        // do nothing; all 4 particles are alive
        break;
      case 1: // f f f t
        // particle [i] is dead
        break;
      case 2: // f f t f
        // particle [i+1] is dead
        break;
      case 3: // f f t t
        // particles [i] and [i+1] are dead
        break;
      case 4: // f t f f
        // particle [i+2] is dead
        break;
      case 5: // f t f t
        // particles [i] and [i+2] are dead
        break;
      case 6: // f t t f
        // particles [i+1] and [i+2] are dead
        break;
      case 7: // f t t t
        // particles [i] and [i+1] and [i+2] are dead
        break;
      case 8: // t f f f
        // particle [i+3] is dead
        break;
      case 9: // t f f t
        // particles [i] and [i+3] are dead
        break;
      case 10: // t f t f
        // particles [i+1] and [i+3] are dead
        break;
      case 11: // t f t t
        // particles [i] and [i+1] and [i+3] are dead
        break;
      case 12: // t t f f
        // particles [i+2] and [i+3] are dead
        break;
      case 13: // t t f t
        // particles [i] and [i+2] and [i+3] are dead
        break;
      case 14: // t t t f
        // particles [i+1] and [i+2] and [i+3] are dead
        break;
      case 15: // t t t t
        // all 4 particles are dead
        resetParticle(_i);
        break;

      default:
        std::cout<<"Beyond bounds! \n" ;
        break;
  }
}
*/
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