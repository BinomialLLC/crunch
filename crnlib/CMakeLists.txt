set(API_FILES
    crnlib.cpp
)

set(COMP_FILES
    crn_clusterizer.h
    crn_dxt.h
    crn_dxt.cpp
    crn_dxt_endpoint_refiner.h
    crn_dxt_endpoint_refiner.cpp
    crn_dxt_fast.h
    crn_dxt_fast.cpp
    crn_dxt_hc_common.h
    crn_dxt_hc_common.cpp
    crn_dxt1.h
    crn_dxt1.cpp
    crn_dxt5a.h
    crn_dxt5a.cpp
    crn_etc.h
    crn_etc.cpp
    crn_huffman_codes.h
    crn_huffman_codes.cpp
    crn_lzma_codec.h
    crn_lzma_codec.cpp
    crn_miniz.h
    crn_miniz.cpp
    crn_prefix_coding.h
    crn_prefix_coding.cpp
    crn_qdxt1.h
    crn_qdxt1.cpp
    crn_qdxt5.h
    crn_qdxt5.cpp
    crn_rg_etc1.h
    crn_rg_etc1.cpp
    crn_ryg_dxt.hpp
    crn_ryg_dxt.cpp
    crn_ryg_types.hpp
    crn_symbol_codec.h
    crn_symbol_codec.cpp
    crn_threaded_clusterizer.h
    crn_tree_clusterizer.h
    crn_zeng.h
    crn_zeng.cpp
)

set(CONSOLE_FILES
    crn_colorized_console.h
    crn_colorized_console.cpp
    crn_console.h
    crn_console.cpp
)

set(CONTAINER_FILES
    crn_hash_map.h
    crn_hash_map.cpp
    crn_sparse_array.h
    crn_sparse_bit_array.h
    crn_sparse_bit_array.cpp
    crn_value.h
    crn_value.cpp
    crn_vector.h
    crn_vector.cpp
    crn_vector2d.h
)

set(CRN_FILES
    crn_comp.h
    crn_comp.cpp
    crn_dds_comp.h
    crn_dds_comp.cpp
    crn_decomp.cpp
    crn_dxt_hc.h
    crn_dxt_hc.cpp
    crn_texture_comp.h
    crn_texture_comp.cpp
)

set(FILE_FILES
    crn_file_utils.h
    crn_file_utils.cpp
    crn_find_files.h
    crn_find_files.cpp
)

set(IMAGE_FILES
    crn_color.h
    crn_image.h
    crn_image_utils.h
    crn_image_utils.cpp
    crn_jpgd.h
    crn_jpgd.cpp
    crn_jpge.h
    crn_jpge.cpp
    crn_pixel_format.h
    crn_pixel_format.cpp
    crn_resampler.h
    crn_resampler.cpp
    crn_resample_filters.h
    crn_resample_filters.cpp
    crn_stb_image.cpp
    crn_threaded_resampler.h
    crn_threaded_resampler.cpp
)

set(INC_FILES
    ${CMAKE_SOURCE_DIR}/inc/crn_decomp.h
    ${CMAKE_SOURCE_DIR}/inc/crnlib.h
    ${CMAKE_SOURCE_DIR}/inc/dds_defs.h
)

set(LZMA_FILES
    lzma_7zBuf.h
    lzma_7zBuf.cpp
    lzma_7zBuf2.cpp
    lzma_7zCrc.h
    lzma_7zCrc.cpp
    lzma_7zFile.h
    lzma_7zFile.cpp
    lzma_7zStream.cpp
    lzma_7zVersion.h
    lzma_Alloc.h
    lzma_Alloc.cpp
    lzma_Bcj2.h
    lzma_Bcj2.cpp
    lzma_Bra.h
    lzma_Bra.cpp
    lzma_Bra86.cpp
    lzma_BraIA64.cpp
    lzma_CpuArch.h
    lzma_LzFind.h
    lzma_LzFind.cpp
    lzma_LzFindMt.h
    lzma_LzFindMt.cpp
    lzma_LzHash.h
    lzma_LzmaDec.h
    lzma_LzmaDec.cpp
    lzma_LzmaEnc.h
    lzma_LzmaEnc.cpp
    lzma_LzmaLib.h
    lzma_LzmaLib.cpp
    lzma_MyVersion.h
    lzma_Threads.h
    lzma_Threads.cpp
    lzma_Types.h
)

