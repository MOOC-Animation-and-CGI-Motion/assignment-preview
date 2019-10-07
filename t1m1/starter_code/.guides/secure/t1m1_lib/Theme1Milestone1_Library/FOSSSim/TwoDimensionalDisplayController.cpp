#include "TwoDimensionalDisplayController.h"

TwoDimensionalDisplayController::TwoDimensionalDisplayController( int width, int height )
: m_window_width(width)
, m_window_height(height)
, m_scale_factor(1.0)
, m_center_x(0.0)
, m_center_y(0.0)
, m_left_drag(false)
, m_right_drag(false)
, m_last_x(0)
, m_last_y(0)
{}

void TwoDimensionalDisplayController::reshape( int w, int h ) 
{
    assert( renderingutils::checkGLErrors() );
    // Record the new width and height
    m_window_width = w;
    m_window_height = h;
    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Set the coordinate system to achieve the desired zoom level, center
    double ratio = (double)h/(double)w;
    gluOrtho2D(m_center_x-m_scale_factor/ratio,m_center_x+m_scale_factor/ratio,m_center_y-m_scale_factor,m_center_y+m_scale_factor);
    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);
    // Render the scene
    glutPostRedisplay();
    assert( renderingutils::checkGLErrors() );
}

void TwoDimensionalDisplayController::keyboard( unsigned char key, int x, int y )
{
    IGNORE_UNUSED(x);
    IGNORE_UNUSED(y);
    if( key == '-' || key == '_' )
    {
        m_scale_factor += 0.1;
        reshape(m_window_width,m_window_height);
    }
    else if( key == '=' || key == '+' )
    {
        m_scale_factor = std::max(0.1,m_scale_factor-0.1);
        reshape(m_window_width,m_window_height);
    }
}

void TwoDimensionalDisplayController::special( int key, int x, int y )
{
    IGNORE_UNUSED(x);
    IGNORE_UNUSED(y);
    if( GLUT_KEY_UP == key ) 
    {
        m_center_y += 0.1;
        reshape(m_window_width,m_window_height);
    }
    else if( GLUT_KEY_DOWN == key ) 
    {
        m_center_y -= 0.1;
        reshape(m_window_width,m_window_height);
    }
    else if( GLUT_KEY_LEFT == key ) 
    {
        m_center_x -= 0.1;
        reshape(m_window_width,m_window_height);
    }
    else if( GLUT_KEY_RIGHT == key ) 
    {
        m_center_x += 0.1;
        reshape(m_window_width,m_window_height);
    }
}  

void TwoDimensionalDisplayController::mouse( int button, int state, int x, int y )
{
    if( !m_right_drag && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
    {
        m_left_drag = true;
        m_last_x = x;
        m_last_y = y;
    }
    if( button == GLUT_LEFT_BUTTON && state == GLUT_UP )
    {
        m_left_drag = false;
    }
    
    if( !m_left_drag && button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
    {
        m_right_drag = true;
        m_last_x = x;
        m_last_y = y;
    }
    if( button == GLUT_RIGHT_BUTTON && state == GLUT_UP )
    {
        m_right_drag = false;
    }
}

void TwoDimensionalDisplayController::translateView( double dx, double dy )
{
    double percent_x = dx/((double)m_window_width);
    double percent_y = dy/((double)m_window_height);
    double translate_x = percent_x*2.0*m_scale_factor*((double)m_window_width)/((double)m_window_height);
    double translate_y = percent_y*2.0*m_scale_factor;
    m_center_x -= translate_x;
    m_center_y += translate_y;
    reshape(m_window_width,m_window_height);
}

void TwoDimensionalDisplayController::zoomView( double dx, double dy )
{
    double percent_x = dx/((double)m_window_width);
    double percent_y = dy/((double)m_window_height);
    
    double scale;
    if( std::fabs(percent_x) > std::fabs(percent_y) ) scale = -percent_x;
    else scale = percent_y;
    
    m_scale_factor += 2.0*scale;
    
    reshape(m_window_width,m_window_height);
}

void TwoDimensionalDisplayController::motion( int x, int y ) 
{
    if( m_left_drag ) 
    {
        double dx = x - m_last_x;
        double dy = y - m_last_y;
        m_last_x = x;
        m_last_y = y;
        translateView( dx, dy );
    }
    if( m_right_drag ) 
    {
        double dx = x - m_last_x;
        double dy = y - m_last_y;
        m_last_x = x;
        m_last_y = y;
        zoomView( dx, dy );
    }
}

int TwoDimensionalDisplayController::getWindowWidth() const
{
    return m_window_width;
}

int TwoDimensionalDisplayController::getWindowHeight() const
{
    return m_window_height;
}

double TwoDimensionalDisplayController::getCenterX() const
{
    return m_center_x;
}

double TwoDimensionalDisplayController::getCenterY() const
{
    return m_center_y;
}


void TwoDimensionalDisplayController::setCenterX( double x )
{
    m_center_x = x;
}

void TwoDimensionalDisplayController::setCenterY( double y )
{
    m_center_y = y;
}

void TwoDimensionalDisplayController::setScaleFactor( double scale )
{
    m_scale_factor = scale;
}


