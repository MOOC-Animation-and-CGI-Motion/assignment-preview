#ifndef __RENDERING_UTILITIES_H__
#define __RENDERING_UTILITIES_H__

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <list>
#include <iostream>
#include <cstdio>

#include "MathDefs.h"
#include "StringUtilities.h"

// TODO: Clean up ParticlePath to fix sse alignment errors

namespace renderingutils
{
    
    // False => error
    bool checkGLErrors();
    
    class Color
    {
    public:
        
        Color();
        
        Color( double r, double g, double b );
        
        double r;
        double g;
        double b;
    };
    
    class ParticlePath
    {
    public:
        ParticlePath( int particle, int max_list_size, const Color& color );
        
        void addToPath( const Vector2s& newpoint );
        
        int getParticleIdx() const;
        
        const std::list<Vector2s>& getPath() const;
        
        const Color& getColor() const;
        
    private:
        int m_particle;
        int m_max_list_size;
        std::list<Vector2s> m_path;
        Color m_color;
    };
    
}

#endif
