#ifndef GRID_H_
#define GRID_H_
#include <vector>
#include <cstdint>
#include <ngl/AbstractVAO.h>
#include <ngl/Vec3.h>
#include <thread>
#include <vector>
class Grid
{
    public :
        Grid(uint32_t _w, uint32_t _h,size_t _numParticles);
        void draw() const;
        void update(float _dt);

    private :
        void initGrid();
        void resetParticle(size_t _i);
        void updateParticle(size_t _i,float _dt);
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
        std::vector<ngl::Vec3> m_pos;
        std::vector<ngl::Vec3> m_dir;
        std::vector<float> m_acceleration;
        std::vector<float> m_maxspeed;
        std::vector<std::thread> m_threadPool;
        size_t m_nthreads;



};


#endif