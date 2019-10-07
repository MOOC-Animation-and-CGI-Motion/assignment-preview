#ifndef __TWO_D_SCENE_RENDERER_H__
#define __TWO_D_SCENE_RENDERER_H__

#include <Eigen/StdVector>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <iostream>

#include "TwoDScene.h"
#include "MathUtilities.h"
#include "RenderingUtilities.h"

class TwoDSceneRenderer
{
public:
    
    TwoDSceneRenderer( const TwoDScene& scene, const std::vector<renderingutils::Color>& particle_colors, const std::vector<renderingutils::Color>& edge_colors, const std::vector<renderingutils::ParticlePath>& particle_paths );
    
    void renderScene() const;
    
    void circleMajorResiduals( const TwoDScene& oracle_scene, const TwoDScene& testing_scene, scalar eps = 1.0e-9 ) const;
    
    void updateState();
    
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    
private:
    void initializeCircleRenderer( int num_points );
    void initializeSemiCircleRenderer( int num_points );
    
    void renderSolidCircle( const Eigen::Vector2d& center, double radius ) const;
    void renderCircle( const Eigen::Vector2d& center, double radius ) const;
    
    void renderSweptEdge( const Eigen::Vector2d& x0, const Eigen::Vector2d& x1, double radius ) const;
    
    const TwoDScene& m_scene;
    
    std::vector<renderingutils::Color> m_particle_colors;
    std::vector<renderingutils::Color> m_edge_colors;
    std::vector<renderingutils::ParticlePath> m_particle_paths;
    
    std::vector<std::pair<double,double> > m_circle_points;
    std::vector<std::pair<double,double> > m_semi_circle_points;
};

#endif
