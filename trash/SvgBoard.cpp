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
      _deltaX = margin;
      _deltaY = margin;
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
    /*
    auto ppmm = 720.0f / 254.0f;
    setUnit(1.0, Board::UPoint);
    */
    //setUnit(80, Unit::UPoint);
    std::ofstream file(filename);
    
    LibBoard::TransformSVG transform;
    LibBoard::Rect box = boundingBox();
    double margin = 0;
    trace.info() << "boundingBox: " << box << std::endl;
    transform.setBoundingBox(box, 1000, 1000, 10);

    auto viewBox = LibBoard::Rect(box.left - margin,
      box.top - margin,
      box.width + 2 * margin,
      box.height + 2 * margin);
    auto maxLeg = std::max(viewBox.width, viewBox.height);
    auto svgScale = transform.scale(1);
    auto svgWidth = 1000; // transform.scale(box.width);
    auto svgHeight = 1000; //  transform.scale(box.height);

    trace.info() << "[" << maxLeg << ", " << svgScale << ", " << svgWidth << "x" << svgHeight << std::endl;

    
    file << "<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"" << std::endl;
    file << "     width=\"" << svgWidth << "\" "
                  << "height=\"" << svgHeight << "\" "
                  << "preserveAspectRatio=\"xMinYMin meet\"" << std::endl;
    file << "     viewBox=\"" << viewBox.left << " " << viewBox.height - viewBox.top << " " << viewBox.width << " " << viewBox.height << "\"" << std::endl;
    file << "     style=\""
                  << "background-color: " << _backgroundColor.svg() << "; "
                  << "display: block; "
                  << "aspect-ratio: " << viewBox.width << " / " << viewBox.height << "; "
                  << "width: " << svgWidth << "px; "
                  << "height: " << svgHeight << "px; "
                  << "transform: scale(1,-1); "
                  << "\"";
    file << ">" << std::endl;
    file << "<style>" << std::endl;
    file << "  image { "
             << "transform: scale(1,-1); "
             << "image-rendering: pixelated; "
             << "}" << std::endl;
    file << "</style>" << std::endl;
    
    // Draw the shapes.
    std::vector< Shape* > shapes = _shapes;
    stable_sort( shapes.begin(), shapes.end(), LibBoard::shapeGreaterDepth );
    std::vector< Shape* >::const_iterator i = shapes.begin();
    std::vector< Shape* >::const_iterator end = shapes.end();
    while ( i != end ) {
      (*i)->flushSVG( file, transform );
      ++i;
    }  
    
    file << "</svg>" << std::endl;
    file.close();
    _state = oldState;
  }
};