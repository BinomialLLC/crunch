// File: rg_etc1.h - Fast, high quality ETC1 block packer/unpacker - Rich Geldreich <richgel99@gmail.com>
// This software is in the public domain. Please see license.txt.
#pragma once

namespace crnlib {

namespace rg_etc1
{
   // Unpacks an 8-byte ETC1 compressed block to a block of 4x4 32bpp RGBA pixels.
   // Returns false if the block is invalid. Invalid blocks will still be unpacked with clamping.
   // This function is thread safe, and does not dynamically allocate any memory.
   // If preserve_alpha is true, the alpha channel of the destination pixels will not be overwritten. Otherwise, alpha will be set to 255.
   bool unpack_etc1_block(const void *pETC1_block, unsigned int* pDst_pixels_rgba, bool preserve_alpha = false);

   // Quality setting = the higher the quality, the slower. 
   // To pack large textures, it is highly recommended to call pack_etc1_block() in parallel, on different blocks, from multiple threads (particularly when using cHighQuality).
   enum etc1_quality
   { 
      cLowQuality,
      cMediumQuality,
      cHighQuality,
   };
      
   struct etc1_pack_params
   {
      etc1_quality m_quality;
      bool m_dithering;
                              
      inline etc1_pack_params() 
      {
         clear();
      }

      void clear()
      {
         m_quality = cHighQuality;
         m_dithering = false;
      }
   };

   // Important: pack_etc1_block_init() must be called before calling pack_etc1_block().
   void pack_etc1_block_init();

   // Packs a 4x4 block of 32bpp RGBA pixels to an 8-byte ETC1 block.
   // 32-bit RGBA pixels must always be arranged as (R,G,B,A) (R first, A last) in memory, independent of platform endianness. A should always be 255.
   // Returns squared error of result.
   // This function is thread safe, and does not dynamically allocate any memory.
   // pack_etc1_block() does not currently support "perceptual" colorspace metrics - it primarily optimizes for RGB RMSE.
   unsigned int pack_etc1_block(void* pETC1_block, const unsigned int* pSrc_pixels_rgba, etc1_pack_params& pack_params);
            
} // namespace rg_etc1

} // namespace crnlib

