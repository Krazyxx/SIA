cmake_minimum_required (VERSION 2.6)
project (SOIL)

add_definitions (-D_CRT_SECURE_NO_WARNINGS)
include_directories (src)
add_library (SOIL STATIC
    src/SOIL2.c
    src/SOIL2.h
    src/etc1_utils.c
    src/etc1_utils.h
    src/image_DXT.c
    src/image_DXT.h
    src/image_helper.c
    src/image_helper.h
    src/pkm_helper.h
    src/pvr_helper.h
    src/stbi_DDS_c.h
    src/stbi_DDS.h
    src/stbi_ext_c.h
    src/stbi_ext.h
    src/stb_image.h
    src/stb_image_write.h
    src/stbi_pkm_c.h
    src/stbi_pkm.h
    src/stbi_pvr_c.h
    src/stbi_pvr.h)

target_link_libraries (SOIL ${LIBRARIES})
