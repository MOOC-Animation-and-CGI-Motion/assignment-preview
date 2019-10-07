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

#ifndef EIGEN_TAUCSSUPPORT_H
#define EIGEN_TAUCSSUPPORT_H

template<typename MatrixType>
taucs_ccs_matrix ei_asTaucsMatrix(MatrixType& mat)
{
  typedef typename MatrixType::Scalar Scalar;
enum { Flags = MatrixType::Flags };
  taucs_ccs_matrix res;
  res.n         = mat.cols();
  res.m         = mat.rows();
  res.flags     = 0;
  res.colptr    = mat._outerIndexPtr();
  res.rowind    = mat._innerIndexPtr();
  res.values.v  = mat._valuePtr();
  if (ei_is_same_type<Scalar,int>::ret)
    res.flags |= TAUCS_INT;
  else if (ei_is_same_type<Scalar,float>::ret)
    res.flags |= TAUCS_SINGLE;
  else if (ei_is_same_type<Scalar,double>::ret)
    res.flags |= TAUCS_DOUBLE;
  else if (ei_is_same_type<Scalar,std::complex<float> >::ret)
    res.flags |= TAUCS_SCOMPLEX;
  else if (ei_is_same_type<Scalar,std::complex<double> >::ret)
    res.flags |= TAUCS_DCOMPLEX;
  else
  {
    ei_assert(false && "Scalar type not supported by TAUCS");
  }

  // FIXME 1) shapes are not in the Flags and 2) it seems Taucs ignores these flags anyway and only accept lower symmetric matrices
  if (Flags & Upper)
    res.flags |= TAUCS_UPPER;
  if (Flags & Lower)
    res.flags |= TAUCS_LOWER;
  if (Flags & SelfAdjoint)
    res.flags |= (NumTraits<Scalar>::IsComplex ? TAUCS_HERMITIAN : TAUCS_SYMMETRIC);
  else if ((Flags & Upper) || (Flags & Lower))
    res.flags |= TAUCS_TRIANGULAR;

  return res;
}

template<typename Scalar, int Flags, typename Index>
MappedSparseMatrix<Scalar,Flags,Index> ei_map_taucs(taucs_ccs_matrix& taucsMat)
{
  return MappedSparseMatrix<Scalar,Flags,Index>
    (taucsMat.m, taucsMat.n, taucsMat.colptr[taucsMat.n],
     taucsMat.colptr, taucsMat.rowind, reinterpret_cast<Scalar*>(taucsMat.values.v));
}

template<typename MatrixType>
class SparseLLT<MatrixType,Taucs> : public SparseLLT<MatrixType>
{
  protected:
    typedef SparseLLT<MatrixType> Base;
    typedef typename Base::Index Index;
    typedef typename Base::Scalar Scalar;
    typedef typename Base::RealScalar RealScalar;
    typedef typename Base::CholMatrixType CholMatrixType;
    using Base::MatrixLIsDirty;
    using Base::SupernodalFactorIsDirty;
    using Base::m_flags;
    using Base::m_matrix;
    using Base::m_status;
    using Base::m_succeeded;

  public:

    SparseLLT(int flags = SupernodalMultifrontal)
      : Base(flags), m_taucsSupernodalFactor(0)
    {
    }

    SparseLLT(const MatrixType& matrix, int flags = SupernodalMultifrontal)
      : Base(flags), m_taucsSupernodalFactor(0)
    {
      compute(matrix);
    }

    ~SparseLLT()
    {
      if (m_taucsSupernodalFactor)
        taucs_supernodal_factor_free(m_taucsSupernodalFactor);
    }

    inline const CholMatrixType& matrixL() const;

    template<typename Derived>
    void solveInPlace(MatrixBase<Derived> &b) const;

    void compute(const MatrixType& matrix);

  protected:
    void* m_taucsSupernodalFactor;
};

