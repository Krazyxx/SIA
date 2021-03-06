project(sia_td4)

cmake_minimum_required (VERSION 2.8.3)

add_subdirectory(ext ext_build)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

include_directories(
  # include files
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  # tinyformat string formatting library
  ${TFM_INCLUDE_DIR}
  # Eigen linear algebra library
  ${EIGEN_INCLUDE_DIR}
  # OpenEXR high dynamic range bitmap library
  ${TINYEXR_INCLUDE_DIR}
  # PNG library
  ${LODEPNG_INCLUDE_DIR}
  # PugiXML parser
  ${PUGIXML_INCLUDE_DIR}
  # GLFW library for OpenGL context creation
  ${GLFW_INCLUDE_DIR}
  # GLEW library for accessing OpenGL functions
  ${GLEW_INCLUDE_DIR}
  # NanoVG drawing library
  ${NANOVG_INCLUDE_DIR}
  # NanoGUI user interface library
  ${NANOGUI_INCLUDE_DIR}
  # Portable filesystem API
  ${FILESYSTEM_INCLUDE_DIR}
  # OBJ mesh loader
  ${TINYOBJLOADER_INCLUDE_DIR}
  # 3DS mesh loader
  ${LIB3DS_INCLUDE_DIR}
)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_definitions(-DDATA_DIR="${PROJECT_SOURCE_DIR}")

# the list of source files
set(SRCS
    include/common.h
    include/vector.h
    include/viewer.h
    include/block.h
    include/mesh.h
    include/bitmap.h
    include/camera.h
    include/trackball.h
    include/ray.h
    include/object.h
    include/sphere.h
    include/scene.h
    include/shape.h
    include/material.h
    include/light.h
    include/areaLight.h
    include/plane.h
    include/glPrimitives.h
    include/bvh.h
    include/sphere.h
    include/color.h
    include/parser.h
    include/proplist.h
    include/transform.h
    include/integrator.h
    include/lightProbe.h
    include/quad.h
    include/warp.h
    include/photon.h
    include/kdtree.h

    src/common.cpp
    src/viewer.cpp
    src/bitmap.cpp
    src/block.cpp
    src/mesh.cpp
    src/camera.cpp
    src/trackball.cpp
    src/main.cpp
    src/object.cpp
    src/shape.cpp
    src/sphere.cpp
    src/scene.cpp
    src/material.cpp
    src/directionalLight.cpp
    src/pointLight.cpp
    src/areaLight.cpp
    src/plane.cpp
    src/phong.cpp
    src/ward.cpp
    src/bvh.cpp
    src/parser.cpp
    src/proplist.cpp
    src/direct.cpp
    src/normals.cpp
    src/flat.cpp
    src/quad.cpp
    src/texcoords.cpp
    src/whitted.cpp
    src/direct_lms.cpp
    src/direct_mats.cpp
    src/path_mats.cpp
    src/lightProbe.cpp
    src/ao.cpp
    src/warp.cpp
    src/photon.cpp
    src/photonmapper.cpp
)

add_executable(sia_raytracer ${SRCS})

target_link_libraries(sia_raytracer pugixml lodepng lib3ds tinyobjloader nanogui ${NANOGUI_EXTRA_LIBS})
