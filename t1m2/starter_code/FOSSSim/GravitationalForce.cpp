#include "GravitationalForce.h"

void GravitationalForce::addEnergyToTotal( const VectorXs& x, const VectorXs& v, const VectorXs& m, scalar& E )
{
  assert( x.size() == v.size() );
  assert( x.size() == m.size() );
  assert( x.size()%2 == 0 );
  assert( m_particles.first >= 0 );  assert( m_particles.first < x.size()/2 );
  assert( m_particles.second >= 0 ); assert( m_particles.second < x.size()/2 );

  // Your code goes here!
}

void GravitationalForce::addGradEToTotal( const VectorXs& x, const VectorXs& v, const VectorXs& m, VectorXs& gradE )
{
  assert( x.size() == v.size() );
  assert( x.size() == m.size() );
  assert( x.size() == gradE.size() );
  assert( x.size()%2 == 0 );
  assert( m_particles.first >= 0 );  assert( m_particles.first < x.size()/2 );
  assert( m_particles.second >= 0 ); assert( m_particles.second < x.size()/2 );

  // Your code goes here!
}
