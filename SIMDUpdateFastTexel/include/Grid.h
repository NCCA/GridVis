#ifndef GRID_H_
#define GRID_H_
#include <vector>
#include <cstdint>
#include <ngl/AbstractVAO.h>
#include <ngl/Vec3.h>
#include "simd.h"
#include "AlignedArray.h"
class Grid
{
    public :
        Grid(uint32_t _w, uint32_t _h,size_t _numParticles);
        void draw() const;
        void update(float _dt);
        size_t getNumParticles() const {return m_numParticles;}
        void updateTextureBuffer();

    private :
        void initGrid();
        void resetParticle(size_t i);
        void updateParticle(size_t _i,float _dt) noexcept;
        void checkForReset(size_t _i) noexcept;
        void checkForBounds(size_t _i) noexcept;

        void createTextureBuffer();

        GLuint m_tbo[4];
        GLuint m_posXBufferID;
        GLuint m_posZBufferID;
        
        GLuint m_dirXBufferID;
        GLuint m_dirZBufferID;
        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_numParticles;
        std::unique_ptr<ngl::AbstractVAO> m_vao;
        GLuint m_svao;
        GLuint m_vboID;
        
       AlignedArray<f256,32> m_posX;
       AlignedArray<f256,32> m_posZ;
       AlignedArray<f256,32> m_dirX;
       AlignedArray<f256,32> m_dirZ;
       AlignedArray<f256,32> m_acceleration;
       AlignedArray<f256,32> m_maxspeed;


};


#endif