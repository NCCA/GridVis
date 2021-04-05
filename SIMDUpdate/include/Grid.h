#ifndef GRID_H_
#define GRID_H_
#include <vector>
#include <cstdint>
#include <ngl/AbstractVAO.h>
#include <ngl/Vec3.h>
#include "Vec3x8.h"
class Grid
{
    public :
        Grid(uint32_t _w, uint32_t _h,size_t _numParticles);
        void draw() const;
        void update(float _dt);

    private :
        void initGrid();
        void resetParticle(size_t i);
        void createTextureBuffer();
        void updateTextureBuffer();
        GLuint m_tbo[2];
        GLuint m_posBufferID;
        GLuint m_dirBufferID;
        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_numParticles;
        std::unique_ptr<ngl::AbstractVAO> m_vao;
        GLuint m_svao;
        GLuint m_vboID;
        std::vector<Vec3x8> m_pos;
        std::vector<Vec3x8> m_dir;
        std::vector<__m256> m_acceleration; // float[8]
        std::vector<__m256> m_maxspeed; // float [8]



};


#endif