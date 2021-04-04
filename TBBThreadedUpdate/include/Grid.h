#ifndef GRID_H_
#define GRID_H_
#include <vector>
#include <cstdint>
#include <ngl/AbstractVAO.h>
#include <ngl/Vec3.h>
#include <vector>
class Grid
{
    public :
        Grid(uint32_t _w, uint32_t _h,size_t _numParticles) noexcept;
        void draw() const noexcept;
        void update(float _dt) noexcept;

    private :
        void initGrid() noexcept;
        void resetParticle(size_t _i) noexcept;
        void updateParticle(size_t _i,float _dt) noexcept;
        void createTextureBuffer() noexcept;
        void updateTextureBuffer() noexcept;
        GLuint m_tbo[2];
        GLuint m_posBufferID;
        GLuint m_dirBufferID;
        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_numParticles;
        std::unique_ptr<ngl::AbstractVAO> m_vao;
        GLuint m_svao;
        GLuint m_vboID;
        std::vector<ngl::Vec3> m_pos;
        std::vector<ngl::Vec3> m_dir;
        std::vector<float> m_acceleration;
        std::vector<float> m_maxspeed;
        size_t m_nthreads;



};


#endif