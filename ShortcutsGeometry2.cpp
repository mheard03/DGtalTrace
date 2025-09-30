#pragma once

#if defined(ShortcutsGeometry2_RECURSES)
#error Recursive header files inclusion detected in ShortcutsGeometry2.h
#else // defined(ShortcutsGeometry_RECURSES)
/** Prevents recursive inclusion of headers. */
#define ShortcutsGeometry2_RECURSES

#if !defined ShortcutsGeometry2_h
/** Prevents repeated inclusion of headers. */
#define ShortcutsGeometry2_h

//////////////////////////////////////////////////////////////////////////////
#include "DGtal/helpers/Shortcuts.h"
#include "DGtal/geometry/volumes/distance/LpMetric.h"
#include "DGtal/geometry/volumes/distance/ExactPredicateLpSeparableMetric.h"
#include "DGtal/geometry/surfaces/estimation/TrueDigitalSurfaceLocalEstimator.h"
#include "DGtal/geometry/surfaces/estimation/VoronoiCovarianceMeasureOnDigitalSurface.h"
#include "DGtal/geometry/surfaces/estimation/VCMDigitalSurfaceLocalEstimator.h"
#include "DGtal/geometry/surfaces/estimation/IIGeometricFunctors.h"
#include "DGtal/geometry/surfaces/estimation/IntegralInvariantVolumeEstimator.h"
#include "DGtal/geometry/surfaces/estimation/IntegralInvariantCovarianceEstimator.h"

#include "DGtal/dec/DiscreteExteriorCalculusFactory.h"
#include "DGtal/dec/ATSolver2D.h"
//////////////////////////////////////////////////////////////////////////////

namespace DGtal
{
  namespace sgf = ::DGtal::functors::ShapeGeometricFunctors;

  template  < typename TKSpace >
    class ShortcutsGeometry : public Shortcuts< TKSpace >
    {
      BOOST_CONCEPT_ASSERT(( concepts::CCellularGridSpaceND< TKSpace > ));
    public:
      typedef Shortcuts< TKSpace >                     Base;
      typedef ShortcutsGeometry< TKSpace >             Self;
      using Base::parametersKSpace;
      using Base::getKSpace;

      /// Given an arbitrary PointPredicate \a shape: Point -> boolean, a Khalimsky
      /// space \a K, a sequence of \a surfels, and some parameters \a
      /// params, returns the normal Integral Invariant (II) estimation
      /// at the specified surfels, in the same order.
      ///
      /// @tparam TPointPredicate any type of map Point -> boolean.
      /// @param[in] shape a function Point -> boolean telling if you are inside the shape.
      /// @param[in] K the Khalimsky space where the shape and surfels live.
      /// @param[in] surfels the sequence of surfels at which we compute the normals
      /// @param[in] params the parameters:
      ///   - verbose         [     1]: verbose trace mode 0: silent, 1: verbose.
      ///   - r-radius        [   3.0]: the constant for kernel radius parameter r in r(h)=r h^alpha (VCM,II,Trivial).
      ///   - alpha           [  0.33]: the parameter alpha in r(h)=r h^alpha (VCM, II)."
      ///   - gridstep        [   1.0]: the digitization gridstep (often denoted by h).
      ///
      /// @return the vector containing the estimated normals, in the
      /// same order as \a surfels.
      ///
      /// @note Be careful, normals are reoriented with respect to
      /// Trivial normals. If you wish a more robust orientation, use
      /// getCTrivialNormalVectors.
      ///
      /// @note It is better to have surfels in a specific order, as
      /// given for instance by a depth-first traversal (see @ref getSurfelRange)
      template <typename TPointPredicate>
        static RealVectors
        getIINormalVectors( const TPointPredicate&  shape,
                            const KSpace&           K,
                            const SurfelRange&      surfels,
                            const Parameters&       params
                            = parametersGeometryEstimation()
                            | parametersKSpace() )
        {
          typedef functors::IINormalDirectionFunctor<Space> IINormalFunctor;
          typedef IntegralInvariantCovarianceEstimator
            <KSpace, TPointPredicate, IINormalFunctor>          IINormalEstimator;

          RealVectors n_estimations;
          int        verbose = params[ "verbose"   ].as<int>();
          Scalar     h       = params[ "gridstep"  ].as<Scalar>();
          Scalar     r       = params[ "r-radius"  ].as<Scalar>();
          Scalar     alpha   = params[ "alpha"     ].as<Scalar>();
          if ( alpha != 1.0 ) r *= pow( h, alpha-1.0 );
          if ( verbose > 0 )
            {
              trace.info() << "- II normal alpha=" << alpha << std::endl;
              trace.info() << "- II normal r=" << (r*h)  << " (continuous) "
                           << r << " (discrete)" << std::endl;
            }
          IINormalFunctor     functor;
          functor.init( h, r*h );
          IINormalEstimator   ii_estimator( functor );
          ii_estimator.attach( K, shape );
          ii_estimator.setParams( r );
          ii_estimator.init( h, surfels.begin(), surfels.end() );
          ii_estimator.eval( surfels.begin(), surfels.end(),
                             std::back_inserter( n_estimations ) );
          const RealVectors n_trivial = getTrivialNormalVectors( K, surfels );
          orientVectors( n_estimations, n_trivial );
          return n_estimations;
        }