template<typename MatrixType>
void SparseLLT<MatrixType,Taucs>::compute(const MatrixType& a)
{
  if (m_taucsSupernodalFactor)
  {
    taucs_supernodal_factor_free(m_taucsSupernodalFactor);
    m_taucsSupernodalFactor = 0;
  }

  taucs_ccs_matrix taucsMatA = ei_asTaucsMatrix(const_cast<MatrixType&>(a));

  if (m_flags & IncompleteFactorization)
  {
    taucs_ccs_matrix* taucsRes = taucs_ccs_factor_llt(&taucsMatA, Base::m_precision, 0);
    if(!taucsRes)
    {
      m_succeeded = false;
      return;
    }
    // the matrix returned by Taucs is not necessarily sorted,
    // so let's copy it in two steps
    DynamicSparseMatrix<Scalar,RowMajor> tmp = ei_map_taucs<Scalar,ColMajor,Index>(*taucsRes);
    m_matrix = tmp;
    free(taucsRes);
    m_status = (m_status & ~(CompleteFactorization|MatrixLIsDirty))
             | IncompleteFactorization
             | SupernodalFactorIsDirty;
  }
  else
  {
    if ( (m_flags & SupernodalLeftLooking)
      || ((!(m_flags & SupernodalMultifrontal)) && (m_flags & MemoryEfficient)) )
    {
      m_taucsSupernodalFactor = taucs_ccs_factor_llt_ll(&taucsMatA);
    }
    else
    {
      // use the faster Multifrontal routine
      m_taucsSupernodalFactor = taucs_ccs_factor_llt_mf(&taucsMatA);
    }
    m_status = (m_status & ~IncompleteFactorization) | CompleteFactorization | MatrixLIsDirty;
  }
  m_succeeded = true;
}

template<typename MatrixType>
inline const typename SparseLLT<MatrixType,Taucs>::CholMatrixType&
SparseLLT<MatrixType,Taucs>::matrixL() const
{
  if (m_status & MatrixLIsDirty)
  {
    ei_assert(!(m_status & SupernodalFactorIsDirty));

    taucs_ccs_matrix* taucsL = taucs_supernodal_factor_to_ccs(m_taucsSupernodalFactor);

    // the matrix returned by Taucs is not necessarily sorted,
    // so let's copy it in two steps
    DynamicSparseMatrix<Scalar,RowMajor> tmp = ei_map_taucs<Scalar,ColMajor,Index>(*taucsL);
    const_cast<typename Base::CholMatrixType&>(m_matrix) = tmp;
    free(taucsL);
    m_status = (m_status & ~MatrixLIsDirty);
  }
  return m_matrix;
}

template<typename MatrixType>
template<typename Derived>
void SparseLLT<MatrixType,Taucs>::solveInPlace(MatrixBase<Derived> &b) const
{
  bool inputIsCompatibleWithTaucs = (Derived::Flags&RowMajorBit)==0;

  if (!inputIsCompatibleWithTaucs)
  {
    matrixL();
    Base::solveInPlace(b);
  }
  else if (m_flags & IncompleteFactorization)
  {
    taucs_ccs_matrix taucsLLT = ei_asTaucsMatrix(const_cast<typename Base::CholMatrixType&>(m_matrix));
    typename ei_plain_matrix_type<Derived>::type x(b.rows());
    for (int j=0; j<b.cols(); ++j)
    {
      taucs_ccs_solve_llt(&taucsLLT,x.data(),&b.col(j).coeffRef(0));
      b.col(j) = x;
    }
  }
  else
  {
    typename ei_plain_matrix_type<Derived>::type x(b.rows());
    for (int j=0; j<b.cols(); ++j)
    {
      taucs_supernodal_solve_llt(m_taucsSupernodalFactor,x.data(),&b.col(j).coeffRef(0));
      b.col(j) = x;
    }
  }
}

#endif // EIGEN_TAUCSSUPPORT_H
