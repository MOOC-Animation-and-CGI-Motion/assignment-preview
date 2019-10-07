#include "TwoDSceneRenderer.h"

TwoDSceneRenderer::TwoDSceneRenderer( const TwoDScene& scene, const std::vector<renderingutils::Color>& particle_colors, const std::vector<renderingutils::Color>& edge_colors, const std::vector<renderingutils::ParticlePath>& particle_paths )
: m_scene(scene)
, m_particle_colors(particle_colors)
, m_edge_colors(edge_colors)
, m_particle_paths(particle_paths)
, m_circle_points()
, m_semi_circle_points()
{
    assert( (int) m_particle_colors.size() == m_scene.getNumParticles() );
    assert( (int) m_edge_colors.size() == m_scene.getNumEdges() );
    
    initializeCircleRenderer( 16 );
    initializeSemiCircleRenderer( 16 );
}

void TwoDSceneRenderer::initializeCircleRenderer( int num_points )
{
    m_circle_points.resize(num_points);
    double dtheta = 2.0*PI/((double)num_points);
    for( int i = 0; i < num_points; ++i )
    {
        m_circle_points[i].first =  cos(((double)i)*dtheta);
        m_circle_points[i].second = sin(((double)i)*dtheta);
    }
}

void TwoDSceneRenderer::initializeSemiCircleRenderer( int num_points )
{
    double dtheta = PI/((double)(num_points-1));
    m_semi_circle_points.resize(num_points);
    for( int i = 0; i < num_points; ++i )
    {
        m_semi_circle_points[i].first =  -sin(((double)i)*dtheta);
        m_semi_circle_points[i].second = cos(((double)i)*dtheta);
    }  
}

void TwoDSceneRenderer::updateState()
{
    const VectorXs& x = m_scene.getX();
    for( std::vector<renderingutils::ParticlePath>::size_type i = 0; i < m_particle_paths.size(); ++i )
        m_particle_paths[i].addToPath(x.segment<2>(2*m_particle_paths[i].getParticleIdx()));
}


void TwoDSceneRenderer::renderSolidCircle( const Eigen::Vector2d& center, double radius ) const
{  
	glBegin(GL_TRIANGLE_FAN);
    glVertex2d(center.x(),center.y());
    
    for( std::vector<std::pair<double,double> >::size_type i = 0; i < m_circle_points.size(); ++i )
        glVertex2d(radius*m_circle_points[i].first+center.x(), radius*m_circle_points[i].second+center.y());
    
    glVertex2d(radius*m_circle_points.front().first+center.x(), radius*m_circle_points.front().second+center.y());  
	glEnd();  
}

void TwoDSceneRenderer::renderCircle( const Eigen::Vector2d& center, double radius ) const
{
    glBegin(GL_LINE_LOOP);
    for( std::vector<Eigen::Vector2d>::size_type i = 0; i < m_circle_points.size(); ++i )
        glVertex2d(radius*m_circle_points[i].first+center.x(), radius*m_circle_points[i].second+center.y());
    glEnd();
}

void TwoDSceneRenderer::renderSweptEdge( const Eigen::Vector2d& x0, const Eigen::Vector2d& x1, double radius ) const
{
    Eigen::Vector2d e = x1-x0;
    double length = e.norm();
    double theta = 360.0*atan2(e.y(),e.x())/(2.0*PI);
    
    glPushMatrix();
    
    glTranslated(x0.x(), x0.y(),0.0);
    glRotated(theta, 0.0, 0.0, 1.0);
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(0.0,0.0);
    for( std::vector<Eigen::Vector2d>::size_type i = 0; i < m_semi_circle_points.size(); ++i )
        glVertex2d(radius*m_semi_circle_points[i].first, radius*m_semi_circle_points[i].second);
    for( std::vector<Eigen::Vector2d>::size_type i = 0; i < m_semi_circle_points.size(); ++i )
        glVertex2d(-radius*m_semi_circle_points[i].first+length, -radius*m_semi_circle_points[i].second);
    glVertex2d(radius*m_semi_circle_points.front().first, radius*m_semi_circle_points.front().second);
	glEnd();  
    
    glPopMatrix();
}

void TwoDSceneRenderer::renderScene() const
{
    const VectorXs& x = m_scene.getX();
    assert( x.size()%2 == 0 );
    assert( 2*m_scene.getNumParticles() == x.size() );
    
    for( std::vector<renderingutils::ParticlePath>::size_type i = 0; i < m_particle_paths.size(); ++i )
    {
        const std::list<Vector2s>& ppath = m_particle_paths[i].getPath();
        const renderingutils::Color& pathcolor = m_particle_paths[i].getColor();
        glColor3d(pathcolor.r,pathcolor.g,pathcolor.b);
        glBegin(GL_LINE_STRIP);
        for( std::list<Vector2s>::const_iterator itr = ppath.begin(); itr != ppath.end(); ++itr )
        {
            glVertex2d(itr->x(),itr->y());
        }
        glEnd();
    }
    
    // Render edges
    //glColor3d(0.0,0.388235294117647,0.388235294117647);
    const std::vector<std::pair<int,int> >& edges = m_scene.getEdges();
    const std::vector<scalar>& edgeradii = m_scene.getEdgeRadii();
    assert( edgeradii.size() == edges.size() );
    for( std::vector<std::pair<int,int> >::size_type i = 0; i < edges.size(); ++i )
    {
        assert( edges[i].first >= 0 );  assert( edges[i].first < m_scene.getNumParticles() );
        assert( edges[i].second >= 0 ); assert( edges[i].second < m_scene.getNumParticles() );
        glColor3d(m_edge_colors[i].r,m_edge_colors[i].g,m_edge_colors[i].b);
        renderSweptEdge( x.segment<2>(2*edges[i].first), x.segment<2>(2*edges[i].second), edgeradii[i] );
    }
    
    // Render particles
    const std::vector<scalar>& radii = m_scene.getRadii();
    assert( (int) radii.size() == m_scene.getNumParticles() );
    for( int i = 0; i < m_scene.getNumParticles(); ++i ) 
    {
        glColor3d(m_particle_colors[i].r,m_particle_colors[i].g,m_particle_colors[i].b);
        renderSolidCircle( x.segment<2>(2*i), radii[i] );
    }  
}

void TwoDSceneRenderer::circleMajorResiduals( const TwoDScene& oracle_scene, const TwoDScene& testing_scene, scalar eps ) const
{
    assert(   oracle_scene.getNumParticles() == testing_scene.getNumParticles() );
    assert( 2*oracle_scene.getNumParticles() == oracle_scene.getX().size() );
    assert( 2*oracle_scene.getNumParticles() == testing_scene.getX().size() );
    assert( 2*oracle_scene.getNumParticles() == oracle_scene.getV().size() );
    assert( 2*oracle_scene.getNumParticles() == testing_scene.getV().size() );
    
    const VectorXs& oracle_x = oracle_scene.getX();
    const VectorXs& testing_x = testing_scene.getX();
    
    const VectorXs& oracle_v = oracle_scene.getV();
    const VectorXs& testing_v = testing_scene.getV();
    
    glColor3d(1.0,0.0,0.0);
    for( int i = 0; i < oracle_scene.getNumParticles(); ++i )
    {
        scalar x_resid = (oracle_x.segment<2>(2*i)-testing_x.segment<2>(2*i)).norm();
        scalar v_resid = (oracle_v.segment<2>(2*i)-testing_v.segment<2>(2*i)).norm();
        if( x_resid > eps || v_resid > eps )
            renderCircle( oracle_x.segment<2>(2*i), 2.0*oracle_scene.getRadius(i) );
    }
}

