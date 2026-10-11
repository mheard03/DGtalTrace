cmake_minimum_required(VERSION 3.22.1)
project(DGtalTrace)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
set(CPM_VERBOSE ON)

# CPM packages need to build in Release mode  ---------------------------------
set(CMAKE_BUILD_TYPE_SAVED ${CMAKE_BUILD_TYPE})
set(CMAKE_BUILD_TYPE Release)


# CPM  ------------------------------------------------------------------------
SET(CPM_DOWNLOAD_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake")
IF(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
  FILE(DOWNLOAD
    "https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.1/CPM.cmake"
    ${CPM_DOWNLOAD_LOCATION}
    STATUS download_status
  )
  LIST(GET download_status 0 status_code)
  IF(NOT status_code EQUAL 0)
    MESSAGE(FATAL_ERROR "Failed to download CPM.cmake: ${download_status}")    
  ENDIF()
ENDIF()
IF(EXISTS ${CPM_DOWNLOAD_LOCATION})
  MESSAGE(STATUS "CPM.cmake found at: ${CPM_DOWNLOAD_LOCATION}")
ELSE()
  MESSAGE(FATAL_ERROR "CPM.cmake not found at: ${CPM_DOWNLOAD_LOCATION}")
ENDIF()
INCLUDE(${CPM_DOWNLOAD_LOCATION})


# DGtal  ----------------------------------------------------------------------

# fudge polyscope
set(POLYSCOPE_BACKEND_OPENGL3_GLFW "ON" CACHE BOOL "Enable openGL3_glfw backend" FORCE)
set(POLYSCOPE_BACKEND_OPENGL_MOCK "OFF" CACHE BOOL "Enable openGL_mock backend" FORCE)
set(POLYSCOPE_BACKEND_OPENGL3_EGL "OFF" CACHE STRING "Enable openGL3_egl backend" FORCE)

# CPMAddPackage
SET(DGTAL_ENABLE_TARGET_INSTALL OFF) # Necessary for use with CPM
SET(WITH_EIGEN OFF) # might need this on later
CPMAddPackage(
  NAME dgtal
  GITHUB_REPOSITORY DGtal-team/DGtal
  GIT_TAG v2.2 # Replace x.y.z with the desired release version
#  URI "gh:DGtal-team/DGtal@2.1"
  OPTIONS
    # More in the [build page](https://www.dgtal.org/doc/stable/moduleBuildDGtal.html)
    "DGTAL_WITH_POLYSCOPE_VIEWER ON"
    "DGTAL_WITH_OPENMP OFF"
    "DGTAL_WITH_CGAL OFF"
    "DGTAL_WITH_ITK OFF"
    "DGTAL_WITH_CAIRO OFF"
    "DGTAL_WITH_HDF5 OFF"
    "DGTAL_WITH_PONCA OFF"
    "DGTAL_WITH_FFTW3 OFF"
    "DGTAL_WITH_LIBIGL OFF"
    "DGTAL_WRAP_PYTHON OFF"    
)


# Restore original build type  ------------------------------------------------
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_SAVED}) 


# Build that thang
ADD_EXECUTABLE(DGtalTrace main.cpp)
TARGET_LINK_LIBRARIES(DGtalTrace DGtal::DGtal)  