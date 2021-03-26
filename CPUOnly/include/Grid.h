#ifndef GRID_H_
#define GRID_H_
#include <vector>
#include <cstdint>
#include "Particle.h"
#include <ngl/AbstractVAO.h>
class Grid
{
    public :
        Grid(uint32_t _w, uint32_t _h,size_t _numParticles);
        void draw() const;
        void update(float _dt);

    private :
        void initGrid();
        uint32_t m_width;
        uint32_t m_height;
        std::vector<Particle> m_particles;
        std::unique_ptr<ngl::AbstractVAO> m_vao;


};


#endif