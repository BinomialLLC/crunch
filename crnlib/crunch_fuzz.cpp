#include <cstddef>
#include <cstdint>
#include <string>
#include "crnlib.h"
#include "dds_defs.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  crn_uint32 crn_size = static_cast<crn_uint32>(size);
  void *dds = crn_decompress_crn_to_dds(data, crn_size);
  if (!dds) {
    return 0;
  }
  crn_texture_desc tex_desc;

  // See crnlib.h where cCRNMaxFaces and cCRNMaxLevels are defined for details
  // on the library/file limits used within crunch.
  crn_uint32 *images[cCRNMaxFaces * cCRNMaxLevels];
  bool success = crn_decompress_dds_to_images(dds, crn_size, images, tex_desc);
  crn_free_block(dds);
  if (!success) {
    return 0;
  }
  crn_free_all_images(images, tex_desc);
  return 0;
}
