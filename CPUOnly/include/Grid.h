#ifndef GRID_H_
#define GRID_H_
#include <vector>
#include <cstdint>
#include "Particle.h"
#include <ngl/MultiBufferVAO.h>


class Grid
{
    public :
        Grid(uint32_t _w, uint32_t _h,size_t _numParticles);
        void draw() const;
        void update(float _dt);
        size_t getNumParticles() const {return m_particles.size();}
    private :
        void initGrid();
        void resetParticle(Particle &io_p);
        uint32_t m_width;
        uint32_t m_height;
        std::vector<Particle> m_particles;
        std::unique_ptr<ngl::MultiBufferVAO> m_vao;


};


#endif