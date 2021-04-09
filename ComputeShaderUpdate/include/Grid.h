#ifndef GRID_H_
#define GRID_H_
#include <vector>
#include <cstdint>
#include <ngl/AbstractVAO.h>
#include <ngl/Vec4.h>
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
        GLuint m_ssbo[2];
        GLuint m_posBufferID;
        GLuint m_dirBufferID;
        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_numParticles;
        std::unique_ptr<ngl::AbstractVAO> m_vao;
        GLuint m_svao;
        GLuint m_vboID;
        std::vector<ngl::Vec4> m_posAccel;
        std::vector<ngl::Vec4> m_dirSpeed;



};


#endif