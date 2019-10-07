#include "TwoDScene.h"

TwoDScene::TwoDScene()
: m_x()
, m_v()
, m_m()
, m_fixed()
, m_radii()
, m_edges()
, m_edge_radii()
, m_forces()
{}

TwoDScene::TwoDScene( int num_particles )
: m_x(2*num_particles)
, m_v(2*num_particles)
, m_m(2*num_particles)
, m_fixed(num_particles)
, m_radii()
, m_edges()
, m_edge_radii()
, m_forces()
{
    assert( num_particles >= 0 );
}

TwoDScene::TwoDScene( const TwoDScene& otherscene )
: m_x(otherscene.m_x)
, m_v(otherscene.m_v)
, m_m(otherscene.m_m)
, m_fixed(otherscene.m_fixed)
, m_radii()
, m_edges()
, m_edge_radii()
, m_forces()
{
    m_forces.resize(otherscene.m_forces.size());
    for( std::vector<Force*>::size_type i = 0; i < m_forces.size(); ++i ) m_forces[i] = otherscene.m_forces[i]->createNewCopy();
}

TwoDScene::~TwoDScene()
{
    for( std::vector<Force*>::size_type i = 0; i < m_forces.size(); ++i )
    {
        assert( m_forces[i] != NULL );
        delete m_forces[i];
        m_forces[i] = NULL;
    }
}

int TwoDScene::getNumParticles() const
{
    return m_x.size()/2;
}

int TwoDScene::getNumEdges() const
{
    return m_edges.size();
}

const VectorXs& TwoDScene::getX() const
{
    return m_x;
}

VectorXs& TwoDScene::getX()
{
    return m_x;
}

const VectorXs& TwoDScene::getV() const
{
    return m_v;
}

VectorXs& TwoDScene::getV()
{
    return m_v;
}

const VectorXs& TwoDScene::getM() const
{
    return m_m;
}

VectorXs& TwoDScene::getM()
{
    return m_m;
}

const std::vector<scalar>& TwoDScene::getRadii() const
{
    return m_radii;
}

// TODO: Does this overwrite old data?
void TwoDScene::resizeSystem( int num_particles )
{
    assert( num_particles >= 0 );
    
    m_x.resize(2*num_particles);
    m_v.resize(2*num_particles);
    m_m.resize(2*num_particles);
    m_fixed.resize(num_particles);
    m_radii.resize(num_particles);
}

void TwoDScene::setPosition( int particle, const Vector2s& pos )
{
    assert( particle >= 0 );
    assert( particle < getNumParticles() );
    
    m_x.segment<2>(2*particle) = pos;
}

void TwoDScene::setVelocity( int particle, const Vector2s& vel )
{
    assert( particle >= 0 );
    assert( particle < getNumParticles() );
    
    m_v.segment<2>(2*particle) = vel;
}

void TwoDScene::setMass( int particle, const scalar& mass )
{
    assert( particle >= 0 );
    assert( particle < getNumParticles() );
    
    m_m(2*particle)   = mass;
    m_m(2*particle+1) = mass;
}

void TwoDScene::setFixed( int particle, bool fixed )
{
    assert( particle >= 0 );
    assert( particle < getNumParticles() );
    
    m_fixed[particle] = fixed;
}

bool TwoDScene::isFixed( int particle ) const
{
    assert( particle >= 0 );
    assert( particle < getNumParticles() );
    
    return m_fixed[particle];
}

const scalar& TwoDScene::getRadius( int particle ) const
{
    assert( particle >= 0 );
    assert( particle < getNumParticles() );
    
    return m_radii[particle];  
}

void TwoDScene::setRadius( int particle, scalar radius )
{
    assert( particle >= 0 );
    assert( particle < getNumParticles() );
    
    m_radii[particle] = radius;
}

void TwoDScene::clearEdges()
{
    m_edges.clear();
}

void TwoDScene::insertEdge( const std::pair<int,int>& edge, scalar radius )
{
    assert( edge.first >= 0 );  assert( edge.first < getNumParticles() );
    assert( edge.second >= 0 ); assert( edge.second < getNumParticles() );
    assert( radius > 0.0 );
    
    m_edges.push_back(edge);
    m_edge_radii.push_back(radius);
}

const std::vector<std::pair<int,int> >& TwoDScene::getEdges() const
{
    return m_edges;
}

const std::vector<scalar>& TwoDScene::getEdgeRadii() const
{
    return m_edge_radii;
}

