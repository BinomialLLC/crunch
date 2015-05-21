## Contents ##

  * [Introduction](API_Docs#Introduction.md)
  * [Public API Overview](API_Docs#Public_API_Overview.md)
  * [Public Enums](API_Docs#Public_Enums.md)
  * [Public Structs](API_Docs#Public_Structs.md)
  * [Public Functions](API_Docs#Public_Functions.md)
    * [Memory Allocation](API_Docs#Memory_Allocation.md)
    * [Compression](API_Docs#Compression.md)
    * [Transcoding](API_Docs#Transcoding.md)
    * [Decompression](API_Docs#Decompression.md)
    * [DXTn Block Compression](API_Docs#DXTn_Block_Compression.md)
    * [Helper Functions](API_Docs#Helper_Functions.md)


---


## Introduction ##

crnlib is a C++ library designed to be statically linked into the calling application. It can compress to .CRN, regular .DDS, or clustered .DDS files. It can also transcode .CRN to .DDS, and unpack .DDS files to individual 24/32-bit images. For completeness, crnlib's high-quality DXTn block compressor is also accessible.

The library does not use C++ exceptions, but it does use some C++ features such as templates, virtual functions, and inheritance. It also makes heavy use of heap allocation. Due to porting, exception, and inconsistent performance issues (especially in debug builds) crnlib  mostly uses custom containers instead of STL.

The VC9 (Visual Studio 2008) .LIB files are built here:

```
  lib\VC9\release\win32\crnlib_vc9.lib
  lib\VC9\release\win64\crnlib_x64_vc9.lib
  lib\VC9\release_dll\win32\crnlib_DLL_vc9.lib
  lib\VC9\release_dll\win64\crnlib_DLL_x64_vc9.lib
```

crnlib should also build with VC10 (Visual Studio 2010), and Codeblocks 10.05 using TDM-GCC, but the majority of my testing has been with VC9.

Currently crnlib is Win32 only, but it already compiles with GCC so a Linux/BSD/Mac port shouldn't be too difficult. (The threading related code is the biggest blocker to porting.) crnlib itself has only been tested on PC's, but [crn\_decomp.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crn_decomp.h) (the stand-alone transcoder header file library) should work fine on consoles.

A Xbox 360 specific version of crn\_decomp.h is available that can transcode .CRN textures into X360 tiled textures located in cached or write combined memory at only a ~10% slowdown. Please email me if you're interested (it's bitrotted a bit since the public release).


---


## Public API Overview ##

There are two header files of interest, both under the [inc](http://code.google.com/p/crunch/source/browse/trunk/#trunk%2Finc) directory. crnlib exposes a simple high level, C-style function based API, which is defined in the single public header file [inc/crnlib.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crnlib.h).

The second public header file, [inc/crn\_decomp.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crn_decomp.h), contains all the functionality needed to transcode .CRN files to raw DXTn bits. It does not depend on crnlib in any way, although crnlib internally uses `crn_decomp.h` itself to transcode, examine, and validate .CRN files.

Each crnlib API falls into one of the following categories:

  * **Memory management**:
    * `crn_set_memory_callbacks()`
    * `crn_free_block()`

  * **Image or texture compression** from memory to .CRN or .DDS file in memory:
    * crn\_compress()

  * **Texture decompression** from a .CRN or .DDS file memory to memory:
    * `crn_decompress_crn_to_dds()`
    * `crn_decompress_dds_to_images()`
    * `crn_free_all_images()`

  * **Plain DXTn block compression** of 4x4 pixel blocks to DXTn compressed blocks:
    * `crn_create_block_compressor()`
    * `crn_compress_block()`
    * `crn_free_block_compressor()`

  * **Misc. helpers**:
    * **crn\_format info**:
      * `crn_get_format_fourcc()`
      * `crn_get_format_bits_per_texel()`
      * `crn_get_bytes_per_dxt_block()`
      * `crn_get_fundamental_dxt_format()`
    * **crn\_format to/from ANSI and UTF16 string**:
      * `crn_get_file_type_exta()`
      * `crn_get_file_type_ext()`
      * `crn_get_format_stringa()`
      * `crn_get_format_string()`
      * `crn_get_dxt_quality_stringa()`
      * `crn_get_dxt_quality_string()`

Several custom types and parameter structs are also defined in [inc/crnlib.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crnlib.h). The most important structs are:
  * [crn\_comp\_params](API_Docs#enum_crn_comp_params.md), which contains all the parameters passed to the compression function `crn_compress()`
  * `struct crn_mipmap_params`, which contains a bunch of parameters that control crnlib's optional mipmap generator. (This struct is not yet documented here.)


---


## Public Enums ##

### enum crn\_file\_type ###
```
enum crn_file_type
{
   cCRNFileTypeCRN = 0,
   cCRNFileTypeDDS,
};
```

`crn_file_type` contains the supported file types. crnlib only supports DX9-style .DDS files.

`cCRNFileTypeCRN`: .CRN file format

`cCRNFileTypeDDS`: .DDS file format


### enum crn\_format ###
```
enum crn_format
{
   cCRNFmtInvalid = -1,

   cCRNFmtDXT1 = 0,
   
   cCRNFmtFirstValid = cCRNFmtDXT1,

   // cCRNFmtDXT3 is not currently supported when writing to CRN - only DDS.
   cCRNFmtDXT3,

   cCRNFmtDXT5,
   
   // Various DXT5 derivatives
   cCRNFmtDXT5_CCxY,    // Luma-chroma
   cCRNFmtDXT5_xGxR,    // Swizzled 2-component
   cCRNFmtDXT5_xGBR,    // Swizzled 3-component
   cCRNFmtDXT5_AGBR,    // Swizzled 4-component

   // ATI 3DC and X360 DXN
   cCRNFmtDXN_XY,       
   cCRNFmtDXN_YX,

   // DXT5 alpha blocks only
   cCRNFmtDXT5A,

   cCRNFmtTotal,
};
```

The `crn_format` enum contains the supported compressed pixel formats. It lists all the standard DX9 compressed pixel formats (BC1-BC5), with some swizzled DXT5 formats (most of them supported by ATI's Compressonator).

### enum crn\_limits ###
```
enum crn_limits
{
   cCRNMaxLevelResolution     = 4096,

   cCRNMinPaletteSize         = 8,
   cCRNMaxPaletteSize         = 8192,

   cCRNMaxFaces               = 6,
   cCRNMaxLevels              = 16,

   cCRNMaxHelperThreads       = 16,

   cCRNMinQualityLevel        = 0,
   cCRNMaxQualityLevel        = 255
};
```

The `crn_limits` enum lists various library limits. Notably, the max supported texture resolution is currently 4096x4096 (this can be easily increased in the x64 version).

### enum crn\_comp\_flags ###
```
enum crn_comp_flags
{
   cCRNCompFlagPerceptual = 1,    
   cCRNCompFlagHierarchical = 2,    
   cCRNCompFlagQuick = 4,
   cCRNCompFlagUseBothBlockTypes = 8,    
   cCRNCompFlagUseTransparentIndicesForBlack = 16,
   cCRNCompFlagDisableEndpointCaching = 32, 
   cCRNCompFlagManualPaletteSizes = 64,
   cCRNCompFlagDXT1AForTransparency = 128,
   cCRNCompFlagGrayscaleSampling = 256,
   cCRNCompFlagDebugging = 0x80000000,
};
```

The `crn_comp_flags` enum contains a number of compression related flags:

`cCRNCompFlagPerceptual`: Default: Enabled. If enabled, perceptual colorspace distance metrics are enabled. **Important**: Be sure to **disable** this flag when compressing non-sRGB colorspace images, like normal maps!

`cCRNCompFlagHierarchical`: Default: Enabled. If enabled, 4x4, 4x8, 8x4, and 8x8 tiles may be used in each macroblock. If disabled, all macroblocks are forced to use four 4x4 pixel tiles. Compression ratio will be lower when disabled, and transcoding will be a bit slower, but this will reduce macroblock tiling artifacts.

`cCRNCompFlagQuick`: Default: Disabled. If enabled, this flag disables several output file optimizations. Intended for things like quicker previews.

`cCRNCompFlagUseBothBlockTypes`: Default: Enabled. This flag controls which block types are used when compressing to .DDS. (This flag is not relevant when compressing to .CRN, which only uses a subset of the possible DXTn block types.)

> DXT1: OK to use DXT1A (3 color) alpha blocks if doing so results in lower RGB error, or for transparent pixels.

> DXT5: OK to use both DXT5 block types.

`cCRNCompFlagUseTransparentIndicesForBlack`: Default: Disabled. If enabled, it's OK to use DXT1A transparent indices to encode full black colors (assumes pixel shader ignores fetched alpha). (Not relevant when compressing to .CRN files, because it never uses alpha blocks.)

`cCRNCompFlagDisableEndpointCaching`: Default: Disabled. When set, this flag disables endpoint caching, for deterministic output. Only relevant when compressing to .DDS.

`cCRNCompFlagManualPaletteSizes`: Default: Disabled. If enabled, use the cCRNColorEndpointPaletteSize, etc. params to control the CRN palette sizes. Only relevant when compressing to .CRN.

`cCRNCompFlagDXT1AForTransparency`: Default: Disabled. If enabled, DXT1A alpha blocks are used to encode single bit transparency. Only relevant when compressing to .DDS, .CRN does not support DXT1A alpha blocks.

`cCRNCompFlagGrayscaleSampling`: Default: Disabled. If enabled, the DXT1 compressor's color distance metric assumes the pixel shader will be converting the fetched RGB results to luma (Y part of YCbCr).

This increases quality when compressing grayscale images, because the compressor can spread the luma error amoung all three channels (i.e. it can generate blocks with some chroma present if doing so will ultimately lead to lower luma error). Of course, only enable on grayscale source images.

`cCRNCompFlagDebugging`: Default: Disabled. If enabled, the frontend and backend gather and dump various statistics during the compression process. Only used for development/debugging purposes.

### enum crn\_dxt\_quality ###
```
enum crn_dxt_quality
{
   cCRNDXTQualitySuperFast,
   cCRNDXTQualityFast,
   cCRNDXTQualityNormal,
   cCRNDXTQualityBetter,
   cCRNDXTQualityUber,
};
```

The `crn_dxt_quality` enum lists the various quality modes supported by the endpoint optimizers. This enum is only relevant when compressing to .DDS. cCRNDXTQualityUber is slower, but it has the best PSNR.


### enum crn\_dxt\_compressor\_type ###
```
enum crn_dxt_compressor_type
{
   cCRNDXTCompressorCRN,
   cCRNDXTCompressorCRNF,
   cCRNDXTCompressorRYG
};
```

This enum lists the DXTn block compressors supported by the library. This enum is only relevant when compressing to non-clustered .DDS files.

`cCRNDXTCompressorCRN`: crnlib's default endpoint optimizer.

`cCRNDXTCompressorCRNF`: A faster version of the default optimizer.

`cCRNDXTCompressorRYG`: RYG's public domain endpoint optimizer.


---


## Public Structs ##

### enum crn\_comp\_params ###
```
typedef crn_bool (*crn_progress_callback_func)(crn_uint32 phase_index, crn_uint32 total_phases, 
  crn_uint32 subphase_index, crn_uint32 total_subphases, void* pUser_data_ptr);

struct crn_comp_params
{
   inline crn_comp_params();

   inline void clear();

   inline bool check() const;

   inline bool get_flag(crn_comp_flags flag) const;
   inline void set_flag(crn_comp_flags flag, bool val);
   
   crn_uint32                 m_size_of_obj;
      
   crn_file_type              m_file_type;               

   crn_uint32                 m_faces;                   
   crn_uint32                 m_width;                   
   crn_uint32                 m_height;                  
   crn_uint32                 m_levels;                  
   
   crn_format                 m_format;                  

   crn_uint32                 m_flags;                   

   const crn_uint32*          m_pImages[cCRNMaxFaces][cCRNMaxLevels];

   float                      m_target_bitrate;
   
   crn_uint32                 m_quality_level;           
   
   crn_uint32                 m_dxt1a_alpha_threshold;
   crn_dxt_quality            m_dxt_quality;
   crn_dxt_compressor_type    m_dxt_compressor_type;
   
   crn_uint32                 m_alpha_component;

   float                      m_crn_adaptive_tile_color_psnr_derating;
   float                      m_crn_adaptive_tile_alpha_psnr_derating;

   crn_uint32                 m_crn_color_endpoint_palette_size;  
   crn_uint32                 m_crn_color_selector_palette_size;  

   crn_uint32                 m_crn_alpha_endpoint_palette_size;  
   crn_uint32                 m_crn_alpha_selector_palette_size;  

   crn_uint32                 m_num_helper_threads;

   crn_uint32                 m_userdata0;
   crn_uint32                 m_userdata1;

   crn_progress_callback_func m_pProgress_func;
   void*                      m_pProgress_func_data;
};
```

The `crn_comp_params` struct contains all parameters passed to the compressor. The caller must fill in this struct before calling `crn_compress()`. Note that some parameters/flags are relevant only when compressing to .CRN, clustered .DDS, or regular .DDS (I've tried to document all dependencies).

This struct contains several simple inline methods defined in this header. The constructor calls `clear()`, and the `clear()` method sets all parameters to their defaults. The `check()` method returns true if all parameters are within reasonable/supported ranges. The `get_flag()` and `set_flag()` helpers directly manipulate the `m_flags` member.

**crn\_file\_type m\_file\_type**: Default: cCRNFileTypeCRN. Output file type. May be `cCRNFileTypeCRN` or `cCRNFileTypeDDS`.

**crn\_uint32 m\_faces**: Default: 1. Set to 1 to compress 2D textures, or 6 to compress cubemaps.

**crn\_uint32 m\_width** and **crn\_uint32 m\_height**: Default: (0,0). The source texture's topmost (largest) mipmap dimensions in pixels. Must be in the range [1, cCRNMaxLevelResolution], non-power of 2 is OK, non-square OK. Textures that don't have dimensions divisible by 4 will be padded to the next multiple of 4.

**crn\_uint32 m\_levels**: Default: 1. The source texture's total mipmap chain size, where 1 is not mipmapped. Must be in the range [1, cCRNMaxLevels].

**crn\_format m\_format**: Default: cCRNFmtDXT1. Sets the output file's compressed pixel format.

**crn\_uint32 m\_flags**: Defualt: `cCRNCompFlagPerceptual` | `cCRNCompFlagHierarchical` | `cCRNCompFlagUseBothBlockTypes`. Compressor flags logically OR'd together, see the [crn\_comp\_flags enum](API_Docs#enum_crn_comp_flags.md).


**const crn\_uint32`*` m\_pImages`[`cCRNMaxFaces`]``[`cCRNMaxLevels`]`**: Default: All NULL. 2D array of pointers to 32bpp RGBA input images. The red component is always first in memory, independent of platform endianness.

**float m\_target\_bitrate**: Default: 0. Target bitrate. If non-zero, the compressor will use an interpolative search to find the highest quality level that results in a file length that is <= the target bitrate. If it fails to find a bitrate high enough, the compressor will disable adaptive block sizes (by disabled the cCRNCompFlagHierarchical flag) and try again. This process can be pretty slow.

**crn\_uint32 m\_quality\_level**: Default: cCRNMaxQualityLevel (255). Sets the desired quality level (higher=better). Must range between [cCRNMinQualityLevel, cCRNMaxQualityLevel]. Note that .CRN and .DDS quality levels are not compatible with each other from an image quality standpoint.

m\_quality\_level directly controls the endpoint/selector palette sizes used by the .CRN/clustered .DDS frontends.

**crn\_uint32 m\_dxt1a\_alpha\_threshold**, **crn\_dxt\_quality m\_dxt\_quality**, **crn\_dxt\_compressor\_type m\_dxt\_compressor\_type**: These parameters are only relevant when compressing to .DDS files.

**crn\_uint32 m\_alpha\_component**: Default: 3. Specifies which source image component contains the alpha channel.

**crn\_uint32 m\_num\_helper\_threads**: Number of helper threads to create to assist the compressor. 0=no threading. Must be in the range [0,cCRNMaxHelperThreads].

**crn\_uint32 m\_userdata0**, **crn\_uint32 m\_userdata1**: Default: 0. These two 32-bit values are written directly to the header of the output .CRN file. They can be retrieved from a .CRN file by using the `crnd::crnd_get_texture_info()` helper function in `inc/crn_decomp.h`.

**crn\_progress\_callback\_func m\_pProgress\_func**, **void`*`                      m\_pProgress\_func\_data**: Pointer to a user-provided progress function and user data. This function is called periodically during compression, and can be used to terminate compression before it completes.

Various low-level .CRN specific parameters:

**float m\_crn\_adaptive\_tile\_color\_psnr\_derating** and **float m\_crn\_adaptive\_tile\_alpha\_psnr\_derating**: Default: 2.0f PSNR. Controls how aggressively the frontend uses large (non-4x4) tiles. Higher settings result in fewer tiles, resulting in lower quality/more blockiness, but smaller files. If this value is set too high the output may become too blocky.

**crn\_uint32 m\_crn\_color\_endpoint\_palette\_size**, **crn\_uint32 m\_crn\_color\_selector\_palette\_size**, **crn\_uint32 m\_crn\_alpha\_endpoint\_palette\_size**, **crn\_uint32 m\_crn\_alpha\_selector\_palette\_size**: Default: 0. These parameters allow the caller to directly control the palette sizes used by the frontend. The `cCRNCompFlagManualPaletteSizes` flag must be set.


---


## Public Functions ##

### Memory Allocation ###
```
#define CRNLIB_MIN_ALLOC_ALIGNMENT sizeof(size_t) * 2

typedef void*  (*crn_realloc_func)(void* p, size_t size, size_t* pActual_size, bool movable, void* pUser_data);
typedef size_t (*crn_msize_func)(void* p, void* pUser_data);

void crn_set_memory_callbacks(crn_realloc_func pRealloc, crn_msize_func pMSize, void* pUser_data);

void crn_free_block(void *pBlock);
```

By default, crnlib calls the usual C-API's to manage memory (`malloc`, `realloc`, `free`, etc.). Call `crn_set_memory_callbacks` to globally override this behavior. The user must implement two callbacks, one to handle block allocation/reallocation/freeing, and another that returns the size of allocated blocks.

This function is not thread safe, so don't call it while another thread is inside the library.

The custom realloc and msize functions must be implemented in a thread safe manner. These functions can be called from multiple threads when threaded compression is enabled.

All block pointers returned by the realloc callback must be aligned to at least `CRNLIB_MIN_ALLOC_ALIGNMENT` bytes.

### realloc callback ###

The custom reallocation function callback `crn_realloc_func` must examine its input parameters to determine the caller's actual intent. If the input pointer `p` is NULL, the caller wants to allocate a block which must be at least as large as `size`. NULL is returned if the allocation fails.

If `p` is not NULL but `size` is 0, the caller wants to free the block pointed to by `p`.

Otherwise, the caller wants to attempt to change the size of the block pointed to by `p`. In this case, if `movable` is true, it is acceptable to physically move the block to satisfy the reallocation request. If `movable` is false, the block **must not** be moved. NULL is returned if reallocation fails for any reason. In this case, the original allocated block must remain allocated.

If `pActual_size` is not NULL, `*pActual_size` should be set to the actual size of the returned block.

### crn\_free\_block function ###

Call this function to free the memory blocks allocated and returned by `crn_compress()`, `crn_decompress_crn_to_dds()`, or `crn_decompress_dds_to_images()`.

## Compression ##

### crn\_compress functions (overloaded) ###
```
void *crn_compress(const crn_comp_params &comp_params, 
  crn_uint32 &compressed_size, crn_uint32 *pActual_quality_level = NULL, float *pActual_bitrate = NULL);

void *crn_compress(const crn_comp_params &comp_params, const crn_mipmap_params &mip_params, 
  crn_uint32 &compressed_size, crn_uint32 *pActual_quality_level = NULL, float *pActual_bitrate = NULL);
```

These functions compress a 32-bit/pixel texture to either: a regular DX9-style .DDS file, a "clustered" (or reduced entropy) .DDS file, or a .CRN file in memory.

This function is overloaded. The first variant cannot automatically generate mipmap levels, and the second one can.

Input parameters:

  * **comp\_params** is the [compression parameters struct](API_Docs#enum_crn_comp_params.md).

  * **compressed\_size** will be set to the size of the returned memory block containing the output file. The returned block must be freed by calling `crn_free_block()`.

  * **`*`pActual\_quality\_level** will be set to the actual quality level used to compress the image. May be NULL.

  * **`*`pActual\_bitrate** will be set to the output file's effective bitrate, possibly taking into account LZMA compression. May be NULL.

Return values:
> A pointer to the compressed file data, or NULL on failure. The returned block must be freed by calling `crn_free_block()`. The **compressed\_size** parameter will be set to the size of the returned memory buffer.

Notes:
  * A "regular" .DDS file is compressed using normal (plain block by block) DXTn compression at the specified DXT quality level, using multiple threads if threading is enabled.
  * A "clustered" DDS file is compressed using clustered DXTn compression to either the target bitrate or the specified integer quality factor.
  * The output file is a standard DX9 format DDS file, except the compressor assumes you will be later losslessly compressing the DDS output file using the LZMA algorithm.
  * A texture is defined as an array of 1 or 6 "faces" (6 faces=cubemap), where each "face" consists of between [1,cCRNMaxLevels] mipmap levels.
  * Mipmap levels are simple 32-bit 2D images with a pitch of width\*sizeof(uint32), arranged in the usual raster order (top scanline first). Each pixel is arranged in memory as [R,G,B,A], where R is always first independent of platform endianness.
  * The image pixels may be grayscale (YYYX), grayscale/alpha (YYYA), 24-bit RGBX, or 32-bit RGBA colors (where "X"=don't care).
  * If the input is not sRGB, be sure to clear the `cCRNCompFlagPerceptual` flag in the [crn\_comp\_params](API_Docs#enum_crn_comp_params.md) struct.

For a usage example, see [example1.cpp](http://code.google.com/p/crunch/source/browse/trunk/example1/example1.cpp).

## Transcoding ##

### crn\_decompress\_crn\_to\_dds function ###
```
void *crn_decompress_crn_to_dds(const void *pCRN_file_data, crn_uint32 &file_size);
```

`crn_decompress_crn_to_dds()` transcodes an entire .CRN file to .DDS using the [inc/crn\_decomp.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crn_decomp.h) header file library to do most of the heavy lifting. The output .DDS file's format is guaranteed to be one of the DXTn formats in the `crn_format` enum. This is a very fast operation, because the .CRN format is explicitly designed to be efficiently transcodable to DXTn.

For more control over decompression (particularly over memory management, and to implement palette caching), see the lower-level helper functions in [inc/crn\_decomp.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crn_decomp.h), which do not depend at all on crnlib.

For a usage example, see [example1.cpp](http://code.google.com/p/crunch/source/browse/trunk/example1/example1.cpp).

## Decompressing ##

### crn\_decompress\_dds\_to\_images function ###
```
struct crn_texture_desc
{
   crn_uint32 m_faces;
   crn_uint32 m_width;
   crn_uint32 m_height;
   crn_uint32 m_levels;
   crn_uint32 m_fmt_fourcc; // Same as crnlib::pixel_format
};
bool crn_decompress_dds_to_images(const void *pDDS_file_data, crn_uint32 dds_file_size, 
  crn_uint32 **ppImages, crn_texture_desc &tex_desc);

void crn_free_all_images(crn_uint32 **ppImages, const crn_texture_desc &desc);
```

`crn_decompress_dds_to_images()` decompresses an entire .DDS file in any supported compressed/uncompressed pixel format to one or more uncompressed 32-bit/pixel images. See the crnlib::pixel\_format enum in [inc/dds\_defs.h](http://code.google.com/p/crunch/source/browse/trunk/inc/dds_defs.h) for a list of the supported .DDS pixel formats.

The caller is responsible for freeing each returned image, either by calling `crn_free_all_images()` or by manually calling `crn_free_block()` on each image pointer.

For a usage example, see [example1.cpp](http://code.google.com/p/crunch/source/browse/trunk/example1/example1.cpp).

## DXTn Block Compression ##

```
typedef void *crn_block_compressor_context_t;

crn_block_compressor_context_t crn_create_block_compressor(const crn_comp_params &params);

void crn_compress_block(crn_block_compressor_context_t pContext, const crn_uint32 *pPixels, void *pDst_block);

void crn_free_block_compressor(crn_block_compressor_context_t pContext);
```

These functions allow the caller to compress 4x4 pixel image blocks to any non-swizzled DXTn format supported by crnlib: DXT1, DXT3, DXT5, DXT5A, DXN\_XY and DXN\_YX (basically BC1-BC5). For a usage example, see [example3.cpp](http://code.google.com/p/crunch/source/browse/trunk/example3/example3.cpp).

Unlike most other DXTn block compressors (such as ATI\_Compress or squish) crnlib's is stateful, so for efficient usage you should call `crn_create_block_compressor()` to create a state object and reuse it as many times as possible. (If you're curious, the state consists of an endpoint cache, and a bunch of heap memory used by the compressor for temporary arrays.) Don't call `crn_create_block_compressor()` once for each block to compress, or performance will be dreadful.

crnlib's DXTn endpoint optimizer actually supports any number of source pixels (i.e. from 1 to thousands, not just 16), but for simplicity this API currently only supports 4x4 texel blocks.

`crn_compress_block()` is thread safe (it may be called in parallel from multiple threads), as long as each thread uses its own state context.

## Helper Functions ##

crnlib exposes a number of straightforward functions to convert the crn-related enums defined above to ANSI/Unicode strings and back. There are also functions to retrieve various bits of info about the supported pixel formats.

They don't seem worth individually listing here, just see the [inc/crnlib.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crnlib.h).