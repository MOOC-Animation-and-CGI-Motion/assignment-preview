#ifndef __TWO_DIMENSIONAL_DISPLAY_CONTROLLER_H__
#define __TWO_DIMENSIONAL_DISPLAY_CONTROLLER_H__

#include <cmath>
#include <iostream>
#include <cassert>

#include "RenderingUtilities.h"
#include "Macros.h"

class TwoDimensionalDisplayController
{
public:
    TwoDimensionalDisplayController( int width, int height );
    
    void reshape( int w, int h );
    
    void keyboard( unsigned char key, int x, int y );
    
    void special( int key, int x, int y );
    
    void mouse( int button, int state, int x, int y );
    
    void translateView( double dx, double dy );
    
    void zoomView( double dx, double dy );
    
    void motion( int x, int y );
    
    int getWindowWidth() const;  
    int getWindowHeight() const;
    
    double getCenterX() const;
    double getCenterY() const;
    void setCenterX( double x );
    void setCenterY( double y );
    
    void setScaleFactor( double scale );
    
private:
    // Width of the window in pixels
    int m_window_width;
    // Height of the window in pixels
    int m_window_height;
    // Factor to 'zoom' in or out by
    double m_scale_factor;
    // Center of the display, x coord
    double m_center_x;
    // Center of the display, y coord
    double m_center_y;
    // True if the user is dragging the display left
    bool m_left_drag;
    // True if the user is dragging the display right
    bool m_right_drag;
    // Last position of the cursor in a drag, x coord
    int m_last_x;
    // Last position of the cursor in a drag, y coord
    int m_last_y;
};

#endif
