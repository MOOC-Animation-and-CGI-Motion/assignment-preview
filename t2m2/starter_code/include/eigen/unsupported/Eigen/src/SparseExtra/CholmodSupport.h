// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2009 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#ifndef EIGEN_CHOLMODSUPPORT_H
#define EIGEN_CHOLMODSUPPORT_H

template<typename Scalar, typename CholmodType>
void ei_cholmod_configure_matrix(CholmodType& mat)
{
  if (ei_is_same_type<Scalar,float>::ret)
  {
    mat.xtype = CHOLMOD_REAL;
    mat.dtype = CHOLMOD_SINGLE;
  }
  else if (ei_is_same_type<Scalar,double>::ret)
  {
    mat.xtype = CHOLMOD_REAL;
    mat.dtype = CHOLMOD_DOUBLE;
  }
  else if (ei_is_same_type<Scalar,std::complex<float> >::ret)
  {
    mat.xtype = CHOLMOD_COMPLEX;
    mat.dtype = CHOLMOD_SINGLE;
  }
  else if (ei_is_same_type<Scalar,std::complex<double> >::ret)
  {
    mat.xtype = CHOLMOD_COMPLEX;
    mat.dtype = CHOLMOD_DOUBLE;
  }
  else
  {
    ei_assert(false && "Scalar type not supported by CHOLMOD");
  }
}

template<typename MatrixType>
cholmod_sparse ei_asCholmodMatrix(MatrixType& mat)
{
  typedef typename MatrixType::Scalar Scalar;
  cholmod_sparse res;
  res.nzmax   = mat.nonZeros();
  res.nrow    = mat.rows();;
  res.ncol    = mat.cols();
  res.p       = mat._outerIndexPtr();
  res.i       = mat._innerIndexPtr();
  res.x       = mat._valuePtr();
  res.xtype   = CHOLMOD_REAL;
  res.itype   = CHOLMOD_INT;
  res.sorted  = 1;
  res.packed  = 1;
  res.dtype   = 0;
  res.stype   = -1;

  ei_cholmod_configure_matrix<Scalar>(res);


  if (MatrixType::Flags & SelfAdjoint)
  {
    if (MatrixType::Flags & Upper)
      res.stype = 1;
    else if (MatrixType::Flags & Lower)
      res.stype = -1;
    else
      res.stype = 0;
  }
  else
    res.stype = -1; // by default we consider the lower part

  return res;
}

template<typename Derived>
cholmod_dense ei_cholmod_map_eigen_to_dense(MatrixBase<Derived>& mat)
{
  EIGEN_STATIC_ASSERT((ei_traits<Derived>::Flags&RowMajorBit)==0,THIS_METHOD_IS_ONLY_FOR_COLUMN_MAJOR_MATRICES);
  typedef typename Derived::Scalar Scalar;

  cholmod_dense res;
  res.nrow   = mat.rows();
  res.ncol   = mat.cols();
  res.nzmax  = res.nrow * res.ncol;
  res.d      = Derived::IsVectorAtCompileTime ? mat.derived().size() : mat.derived().outerStride();
  res.x      = mat.derived().data();
  res.z      = 0;

  ei_cholmod_configure_matrix<Scalar>(res);

  return res;
}

template<typename Scalar, int Flags, typename Index>
MappedSparseMatrix<Scalar,Flags,Index> ei_map_cholmod(cholmod_sparse& cm)
{
  return MappedSparseMatrix<Scalar,Flags,Index>
         (cm.nrow, cm.ncol, reinterpret_cast<Index*>(cm.p)[cm.ncol],
          reinterpret_cast<Index*>(cm.p), reinterpret_cast<Index*>(cm.i),reinterpret_cast<Scalar*>(cm.x) );
}

template<typename MatrixType>
class SparseLLT<MatrixType,Cholmod> : public SparseLLT<MatrixType>
{
  protected:
    typedef SparseLLT<MatrixType> Base;
    typedef typename Base::Scalar Scalar;
    typedef typename Base::RealScalar RealScalar;
    typedef typename Base::CholMatrixType CholMatrixType;
    typedef typename MatrixType::Index Index;
    using Base::MatrixLIsDirty;
    using Base::SupernodalFactorIsDirty;
    using Base::m_flags;
    using Base::m_matrix;
    using Base::m_status;

