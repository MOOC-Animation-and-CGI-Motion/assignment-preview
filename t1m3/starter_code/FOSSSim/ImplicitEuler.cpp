#include "ImplicitEuler.h"

bool ImplicitEuler::stepScene( TwoDScene& scene, scalar dt )
{
  VectorXs& x = scene.getX();
  VectorXs& v = scene.getV();
  const VectorXs& m = scene.getM();
  assert(x.size() == v.size());
  assert(x.size() == m.size());

  // Implement implicit Euler here for extra credit!

  
  
  /////////////
  // Some examples of working with vectors, matrices, etc.
  
  // How to get the force Jacobian from two d scene
  int ndof = x.size();
  assert( ndof%2 == 0 );
  // Note that the system's state is passed to two d scene as a change from the last timestep's solution
  VectorXs dx = dt*v;
  VectorXs dv = VectorXs::Zero(ndof);
  MatrixXs A = MatrixXs::Zero(ndof,ndof);
  scene.accumulateddUdxdx(A,dx,dv);
  scene.accumulateddUdxdv(A,dx,dv);

  // Some useful vector operations
  Vector2s egvec(1.0,0.0);
  // Outer product
  Matrix2s egmat = egvec*egvec.transpose();

  // Manipulating blocks of matrix
  MatrixXs egmat2(20,20);
  // 2x2 block that begins at position 3,4
  egmat2.block(3,4,2,2) += egmat;

  // Add a vector to a matrix's diagonal
  VectorXs egvec2(20);
  egvec2.setConstant(2.0);
  egmat2.diagonal() += egvec;

  // Invert a 10 x 10 matrix
  MatrixXs B(10,10);
  // Fill the matrix with random numbers
  B.setRandom();
  // Create a vector of length 10
  VectorXs b(10);
  // Fill the vector with random numbers
  b.setRandom();
  // Compute the solution to A*x = b
  VectorXs sln = B.fullPivLu().solve(b);
  // Verify that we computed the solution
  //std::cout << (B*sln-b).norm() << std::endl;  

  // End of example code.
  /////////////
  
  return true;
}
