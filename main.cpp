#include <iostream>
#include <DGtal/base/Common.h>
#include <DGtal/images/Image.h>
#include <DGtal/images/ImageSelector.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/helpers/Shortcuts.h>
#include <DGtal/helpers/ShortcutsGeometry.h>
#include <DGtal/io/readers/STBReader.h>
#include <polyscope/curve_network.h>
#include <string>
#include <unordered_map>
#include <DGtal/geometry/curves/ArithmeticalDSSComputer.h>
#include <DGtal/geometry/curves/SaturatedSegmentation.h>


#include <DGtal/shapes/parametric/Flower2D.h>
#include <DGtal/shapes/GaussDigitizer.h>
#include <DGtal/topology/LightImplicitDigitalSurface.h>
#include <DGtal/topology/DigitalSurface.h>
#include <DGtal/graph/DepthFirstVisitor.h>
#include <DGtal/graph/GraphVisitorRange.h>
#include <DGtal/images/ImageHelper.h>
#include <DGtal/images/ImageSelector.h>

/// Estimator
#include <DGtal/geometry/surfaces/estimation/IIGeometricFunctors.h>
#include <DGtal/geometry/surfaces/estimation/IntegralInvariantVolumeEstimator.h>

// Drawing
#include <DGtal/io/boards/Board2D.h>
#include <DGtal/io/colormaps/GradientColorMap.h>

// Polyscope
#include <polyscope/pick.h>
#include <polyscope/polyscope.h>
#include <polyscope/surface_mesh.h>
#include <polyscope/point_cloud.h>

#include "DataTable.hpp"

using namespace DGtal;
using namespace Z2i;


int main(int argc, char** argv) {
      /// Construction of the shape + digitalization
    double h = 0.5;

    typedef Surfaces<Z2i::KSpace> Surf;
    typedef Flower2D< Z2i::Space > MyShape;
    typedef GaussDigitizer< Z2i::Space, MyShape > MyGaussDigitizer;
    typedef LightImplicitDigitalSurface< Z2i::KSpace, MyGaussDigitizer > LightImplicitDigSurface;
    typedef DigitalSurface< LightImplicitDigSurface > MyDigitalSurface;

    MyShape shape(0, 0, 5.00000124, 10.0000123, 6, 3.0);

    MyGaussDigitizer digShape;
    digShape.attach( shape );
    digShape.init( shape.getLowerBound() + Z2i::Point::diagonal(-1), shape.getUpperBound() + Z2i::Point::diagonal(1), h );
    Z2i::Domain domainShape = digShape.getDomain();
    Z2i::KSpace KSpaceShape;
    bool space_ok = KSpaceShape.init( domainShape.lowerBound(), domainShape.upperBound(), true );
    if ( !space_ok )
    {
        trace.error() << "Error in the Khamisky space construction." << std::endl;
        return 2;
    }

    typedef ImageSelector< Z2i::Domain, unsigned int >::Type Image;
    Image image( domainShape );
    DGtal::imageFromRangeAndValue( domainShape.begin(), domainShape.end(), image );

    SurfelAdjacency<Z2i::KSpace::dimension> SAdj( true );
    Z2i::SCell bel = Surf::findABel( KSpaceShape, digShape, 1000 );
    std::vector<Z2i::SCell> surfels;
    Surf::track2DBoundary(surfels, KSpaceShape, SAdj, digShape, bel);
    GridCurve<Z2i::KSpace> gc(KSpaceShape);
    gc.initFromSCellsVector(surfels);
    auto pr = gc.getPointsRange();
    std::vector<Z2i::Point> boundaryPoints(pr.begin(), pr.end());
    if (gc.isClosed()) {
      boundaryPoints.push_back(boundaryPoints[0]);
    }

    trace.info() << "Measuring..." << std::endl;

    /// Integral Invariant stuff
    //! [IntegralInvariantUsage]
    double radius = 2.0 * pow(h, 1. / 3.);

    typedef functors::IINormalDirectionFunctor<Z2i::Space> MyIINormalFunctor;
    typedef IntegralInvariantCovarianceEstimator< Z2i::KSpace, MyGaussDigitizer, MyIINormalFunctor > MyIINormalEstimator;
    typedef MyIINormalFunctor::Value Value;

    MyIINormalFunctor normalFunctor; /// Functor used to convert volume -> curvature
    normalFunctor.init( h, radius ); // Initialisation for a grid step and a given Euclidean radius of convolution kernel

    MyIINormalEstimator normalEstimator( normalFunctor ); 
    normalEstimator.attach( KSpaceShape, digShape ); /// Setting a KSpace and a predicate on the object to evaluate
    normalEstimator.setParams( radius/h ); /// Setting the digital radius of the convolution kernel
    normalEstimator.init( h, surfels.begin(), surfels.end() ); /// Initialisation for a given h

    std::vector< Value > results;
    // std::back_insert_iterator< std::vector< Value > > resultsIt( results ); /// output iterator for results of Integral Invariant curvature computation
    // normalEstimator.eval( surfels.begin(), surfels.end(), resultsIt ); /// Computation

    for (auto& surfel : surfels) {
      RealPoint normalEstimated = normalEstimator.eval(&surfel).getNormalized();
      results.push_back(normalEstimated);
    }
    //! [IntegralInvariantUsage]


    /// Drawing results
    trace.info() << "Drawing..." << std::endl;
    polyscope::init();
    polyscope::view::style = polyscope::view::NavigateStyle::Planar;
    auto displayCurve = polyscope::registerCurveNetworkLine2D("curve", boundaryPoints);
    displayCurve->addEdgeVectorQuantity2D("curve", results);


    polyscope::show();
    return 0;

}
