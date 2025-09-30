#pragma once
#include <Board/Board.h>
#include <DGtal/io/boards/Board2D.h>
#include <Board/Transforms.h>
#include <DGtal/base/Trace.h>
#include <DGtal/io/Color.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

using namespace DGtal;

namespace {     
  struct TransformWebSVG : public LibBoard::TransformSVG {
    double mapWidth( double width ) const {
      return Transform::round(1000 * width) / 1000.0;
    }

    void setBoundingBox(const LibBoard::Rect & rect,
                const double pageWidth,
                const double pageHeight,
                const double margin) {
      _scale = 1;
      _deltaX = 0;
      _deltaY = 0;
      _height = rect.height;
    }

  };
}


class SvgBoard : public Board2D {
public:
  using Board2D::Board2D;
  SvgBoard(const Board2D& other) : Board2D(other) {}

  void saveWebSVG(const char* filename) {
    auto oldState = Board::State(_state);
    _state = Board::State();
    auto ppmm = 720.0f / 254.0f;
    setUnit(ppmm, Board::UPoint);

    _state.unitFactor = 1;

    std::ofstream file(filename);
    
    TransformWebSVG transform;
    LibBoard::Rect box = boundingBox();
    transform.setBoundingBox(box, box.width, box.height, 0);

    bool clipping = _clippingPath.size() > 2;
    if ( clipping ) box = box && _clippingPath.boundingBox();
    
    file << "<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\"" << std::endl;
    file << "     width=\"" << ceil(box.width) << "\" "
                  << "height=\"" << ceil(box.height) << "\" "
                  << "preserveAspectRatio=\"xMinYMin meet\"" << std::endl;
    file << "     viewBox=\"" << box.left << " " << (box.height - box.top) << " " << box.width << " " << box.height << "\"" << std::endl;
    file << "     style=\""
                  << "background-color: " << _backgroundColor.svg() << "; "
                  << "display: block; "
                  << "aspect-ratio: " << box.width << " / " << box.height << "; "
                  << "width: auto; "
                  << "max-width: 100%; "
                  << "max-height: 100vh; "
                  << "transform: scale(1,-1);\"";
    file << ">" << std::endl;

    if (clipping) {
      file << "<g clip-rule=\"nonzero\">\n"
                << " <clipPath id=\"GlobalClipPath\">\n"
                << "  <path clip-rule=\"evenodd\"  d=\"";
      _clippingPath.flushSVGCommands( file, transform );
      file << "\" />\n";
      file << " </clipPath>\n";
      file << "<g clip-path=\"url(#GlobalClipPath)\">\n";
    }
    
    // Draw the shapes.
    std::vector< Shape* > shapes = _shapes;
    stable_sort( shapes.begin(), shapes.end(), LibBoard::shapeGreaterDepth );
    std::vector< Shape* >::const_iterator i = shapes.begin();
    std::vector< Shape* >::const_iterator end = shapes.end();
    while ( i != end ) {
      (*i)->flushSVG( file, transform );
      ++i;
    }  

    if (clipping) file << "</g>\n</g>";
    
    file << "</svg>" << std::endl;
    file.close();
    _state = oldState;
  }
};