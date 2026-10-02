#pragma once
#include "DGtal/base/Common.h"
#include "DGtal/base/ConstAlias.h"
#include "DGtal/topology/DigitalSurface.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>

#include "DGtal/helpers/Shortcuts.h"
#include "DGtal/helpers/ShortcutsGeometry.h"
#include "DGtal/helpers/StdDefs.h"

namespace DGtal {

/////////////////////////////////////////////////////////////////////////////
// template class DigitalSurfaceRegularization
/**
 * Description of template class 'DigitalSurfaceRegularization' <p>
 * \brief Aim: Implements Digital Surface Regularization as described in @cite
 * coeurjolly17regDGCI.
 *
 *  Given a digital surface and a normal vector field attached to the surfels,
 * this class performs a regularization of the digital surface vertices to
 * obtain a piecewise smooth surface such that each quad of the output surface
 * is as perpendicular as possible to the given input normal vector field.
 *
 * @note The digital surface must be a closed.
 *
 * If @f$P@f$ denotes the vertices of the input digital surface, @f$F@f$ the set
 * of (quadrilateral) faces and @f$n_f@f$ an estimated normal vector on the face
 * @f$f@f$, we want the quad surface vertex positions @f$P^*@f$ that minimizes
 * the following energy function:
 * @f[\mathcal{E}(P) := \alpha \sum_{i=1}^{n} \|p_i - \hat{p}_i\|^2  +
 *      \beta \sum_{f\in F} \sum_{{e_j} \in \partial{f} } ( e_j \cdot n_{f} )^2
 * + \gamma \sum_{i=1}^{n} \|\hat{p}_i - \hat{b}_i\|^2\,.@f] where @f$"\cdot"@f$
 * is the standard @f$\mathbb{R}^3@f$ scalar product, @f$e_j\in
 * \partial{f}@f$ is an edge of the face @f$f@f$ (and is equal to some @f$p_k -
 * p_l@f$) and @f$ \hat{b}_i@f$ is the barycenter of the vertices adjacent to
 * @f$\hat{p}_i@f$.
 *
 * To minimize this energy, instead of solving the associated sparse linear
 * system as described in @cite coeurjolly17regDGCI, we perform a gradient
 * descent strategy which allows us a finer control over the vertices
 * displacement (see advection methods).
 *
 * @see testDigitalSurfaceRegularization.cpp
 *
 * @tparam TDigitalSurface a Digital Surface type (see DigitalSurface).
 */
template <typename TDigitalSurface> class DigitalSurfaceRegularization {
  // ----------------------- Standard services ------------------------------
public:
  /// DigitalSurface type
  typedef TDigitalSurface DigSurface;
  /// Digital Surface Container type
  typedef
      typename TDigitalSurface::DigitalSurfaceContainer DigitalSurfaceContainer;
  BOOST_CONCEPT_ASSERT(
      (concepts::CDigitalSurfaceContainer<DigitalSurfaceContainer>));

  /// We rely on the Shortcuts 2D types
  typedef Shortcuts<Z2i::KSpace> SH2;


  /// Pointels position container
  typedef std::vector<Z2i::RealPoint> Positions;

  /// Pointels position container
  typedef std::vector<Z2i::RealVector> Normals;

  /**
   * Default constructor.
   */
  DigitalSurfaceRegularization(CountedPtr<DigSurface> aDigSurf)
      : myInit(false), myVerbose(false), myDigitalSurface(aDigSurf) {
    myK = SH2::getKSpace(myDigitalSurface);
  }

  /**
   * Destructor.
   */
  ~DigitalSurfaceRegularization() = default;

  /**
   * Copy constructor.
   * @param other the object to clone.
   */
  DigitalSurfaceRegularization(const DigitalSurfaceRegularization &other) =
      delete;

  /**
   * Move constructor.
   * @param other the object to move.
   */
  DigitalSurfaceRegularization(DigitalSurfaceRegularization &&other) = delete;

  /**
   * Copy assignment operator.
   * @param other the object to copy.
   * @return a reference on 'this'.
   */
  DigitalSurfaceRegularization &
  operator=(const DigitalSurfaceRegularization &other) = delete;

  /**
   * Move assignment operator.
   * @param other the object to move.
   * @return a reference on 'this'.
   */
  DigitalSurfaceRegularization &
  operator=(DigitalSurfaceRegularization &&other) = delete;

  // ----------------------- Interface --------------------------------------
public:
  /**
   * Attach normal vectors from a generic function that associates
   * a normal vector to each surfel.
   *
   * @param normalFunc a function (or a lambda, or a functor) that maps
   * surfels (SH2::SCell) to normal vectors (SH2::RealVector).
   */
  void attachNormalVectors(
      const std::function<Z2i::RealVector(SH2::SCell &)> &normalFunc) {
    ASSERT_MSG(myInit,
               "The init() method must be called before setting the normals");
    myNormals.resize(mySurfels.size());
    for (auto i = 0u; i < mySurfels.size(); ++i)
      myNormals[i] = normalFunc(mySurfels[i]);
  }

  /**
   * Attach convolved trivial normal vectors to the digital surface
   * (cf ShortCutsGeometry::getCTrivialNormalVectors).
   *
   * An important parameter is the radius used to estimate the normal vectors
   * (@a t-ring, default=3.0).
   *
   * @param someParams the parameters (@a t-ring, default=3.0 used in the local
   * convolution).
   */
  void attachConvolvedTrivialNormalVectors(
      const Parameters someParams = SH2::defaultParameters() |
                                    ShortcutsGeometry<Z3i::KSpace>::defaultParameters()) {
    ASSERT_MSG(myInit,
               "The init() method must be called before setting the normals");
    myNormals =
        getCTrivialNormalVectors(myDigitalSurface, mySurfels, someParams);
  }


  /// Given a digital surface \a surface, a sequence of \a surfels,
  /// and some parameters \a params, returns the convolved trivial
  /// normal vector estimations at the specified surfels, in the
  /// same order.
  ///
  /// @tparam TAnyDigitalSurface either kind of DigitalSurface, like ShortcutsGeometry::LightDigitalSurface or ShortcutsGeometry::DigitalSurface.
  ///
  /// @param[in] surface the digital surface
  /// @param[in] surfels the sequence of surfels at which we compute the normals
  /// @param[in] params the parameters:
  ///   - verbose         [     1]: verbose trace mode 0: silent, 1: verbose.
  ///   - t-ring          [   3.0]: the radius used when computing convolved trivial normals (it is a graph distance, not related to the grid step).
  ///
  /// @return the vector containing the estimated normals, in the
  /// same order as \a surfels.
  template <typename TAnyDigitalSurface>
    static std::vector<Z2i::RealVector>
    getCTrivialNormalVectors
    ( CountedPtr<TAnyDigitalSurface> surface,
      const std::vector<Z2i::KSpace::Surfel>&             surfels,
      const Parameters&              params)
    {
      typedef Z2i::RealVector RealVector;
      typedef RealVector::Component  Scalar;
      typedef Z2i::KSpace KSpace;
      typedef KSpace::Space Space;
      typedef KSpace::Surfel Surfel;

      int    verbose = params[ "verbose"  ].as<int>();
      Scalar       t = params[ "t-ring"   ].as<double>();
      typedef typename TAnyDigitalSurface::DigitalSurfaceContainer  SurfaceContainer;
      typedef LpMetric<Space>                                       Metric;
      typedef functors::HatFunction<Scalar>                         Functor;
      typedef functors::ElementaryConvolutionNormalVectorEstimator
        < Surfel, CanonicSCellEmbedder<KSpace> >                    SurfelFunctor;
      typedef LocalEstimatorFromSurfelFunctorAdapter
        < SurfaceContainer, Metric, SurfelFunctor, Functor>         NormalEstimator;
      if ( verbose > 0 )
        trace.info() << "- CTrivial normal t-ring=" << t << " (discrete)" << std::endl;
      const Functor fct( 1.0, t );
      const KSpace &  K = surface->container().space();
      Metric    aMetric( 2.0 );
      CanonicSCellEmbedder<KSpace> canonic_embedder( K );
      std::vector< RealVector >    n_estimations;
      SurfelFunctor                surfelFct( canonic_embedder, 1.0 );
      NormalEstimator              estimator;
      estimator.attach( *surface);
      estimator.setParams( aMetric, surfelFct, fct, t );
      estimator.init( 1.0, surfels.begin(), surfels.end());
      estimator.eval( surfels.begin(), surfels.end(),
                      std::back_inserter( n_estimations ) );
      std::transform( n_estimations.cbegin(), n_estimations.cend(), n_estimations.begin(),
                      [] ( RealVector v ) { return -v; } );
      return n_estimations;
    }


  /**
   * @brief Initialize the parameters of the energy function.
   *
   * This init() method attaches the same weights to all vertices.
   *
   * @param [in] alpha the data attachment term coeff. (default=0.001)
   * @param [in] beta  the alignment term coeff. (default=1.0)
   * @param [in] gamma the fairness term coeef. (default=0.05)
   */
  void init(const double alpha = 0.001, const double beta = 1.0,
            const double gamma = 0.05) {
    myAlpha = alpha;
    myBeta = beta;
    myGamma = gamma;
    myInit = true;
    myConstantCoeffs = true;
    cacheInit();
  }

  /**
   * @brief Initialize the parameters of the energy function.
   *
   * This init() method considers local weights per vertex (all vectors
   * must have the same size: the number of pointels of the original surface).
   *
   * @param [in] alphas the data attachment term coeff.
   * @param [in] betas  the alignment term coeff.
   * @param [in] gammas the fairness term coeef.
   */
  void init(ConstAlias<std::vector<double>> alphas,
            ConstAlias<std::vector<double>> betas,
            ConstAlias<std::vector<double>> gammas) {
    myAlpha = 0.0;
    myBeta = 0.0;
    myGamma = 0.0;

    myAlphas = &alphas;
    myBetas = &betas;
    myGammas = &gammas;

    myInit = true;
    myConstantCoeffs = false;
    cacheInit();

    ASSERT_MSG(alphas->size() == myOriginalPositions.size(),
               "The alpha vector size must equal the number of pointels");
    ASSERT_MSG(betas->size() == myOriginalPositions.size(),
               "The beta vector size must equal the number of pointels");
    ASSERT_MSG(gammas->size() == myOriginalPositions.size(),
               "The gamma vector size must equal the number of pointels");
  }

  /**
   * Compute the energy gradient vector and return the energy value.
   *
   * @note init() method must have been called and normal vectors
   * must be have been attached to the surfels.
   *
   * @return the energy value.
   **/
  double computeGradient() {
    double energy = 0.0;
    auto zero = SH2::RealPoint(0, 0, 0);

    ASSERT_MSG(
        myInit,
        "The init() method must be called before computing the gradient");
    ASSERT_MSG(myNormals.size() != 0,
               "Some normal vectors must be attached to the digital surface "
               "before computing the gradient");

    // data attachment term
    for (size_t i = 0; i < myOriginalPositions.size(); ++i) {
      const auto delta_d = myOriginalPositions[i] - myRegularizedPositions[i];
      energy += myAlpha * delta_d.squaredNorm();
      myGradient[i] = 2.0 * myAlpha * delta_d;
      // reset for align
      myGradientAlign[i] = zero;
    }

    // align
    auto it = myAlignPointelsIdx.cbegin();
    for (size_t i = 0; i < mySurfels.size(); ++i) {
      const auto cell_p0 = *it++;
      const auto cell_p1 = *it++;
      const auto cell_p2 = *it++;
      const auto cell_p3 = *it++;
      const auto e0 =
          myRegularizedPositions[cell_p0] - myRegularizedPositions[cell_p1];
      const auto e1 =
          myRegularizedPositions[cell_p1] - myRegularizedPositions[cell_p2];
      const auto e2 =
          myRegularizedPositions[cell_p2] - myRegularizedPositions[cell_p3];
      const auto e3 =
          myRegularizedPositions[cell_p3] - myRegularizedPositions[cell_p0];
      const auto cos_a0 = e0.dot(myNormals[i]);
      const auto cos_a1 = e1.dot(myNormals[i]);
      const auto cos_a2 = e2.dot(myNormals[i]);
      const auto cos_a3 = e3.dot(myNormals[i]);
      energy += myBeta * (cos_a0 * cos_a0 + cos_a1 * cos_a1 + cos_a2 * cos_a2 +
                          cos_a3 * cos_a3);
      myGradientAlign[cell_p0] += cos_a0 * myNormals[i];
      myGradientAlign[cell_p1] += cos_a1 * myNormals[i];
      myGradientAlign[cell_p2] += cos_a2 * myNormals[i];
      myGradientAlign[cell_p3] += cos_a3 * myNormals[i];
    }
    for (auto i = 0u; i < myOriginalPositions.size(); ++i) {
      ASSERT(myNumberAdjEdgesToPointel[i] > 0);
      myGradient[i] += 2.0 * myBeta * myGradientAlign[i] /
                       (double)myNumberAdjEdgesToPointel[i];
    }

    // fairness
    auto itP = myFairnessPointelsIdx.cbegin();
    auto itNb = myNbAdjacent.cbegin();
    SH2::RealPoint barycenter, phat;
    for (size_t faceId = 0; faceId < myFaces.size(); ++faceId) {
      auto idx = *itP++;
      unsigned int nbAdj = *itNb++;
      barycenter = zero;
      phat = myRegularizedPositions[idx];
      for (unsigned int i = 0; i < nbAdj; ++i)
        barycenter += myRegularizedPositions[*itP++];
      ASSERT(nbAdj > 0);
      barycenter /= (double)nbAdj;
      auto delta_f = phat - barycenter;
      energy += myGamma * delta_f.squaredNorm();
      myGradient[idx] += 2.0 * myGamma * delta_f;
    }

    return energy;
  }

  /**
   * Compute the energy gradient vector and return the energy value.
   *
   * This method assumes that you have local alpha, beta and gamma weights.
   *
   * @note init(alphas,betas,gammas) method must have been called and normal
   * vectors must have been attached to the surfels.
   *
   * @return the energy value.
   **/
  double computeGradientLocalWeights() {
    double energy = 0.0;
    auto zero = SH2::RealPoint(0, 0, 0);

    ASSERT_MSG(
        myInit,
        "The init() method must be called before computing the gradient");
    ASSERT_MSG(myNormals.size() != 0,
               "Some normal vectors must be attached to the digital surface "
               "before computing the gradient");

    // data attachment term
    for (size_t i = 0; i < myOriginalPositions.size(); ++i) {
      const auto delta_d = myOriginalPositions[i] - myRegularizedPositions[i];
      energy += (*myAlphas)[i] * delta_d.squaredNorm();
      myGradient[i] = 2.0 * (*myAlphas)[i] * delta_d;
      // reset for align
      myGradientAlign[i] = zero;
    }

    // align
    auto it = myAlignPointelsIdx.cbegin();
    for (size_t i = 0; i < mySurfels.size(); ++i) {
      const auto cell_p0 = *it++;
      const auto cell_p1 = *it++;
      const auto cell_p2 = *it++;
      const auto cell_p3 = *it++;
      const auto e0 =
          myRegularizedPositions[cell_p0] - myRegularizedPositions[cell_p1];
      const auto e1 =
          myRegularizedPositions[cell_p1] - myRegularizedPositions[cell_p2];
      const auto e2 =
          myRegularizedPositions[cell_p2] - myRegularizedPositions[cell_p3];
      const auto e3 =
          myRegularizedPositions[cell_p3] - myRegularizedPositions[cell_p0];
      const auto cos_a0 = e0.dot(myNormals[i]);
      const auto cos_a1 = e1.dot(myNormals[i]);
      const auto cos_a2 = e2.dot(myNormals[i]);
      const auto cos_a3 = e3.dot(myNormals[i]);
      energy += (*myBetas)[cell_p0] * cos_a0 * cos_a0 +
                (*myBetas)[cell_p1] * cos_a1 * cos_a1 +
                (*myBetas)[cell_p2] * cos_a2 * cos_a2 +
                (*myBetas)[cell_p3] * cos_a3 * cos_a3;
      myGradientAlign[cell_p0] += cos_a0 * myNormals[i];
      myGradientAlign[cell_p1] += cos_a1 * myNormals[i];
      myGradientAlign[cell_p2] += cos_a2 * myNormals[i];
      myGradientAlign[cell_p3] += cos_a3 * myNormals[i];
    }
    for (size_t i = 0; i < myOriginalPositions.size(); ++i) {
      ASSERT(myNumberAdjEdgesToPointel[i] > 0);
      myGradient[i] += 2.0 * (*myBetas)[i] * myGradientAlign[i] /
                       (double)myNumberAdjEdgesToPointel[i];
    }

    // fairness
    auto itP = myFairnessPointelsIdx.cbegin();
    auto itNb = myNbAdjacent.cbegin();
    SH2::RealPoint barycenter, phat;
    for (auto faceId = 0u; faceId < myFaces.size(); ++faceId) {
      const auto idx = *itP++;
      const unsigned int nbAdj = *itNb++;
      barycenter = zero;
      phat = myRegularizedPositions[idx];
      for (unsigned int i = 0; i < nbAdj; ++i)
        barycenter += myRegularizedPositions[*itP++];
      ASSERT(nbAdj > 0);
      barycenter /= (double)nbAdj;
      auto delta_f = phat - barycenter;
      energy += (*myGammas)[idx] * delta_f.squaredNorm();
      myGradient[idx] += 2.0 * (*myGammas)[idx] * delta_f;
    }

    return energy;
  }

  /**
   * @brief Main regularization loop.
   *
   * This method performs the main minimization loop of the energy
   * using a gradient descent scheme (with automatic update of the learning
   * step). The iterative process stops either when the number of steps reaches
   * @a nbIters, or when the @f$l_\infty@f$ norm of the energy gradient is below
   * @a epsilon.
   *
   * The last parameter is a function that describes how regularized points are
   * advected during the gradient descent. By default, points are shifted by a
   * fraction of the energy gradient vector (and the default function is thus
   * @f$ p \leftarrow p + v@f$ with
   * @f$ v = -dt  \nabla E_p@f$). See @see clampedAdvection for another
   * advection strategy.
   *
   * The energy at the final step is returned.
   *
   * @param [in] nbIters maximum number of steps
   * @param [in] dt initial learning rate
   * @param [in] epsilon minimum l_infity norm of the gradient vector
   * @param [in] advectionFunc advection function/functor/lambda to move a
   * regularized point &a p associated with the original point @a o w.r.t to a
   * displacement vector @a v (default = p+v)
   * @tparam AdvectionFunction type of advection function, functor or lambda
   * (RealPoint, RealPoint, RealVector)->RealPoint.
   * @return the energy at the final step.
   */
  template <typename AdvectionFunction>
  double regularize(const unsigned int nbIters, const double dt,
                    const double epsilon,
                    const AdvectionFunction &advectionFunc) {
    double energy = 0.0;
    double last_energy = 0.0;
    double mydt = dt;
    bool first_iter = true;
    for (unsigned int i = 0; i < nbIters; ++i) {
      if (myConstantCoeffs)
        energy = computeGradient();
      else
        energy = computeGradientLocalWeights();

      double gradnorm = 0.0;
      for (auto &v : myGradient)
        gradnorm = std::max(gradnorm, v.norm());

      if (myVerbose)
        trace.info() << "Step " << i << " dt=" << mydt << " energy = " << energy
                     << " gradnorm= " << gradnorm << std::endl;

      // Naive linesearch by doubling the learning rate
      if (!first_iter)
        mydt *= (energy > last_energy) ? 0.5 : 1.1;

      // Stopping criterion
      if (mydt < epsilon)
        return energy;

      last_energy = energy;
      first_iter = false;

      // One step advection
      Z2i::RealVector v;
      for (size_t ii = 0; ii < myRegularizedPositions.size(); ++ii) {
        v = -mydt * myGradient[ii];
        advectionFunc(myRegularizedPositions[ii], myOriginalPositions[ii], v);
      }
    }
    return energy;
  }
  /**
   * @brief Main regularization loop.
   *
   * This method performs the main minimization loop of the energy
   * using a gradient descent scheme (with automatic update of the learning
   * step). The iterative process stops either when the number of steps reaches
   * @a nbIters, or when the @f$l_\infty@f$ norm of the energy gradient is below
   * @a epsilon.
   *
   * This methods uses the default advection function @f$ p = p + v@f$.
   *
   * The energy at the final step is returned.
   *
   * @param [in] nbIters maximum number of steps (default=200)
   * @param [in] dt initial learning rate (default = 1.0)
   * @param [in] epsilon minimum l_infity norm of the gradient vector (default =
   * 0.0001)
   * @return the energy at the final step.
   */
  double regularize(const unsigned int nbIters = 200, const double dt = 1.0,
                    const double epsilon = 0.0001) {
    return regularize(
        nbIters, dt, epsilon,
        [](Z2i::RealPoint &p, Z2i::RealPoint &o, Z2i::RealVector &v) {
          (void)o;
          p += v;
        });
  }

  /**
   * Static method to be used in @e regularize() that
   * clamps to regularized point @a p when shifted by @a v
   * in the unit cube centered at @a orig.
   *
   * @param [in,out] p the point to advect.
   * @param [in] orig the associated point in the original surface.
   * @param [in] v the advection vector.
   */
  static void clampedAdvection(Z2i::RealPoint &p, const Z2i::RealPoint &orig,
                               const Z2i::RealVector &v) {
    p += v;
    for (auto i = 0; i < 3; ++i)
      if ((p[i] - orig[i]) > 0.5)
        p[i] = orig[i] + 0.5;
      else if ((p[i] - orig[i]) < -0.5)
        p[i] = orig[i] - 0.5;
  }

  /**
   * @returns the regularized position of a given pointel @a aPointel.
   * @param[in] aPointel the digital surface pointel.
   *
   * @note the init() method must have been called. For relevant results, the
   * regularize() methods should have been also called before accessing the new
   * positions.
   */
  Z2i::RealPoint getRegularizedPosition(const SH2::Cell &aPointel) {
    ASSERT_MSG(myInit, "The init() method must be called first.");
    ASSERT_MSG(myK.uDim(aPointel) == 0, "The cell must be a pointel (0-cell)");
    return myRegularizedPositions[myPointelIndex[aPointel]];
  }

  /**
   * @return the regularized vertices positions
   * (see getCellIndex for the Cell->Index map).
   * @note the init() method must have been called.
   */
  const Positions &getRegularizedPositions() const {
    ASSERT_MSG(myInit, "The init() method must be called first.");
    return myRegularizedPositions;
  }
  /**
   * @return the input vertices positions
   * (see getCellIndex for the Cell->Index map).
   * @note the init() method must have been called.
   */
  const Positions &getOriginalPositions() const {
    ASSERT_MSG(myInit, "The init() method must be called first.");
    return myOriginalPositions;
  }
  /**
   * @return the CellIndex (Cell->Index map) for
   * the positions and normal vectors containers.
   * @note the init() method must have been called.
   */
  const SH2::Cell2Index &getCellIndex() const {
    ASSERT_MSG(myInit, "The init() method must be called first.");
    return myPointelIndex;
  }
  /**
   * @return the input normal vectors
   * (see getSurfelIndex for the Cell->Index map).
   * @note the init() method must have been called.
   */
  const Normals &getNormalVectors() const {
    ASSERT_MSG(myInit, "The init() method must be called first.");
    return myNormals;
  }

  /**
   * @return the CellIndex (Cell->Index map) for
   the positions and normal vectors containers.
   * @note the init() method must have been called.
   */
  const SH2::Surfel2Index &getSurfelIndex() const {
    ASSERT_MSG(myInit, "The init() method must be called first.");
    return mySurfelIndex;
  }

  /**
   * Reset the regularized vertices positions to the original one.
   * @note the init() method must have been called.
   */
  void reset() {
    ASSERT_MSG(myInit, "The init() method must be called first.");
    std::copy(myOriginalPositions.begin(), myOriginalPositions.end(),
              myRegularizedPositions.begin());
  }

  // ----------------------- Services --------------------------------------

  /**
   * Writes/Displays the object on an output stream.
   * @param out the output stream where the object is written.
   */
  void selfDisplay(std::ostream &out) const {
    out << "[DigitalSurfaceRegularization] alpha= " << myAlpha
        << " beta= " << myBeta << " gamma= " << myGamma
        << " number of surfels= " << mySurfels.size();
  }

  /**
   * Checks the validity/consistency of the object.
   * @return 'true' if the object is valid, 'false' otherwise.
   */
  bool isValid() const { return myDigitalSurface.isValid(); }

  /**
   * Enable verbose messages.
   */
  void enableVerbose() { myVerbose = true; }

  /**
   * Disable verbose messages.
   */
  void disasbleVerbose() { myVerbose = false; }

  // ------------------------- Private methods --------------------------------
private:
  /**
   * Internal init method to set up topological caches.
   */
  void cacheInit() {
    // Collecting the surfels range
    auto params = SH2::defaultParameters() | ShortcutsGeometry<Z3i::KSpace>::defaultParameters();
    mySurfels.clear();
    mySurfels = SH2::getSurfelRange(myDigitalSurface, params);

    myPointelIndex.clear();
    // Collecting the input points
    auto pointels = SH2::getPointelRange(myPointelIndex, myDigitalSurface);
    auto embedder = SH2::getCellEmbedder(myK); // /!\ no grid step here
    myOriginalPositions.clear();
    std::transform(
        pointels.begin(), pointels.end(),
        std::back_inserter(myOriginalPositions),
        [embedder](SH2::Cell &pointel) { return embedder(pointel); });

    // Init the regularized positions.
    myRegularizedPositions.resize(myOriginalPositions.size());
    std::copy(myOriginalPositions.begin(), myOriginalPositions.end(),
              myRegularizedPositions.begin());

    // Allocating Gradient vector
    myGradient.clear();
    myGradientAlign.clear();
    myGradient.resize(myOriginalPositions.size());
    myGradientAlign.resize(myOriginalPositions.size());

    /////
    /// Caching some topological information
    auto polySurf =
        SH2::makeDualPolygonalSurface(mySurfelIndex, myDigitalSurface);
    myFaces = polySurf->allFaces(); // All faces of the dual digital surface
    auto dsurf_faces =
        myDigitalSurface
            ->allClosedFaces(); // Faces of digital surface (umbrellas)
    // dsurf_pointels list the pointels in the same order as the faces of
    // polySurf.
    std::vector<SH2::Cell> dsurf_pointels(dsurf_faces.size());
    std::transform(dsurf_faces.cbegin(), dsurf_faces.cend(),
                   dsurf_pointels.begin(),
                   [&](const SH2::DigitalSurface::Face f) {
                     return myK.unsigns(myDigitalSurface->pivot(f));
                   });

    myNumberAdjEdgesToPointel.resize(myOriginalPositions.size(), 0);

    // Precompute all relations for align energy
    myAlignPointelsIdx.resize(mySurfels.size() * 4);
    myAlignPointels.resize(mySurfels.size() * 4);

    for (size_t i = 0; i < mySurfels.size(); ++i) {
      auto pointelsAround =
          myDigitalSurface->facesAroundVertex(mySurfels[i], true);
      ASSERT(pointelsAround.size() == 4);
      for (auto j = 0; j < 4; ++j) {
        auto p = myK.unsigns(myDigitalSurface->pivot(pointelsAround[j]));
        auto cell_p = myPointelIndex[p];
        myAlignPointelsIdx[4 * i + j] = cell_p;
        myAlignPointels[4 * i + j] = p;
        myNumberAdjEdgesToPointel[cell_p]++;
      }
    }

    // Precompute all relations for fairness energy
    myNbAdjacent.resize(myFaces.size());
    for (size_t faceId = 0; faceId < myFaces.size(); ++faceId) {
      auto idx = myPointelIndex[dsurf_pointels[faceId]];
      myFairnessPointelsIdx.push_back(idx);
      unsigned char nbAdj = 0;
      auto arcs = polySurf->arcsAroundFace(faceId);
      for (auto anArc : arcs) {
        // /!\ we must check that the opposite exists (aka surface with
        // boundaries)
        auto op = polySurf->opposite(anArc);
        auto adjFace = polySurf->faceAroundArc(op);
        ASSERT(adjFace != faceId);
        myFairnessPointelsIdx.push_back(
            myPointelIndex[dsurf_pointels[adjFace]]);
        nbAdj++;
      }
      ASSERT(nbAdj > 0);
      myNbAdjacent[faceId] = nbAdj;
    }
  }

  // ------------------------- Private Data --------------------------------
private:
  /// Data attachment term coefficient
  double myAlpha;
  /// Alignment term coefficient
  double myBeta;
  /// Fairness term coefficient
  double myGamma;

  /// Data attachment term local coefficients
  const std::vector<double> *myAlphas;
  /// Alignment attachment term local coefficients
  const std::vector<double> *myBetas;
  /// Fairness attachment term local coefficients
  const std::vector<double> *myGammas;

  /// Flag if the gradient has constant weights for the init method and
  /// gradient.
  bool myConstantCoeffs;

  /// Flag if the object has been set up properly
  bool myInit;

  /// Flag for verbose messages
  bool myVerbose;

  /// Input DigitalSurface to regularize
  CountedPtr<DigSurface> myDigitalSurface;

  /// Copy of the input pointels positions.
  Positions myOriginalPositions;

  /// Regularized vertices
  Positions myRegularizedPositions;

  /// Normals
  Normals myNormals;

  /// Gradient of the energy w.r.t. vertex positons
  Positions myGradient;

  // ---------------------------------------------------------------
  /// Internal members to store precomputed topological information

  /// Gradient of the quad alignment w.r.t. vertex positons
  Positions myGradientAlign;

  /// Instance of the KSpace
  SH2::KSpace myK;

  /// Indexed surface elements
  SH2::SurfelRange mySurfels;

  /// Surfel Index
  SH2::Surfel2Index mySurfelIndex;

  /// Indices for pointels
  SH2::Cell2Index myPointelIndex;

  /// Indices of adjacent pointels for the Alignment energy term
  std::vector<SH2::Idx> myAlignPointelsIdx;
  /// Adjacent pointels for the Alignment energy term
  std::vector<SH2::Cell> myAlignPointels;
  /// Number of adjacent edges to pointels
  std::vector<unsigned char> myNumberAdjEdgesToPointel;
  /// Indices of cells foor the Fairness term
  std::vector<SH2::Idx> myFairnessPointelsIdx;
  /// Number of adjacent faces to given vertex
  std::vector<unsigned char> myNbAdjacent;
  /// All faces of the dual digital surfacce
  SH2::PolygonalSurface::FaceRange myFaces;

}; // end of class DigitalSurfaceRegularization

/**
 * Overloads 'operator<<' for displaying objects of class
 * 'DigitalSurfaceRegularization'.
 * @param out the output stream where the object is written.
 * @param object the object of class 'DigitalSurfaceRegularization' to write.
 * @return the output stream after the writing.
 */
template <typename T>
std::ostream &operator<<(std::ostream &out,
                         const DigitalSurfaceRegularization<T> &object);

} // namespace DGtalTrace


template <typename T>
inline std::ostream &
DGtal::operator<<(std::ostream &out,
                  const DGtal::DigitalSurfaceRegularization<T> &object) {
  object.selfDisplay(out);
  return out;
}

#endif // !defined DigitalSurfaceRegularization_h