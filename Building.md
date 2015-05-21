# Building #

## Windows ##

`crn.2008.sln` builds crnlib and the command line tool, and `crn_examples.2008.sln` builds the examples. Both are Visual Studio 2008 (VC9) solution file containing projects for Win32 and x64.

crnlib and crunch have also been built with VS2005, VS2010, and gcc 4.5.0 ([TDM GCC+MinGW](http://tdm-gcc.tdragon.net/)).  A Codeblocks 10.05 workspace is also included (but building crnlib this way hasn't been tested a whole lot - it mostly exists to make porting to Linux using gcc a little easier).

## Linux ##

I simple makefile to build only the crunch executable is in crnlib/Makefile. I've only built/tested under 32-bit Ubuntu 12.04, however 64-bit should be easy to get working with minimal tweaks. Alternately, you can use the Codeblocks v10.05 Linux workspace in "crn\_linux.workspace".

**Important**: When compiling with gcc, be sure to use **-fno-strict-aliasing** otherwise crnlib will randomly misbehave. This also applies to the transcoder library in [inc/crn\_decomp.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crn_decomp.h).

## [example1](http://code.google.com/p/crunch/source/browse/trunk/example1/example1.cpp) ##
Demonstrates how to use crnlib's high-level C-helper
compression/decompression/transcoding functions in [inc/crnlib.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crnlib.h). It's a
fairly complete example of crnlib's functionality.

## [example2](http://code.google.com/p/crunch/source/browse/trunk/example2/example2.cpp) ##
Shows how to transcodec .CRN files to .DDS using **only**
the functionality in [inc/crn\_decomp.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crn_decomp.h). It does not link against against
crnlib.lib or depend on it in any way. (Note: The complete source code,
approx. 4800 lines, to the CRN transcoder is included in [inc/crn\_decomp.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crn_decomp.h).)

example2 is intended to show how simple it is to integrate CRN textures
into your application.

## [example3](http://code.google.com/p/crunch/source/browse/trunk/example3/example3.cpp) ##
Shows how to use the regular, low-level DXTn block compressor
functions in [inc/crnlib.h](http://code.google.com/p/crunch/source/browse/trunk/inc/crn_decomp.h). This functionality is included for
completeness. (Your engine or toolchain most likely already has its own
DXTn compressor. crnlib's compressor is very competitive to most available closed and open source CPU-based
compressors.)