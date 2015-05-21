# Compression Algorithm Details #

This is pretty high level and could be much better. I'll improve this over time, for now I hope this is enough:


---


## Data Format ##

The easiest way to describe how crnlib works is to start at the compressed data stream and the transcoding process and work backwards to the compressor, which also mirrors the design process followed when I designed crnlib.

.CRN DXT1 files consist of a small header, followed by a [DPCM](http://en.wikipedia.org/wiki/DPCM)+[Huffman](http://en.wikipedia.org/wiki/Huffman_Compression) compressed endpoint palette, and a DPCM+Huffman compressed selector palette. (DXT5 files contain two more palettes for alpha endpoints and selectors. Also, I'm not sure that "palette" is the best word. "Codebook" may be more appropriate, but graphics programmers seem more familiar with the concept of palettes.)

Here's a visualization of the DXT1 color selector palette for kodim04.png. It's 2692x16. The 2-bit selectors where scaled to [0,255]. (I think this palette was sorted by similarity, which is one of the palette orderings tested by the compressor's backend.)

![http://crunch.googlecode.com/svn/wiki/crunch_selectors.png](http://crunch.googlecode.com/svn/wiki/crunch_selectors.png)

And here's a visualization of the DXT1 color endpoint palette:
![http://crunch.googlecode.com/svn/wiki/crunch_color_endpoints.png](http://crunch.googlecode.com/svn/wiki/crunch_color_endpoints.png)

(These a very wide images, so they get downsampled when viewed in the wiki.)

This particular color endpoint palette contains 2415 entries (horizontal axis), where each entry contains a 32-bit integer containing two 565 colors (vertical axis, enlarged by 8x in this image).

Each mipmap is divided up into 8x8 pixel "macroblocks". Each macroblock corresponds to four 4x4 pixel DXTn blocks arranged in a 2x2 checkerboard pattern. Each macroblock is adaptively subdivided by the compressor into one or more "tiles". Very simple macroblocks (say solid ones that use only a single color) can use a single 8x8 pixel tile, but more complex macroblocks can use any non-overlapping combination of 8x4, 4x8, or 4x4 tiles. (There are 9 possible ways of arranging the tiles in a single macroblock.)

In this image, the macroblock tile boundaries are outlined in gray:

![http://crunch.googlecode.com/svn/wiki/crunch_macroblock_tiles.png](http://crunch.googlecode.com/svn/wiki/crunch_macroblock_tiles.png)

Notice that the more complex areas of the image contain smaller tiles, so these image areas get assigned more endpoints. Simpler areas use larger tiles, so the DXT1 blocks in these tiles are constrained to share the same endpoints. Also, a single color endpoint pair can be shared by many tiles, independent of their location in the image.

The endpoint/selector palettes are shared by all mipmap levels present in the .CRN file.

For each tile, a compressed index is sent to select the macroblock tile arrangement, followed by between one to four DPCM+Huffman compressed endpoint palette indices. Four selector indices (again coded using DPCM+Huffman) are always sent immediately after the endpoint(s). The macroblock rows are raster scanned in a serpentine order: left->right, then right->left, etc.

The C++ code for the transcoder's inner loop is in [crn\_decomp.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crn_decomp.h). DXT1 textures are handled by `crn_unpacker::unpack_dxt1()`.

Zeng's technique is used to order the palettes so DPCM coding of the various block palette indices works efficiently. See [An efficient color re-indexing scheme for palette-based compression](http://ieeexplore.ieee.org/xpl/freeabs_all.jsp?arnumber=899448)

For some textures, it's more efficient to reorder the palettes by similarity (effectively the [traveling salesman problem](http://en.wikipedia.org/wiki/Traveling_salesman_problem)) so they compress more effectively, but this can hurt index compression. The compressor tries several palette orderings and chooses whatever is cheapest overall.

The example1 tool can display a bunch of information about .CRN files, such as the compressed size of each palette, Huffman tables, and mip levels. For example:

```
   E:\crunch17_3\bin>example1 i kodim04.crn
   example1 - Version v1.00 Built Dec 27 2011, 17:18:08
   Loading source file: kodim04.crn
   crnd_validate_file:
   File size: 85949
   ActualDataSize: 85949
   HeaderSize: 110
   TotalPaletteSize: 12687
   TablesSize: 1448
   Levels: 10
   LevelCompressedSize: 51830 14460 3968 1050 271 68 26 12 13 6 0 0 0 0 0 0
   ColorEndpointPaletteSize: 2415
   ColorSelectorPaletteSize: 2692
   AlphaEndpointPaletteSize: 0
   AlphaSelectorPaletteSize: 0
   crnd_get_texture_info:
   Dimensions: 512x768
   Levels: 10
   Faces: 1
   BytesPerBlock: 8
   UserData0: 0
   UserData1: 0
   CrnFormat: DXT1
```


---


## Transcoding to DXTn ##

To transcode a mipmap level to DXTn, the palettes must be first unpacked, either into a temporary array or cache. Currently, all mipmaps in a .CRN file share the same set of endpoint/selector palettes. To generate the DXTn bits, the transcoder iterates through each macroblock and decodes the palette indices. The actual DXTn bits are effectively just memcpy'd from the palette arrays directly into the destination DXTn texture. The transcoder doesn't care at all what the endpoint/selector palette entries actually consist of during transcoding -- it just copies the bits. Transcoding is quite fast because it works at the macroblock/block level, never at the pixel level.


---


## Compression ##

The compressor is very complex, partially due to the weird and surprisingly deep properties imposed by the DXTn block format. It consists of two independent parts, called the "frontend" and "backend". The frontend is by far the most complex, and a good chunk of crnlib is devoted to helper classes used by the frontend.

The frontend, located in [dxt\_hc.cpp/h](http://code.google.com/p/crunch/source/browse/trunk/crnlib/crn_dxt_hc.cpp), takes the 24/32bpp source texture mipmaps as inputs. It adaptively subdivides the texture macroblocks into tiles, finds the endpoint and selector clusters, and then generates optimized, but unordered palettes based off these clusters. The backend, located in [crn\_comp.cpp/h](http://code.google.com/p/crunch/source/browse/trunk/crnlib/crn_comp.cpp) takes the raw palettes, macroblock tile layouts, and indices supplied by the frontend and tries to efficiently code them.

The color endpoint palettes are created from their source clusters using a very high quality, scalable DXT1 endpoint optimizer located in [crn\_dxt1.cpp/h](http://code.google.com/p/crunch/source/browse/trunk/crnlib/crn_dxt1.cpp). This custom optimizer is capable of processing any number of source pixels, instead of the typical hard coded 16. crnlib's DXT1 endpoint optimizer's quality (in a PSNR sense) is comparable to ATI's, NVidia's, or squish's. (I verified this while building the endpoint optimizer by randomly extracting millions of 4x4 pixel blocks from a large corpus of game textures and photos, compressing->decompressing them using each compressor, comparing the results, and ruthlessly investigating and fixing any blocks where crnlib's output was lower quality. I hope to eventually release this tool.)

Interestingly, crnlib's DXT1 endpoint optimizer is equal or better than squish or ATI\_Compress (in a PSNR sense), and of comparable speed, without using a single line of SIMD or assembly code.

The endpoint clusterization step uses top-down [cluster analysis](http://en.wikipedia.org/wiki/Cluster_analysis), and [vector quantization](http://en.wikipedia.org/wiki/Vector_quantization) is used to create the initial selector palette. The frontend performs several feedback passes, in between the clusterization and VQ steps, to optimize quality, and the compressor uses several brute force refinement stages to improve quality even more.

Most of the compression steps are multithreaded in a relatively straightforward way: subdivide the work into independent threadpool tasks, fork to multiple threads, then join. The clusterizer is also multithreaded, where it forks to multiple threads after the initial tree subdivision steps.

The .CRN format currently utilizes [canonical Huffman coding](http://en.wikipedia.org/wiki/Canonical_Huffman_code) for speed. The symbol codelengths for each Huffman table are sent in a simple compressed manner after the header (like Deflate).


---


## The Path Forward ##

Given a fixed amount of additional developer time to improve .CRN's bitrate/quality, I think the backend would benefit the most from more work. (So far, much more effort has been devoted to the DXT1 endpoint optimizer and the frontend stages.) The current format is probably favoring transcoding speed too highly vs. ratio. Also, the Huffman tables contain too many symbols, and alternatives to the DPCM coding should be explored.

Ideas for crunch v2.0:
  * Use techniques from LZHAM to improve backend coding (mix bitwise arithmetic with semi-adaptive Huffman).
  * Port transcoder library to plain C vs. C++
  * Add smarter prediction to the macroblock tile layout selector indices
  * Native Javascript transcoders
  * Palette compression improvements
  * Split mipchain from mip0, so individual mips can be transcoded more quickly
  * Support "raw" CRN files that use no additional compression (assume they will be post-compressed by the user using gzip or LZMA - useful for Javascript/WebGL)
  * Support uncompressed palettes, for high speed random access in the transcoder
  * Investigate 16x16 or 32x32 macroblock sizes. Optimize .CRN for bitrates below 1.0 bpp.
  * Clustered (rate distortion optimized) DDS: Add support for ZLIB, LZO, and Snappy lossless post-compression