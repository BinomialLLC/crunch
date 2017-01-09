#define PLATFORM_NACL

#include <stddef.h> // For NULL, size_t
#include <cstring> // for malloc etc

#include "crn_decomp.h"

extern "C" {
  unsigned int crn_get_width(void *src, unsigned int src_size);
  unsigned int crn_get_height(void *src, unsigned int src_size);
  unsigned int crn_get_levels(void *src, unsigned int src_size);
  unsigned int crn_get_dxt_format(void *src, unsigned int src_size);
  unsigned int crn_get_uncompressed_size(void *p, unsigned int size);
  void crn_decompress(void *src, unsigned int src_size, void *dst, unsigned int dst_size);
}

unsigned int crn_get_width(void *src, unsigned int src_size) {
  crnd::crn_texture_info tex_info;
  crnd::crnd_get_texture_info(static_cast<crn_uint8*>(src), src_size, &tex_info);
  return tex_info.m_width;
}

unsigned int crn_get_height(void *src, unsigned int src_size) {
  crnd::crn_texture_info tex_info;
  crnd::crnd_get_texture_info(static_cast<crn_uint8*>(src), src_size, &tex_info);
  return tex_info.m_height;
}

unsigned int crn_get_levels(void *src, unsigned int src_size) {
  crnd::crn_texture_info tex_info;
  crnd::crnd_get_texture_info(static_cast<crn_uint8*>(src), src_size, &tex_info);
  return tex_info.m_levels;
}

unsigned int crn_get_dxt_format(void *src, unsigned int src_size) {
  crnd::crn_texture_info tex_info;
  crnd::crnd_get_texture_info(static_cast<crn_uint8*>(src), src_size, &tex_info);
  return tex_info.m_format;
}

unsigned int crn_get_uncompressed_size(void *src, unsigned int src_size) {
  crnd::crn_texture_info tex_info;
  crnd::crnd_get_texture_info(static_cast<crn_uint8*>(src), src_size, &tex_info);
  const crn_uint32 width = tex_info.m_width;
  const crn_uint32 height = tex_info.m_height;
  const crn_uint32 blocks_x = (width + 3) >> 2;
  const crn_uint32 blocks_y = (height + 3) >> 2;
  const crn_uint32 row_pitch = blocks_x * crnd::crnd_get_bytes_per_dxt_block(tex_info.m_format);
  const crn_uint32 total_face_size = row_pitch * blocks_y;
  return total_face_size;
}

void crn_decompress(void *src, unsigned int src_size, void *dst, unsigned int dst_size) {
  crnd::crn_texture_info tex_info;
  crnd::crnd_get_texture_info(static_cast<crn_uint8*>(src), src_size, &tex_info);
  const crn_uint32 width = tex_info.m_width;
  const crn_uint32 height = tex_info.m_height;
  const crn_uint32 blocks_x = (width + 3) >> 2;
  const crn_uint32 blocks_y = (height + 3) >> 2;
  const crn_uint32 row_pitch = blocks_x * crnd::crnd_get_bytes_per_dxt_block(tex_info.m_format);

  crnd::crnd_unpack_context pContext =
      crnd::crnd_unpack_begin(static_cast<crn_uint8*>(src), src_size);
  void *pDecomp_images[1];
  pDecomp_images[0] = dst;
  crnd::crnd_unpack_level(pContext, pDecomp_images, dst_size, row_pitch, 0);
  crnd::crnd_unpack_end(pContext);
}