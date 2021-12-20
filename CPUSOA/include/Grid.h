#ifndef GRID_H_
#define GRID_H_
#include <vector>
#include <cstdint>
#include <ngl/MultiBufferVAO.h>
#include <ngl/Vec3.h>
class Grid
{
    public :
        Grid(uint32_t _w, uint32_t _h,size_t _numParticles);
        void draw() const;
        void update(float _dt);
        enum class DrawMode{SINGLEBUFFER,MULTIBUFFER};
        void toggleDrawMode(DrawMode _mode);
        uint32_t getNumParticles() const {return m_numParticles;}
    private :
        void initGrid();
        void resetParticle(size_t i);
        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_numParticles;
        std::unique_ptr<ngl::MultiBufferVAO> m_vao;
        GLuint m_svao;
        GLuint m_vboID;
        std::vector<ngl::Vec3> m_pos;
        std::vector<ngl::Vec3> m_dir;
        std::vector<float> m_acceleration;
        std::vector<float> m_maxspeed;
        DrawMode m_drawMode=DrawMode::SINGLEBUFFER;



};


#endif