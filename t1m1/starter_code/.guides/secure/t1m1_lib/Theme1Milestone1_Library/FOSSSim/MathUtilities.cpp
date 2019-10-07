#include "MathUtilities.h"

namespace mathutils
{
    
    bool approxSymmetric( const MatrixXs& A, const scalar& eps )
    {
        for( int i = 0; i < A.rows(); ++i ) for( int j = i+1; j < A.cols(); ++j ) if( fabs(A(i,j)-A(j,i)) >= eps ) return false;
        return true;
    }
    
}
