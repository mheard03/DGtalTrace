#pragma once
#include "../concepts/dgtal/CUnaryFunctor.cpp"
#include <DGtal/base/BasicFunctors.h>
#include <DGtal/io/colormaps/BasicColorToScalarFunctors.h>
#include <DGtal/base/Common.h>
#include <DGtal/images/ImageContainerBySTLVector.h>
#include <DGtal/images/ImageSelector.h>
#include <cassert>
#include <concepts>
#include <iostream>
#include <limits>
#include <span>
#ifndef STBI_INCLUDE_STB_IMAGE_H
#include <stb/stb_image.h>
#endif
#include <tuple>
#include <type_traits>
#include <DGtal/helpers/StdDefs.h>

namespace DGtalTrace {

template <typename TImageContainer, typename TFunctor = DGtal::functors::Identity>
requires requires (TImageContainer img, typename TImageContainer::Point p, unsigned short v, TFunctor fn) {
  requires (TImageContainer::Domain::dimension == 2);
  img.setValue(p, fn(v));
}
class STBReaderGray16 {
  typedef TImageContainer ImageContainer;
  typedef TFunctor Functor;
  typedef ImageContainer::Point Point;
  typedef ImageContainer::Domain Domain;
public:
  static ImageContainer import(const std::string &filename,
                               const Functor &aFunctor = Functor()) {
    int width, height, channels;
    unsigned short *data = stbi_load_16(filename.c_str(), &width, &height, &channels, 1);

    Point firstPoint(0, 0);
    Point lastPoint(width - 1, height - 1);
    Domain domain(firstPoint, lastPoint);

    ImageContainer image(domain);

    DGtal::trace.info() << "Loading " << filename;
    unsigned short value;
    for (auto y = 0; y < height; ++y) {
      if (y % 100 == 0) {
        DGtal::trace.info() << ".";
      }
      for (auto x = 0; x < width; ++x) {
        value = data[y * width + x];
        image.setValue(typename ImageContainer::Point(x, y), aFunctor(value));
      }
    }
    DGtal::trace.info() << std::endl;

    stbi_image_free(data);
    return image;
  }

}; // end of class STBReader

} // namespace DGtalTrace