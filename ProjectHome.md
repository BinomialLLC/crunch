**crunch** is an open source ([ZLIB license](http://www.opensource.org/licenses/Zlib)) lossy texture compression library and command line compression tool for developers that distribute and use
content in the [DXT1/5/N](http://en.wikipedia.org/wiki/S3_Texture_Compression) or [3DC/BC5](http://en.wikipedia.org/wiki/3Dc) compressed [mipmapped](http://en.wikipedia.org/wiki/Mipmap) GPU texture formats. It consists of a command line tool named "crunch", a compression library named "crnlib", and a single-header file, completely stand alone .CRN->DXTc transcoder C++ class located in [inc/crn\_decomp.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crn_decomp.h). crnlib's results are competitive to transform based recompression approaches, as shown [here](http://code.google.com/p/crunch/wiki/Stats).

If you're going to SIGGRAPH this year, Brandon Jones is going to be showing crunch at the WebGL BoF (Birds of a Feather) event: [Brandon Jones: Crunch/DXT/Rage demo](http://www.khronos.org/news/events/siggraph-los-angeles-2012). Wednesday, August 8, 4-5pm, JW Marriott Los Angeles at LA Live, Gold Ballroom – Salon 3

For background info/history of crunch: [blog post](http://richg42.blogspot.com/2012/07/doug-has-updated-his-blog-hes-now.html) (or the original post [here](http://richg42.blogspot.com/2012/07/the-saga-of-crunch.html)).


---


## Technical Summary ##

crnlib can compress mipmapped 2D textures and cubemaps to
approximately .8-1.25 bits/texel, and normal maps to 1.75-2 bits/texel with reasonable quality (comparable or better than JPEG followed by real-time or even offline DXTc compression). The actual bitrate is indirectly controllable using an integer quality factor (like JPEG), or directly by specifying a target bitrate. crnlib implements a form of "clustered DXTn" compression, which is ultimately bounded by the quality achievable by DXTn itself. (DXTn's quality is actually [pretty low](http://cbloomrants.blogspot.com/2008/11/11-18-08-dxtc-part-2.html), but it's directly supported by Direct3D and OpenGL, and in hardware by practically every PC/console GPU.)

The approach used by crnlib differs significantly from [other approaches](http://www.intel.com/jp/software/pix/324337_324337.pdf), such as using JPEG decompression followed by compression using a real-time DXTn compressor. Its compressed texture data format was carefully designed to be quickly transcodable directly to DXTn with no intermediate recompression step.
The single threaded transcode to DXTn rate is approximately 100 (DXT5/3DC) and 250 (DXT5A/DXT1) megatexels/sec. (Core i7 2.6 GHz). Fast random access to individual mipmap levels is supported. No pixel-level operations are performed during transcoding. The core transcode loops operate at the 4x4 block or 8x8 macroblock level.

crnlib can also generate standard .DDS files that, when losslessly post-compressed using LZMA/Deflate/LZO/etc., result in much smaller compressed files. (This is effectively a form of [rate-distortion optimization](http://en.wikipedia.org/wiki/Rate%E2%80%93distortion_optimization) applied to DXT+LZMA.) This feature allows easy integration into any engine or graphics library that already supports .DDS files and applies some form of lossless post-compression to the DXTn bits stored in those files (most engines do). Here's a Windows app that demonstrates this capability: [DDSExport](http://sites.google.com/site/richgel99/ddsexport).

The .CRN file format supports BC1-BC5, corresponding to the following DXTn texture formats: DXT1 (but not DXT1A), DXT5, DXT5A, and DXN/ATI\_3DC (either XY or YX component order).

The library also supports several popular swizzled variants, typically used for normal maps (several are supported by [AMD's Compressonator](http://developer.amd.com/tools/compressonator/pages/default.aspx)):
DXT5\_XGBR, DXT5\_xGxR, DXT5\_AGBR, and DXT5\_CCxY (experimental luma-chroma YCoCg).

crnlib currently compiles under Linux (using gcc, currently only x86 but x64 support should be easy), and Windows (both x86 and x64) using Visual Studio 2008/2010. It also compiles and has been minimally tested with Codeblocks 10.05 using [TDM-GCC x64/MinGW](http://tdm-gcc.tdragon.net/) under Windows.

crnlib also contains some other possibly useful bits of code, like a multithreaded version of my [image resampler](http://code.google.com/p/imageresampler/) class, and my fast symbol\_codec class.


---


## Upcoming Release ##

Planned features for v1.05 as of 11/25/12.

  * Continue experimenting with PVRTC: Implement a PVRTC decompressor, then a basic compressor.
  * Add support for "raw" CRN files, assuming the user will post compress using gzip (useful for Javascript/WebGL apps). This is my highest priority after releasing v1.04.
  * Now that miniz is in the project, add support for DXTc+ZLIB rate distortion optimization, instead of just DXTc+LZMA.
  * Figure out the most elegant way to add support for writing 555, 565, and 4444 .DDS/.KTX textures (useful for mobile).
  * Compile with LLVM
  * Compile and test under 64-bit Linux (I only have 32-bit installed right now). Improve makefile.
  * Add rate distortion optimization for .KTX files


---


## Release History ##

  * v1.04 (SVN trunk) - Nov. 25, 2012: Currently only checked into SVN trunk:
    * Added "-fno-strict-aliasing" gcc compiler option, otherwise crnlib randomly crashes in weird spots.
    * Fixed various DDS reader problems.
    * Better Linux support. Added makefile with proper command line options, see crnlib/Makefile (thanks alonzakai). Modified gcc compiler options used by Codeblocks Linux projects.
    * Basic ETC1 support - vanilla 4x4 block packing/unpacking. (More or less complete - see [rg\_etc1](http://code.google.com/p/rg-etc1/).) No support for rate distortion optimized or .CRN ETC1 files, though, just vanilla block by block ETC1.
    * .KTX file format reading/writing. The .KTX file format is not well supported by any tools yet - I've tested crnlib's KTX writer as best as I can with what's available.
    * Low-level support for reading/writing/flipping/unflipping Y flipped textures in all possible formats (useful to OpenGL/OpenGL ES devs) (more or less complete).
    * Integrate miniz and jpeg-compressor to crnlib so crunch can write PNG's and read progressive JPEG's without adding messy external dependencies (completed).
    * Fixed assertion problems in crn\_threading's "task\_pool" class.

  * v1.03 (SVN tags/v103) - Apr. 26, 2012: Currently only checked in to SVN trunk until I finish the Linux port and fully regression test the codec. If you would like to give the Linux port a spin, you can download prebuilt binaries of v1.03 for 32-bit Linux/Win32/x86 [here](http://www.tenacioussoftware.com/crunch_v103_prerelease_win_linux_execs.7z) (or just build them yourself using Codeblocks v10.05).
  * v1.02 - Apr. 22, 2012: Full Linux port of crnlib and crunch for Evan Parker to test at Google. Lots of files modified: Got rid of all wchar\_t usage (wasn't worth the effort to port), now using LZHAM's more cross platform multithreading/threadpool code, added platform independent file and directory I/O wrappers. Also, I optimized the task\_pool "join" method a bit (it now uses a semaphore compared to spinning with sleep(1) while waiting for the workers to finish).
  * v1.01 - Apr. 15, 2012: DDS reader/writer fixes, -adding -usesourceformat command line option, merged over a few minor fixes from the ddsexport branch. Thanks to the devs at [The Happy Cloud](http://www.thehappycloud.com/) for reporting the DDS header problem.
  * v1.00 - Dec. 27, 2011: Initial release


---


## Applications Using crunch ##

Jean Sabatier reports that the [Fly! Legacy](http://fly.simvol.org/indexus.php) open source flight simulator is using a database of ~30,000 DXT5 .CRN textures as part of its texture streaming system.

[Planetside 2](http://www.planetside2.com/) is using crunch's rate distortion optimized (or "clustered") DXTc compressor and the [LZHAM lossless codec](http://code.google.com/p/lzham/) for most of its texture assets, which greatly reduces the title's download time.

[Evan Parker](http://plus.google.com/104261567553968048744) at Google has compiled the CRN->DXTc transcoder header file library [inc/crn\_decomp.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crn_decomp.h) to Javascript using [Emscripten](https://github.com/kripken/emscripten/wiki) (here's his [post](http://plus.google.com/104261567553968048744/posts/28jEPHtuhq5) with details). This allows him to quickly transcode CRN compressed images/textures directly to DXTc in Javascript (which is ~10x faster than decoding JPG's and packing to DXTc). Here's a [demo](http://www-cs-students.stanford.edu/~eparker/files/crunch/decode_test.html) (needs the latest Chrome beta with DXT texture support to fully function), and here's more [technical info](http://www-cs-students.stanford.edu/~eparker/files/crunch/more_info.html).

Brandon Jones has tested the Javascript emscripten port of crn\_decomp.h and reported his results [here](http://plus.google.com/101501294230020638079/posts/KJ42NGorLTj).

I believe the first shipping product to use crunch/crnlib compressed textures is ["Zombie Track Meat"](http://toucharcade.com/2012/03/10/gdc-2012-a-look-at-the-zombie-track-meat-collaboration/), a free to play NaCL game in the Chrome App Store [here](http://chrome.google.com/webstore/detail/jmfhnfnjfdoplkgbkmibfkdjolnemfdk). More technical info [here](http://fuzzycube.blogspot.com/2012/04/zombie-track-meat-post-mortem.html). ZTM uses .CRN compressed textures and the CRN->DXTc real-time transcoder library.

If you use crunch/crnlib, I would greatly appreciate it if you sent me an email with any feedback, or info on how you're using it in practice. (Credits somewhere would also be much appreciated, but are not required.)


---


## Documents ##

  * [Various RMSE statistics (charts, graphs, etc.), and an example image (kodim14) compressed at various bitrates](http://code.google.com/p/crunch/wiki/Stats)
  * [Building the examples](http://code.google.com/p/crunch/wiki/Building)
  * [crnlib API Documentation](http://code.google.com/p/crunch/wiki/API_Docs)
  * [Supported file formats](http://code.google.com/p/crunch/wiki/SupportedFormats)
  * [Known problems](http://code.google.com/p/crunch/wiki/KnownProblems)
  * [Technical details](http://code.google.com/p/crunch/wiki/TechnicalDetails) is a high level description of the CRN data format, the CRN->DXTn transcoding process, and how the current compressor works.
  * Here's an external website showing the quality achievable with an early version of the lib (called hx/hxc at the time) at various palette (quality) settings: [Kodak test images](http://www.tenacioussoftware.com/hx/kodak/)


---


## Recommended Software ##

[AMD's Compressonator](http://developer.amd.com/gpu/compressonator/pages/default.aspx) tool is recommended to view the .DDS files created by the crunch tool and the included example projects.

Note: Some of the funky swizzled DXTn .DDS output formats (such as DXT5\_xGBR)
read/written by the crunch tool or examples deviate from the DX9 DDS
standard, so DXSDK tools such as DXTEX.EXE won't load them at all or
they won't be properly displayed. AMD's tool can view these files.


---


## Creating Compressed Textures from the Command Line (crunch.exe) ##

The simplest way to create compressed textures using crnlib is to
integrate the bin\crunch.exe (or bin\crunch\_x64.exe) command line tool
into your texture build toolchain or export process. It can write DXTn
compressed 2D/cubemap textures to regular DXTn compressed .DDS,
clustered (or reduced entropy) DXTn compressed .DDS, or .CRN files. It
can also transcode or decompress files to several standard image
formats, such as TGA or BMP. Run crunch.exe with no options for help.

The .CRN files created by crunch.exe can be efficiently transcoded to
DXTn using the stand-alone CRN transcoding header file library located in `inc/crn_decomp.h`.

Here are a few example crunch.exe command lines:

1. Compress blah.tga to blah.dds using normal DXT1 compression:

`crunch -file blah.tga -fileformat dds -dxt1`

2. Compress blah.tga to blah.dds using clustered DXT1 at an effective bitrate of 1.5 bits/texel (after the .DDS file is post-compressed using LZMA), display image statistic:

`crunch -file blah.tga -fileformat dds -dxt1 -bitrate 1.5 -imagestats`

3. Compress blah.tga to blah.dds using clustered DXT1 at quality level 100 (from [0,255]), with no mipmaps, display LZMA statistics:

`crunch -file blah.tga -fileformat dds -dxt1 -quality 100 -mipmode none -lzmastats`

3. Compress blah.tga to blah.crn using clustered DXT1 at a bitrate of 1.2 bits/texel, no mipmaps:

`crunch -file blah.tga -dxt1 -bitrate 1.2 -mipmode none`

4. Decompress blah.dds to a .tga file:

`crunch -file blah.dds -fileformat tga`

5. Transcode blah.crn to a .dds file:

`crunch -file blah.crn`

6. Decompress blah.crn, writing each mipmap level to a separate .tga file:

`crunch -split -file blah.crn -fileformat tga`

crunch.exe can do a lot more, like rescale/crop images before
compression, convert images from one file format to another, compare
images, process multiple images, etc.


---


## Using crnlib ##

The most flexible and powerful way of using crnlib is to integrate the
library into your editor/toolchain/etc. and directly supply it your
raw/source texture bits. See the C-style API's and comments in
[inc/crnlib.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crnlib.h).

To compress, #include "crnlib.h", fill in the `crn_comp_params` struct, and call one function:

```
  void *crn_compress(const crn_comp_params &comp_params, crn_uint32 &compressed_size,   
        crn_uint32 *pActual_quality_level = NULL, float *pActual_bitrate = NULL);
```

The returned pointer will be NULL on failure, or a pointer to the .CRN or .DDS file data.

Or, if you want crnlib to also generate mipmaps, you call this function:

```
  void *crn_compress(const crn_comp_params &comp_params, const crn_mipmap_params &mip_params, 
        crn_uint32 &compressed_size, crn_uint32 *pActual_quality_level = NULL, float *pActual_bitrate = NULL);
```

You can also transcode/uncompress .DDS/.CRN files to raw 32bpp images
using `crn_decompress_crn_to_dds()` and `crn_decompress_dds_to_images()`.

Internally, crnlib just uses `inc/crn_decomp.h` to transcode textures to
DXTn. If you only need to transcode .CRN format files to raw DXTn bits
at runtime (and not compress), you don't actually need to compile or
link against crnlib at all. Just include inc/crn\_decomp.h, which
contains a completely self-contained CRN transcoder in the "crnd"
namespace. The `crnd_get_texture_info()`, `crnd_unpack_begin()`,
`crnd_unpack_level()`, etc. functions are all you need to efficiently get
at the raw DXTn bits, which can be directly supplied to whatever API or
GPU you're using. (See example2.)


---


## Related Links ##

I'm not aware of any other open source libraries that solve this problem in a usable, "out of the box" manner yet, but these links are interesting:

  * [Experiments in Luma-Optimized and Mipmapped DXT1 Compression](http://sites.google.com/site/richgel99/luma_chroma_texture_compression)
  * [ddsexport](http://sites.google.com/site/richgel99/ddsexport) - A GUI demo of crnlib's ability to create rate-distortion optimized DDS textures.
  * Strom and Wennersten, [Lossless Compression of Already Compressed Textures](http://www.jacobstrom.com/publications/StromWennerstenHPG2011.pdf)
  * van Waveren, [Real-Time DXT Compression](http://www.intel.com/jp/software/pix/324337_324337.pdf)
  * Excellent public domain real-time DXTn compressor: [rygDXT](http://www.farb-rausch.de/~fg/code/) and [stb\_dxt.h](http://nothings.org/stb/stb_dxt.h)
  * [Charles Bloom's various blog posts on DXT compression](http://cbloomrants.blogspot.com/2009/06/06-17-09-dxtc-more-followup.html)
  * [FastDXT](http://www.evl.uic.edu/cavern/fastdxt/)
  * [Spiro's DXT Compression Algorithm Experiments](http://lspiroengine.com/?p=260)
  * [LSDxt DXT](http://lspiroengine.com/?p=516) - Spiro's texture compression tool
  * [Variable Bit Rate GPU Texture Compression](http://www.csee.umbc.edu/~olano/papers/#texcompress)
  * [libsquish](http://code.google.com/p/libsquish/) - Open source (MIT license) DXT compression library.
  * [Super Simple Texture Compression](http://github.com/divVerent/s2tc/wiki) - Alternative DXT1-compatible compression method that is purposely limited to a subset of DXT1 (only uses 2 colors per block, and is effectively only 1 bit per selector). The [Quality Comparison](http://github.com/divVerent/s2tc/wiki/QualityComparison) page is interesting.


---


## Special Thanks ##

Thanks to [Colt McAnlis](https://plus.google.com/105062545746290691206/posts) at Google for porting the CRN->DXT transcoder library (in crn\_decomp.h) to Native Client, and for mentioning crunch in his [GDC presentation](http://www.youtube.com/watch?v=7bJ-D1xXEeg) on his texture compression R&D.

Some portions of this software make use of public domain code
originally written by Igor Pavlov (LZMA), RYG's public domain real-time DXTn compressor, and stb\_image.c from [Sean Barrett](http://nothings.org/).

Many thanks to Violet Koppel for funding much of crnlib's development in 2009. Also, thanks to Colt again, and John Brooks at [Blue Shift, Inc.](http://www.blueshiftinc.com/) for helping test and giving feedback on crnlib. Also thanks to Charles Bloom's informative [blog posts](http://cbloomrants.blogspot.com/2008/11/11-18-08-dxtc.html) on his work on DXT compression.


---


## Support Contact ##

For any questions or problems with this software please **email** [Rich Geldreich](http://www.mobygames.com/developer/sheet/view/developerId,190072/) at <richgel99 _at_ gmail _dot_ com>. Here's my [twitter page](http://twitter.com/#!/richgel999).