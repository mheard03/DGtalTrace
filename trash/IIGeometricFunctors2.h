/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#pragma once

/**
 * @file IIGeometricFunctors.h
 * @author Jacques-Olivier Lachaud (\c jacques-olivier.lachaud@univ-savoie.fr )
 * Laboratory of Mathematics (CNRS, UMR 5127), University of Savoie, France
 *
 * @date 2014/05/14
 *
 * Header file for module IIGeometricFunctors.cpp
 *
 * This file is part of the DGtal library.
 */

#include <DGtal/topology/KhalimskySpaceND.h>
#include <exception>
#if defined(IIGeometricFunctors2_RECURSES)
#error Recursive header files inclusion detected in IIGeometricFunctors.h
#else // defined(IIGeometricFunctors2_RECURSES)
/** Prevents recursive inclusion of headers. */
#define IIGeometricFunctors2_RECURSES

#if !defined IIGeometricFunctors2_h
/** Prevents repeated inclusion of headers. */
#define IIGeometricFunctors2_h

//////////////////////////////////////////////////////////////////////////////
// Inclusions
#include <iostream>
#include <tuple>
#include <stdexcept>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/geometry/surfaces/estimation/IIGeometricFunctors.h>
//////////////////////////////////////////////////////////////////////////////

// @since 0.8 In DGtal::functors
namespace DGtal {
  namespace functors {

    using namespace DGtal;
    using namespace Z2i;

    /////////////////////////////////////////////////////////////////////////////
    // template class IIPrincipalCurvaturesAndDirectionsFunctor
    /**
    * Description of template class
    * 'IIPrincipalCurvaturesAndDirectionsFunctor' <p> \brief Aim: A functor
    * Matrix -> std::pair<RealVector,RealVector> that returns the first and
    * the second principal curvature directions by diagonalizing the given
    * covariance matrix. This functor is valid only for 3D space.
    * Note that by second we mean the direction with second greatest curvature
    * in absolute value.
    *
    * @tparam TSpace a model of CSpace, for instance SpaceND.
    * @tparam TMatrix a model of CMatrix, for instance SimpleMatrix.
    *
    * @see IntegralInvariantCovarianceEstimator
    */
    template  <>
    class IIPrincipalCurvaturesAndDirectionsFunctor<Space>
    {
      // ----------------------- Standard services ------------------------------
    public:
      typedef Space TSpace;
      typedef SimpleMatrix<typename TSpace::RealVector::Component, TSpace::dimension, TSpace::dimension> TMatrix;

      typedef IIPrincipalCurvaturesAndDirectionsFunctor<TSpace> Self;
      typedef TSpace Space;
      typedef typename Space::RealVector RealVector;
      typedef typename RealVector::Component Component;
      typedef TMatrix Matrix;
      typedef Matrix Argument;
      typedef std::tuple<double, double, RealVector, RealVector> Quantity;
      typedef Quantity Value;

       /**
      * Apply operator.
      * @param arg any symmetric positive matrix (covariance matrix
      *
      * @return the principal curvature values and directions in
      * a std::tuple.
      */
      Value operator()( const Argument& arg ) const
      {
        throw std::runtime_error("IIPrincipalCurvaturesAndDirectionsFunctor is not valid in R2");
      }

      /**
      * Initializes the functor with the gridstep and the ball
      * Euclidean radius.
       */
      void init( Component h , Component  r )
      {
        throw std::runtime_error("IIPrincipalCurvaturesAndDirectionsFunctor is not valid in R2");
      }
    };


} // namespace functors

} // namespace DGtal


//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#endif // !defined IIGeometricFunctors_h

#undef IIGeometricFunctors2_RECURSES
#endif // else defined(IIGeometricFunctors2_RECURSES)
