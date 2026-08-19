#pragma once
#include <DGtal/io/Color.h>
#include <cstdint>
#include <bitset>

namespace DGtalTrace {

class ColorNum {
  typedef std::uint32_t uint32;
  typedef std::bitset<32> Bits;

  DGtal::Color readColor(const uint32 value) {

  }

};

template <typename T>
requires (std::integral<T> || std::floating_point<T>)
struct IntLike {};


}
/*
r,g,b,label,terrain,state,substate
255,0,0,"California","land","California","Nevada"
0,255,0,"Nevada","land","Nevada","Nevada"
0,0,255,"Utah","land","Utah","Utah"
255,255,255,"Ocean","water","",""
255,255,0,"Lake","water","",""
0,255,255,"Northern Arizona","impassable","Arizona","Northern Arizona"
255,0,255,"Southern Arizona","land","Arizona","Southern Arizona"
*/