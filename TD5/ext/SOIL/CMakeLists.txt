cmake_minimum_required (VERSION 2.6)
project(SOIL)
enable_language(C)

if(ANDROID)
  # Android requires GL ES 2.0 package automatically
  set(LIBRARIES GLESv2)
else()
  find_package (OpenGL REQUIRED)
  include_directories (${OPENGL_INCLUDE_DIR})
  set(LIBRARIES ${OPENGL_LIBRARIES})
endif()

file (GLOB SOIL_SOURCES src/*.c)
file (GLOB SOIL_HEADERS src/*.h)

add_library (SOIL ${SOIL_SOURCES} ${SOIL_HEADERS})
target_link_libraries (SOIL ${LIBRARIES})