set(MATH_FILES
    crn_intersect.h
    crn_math.h
    crn_math.cpp
    crn_matrix.h
    crn_rand.h
    crn_rand.cpp
    crn_ray.h
    crn_rect.h
    crn_vec.h
    crn_vec_interval.h
)

set(STREAM_FILES
    crn_buffer_stream.h
    crn_cfile_stream.h
    crn_data_stream.h
    crn_data_stream.cpp
    crn_data_stream_serializer.h
    crn_dynamic_stream.h
)

set(STRING_FILES
    crn_dynamic_string.h
    crn_dynamic_string.cpp
    crn_strutils.h
    crn_strutils.cpp
)

set(TEXTURE_FILES
    crn_dxt_image.h
    crn_dxt_image.cpp
    crn_ktx_texture.h
    crn_ktx_texture.cpp
    crn_mipmapped_texture.h
    crn_mipmapped_texture.cpp
)

set(TEXTURE_CONVERSION_FILES
    crn_texture_conversion.h
    crn_texture_conversion.cpp
    crn_texture_file_types.h
    crn_texture_file_types.cpp
)

if(WIN32)
    set(THREADING_OS_FILES
        crn_threading_win32.h
        crn_threading_win32.cpp
    )
else()
    set(THREADING_OS_FILES
        crn_threading_pthreads.h
        crn_threading_pthreads.cpp
    )
endif()

set(THREADING_FILES
    crn_threading.h
    crn_threading_null.h
    ${THREADING_OS_FILES}
)

set(UTILS_FILES
    crn_arealist.h
    crn_arealist.cpp
    crn_checksum.h
    crn_checksum.cpp
    crn_command_line_params.h
    crn_command_line_params.cpp
    crn_hash.h
    crn_hash.cpp
    crn_helpers.h
    crn_packed_uint.h
    crn_radix_sort.h
    crn_timer.h
    crn_timer.cpp
    crn_traits.h
    crn_utils.h
    crn_utils.cpp
)

# Add the library.
add_library(crnlib STATIC
    ${API_FILES}
    ${COMP_FILES}
    ${CONSOLE_FILES}
    ${CONTAINER_FILES}
    ${CRN_FILES}
    ${FILE_FILES}
    ${IMAGE_FILES}
    ${INC_FILES}
    ${LZMA_FILES}
    ${MATH_FILES}
    ${STREAM_FILES}
    ${STRING_FILES}
    ${TEXTURE_FILES}
    ${TEXTURE_CONVERSION_FILES}
    ${THREADING_FILES}
    ${UTILS_FILES}
    crn_assert.h
    crn_assert.cpp
    crn_atomics.h
    crn_core.h
    crn_core.cpp
    crn_mem.h
    crn_mem.cpp
    crn_platform.h
    crn_platform.cpp
    crn_types.h
    crn_winhdr.h

    ${CMAKE_SOURCE_DIR}/inc
)

target_compile_options(crnlib
    PRIVATE
        "/W4"           # Warning level 4.
        "/GR-"          # Disable RunRTTI.
)

target_include_directories(crnlib
    PUBLIC
        ${CMAKE_SOURCE_DIR}/inc
)

set_target_properties(crnlib
    PROPERTIES
        PUBLIC_HEADER "${INC_FILES}"
)

install(TARGETS crnlib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION includes
    LIBRARY DESTINATION lib
)

source_group("Source Files\\api"                  FILES ${API_FILES})
source_group("Source Files\\comp"                 FILES ${COMP_FILES})
source_group("Source Files\\console"              FILES ${CONSOLE_FILES})
source_group("Source Files\\containers"           FILES ${CONTAINER_FILES})
source_group("Source Files\\crn"                  FILES ${CRN_FILES})
source_group("Source Files\\file"                 FILES ${FILE_FILES})
source_group("Source Files\\image"                FILES ${IMAGE_FILES})
source_group("Source Files\\inc"                  FILES ${INC_FILES})
source_group("Source Files\\lzmalib"              FILES ${LZMA_FILES})
source_group("Source Files\\math"                 FILES ${MATH_FILES})
source_group("Source Files\\stream"               FILES ${STREAM_FILES})
source_group("Source Files\\string"               FILES ${STRING_FILES})
source_group("Source Files\\texture"              FILES ${TEXTURE_FILES})
source_group("Source Files\\texture_conversion"   FILES ${TEXTURE_CONVERSION_FILES})
source_group("Source Files\\threading"            FILES ${THREADING_FILES})