  public:

    SparseLLT(int flags = 0)
      : Base(flags), m_cholmodFactor(0)
    {
      cholmod_start(&m_cholmod);
    }

    SparseLLT(const MatrixType& matrix, int flags = 0)
      : Base(flags), m_cholmodFactor(0)
    {
      cholmod_start(&m_cholmod);
      compute(matrix);
    }

    ~SparseLLT()
    {
      if (m_cholmodFactor)
        cholmod_free_factor(&m_cholmodFactor, &m_cholmod);
      cholmod_finish(&m_cholmod);
    }

    inline const CholMatrixType& matrixL() const;

    template<typename Derived>
    bool solveInPlace(MatrixBase<Derived> &b) const;

    void compute(const MatrixType& matrix);

  protected:
    mutable cholmod_common m_cholmod;
    cholmod_factor* m_cholmodFactor;
};

template<typename MatrixType>
void SparseLLT<MatrixType,Cholmod>::compute(const MatrixType& a)
{
  if (m_cholmodFactor)
  {
    cholmod_free_factor(&m_cholmodFactor, &m_cholmod);
    m_cholmodFactor = 0;
  }

  cholmod_sparse A = ei_asCholmodMatrix(const_cast<MatrixType&>(a));
//   m_cholmod.supernodal = CHOLMOD_AUTO;
  // TODO
//   if (m_flags&IncompleteFactorization)
//   {
//     m_cholmod.nmethods = 1;
//     m_cholmod.method[0].ordering = CHOLMOD_NATURAL;
//     m_cholmod.postorder = 0;
//   }
//   else
//   {
//     m_cholmod.nmethods = 1;
//     m_cholmod.method[0].ordering = CHOLMOD_NATURAL;
//     m_cholmod.postorder = 0;
//   }
//   m_cholmod.final_ll = 1;
  m_cholmodFactor = cholmod_analyze(&A, &m_cholmod);
  cholmod_factorize(&A, m_cholmodFactor, &m_cholmod);

  m_status = (m_status & ~SupernodalFactorIsDirty) | MatrixLIsDirty;
}

template<typename MatrixType>
inline const typename SparseLLT<MatrixType,Cholmod>::CholMatrixType&
SparseLLT<MatrixType,Cholmod>::matrixL() const
{
  if (m_status & MatrixLIsDirty)
  {
    ei_assert(!(m_status & SupernodalFactorIsDirty));

    cholmod_sparse* cmRes = cholmod_factor_to_sparse(m_cholmodFactor, &m_cholmod);
    const_cast<typename Base::CholMatrixType&>(m_matrix) = ei_map_cholmod<Scalar,ColMajor,Index>(*cmRes);
    free(cmRes);

    m_status = (m_status & ~MatrixLIsDirty);
  }
  return m_matrix;
}

template<typename MatrixType>
template<typename Derived>
bool SparseLLT<MatrixType,Cholmod>::solveInPlace(MatrixBase<Derived> &b) const
{
  const Index size = m_cholmodFactor->n;
  ei_assert(size==b.rows());

  // this uses Eigen's triangular sparse solver
//   if (m_status & MatrixLIsDirty)
//     matrixL();
//   Base::solveInPlace(b);
  // as long as our own triangular sparse solver is not fully optimal,
  // let's use CHOLMOD's one:
  cholmod_dense cdb = ei_cholmod_map_eigen_to_dense(b);
  //cholmod_dense* x = cholmod_solve(CHOLMOD_LDLt, m_cholmodFactor, &cdb, &m_cholmod);
  cholmod_dense* x = cholmod_solve(CHOLMOD_A, m_cholmodFactor, &cdb, &m_cholmod);
  if(!x)
  {
    std::cerr << "Eigen: cholmod_solve failed\n";
    return false;
  }
  b = Matrix<typename Base::Scalar,Dynamic,1>::Map(reinterpret_cast<typename Base::Scalar*>(x->x),b.rows());
  cholmod_free_dense(&x, &m_cholmod);
  return true;
}

#endif // EIGEN_CHOLMODSUPPORT_H