const std::pair<int,int>& TwoDScene::getEdge(int edg) const
{
    assert( edg >= 0 );
    assert( edg < (int) m_edges.size() );
    
    return m_edges[edg];
}

void TwoDScene::insertForce( Force* newforce )
{
    m_forces.push_back(newforce);
}

scalar TwoDScene::computePotentialEnergy() const
{
    scalar U = 0.0;
    for( std::vector<Force*>::size_type i = 0; i < m_forces.size(); ++i ) m_forces[i]->addEnergyToTotal( m_x, m_v, m_m, U );
    return U;  
}

scalar TwoDScene::computeTotalEnergy() const
{
    return computeKineticEnergy()+computePotentialEnergy();
}

void TwoDScene::accumulateGradU( VectorXs& F, const VectorXs& dx, const VectorXs& dv )
{
    assert( F.size() == m_x.size() );
    assert( dx.size() == dv.size() );
    assert( dx.size() == 0 || dx.size() == F.size() );
    
    // Accumulate all energy gradients
    if( dx.size() == 0 ) for( std::vector<Force*>::size_type i = 0; i < m_forces.size(); ++i ) m_forces[i]->addGradEToTotal( m_x, m_v, m_m, F );
    else                 for( std::vector<Force*>::size_type i = 0; i < m_forces.size(); ++i ) m_forces[i]->addGradEToTotal( m_x+dx, m_v+dv, m_m, F );
}

void TwoDScene::accumulateddUdxdx( MatrixXs& A, const VectorXs& dx, const VectorXs& dv )
{
    assert( A.rows() == m_x.size() );
    assert( A.cols() == m_x.size() );
    assert( dx.size() == dv.size() );
    assert( dx.size() == 0 || dx.size() == A.rows() );
    
    if( dx.size() == 0 ) for( std::vector<Force*>::size_type i = 0; i < m_forces.size(); ++i ) m_forces[i]->addHessXToTotal( m_x, m_v, m_m, A );
    else                 for( std::vector<Force*>::size_type i = 0; i < m_forces.size(); ++i ) m_forces[i]->addHessXToTotal( m_x+dx, m_v+dv, m_m, A );
}

void TwoDScene::accumulateddUdxdv( MatrixXs& A, const VectorXs& dx, const VectorXs& dv )
{
    assert( A.rows() == m_x.size() );
    assert( A.cols() == m_x.size() );
    assert( dx.size() == dv.size() );
    assert( dx.size() == 0 || dx.size() == A.rows() );
    
    if( dx.size() == 0 ) for( std::vector<Force*>::size_type i = 0; i < m_forces.size(); ++i ) m_forces[i]->addHessVToTotal( m_x, m_v, m_m, A );
    else                 for( std::vector<Force*>::size_type i = 0; i < m_forces.size(); ++i ) m_forces[i]->addHessVToTotal( m_x+dx, m_v+dv, m_m, A );
}

void TwoDScene::copyState( const TwoDScene& otherscene )
{
    m_x = otherscene.m_x;
    m_v = otherscene.m_v;
    m_m = otherscene.m_m;
    m_fixed = otherscene.m_fixed;
    m_edges = otherscene.m_edges;
    
    for( std::vector<Force*>::size_type i = 0; i < m_forces.size(); ++i )
    {
        assert( m_forces[i] != NULL );
        delete m_forces[i];
        m_forces[i] = NULL;
    }
    m_forces.resize(otherscene.m_forces.size());
    for( std::vector<Force*>::size_type i = 0; i < m_forces.size(); ++i ) m_forces[i] = otherscene.m_forces[i]->createNewCopy();
}

void TwoDScene::checkConsistency()
{
    assert( m_x.size() == m_v.size() );
    assert( m_x.size() == m_m.size() );
    assert( m_x.size() == (int) (2*m_fixed.size()) );
    
    assert( (m_x.array()==m_x.array()).all() );
    assert( (m_v.array()==m_v.array()).all() );
    assert( (m_m.array()==m_m.array()).all() );
    
    for( std::vector<std::pair<int,int> >::size_type i = 0; i < m_edges.size(); ++i )
    {
        assert( m_edges[i].first >= 0 );
        assert( m_edges[i].first < getNumParticles() );
        assert( m_edges[i].second >= 0 );
        assert( m_edges[i].second < getNumParticles() );
    }
    
    // TODO: Add more checks
}

