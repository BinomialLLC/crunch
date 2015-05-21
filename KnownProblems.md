# Known Issues/Bugs #

  * .DDS files written by crunch v1.00 (and output by crnlib v1.00) don't have the pitch/linearsize fields set in the DDS header. Some DDS readers expect valid values here (and expect the DDSD\_LINEARSIZE flag to be set too). This should be fixed in v1.01.

  * You really should provide crnlib with raw, 24/32-bit source textures. Don't provide it with second generation textures that have already been DXTn or JPEG compressed. Of course, you can do so, and I know of one company doing this to repackage existing assets (without source art) so they download more quickly, but obviously don't expect the highest quality.

> crnlib's custom DXT1 endpoint optimizer can detect pixel blocks which have been previously compressed to DXT1 using another DXTn compression library. It attempts to derive the endpoints originally used to compress these blocks in order to reduce artifacts, but it's not always successful.

  * crnlib currently assumes you'll be further losslessly compressing its output .DDS files using LZMA. However, some engines use weaker codecs such as LZO, zlib, etc., so crnlib's bitrate measurements will be inaccurate. It should be easy to allow the caller to plug-in custom lossless compressors for bitrate measurement.

  * Compressing to a desired bitrate can be very (to extremely) time consuming, especially when processing large (2k or 4k) images to the .CRN format. There are several high-level optimizations employed when compressing to clustered DXTn .DDS files using multiple trials, but not so for .CRN.

> The current approach compresses the input image multiple times, using an [interpolation search](http://en.wikipedia.org/wiki/Interpolation_search) to find the quality level index that gets closest to the target bitrate. The lib does have some functionality to save the closest quality level found for later runs, but the command line tool doesn't expose this feature yet.

  * The .CRN compressor doesn't use 3 color (transparent) DXT1 blocks at all, only 4 color blocks. (Supporting both block types would be a major pain at this point.) So it doesn't support DXT1A transparency, and its output quality suffers a little due to this limitation. (Note that the clustered DXTn compressor does not have this limitation.)

  * DXT3 is not supported when writing .CRN or clustered DXTn DDS files. (DXT3 is supported by crnlib when compressing to regular DXTn DDS files.) You'll get DXT5 files if you request DXT3. However, DXT3 is supported by the regular DXTn block compressor.

  * The DXT5\_CCXY format uses a simple YCoCg encoding that seems workable but hasn't been tuned for max. quality yet.

  * Ignore the SSIM statistics printed when using the -imagestats option - it's currently bogus. I've been tuning the codec using PSNR/RMSE so far.

  * The crn\_decomp.h header file library is freaking huge (~4800 lines). It would be nice to port it to C and shrink it.