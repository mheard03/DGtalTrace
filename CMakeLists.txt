cmake_minimum_required(VERSION 3.25)
project(DGtalTrace)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(DCMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CPM_VERBOSE ON)

# CPM packages need to build in Release mode  ---------------------------------
set(CMAKE_BUILD_TYPE_SAVED ${CMAKE_BUILD_TYPE})
set(CMAKE_BUILD_TYPE Release)

# CPM  ------------------------------------------------------------------------
SET(CPM_DOWNLOAD_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake")
IF(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
  FILE(DOWNLOAD
    "https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.2/CPM.cmake"
    ${CPM_DOWNLOAD_LOCATION}
    STATUS download_status
  )
  LIST(GET download_status 0 status_code)
  IF(NOT status_code EQUAL 0)
    MESSAGE(FATAL_ERROR "Failed to download CPM.cmake: ${download_status}")    
  ENDIF()
ENDIF()
INCLUDE(${CPM_DOWNLOAD_LOCATION})
MESSAGE(STATUS "CPM ${CURRENT_CPM_VERSION} ready")

# magic_enum  -----------------------------------------------------------------
CPMAddPackage(
  NAME magic_enum
  GITHUB_REPOSITORY Neargye/magic_enum
  GIT_TAG v0.9.7 # Replace x.y.z with the desired release version
)

# DGtal  ----------------------------------------------------------------------
SET(DGTAL_ENABLE_TARGET_INSTALL OFF) # Necessary for use with CPM
SET(WITH_EIGEN ON)
CPMAddPackage(
  NAME dgtal
  GITHUB_REPOSITORY DGtal-team/DGtal
  GIT_TAG v2.1 # Replace x.y.z with the desired release version
#  URI "gh:DGtal-team/DGtal@2.1"
  OPTIONS
    # More in the [build page](https://www.dgtal.org/doc/stable/moduleBuildDGtal.html)
    "DGTAL_WITH_POLYSCOPE_VIEWER ON"
    "DGTAL_WITH_OPENMP ON"
    "DGTAL_WRAP_PYTHON OFF"
)

# Restore original build type  ------------------------------------------------
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_SAVED}) 


# Link and build  -------------------------------------------------------------
ADD_EXECUTABLE(DGtalTrace main.cpp)
TARGET_LINK_LIBRARIES(DGtalTrace magic_enum::magic_enum)
TARGET_LINK_LIBRARIES(DGtalTrace DGtal::DGtal)


# INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

# TARGET_INCLUDE_DIRECTORIES(DGtalTrace ../magic_enum/)

