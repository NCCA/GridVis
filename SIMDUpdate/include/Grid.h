#ifndef GRID_H_
#define GRID_H_
#include <vector>
#include <cstdint>
#include <ngl/AbstractVAO.h>
#include <ngl/Vec3.h>
#if defined(__x86_64__)
    #include "Vec3x8.h"
#elif defined(__arm64__)
    #include "Vec3x4.h"
#else
    error "Undefined SIMD architecture"
#endif
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
        GLuint m_tbo[2];
        GLuint m_posBufferID;
        GLuint m_dirBufferID;
        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_numParticles;
        std::unique_ptr<ngl::AbstractVAO> m_vao;
        GLuint m_svao;
        GLuint m_vboID;
        #if defined(__x86_64__)
            std::vector<Vec3x8> m_pos;
            std::vector<Vec3x8> m_dir;
        #elif defined(__arm64__)
            std::vector<Vec3x4> m_pos;
            std::vector<Vec3x4> m_dir;
        #endif
        
        std::vector<ngl::Vec3> m_posBuffer;
        std::vector<ngl::Vec3> m_dirBuffer;
        #ifdef __x86_64__
            std::vector<__m256> m_acceleration; // float[8]
            std::vector<__m256> m_maxspeed; // float [8]
        #elif defined(__arm64__) // use neon here
            std::vector<__m128> m_acceleration; // float[4]
            std::vector<__m128> m_maxspeed; // float [4]
        #else
            #error Intrinsics not defined
        #endif

};


#endif