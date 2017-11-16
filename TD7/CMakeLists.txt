project(mesh_proc)

cmake_minimum_required(VERSION 3.2.0)

add_subdirectory(ext/glfw)
add_subdirectory(ext/glbinding)
add_subdirectory(ext/SOIL)

include_directories(ext/glfw/include)
include_directories(ext/glbinding/include)
include_directories(ext/eigen3)
include_directories(ext/SOIL/src)
include_directories(ext)
include_directories(src)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

if (APPLE)
  set(CMAKE_MACOSX_RPATH ON)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  CHECK_CXX_COMPILER_FLAG("-std=c++14" HAS_CPP14_FLAG)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_CPP11_FLAG)

  if (HAS_CPP14_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  elseif (HAS_CPP11_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  else()
    message(FATAL_ERROR "Unsupported compiler -- requires C++11 support!")
  endif()
  # Enable link time optimization and set the default symbol
  # visibility to hidden (very important to obtain small binaries)
  if (NOT ${U_CMAKE_BUILD_TYPE} MATCHES DEBUG)
    # Default symbol visibility
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
  endif()
endif()

set(SRC_FILES 
    src/viewer.h
    src/shader.h
    src/opengl.h
    src/camera.h
    src/mesh.h
    src/trackball.h
    src/bvh.h
    src/mesh_processing_app.h
    src/laplacian.h
    src/neighborhood.h

    src/neighborhood.cpp
    src/mesh_processing_app.cpp
    src/laplacian.cpp
    src/main.cpp
    src/viewer.cpp
    src/shader.cpp
    src/camera.cpp
    src/mesh.cpp
    src/trackball.cpp
    src/bvh.cpp
)

add_definitions(-DDATA_DIR="${PROJECT_SOURCE_DIR}/data")

set(VIEWPORT_FACTOR "1" CACHE STRING "set it to 2 for retina")
add_definitions(-DVPFACTOR=${VIEWPORT_FACTOR})


add_executable(mesh_proc ${SRC_FILES})

target_link_libraries(mesh_proc glfw ${GLFW_LIBRARIES} glbinding SOIL)
