#include <iostream>
#include <DGtal/base/Common.h>
#include <DGtal/images/Image.h>
#include <DGtal/images/ImageSelector.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/io/readers/STBReader.h>
#include <string>
#include <unordered_map>
#include <DGtal/geometry/curves/ArithmeticalDSSComputer.h>
#include <DGtal/geometry/curves/SaturatedSegmentation.h>
#include "DataTable.hpp"

using namespace DGtal;
using namespace Z2i;

typedef ImageSelector<Domain, Color>::Type RgbaImage;
typedef uint16_t Key;
typedef ImageSelector<Domain, Key>::Type KeyImage;

template <typename TSourceImage, typename TTargetImage, typename TKSpace>
struct FloodFiller {
  typedef typename TSourceImage::Domain Domain;
  typedef typename TSourceImage::Point Point;
  typedef typename TSourceImage::Value SourceValue;
  typedef typename TTargetImage::Value TargetValue;
  enum PixelStates : unsigned char { UNREAD = 0, SKIP = 1, REPLACE = 2 };
  typedef std::unordered_map<typename TSourceImage::Point, PixelStates> PixelStateMap;
  
  FloodFiller(const TSourceImage& sourceImage, const TKSpace& K) : sourceImage(&sourceImage), K(&K) {}

  void fill(const Point& startPixel, TTargetImage& targetImage, const TargetValue& newValue) {
    // Get pixels to fill
    const auto sourceValueToReplace = sourceImage->operator()(startPixel);
    PixelStateMap pixelMap = { { startPixel, PixelStates::UNREAD } };

    auto pixelsToScan = getPixelsWithState(pixelMap, PixelStates::UNREAD);
    // DGtal::trace.info() << pixelsToScan.size() << " pixels to scan" << std::endl;
    do {
      // DGtal::trace.info() << "  ";
      for (auto pixel : pixelsToScan) {
        scanPixel(pixelMap, pixel, sourceValueToReplace);
        // DGtal::trace.info() << pixel << ", ";
      }
      // DGtal::trace.info() << std::endl;
      pixelsToScan = getPixelsWithState(pixelMap, PixelStates::UNREAD);
    }
    while (pixelsToScan.size() > 0);

    auto pixelsToFill = getPixelsWithState(pixelMap, PixelStates::REPLACE);
    for (auto px : pixelsToFill) {
      targetImage.setValue(px, newValue);
    }
  }

 private:
  const TSourceImage* sourceImage;
  const TKSpace* K;
  typedef typename TKSpace::Cell Cell;

  std::vector<Point> getPixelsWithState(PixelStateMap& pixelMap, const PixelStates targetState) const {
    std::vector<Point> pixels;
    for (auto [pixel, pixelState] : pixelMap) {
      if (pixelState == targetState) pixels.push_back(pixel);
    }
    return pixels;    
  }

  void scanPixel(PixelStateMap& pixelMap, const Point& pixel, const SourceValue& sourceValueToReplace) const {
    if (sourceImage->operator()(pixel) != sourceValueToReplace) {
      pixelMap[pixel] = PixelStates::SKIP;
      return;
    }
    
    pixelMap[pixel] = PixelStates::REPLACE;
    for (auto neighborCell : K->uProperNeighborhood(K->uSpel(pixel))) {
      Point neighborPixel = K->uCoords(neighborCell);
      if (pixelMap.find(neighborPixel) == pixelMap.end()) {
        pixelMap.insert_or_assign(neighborPixel, PixelStates::UNREAD);
      }
    }
  }
};

struct KeyImageInfo {
 public:
  typedef std::unordered_map<Key, Color> KeyColors;
  KeyImageInfo(const RgbaImage& sourceImage) : myImage(sourceImage.domain()), myKeyColors() {
    auto domain = myImage.domain();   
    K.init(domain.lowerBound(), domain.upperBound(), true);

    typedef DGtal::Linearizer<Domain, ColMajorStorage> L;
    FloodFiller<RgbaImage, KeyImage, KSpace> filler(sourceImage, K);
    // typedef typename ImageSelector<Domain, bool>::Type PixelStateImage;
    int keyCount = 0;
    for (auto i = 0; i < myImage.size(); i++) {
      if (myImage[i] > 0) continue;
      auto point = L::getPoint(i, domain);
      auto pxColor = sourceImage(point);
      filler.fill(point, myImage, ++keyCount);
      myKeyColors[keyCount] = sourceImage(point);
    }
  }
  const KeyImage& image() const {
    return myImage;
  }
  const KeyColors& keyColors() const {
    return myKeyColors;
  }
  void selfDisplay(std::ostream &	out) const {
    out << "[KeyImageInfo] " << myImage << "; " << myKeyColors.size() << " colors";
  }
 private:
  KSpace K;
  KeyImage myImage;
  KeyColors myKeyColors;
};
std::ostream& operator<<(std::ostream& os, const KeyImageInfo& obj) {
  obj.selfDisplay(os);
  return os;
}

int main(int argc, char** argv) {
  using Table = DGtalTrace::DataTable<std::string, std::string, int>;
// using namespace std::string_literals; 
  Table t;
  t.addColumn("col0", 0);
  t.addColumn("col1", 1);
  t.addColumn<std::string>("coltxt", "nil");

  t.insert_or_assign("col0", "key0", 0);
  t.insert_or_assign("col0", "key1", 1);
  t.insert_or_assign("coltxt", "key1", "one");
  t.insert_or_assign("coltxt", "key2", "two");
  t.insert_or_assign("col1", "key3", 3);

  for (auto colName : { "col0", "col1" }) {
    DGtal::trace.info() << colName << std::endl;
    auto col = t.getColumn<int>(colName);
    for (auto key : { "key0", "key1", "key2" }) {
      DGtal::trace.info() << "  " << key << ": " << col.at(key) << std::endl;
    }
    DGtal::trace.info() << std::endl;
  }

  DGtal::trace.info() << "coltxt" << std::endl;
  auto col = t.getColumn<std::string>("coltxt");
  for (auto key : { "key0", "key1", "key2" }) {
    DGtal::trace.info() << "  " << key << ": " << col.at(key) << std::endl;
  }
  DGtal::trace.info() << std::endl;

  DGtal::trace.info() << t;

  return 0;



  std::string inputFilename = (argc >= 1 ? "cali.png" : argv[1]);

  trace.beginBlock("reading " + inputFilename);
  RgbaImage image = STBReader<RgbaImage>::import(inputFilename);
  trace.endBlock();
  trace.info() << image.domain() << std::endl;
  trace.info() << image(Point(0,0)) << std::endl;

  trace.beginBlock("generating keyImage");
  auto keyImage = KeyImageInfo(image);
  trace.endBlock();
  trace.info() << keyImage << std::endl;
  return 0;
}