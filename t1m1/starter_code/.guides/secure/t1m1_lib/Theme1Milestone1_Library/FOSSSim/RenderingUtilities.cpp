#include "RenderingUtilities.h"

namespace renderingutils
{
    
    bool checkGLErrors()
    {
        GLenum errCode;
        const GLubyte *errString;
        
        if ((errCode = glGetError()) != GL_NO_ERROR) 
        {
            errString = gluErrorString(errCode);
            std::cout << outputmod::startred << "OpenGL Error:" << outputmod::endred << std::flush;
            fprintf(stderr, " %s\n", errString);
            return false;
        }
        return true;
    }    
    
    Color::Color()
    : r(0.0), g(0.0), b(0.0)
    {}
    
    Color::Color( double r, double g, double b )
    : r(r), g(g), b(b)
    {
        assert( r >= 0.0 ); assert( r <= 1.0 );
        assert( g >= 0.0 ); assert( g <= 1.0 );
        assert( b >= 0.0 ); assert( b <= 1.0 );
    }
    
    ParticlePath::ParticlePath( int particle, int max_list_size, const Color& color )
    : m_particle(particle)
    , m_max_list_size(max_list_size)
    , m_path()
    , m_color(color)
    {}
    
    void ParticlePath::addToPath( const Vector2s& newpoint )
    {
        if( (int) m_path.size() >= m_max_list_size ) m_path.pop_front();
        m_path.push_back(newpoint);
    }
    
    int ParticlePath::getParticleIdx() const
    {
        return m_particle;
    }
    
    const std::list<Vector2s>& ParticlePath::getPath() const
    {
        return m_path;
    }
    
    const Color& ParticlePath::getColor() const
    {
        return m_color;
    }
    
}
