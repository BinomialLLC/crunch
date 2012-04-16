// File: crunch.cpp - Command line tool for DDS/CRN texture compression/decompression.
// This tool exposes all of crnlib's functionality. It also uses a bunch of internal crlib
// classes that aren't directly exposed in the main crnlib.h header. The actual tool is
// implemented as a single class "crunch" which in theory is reusable. Most of the heavy
// lifting is actually done by functions in the crnlib::texture_conversion namespace,
// which are mostly wrappers over the public crnlib.h functions.
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"

#include <tchar.h>
#include <conio.h>

#include "crn_win32_console.h"
#include "crn_win32_find_files.h"
#include "crn_win32_file_utils.h"
#include "crn_command_line_params.h"

#include "crn_dxt.h"
#include "crn_cfile_stream.h"
#include "crn_texture_conversion.h"

#define CRND_HEADER_FILE_ONLY
#include "crn_decomp.h"

using namespace crnlib;

const int cDefaultCRNQualityLevel = 128;

class crunch
{
   CRNLIB_NO_COPY_OR_ASSIGNMENT_OP(crunch);

   cfile_stream m_log_stream;

   uint m_num_processed;
   uint m_num_failed;
   uint m_num_succeeded;
   uint m_num_skipped;

public:
   crunch() :
      m_num_processed(0),
      m_num_failed(0),
      m_num_succeeded(0),
      m_num_skipped(0)
   {
   }

   ~crunch()
   {
   }

   enum convert_status
   {
      cCSFailed,
      cCSSucceeded,
      cCSSkipped,
      cCSBadParam,
   };

   inline uint get_num_processed() const { return m_num_processed; }
   inline uint get_num_failed() const { return m_num_failed; }
   inline uint get_num_succeeded() const { return m_num_succeeded; }
   inline uint get_num_skipped() const { return m_num_skipped; }

   static void print_usage()
   {
      console::message(L"\nCommand line usage:");
      console::printf(L"crunch [options] -file filename");
      console::printf(L"-file filename - Required input filename, wildcards, multiple /file params OK.");
      console::printf(L"-file @list.txt - List of files to convert.");
      console::printf(L"Supported source file formats: dds,crn,tga,bmp,png,jpg/jpeg,psd");
      console::printf(L"Note: Some file format variants are unsupported, such as progressive JPEG's.");
      console::printf(L"See the docs for stb_image.c: http://www.nothings.org/stb_image.c");

      console::message(L"\nPath/file related parameters:");
      console::printf(L"/out filename - Output filename");
      console::printf(L"/outdir dir - Output directory");
      console::printf(L"/outsamedir - Write output file to input directory");
      console::printf(L"/deep - Recurse subdirectories, default=false");
      console::printf(L"/nooverwrite - Don't overwrite existing files");
      console::printf(L"/timestamp - Update only changed files");
      console::printf(L"/forcewrite - Overwrite read-only files");
      console::printf(L"/recreate - Recreate directory structure");
      console::printf(L"/fileformat [dds,crn,tga,bmp] - Output file format, default=crn or dds");

      console::message(L"\nModes:");
      console::printf(L"/compare - Compare input and output files (no output files are written).");
      console::printf(L"/info - Only display input file statistics (no output files are written).");

      console::message(L"\nMisc. options:");
      console::printf(L"/helperThreads # - Set number of helper threads, 0-16, default=(# of CPU's)-1");
      console::printf(L"/noprogress - Disable progress output");
      console::printf(L"/quiet - Disable all console output");
      console::printf(L"/ignoreerrors - Continue processing files after errors. Note: The default");
      console::printf(L"                behavior is to immediately exit whenever an error occurs.");
      console::printf(L"/logfile filename - Append output to log file");
      console::printf(L"/pause - Wait for keypress on error");
      console::printf(L"/window <left> <top> <right> <bottom> - Crop window before processing");
      console::printf(L"/clamp <width> <height> - Crop image if larger than width/height");
      console::printf(L"/clampscale <width> <height> - Scale image if larger than width/height");
      console::printf(L"/nostats - Disable all output file statistics (faster)");
      console::printf(L"/imagestats - Print various image qualilty statistics");
      console::printf(L"/mipstats - Print statistics for each mipmap, not just the top mip");
      console::printf(L"/lzmastats - Print size of output file compressed with LZMA codec");
      console::printf(L"/split - Write faces/mip levels to multiple separate output files");

      console::message(L"\nImage rescaling (mutually exclusive options)");
      console::printf(L"/rescale <int> <int> - Rescale image to specified resolution");
      console::printf(L"/relscale <float> <float> - Rescale image to specified relative resolution");
      console::printf(L"/rescalemode <nearest | hi | lo> - Auto-rescale non-power of two images");
      console::printf(L" nearest - Use nearest power of 2, hi - Use next, lo - Use previous");

      console::message(L"\nDDS/CRN compression quality control:");
      console::printf(L"/quality # (or /q #) - Set Clustered DDS/CRN quality factor [0-255] 255=best");
      console::printf(L"       DDS default quality is best possible.");
      console::printf(L"       CRN default quality is %u.", cDefaultCRNQualityLevel);
      console::printf(L"/bitrate # - Set the desired output bitrate of DDS or CRN output files.");
      console::printf(L"             This option causes crunch to find the quality factor");
      console::printf(L"             closest to the desired bitrate using a binary search.");

      console::message(L"\nLow-level CRN specific options:");
      console::printf(L"/c # - Color endpoint palette size, 32-8192, default=3072");
      console::printf(L"/s # - Color selector palette size, 32-8192, default=3072");
      console::printf(L"/ca # - Alpha endpoint palette size, 32-8192, default=3072");
      console::printf(L"/sa # - Alpha selector palette size, 32-8192, default=3072");

      console::message(L"\nMipmap filtering options:");
      console::printf(L"/mipMode [UseSourceOrGenerate,UseSource,Generate,None]");
      console::printf(L"         Default mipMode is UseSourceOrGenerate");
      console::printf(L"/mipFilter [box,tent,lanczos4,mitchell,kaiser], default=kaiser");
      console::printf(L"/gamma # - Mipmap gamma correction value, default=2.2, use 1.0 for linear");
      console::printf(L"/blurriness # - Scale filter kernel, >1=blur, <1=sharpen, .01-8, default=.9");
      console::printf(L"/wrap - Assume texture is tiled when filtering, default=clamping");
      console::printf(L"/renormalize - Renormalize filtered normal map texels, default=disabled");
      console::printf(L"/maxmips # - Limit number of generated texture mipmap levels, 1-16, default=16");
      console::printf(L"/minmipsize # - Smallest allowable mipmap resolution, default=1");

      console::message(L"\nCompression options:");
      console::printf(L"/alphaThreshold # - Set DXT1A alpha threshold, 0-255, default=128");
      console::printf(L" Note: /alphaThreshold also changes the compressor's behavior to");
      console::printf(L" prefer DXT1A over DXT5 for images with alpha channels (.DDS only).");
      console::printf(L"/uniformMetrics - Use uniform color metrics, default=use perceptual metrics");
      console::printf(L"/noAdaptiveBlocks - Disable adaptive block sizes (i.e. disable macroblocks).");
      console::printf(L"/compressor [CRN,CRNF,RYG] - Set DXTn compressor, default=CRN");
      console::printf(L"/dxtQuality [superfast,fast,normal,better,uber] - Endpoint optimizer speed.");
      console::printf(L"            Sets endpoint optimizer's max iteration depth. Default=uber.");
      console::printf(L"/noendpointcaching - Don't try reusing previous DXT endpoint solutions.");
      console::printf(L"/grayscalsampling - Assume shader will convert fetched results to luma (Y).");
      console::printf(L"/forceprimaryencoding - Only use DXT1 color4 and DXT5 alpha8 block encodings.");
      console::printf(L"/usetransparentindicesforblack - Try DXT1 transparent indices for dark pixels.");

      console::message(L"\nAll supported texture formats (Note: .CRN only supports DXTn pixel formats):");
      for (uint i = 0; i < pixel_format_helpers::get_num_formats(); i++)
      {
         pixel_format fmt = pixel_format_helpers::get_pixel_format_by_index(i);
         console::printf(L"/%s", pixel_format_helpers::get_pixel_format_string(fmt));
      }
   }

   bool convert(const wchar_t* pCommand_line)
   {
      m_num_processed = 0;
      m_num_failed = 0;
      m_num_succeeded = 0;
      m_num_skipped = 0;

      command_line_params::param_desc std_params[] =
      {
         { L"file", 1, true },

         { L"out", 1 },
         { L"outdir", 1 },
         { L"outsamedir" },
         { L"deep" },
         { L"fileformat", 1 },

         { L"helperThreads", 1 },
         { L"noprogress" },
         { L"quiet" },
         { L"ignoreerrors" },
         { L"logfile", 1 },

         { L"q", 1 },
         { L"quality", 1 },

         { L"c", 1 },
         { L"s", 1 },
         { L"ca", 1 },
         { L"sa", 1 },

         { L"mipMode", 1 },
         { L"mipFilter", 1 },
         { L"gamma", 1 },
         { L"blurriness", 1 },
         { L"wrap" },
         { L"renormalize" },
         { L"noprogress" },
         { L"paramdebug" },
         { L"debug" },
         { L"quick" },
         { L"imagestats" },
         { L"nostats" },
         { L"mipstats" },

         { L"alphaThreshold", 1 },
         { L"uniformMetrics" },
         { L"noAdaptiveBlocks" },
         { L"compressor", 1 },
         { L"dxtQuality", 1 },
         { L"noendpointcaching" },
         { L"grayscalesampling" },
         { L"converttoluma" },
         { L"setalphatoluma" },
         { L"pause" },
         { L"timestamp" },
         { L"nooverwrite" },
         { L"forcewrite" },
         { L"recreate" },
         { L"compare" },
         { L"info" },
         { L"forceprimaryencoding" },
         { L"usetransparentindicesforblack" },

         { L"rescalemode", 1 },
         { L"rescale", 2 },
         { L"relrescale", 2 },
         { L"clamp", 2 },
         { L"clampScale", 2 },
         { L"window", 4 },

         { L"maxmips", 1 },
         { L"minmipsize", 1},

         { L"bitrate", 1 },

         { L"lzmastats" },
         { L"split" },
         { L"csvfile", 1 },
      };

      crnlib::vector<command_line_params::param_desc> params;
      params.append(std_params, sizeof(std_params) / sizeof(std_params[0]));

      for (uint i = 0; i < pixel_format_helpers::get_num_formats(); i++)
      {
         pixel_format fmt = pixel_format_helpers::get_pixel_format_by_index(i);

         command_line_params::param_desc desc;
         desc.m_pName = pixel_format_helpers::get_pixel_format_string(fmt);
         desc.m_num_values = 0;
         desc.m_support_listing_file = false;
         params.push_back(desc);
      }

      if (!m_params.parse(pCommand_line, params.size(), params.get_ptr(), true))
      {
         return false;
      }

      if (!m_params.get_num_params())
      {
         console::error(L"No command line parameters specified!");

         print_usage();

         return false;
      }

      if (m_params.get_count(L""))
      {
         console::error(L"Unrecognized command line parameter: \"%s\"", m_params.get_value_as_string_or_empty(L"", 0).get_ptr());

         return false;
      }

      if (m_params.get_value_as_bool(L"debug"))
      {
         console::debug(L"Command line parameters:");
         for (command_line_params::param_map_const_iterator it = m_params.begin(); it != m_params.end(); ++it)
         {
            console::disable_crlf();
            console::debug(L"Key:\"%s\" Values (%u): ", it->first.get_ptr(), it->second.m_values.size());
            for (uint i = 0; i < it->second.m_values.size(); i++)
               console::debug(L"\"%s\" ", it->second.m_values[i].get_ptr());
            console::debug(L"\n");
            console::enable_crlf();
         }
      }

      dynamic_wstring log_filename;
      if (m_params.get_value_as_string(L"logfile", 0, log_filename))
      {
         if (!m_log_stream.open(log_filename.get_ptr(), cDataStreamWritable | cDataStreamSeekable, true))
         {
            console::error(L"Unable to open log file: \"%s\"", log_filename.get_ptr());
            return false;
         }

         console::printf(L"Appending to ANSI log file \"%s\"", log_filename.get_ptr());

         console::set_log_stream(&m_log_stream);
      }

      bool status = convert();

      if (m_log_stream.is_opened())
      {
         console::set_log_stream(NULL);

         m_log_stream.close();
      }

      return status;
   }

private:
   command_line_params m_params;

   bool convert()
   {
      find_files::file_desc_vec files;

      uint total_input_specs = 0;

      command_line_params::param_map_const_iterator begin, end;
      m_params.find(L"file", begin, end);
      for (command_line_params::param_map_const_iterator it = begin; it != end; ++it)
      {
         total_input_specs++;

         const dynamic_wstring_array& strings = it->second.m_values;
         for (uint i = 0; i < strings.size(); i++)
         {
            if (!process_input_spec(files, strings[i]))
            {
               if (!m_params.get_value_as_bool(L"ignoreerrors"))
                  return false;
            }
         }
      }

      if (!total_input_specs)
      {
         console::error(L"No input files specified!");
         return false;
      }

      if (files.empty())
      {
         console::error(L"No files found to process!");
         return false;
      }

      std::sort(files.begin(), files.end());
      files.resize((uint)(std::unique(files.begin(), files.end()) - files.begin()));

      timer tm;
      tm.start();

      if (!process_files(files))
      {
         if (!m_params.get_value_as_bool(L"ignoreerrors"))
            return false;
      }

      double total_time = tm.get_elapsed_secs();

      console::printf(L"Total time: %3.3fs", total_time);

      console::printf(
         ((m_num_skipped) || (m_num_failed)) ? cWarningConsoleMessage : cInfoConsoleMessage,
         L"%u total file(s) successfully processed, %u file(s) skipped, %u file(s) failed.", m_num_succeeded, m_num_skipped, m_num_failed);

      return true;
   }

   bool process_input_spec(find_files::file_desc_vec& files, const dynamic_wstring& input_spec)
   {
      dynamic_wstring find_name(input_spec);

      if ((find_name.is_empty()) || (!full_path(find_name)))
      {
         console::error(L"Invalid input filename: %s", find_name.get_ptr());
         return false;
      }

      const bool deep_flag = m_params.get_value_as_bool(L"deep");

      dynamic_wstring find_drive, find_path, find_fname, find_ext;
      split_path(find_name.get_ptr(), &find_drive, &find_path, &find_fname, &find_ext);

      dynamic_wstring find_pathname;
      combine_path(find_pathname, find_drive.get_ptr(), find_path.get_ptr());
      dynamic_wstring find_filename;
      find_filename = find_fname + find_ext;

      find_files file_finder;
      bool success = file_finder.find(find_pathname.get_ptr(), find_filename.get_ptr(), find_files::cFlagAllowFiles | (deep_flag ? find_files::cFlagRecursive : 0));
      if (!success)
      {
         console::error(L"Failed finding files: %s", find_name.get_ptr());
         return false;
      }
      if (file_finder.get_files().empty())
      {
         console::warning(L"No files found: %s", find_name.get_ptr());
         return true;
      }

      files.append(file_finder.get_files());

      return true;
   }

   bool read_only_file_check(const wchar_t* pDst_filename)
   {
      uint32 dst_file_attribs = GetFileAttributesW(pDst_filename);
      if (dst_file_attribs == INVALID_FILE_ATTRIBUTES)
         return true;

      if ((dst_file_attribs & FILE_ATTRIBUTE_READONLY) == 0)
         return true;

      if (m_params.get_value_as_bool(L"forcewrite"))
      {
         dst_file_attribs &= ~FILE_ATTRIBUTE_READONLY;
         if (SetFileAttributesW(pDst_filename, dst_file_attribs))
         {
            console::warning(L"Setting read-only file \"%s\" to writable", pDst_filename);
            return true;
         }
         else
         {
            console::error(L"Failed setting read-only file \"%s\" to writable!", pDst_filename);
            return false;
         }
      }

      console::error(L"Output file \"%s\" is read-only!", pDst_filename);
      return false;
   }

   bool process_files(find_files::file_desc_vec& files)
   {
      const bool compare_mode = m_params.get_value_as_bool(L"compare");
      const bool info_mode = m_params.get_value_as_bool(L"info");

      for (uint file_index = 0; file_index < files.size(); file_index++)
      {
         const find_files::file_desc& file_desc = files[file_index];
         const dynamic_wstring& in_filename = file_desc.m_fullname;

         dynamic_wstring in_drive, in_path, in_fname, in_ext;
         split_path(in_filename.get_ptr(), &in_drive, &in_path, &in_fname, &in_ext);

         texture_file_types::format out_file_type = texture_file_types::cFormatCRN;
         dynamic_wstring fmt;
         if (m_params.get_value_as_string(L"fileformat", 0, fmt))
         {
            if (fmt == L"tga")
               out_file_type = texture_file_types::cFormatTGA;
            else if (fmt == L"bmp")
               out_file_type = texture_file_types::cFormatBMP;
            else if (fmt == L"dds")
               out_file_type = texture_file_types::cFormatDDS;
            else if (fmt == L"crn")
               out_file_type = texture_file_types::cFormatCRN;
            else
            {
               console::error(L"Unsupported output file type: %s", fmt.get_ptr());
               return false;
            }
         }

         if (!m_params.has_key(L"fileformat"))
         {
            texture_file_types::format input_file_type = texture_file_types::determine_file_format(in_filename.get_ptr());
            if (input_file_type == texture_file_types::cFormatCRN)
            {
               out_file_type = texture_file_types::cFormatDDS;
            }
         }

         dynamic_wstring out_filename;
         if (m_params.get_value_as_bool(L"outsamedir"))
            out_filename.format(L"%s%s%s.%s", in_drive.get_ptr(), in_path.get_ptr(), in_fname.get_ptr(), texture_file_types::get_extension(out_file_type));
         else if (m_params.has_key(L"out"))
         {
            out_filename = m_params.get_value_as_string_or_empty(L"out");

            if (files.size() > 1)
            {
               dynamic_wstring out_drive, out_dir, out_name, out_ext;
               split_path(out_filename.get_ptr(), &out_drive, &out_dir, &out_name, &out_ext);

               out_name.format(L"%s_%u", out_name.get_ptr(), file_index);

               out_filename.format(L"%s%s%s%s", out_drive.get_ptr(), out_dir.get_ptr(), out_name.get_ptr(), out_ext.get_ptr());
            }

            if (!m_params.has_key(L"fileformat"))
               out_file_type = texture_file_types::determine_file_format(out_filename.get_ptr());
         }
         else
         {
            dynamic_wstring out_dir(m_params.get_value_as_string_or_empty(L"outdir"));

            if (m_params.get_value_as_bool(L"recreate"))
            {
               combine_path(out_dir, out_dir.get_ptr(), file_desc.m_rel.get_ptr());
            }

            if (out_dir.get_len())
               out_filename.format(L"%s\\%s.%s", out_dir.get_ptr(), in_fname.get_ptr(), texture_file_types::get_extension(out_file_type));
            else
               out_filename.format(L"%s.%s", in_fname.get_ptr(), texture_file_types::get_extension(out_file_type));

            if (m_params.get_value_as_bool(L"recreate"))
            {
               if (full_path(out_filename))
               {
                  if ((!compare_mode) && (!info_mode))
                  {
                     dynamic_wstring out_drive, out_path;
                     split_path(out_filename.get_ptr(), &out_drive, &out_path, NULL, NULL);
                     out_drive += out_path;
                     create_path(out_drive.get_ptr());
                  }
               }
            }
         }

         if ((!compare_mode) && (!info_mode))
         {
            WIN32_FILE_ATTRIBUTE_DATA dst_file_attribs;
            const BOOL dest_file_exists = GetFileAttributesExW(out_filename.get_ptr(), GetFileExInfoStandard, &dst_file_attribs);

            if (dest_file_exists)
            {
               if (m_params.get_value_as_bool(L"nooverwrite"))
               {
                  console::warning(L"Skipping already existing file: %s\n", out_filename.get_ptr());
                  m_num_skipped++;
                  continue;
               }

               if (m_params.get_value_as_bool(L"timestamp"))
               {
                  WIN32_FILE_ATTRIBUTE_DATA src_file_attribs;
                  const BOOL src_file_exists = GetFileAttributesExW(in_filename.get_ptr(), GetFileExInfoStandard, &src_file_attribs);

                  if (src_file_exists)
                  {
                     LONG timeComp = CompareFileTime(&src_file_attribs.ftLastWriteTime, &dst_file_attribs.ftLastWriteTime);
                     if (timeComp <= 0)
                     {
                        console::warning(L"Skipping up to date file: %s\n", out_filename.get_ptr());
                        m_num_skipped++;
                        continue;
                     }
                  }
               }
            }
         }

         convert_status status = cCSFailed;

         if (info_mode)
            status = display_file_info(file_index, files.size(), in_filename.get_ptr());
         else if (compare_mode)
            status = compare_file(file_index, files.size(), in_filename.get_ptr(), out_filename.get_ptr(), out_file_type);
         else if (read_only_file_check(out_filename.get_ptr()))
            status = convert_file(file_index, files.size(), in_filename.get_ptr(), out_filename.get_ptr(), out_file_type);

         m_num_processed++;

         switch (status)
         {
            case cCSSucceeded:
            {
               console::info(L"");
               m_num_succeeded++;
               break;
            }
            case cCSSkipped:
            {
               console::info(L"Skipping file.\n");
               m_num_skipped++;
               break;
            }
            case cCSBadParam:
            {
               return false;
            }
            default:
            {
               if (!m_params.get_value_as_bool(L"ignoreerrors"))
                  return false;

               console::info(L"");

               m_num_failed++;
               break;
            }
         }
      }

      return true;
   }

   void print_texture_info(const wchar_t* pTex_desc, texture_conversion::convert_params& params, dds_texture& tex)
   {
      console::info(L"%s: %ux%u, Levels: %u, Faces: %u, Format: %s",
         pTex_desc,
         tex.get_width(),
         tex.get_height(),
         tex.get_num_levels(),
         tex.get_num_faces(),
         pixel_format_helpers::get_pixel_format_string(tex.get_format()));

      console::disable_crlf();
      console::info(L"Apparent type: %s, ", get_texture_type_desc(params.m_texture_type));

      console::info(L"Flags: ");
      if (tex.get_comp_flags() & pixel_format_helpers::cCompFlagRValid) console::info(L"R ");
      if (tex.get_comp_flags() & pixel_format_helpers::cCompFlagGValid) console::info(L"G ");
      if (tex.get_comp_flags() & pixel_format_helpers::cCompFlagBValid) console::info(L"B ");
      if (tex.get_comp_flags() & pixel_format_helpers::cCompFlagAValid) console::info(L"A ");
      if (tex.get_comp_flags() & pixel_format_helpers::cCompFlagGrayscale) console::info(L"Grayscale ");
      if (tex.get_comp_flags() & pixel_format_helpers::cCompFlagNormalMap) console::info(L"NormalMap ");
      if (tex.get_comp_flags() & pixel_format_helpers::cCompFlagLumaChroma) console::info(L"LumaChroma ");
      console::info(L"\n");
      console::enable_crlf();
   }

   static bool progress_callback_func(uint percentage_complete, void* pUser_data_ptr)
   {
      pUser_data_ptr;

      console::disable_crlf();

      wchar_t buf[8];
      for (uint i = 0; i < 7; i++)
         buf[i] = 8;
      buf[7] = '\0';

      for (uint i = 0; i < 130/8; i++)
         console::progress(buf);

      console::progress(L"Processing: %u%%", percentage_complete);

      for (uint i = 0; i < 7; i++)
         buf[i] = L' ';
      console::progress(buf);
      console::progress(buf);

      for (uint i = 0; i < 7; i++)
         buf[i] = 8;
      console::progress(buf);
      console::progress(buf);

      console::enable_crlf();

      return true;
   }

   bool parse_mipmap_params(crn_mipmap_params& mip_params)
   {
      dynamic_wstring val;

      if (m_params.get_value_as_string(L"mipMode", 0, val))
      {
         uint i;
         for (i = 0; i < cCRNMipModeTotal; i++)
         {
            if (val == crn_get_mip_mode_name( static_cast<crn_mip_mode>(i) ))
            {
               mip_params.m_mode = static_cast<crn_mip_mode>(i);
               break;
            }
         }
         if (i == cCRNMipModeTotal)
         {
            console::error(L"Invalid MipMode: \"%s\"", val.get_ptr());
            return false;
         }
      }

      if (m_params.get_value_as_string(L"mipFilter", 0, val))
      {
         uint i;
         for (i = 0; i < cCRNMipFilterTotal; i++)
         {
            if (val == dynamic_wstring(crn_get_mip_filter_name( static_cast<crn_mip_filter>(i) )) )
            {
               mip_params.m_filter = static_cast<crn_mip_filter>(i);
               break;
            }
         }

         if (i == cCRNMipFilterTotal)
         {
            console::error(L"Invalid MipFilter: \"%s\"", val.get_ptr());
            return false;
         }

         if (i == cCRNMipFilterBox)
            mip_params.m_blurriness = 1.0f;
      }

      mip_params.m_gamma = m_params.get_value_as_float(L"gamma", 0, mip_params.m_gamma, .1f, 8.0f);
      mip_params.m_gamma_filtering = (mip_params.m_gamma != 1.0f);

      mip_params.m_blurriness = m_params.get_value_as_float(L"blurriness", 0, mip_params.m_blurriness, .01f, 8.0f);

      mip_params.m_renormalize = m_params.get_value_as_bool(L"renormalize", 0, mip_params.m_renormalize != 0);
      mip_params.m_tiled = m_params.get_value_as_bool(L"wrap");

      mip_params.m_max_levels = m_params.get_value_as_int(L"maxmips", 0, cCRNMaxLevels, 1, cCRNMaxLevels);
      mip_params.m_min_mip_size = m_params.get_value_as_int(L"minmipsize", 0, 1, 1, cCRNMaxLevelResolution);

      return true;
   }

   bool parse_scale_params(crn_mipmap_params &mipmap_params)
   {
      if (m_params.has_key(L"rescale"))
      {
         int w = m_params.get_value_as_int(L"rescale", 0, -1, 1, cCRNMaxLevelResolution, 0);
         int h = m_params.get_value_as_int(L"rescale", 0, -1, 1, cCRNMaxLevelResolution, 1);

         mipmap_params.m_scale_mode = cCRNSMAbsolute;
         mipmap_params.m_scale_x = (float)w;
         mipmap_params.m_scale_y = (float)h;
      }
      else if (m_params.has_key(L"relrescale"))
      {
         float w = m_params.get_value_as_float(L"relrescale", 0, 1, 1, 256, 0);
         float h = m_params.get_value_as_float(L"relrescale", 0, 1, 1, 256, 1);

         mipmap_params.m_scale_mode = cCRNSMRelative;
         mipmap_params.m_scale_x = w;
         mipmap_params.m_scale_y = h;
      }
      else if (m_params.has_key(L"rescalemode"))
      {
         // nearest | hi | lo

         dynamic_wstring mode_str(m_params.get_value_as_string_or_empty(L"rescalemode"));
         if (mode_str == L"nearest")
            mipmap_params.m_scale_mode = cCRNSMNearestPow2;
         else if (mode_str == L"hi")
            mipmap_params.m_scale_mode = cCRNSMNextPow2;
         else if (mode_str == L"lo")
            mipmap_params.m_scale_mode = cCRNSMLowerPow2;
         else
         {
            console::error(L"Invalid rescale mode: \"%s\"", mode_str.get_ptr());
            return false;
         }
      }

      if (m_params.has_key(L"clamp"))
      {
         uint w = m_params.get_value_as_int(L"clamp", 0, 1, 1, cCRNMaxLevelResolution, 0);
         uint h = m_params.get_value_as_int(L"clamp", 0, 1, 1, cCRNMaxLevelResolution, 1);

         mipmap_params.m_clamp_scale = false;
         mipmap_params.m_clamp_width = w;
         mipmap_params.m_clamp_height = h;
      }
      else if (m_params.has_key(L"clampScale"))
      {
         uint w = m_params.get_value_as_int(L"clampscale", 0, 1, 1, cCRNMaxLevelResolution, 0);
         uint h = m_params.get_value_as_int(L"clampscale", 0, 1, 1, cCRNMaxLevelResolution, 1);

         mipmap_params.m_clamp_scale = true;
         mipmap_params.m_clamp_width = w;
         mipmap_params.m_clamp_height = h;
      }

      if (m_params.has_key(L"window"))
      {
         uint xl = m_params.get_value_as_int(L"window", 0, 0, 0, cCRNMaxLevelResolution, 0);
         uint yl = m_params.get_value_as_int(L"window", 0, 0, 0, cCRNMaxLevelResolution, 1);
         uint xh = m_params.get_value_as_int(L"window", 0, 0, 0, cCRNMaxLevelResolution, 2);
         uint yh = m_params.get_value_as_int(L"window", 0, 0, 0, cCRNMaxLevelResolution, 3);

         mipmap_params.m_window_left = math::minimum(xl, xh);
         mipmap_params.m_window_top = math::minimum(yl, yh);
         mipmap_params.m_window_right = math::maximum(xl, xh);
         mipmap_params.m_window_bottom = math::maximum(yl, yh);
      }

      return true;
   }

   bool parse_comp_params(texture_file_types::format dst_file_format, crn_comp_params &comp_params)
   {
      if (dst_file_format == texture_file_types::cFormatCRN)
         comp_params.m_quality_level = cDefaultCRNQualityLevel;

      if (m_params.has_key(L"q") || m_params.has_key(L"quality"))
      {
         const wchar_t *pKeyName = m_params.has_key(L"q") ? L"q" : L"quality";

         if ((dst_file_format == texture_file_types::cFormatDDS) || (dst_file_format == texture_file_types::cFormatCRN))
         {
            uint i = m_params.get_value_as_int(pKeyName, 0, cDefaultCRNQualityLevel, 0, cCRNMaxQualityLevel);

            comp_params.m_quality_level = i;
         }
         else
         {
            console::error(L"/quality or /q option is only invalid when writing DDS or CRN files!");
            return false;
         }
      }
      else
      {
         float desired_bitrate = m_params.get_value_as_float(L"bitrate", 0, 0.0f, .1f, 30.0f);
         if (desired_bitrate > 0.0f)
         {
            comp_params.m_target_bitrate = desired_bitrate;
         }
      }

      int color_endpoints = m_params.get_value_as_int(L"c", 0, 0, cCRNMinPaletteSize, cCRNMaxPaletteSize);
      int color_selectors = m_params.get_value_as_int(L"s", 0, 0, cCRNMinPaletteSize, cCRNMaxPaletteSize);
      int alpha_endpoints = m_params.get_value_as_int(L"ca", 0, 0, cCRNMinPaletteSize, cCRNMaxPaletteSize);
      int alpha_selectors = m_params.get_value_as_int(L"sa", 0, 0, cCRNMinPaletteSize, cCRNMaxPaletteSize);
      if ( ((color_endpoints > 0) && (color_selectors > 0)) ||
           ((alpha_endpoints > 0) && (alpha_selectors > 0)) )
      {
         comp_params.set_flag(cCRNCompFlagManualPaletteSizes, true);
         comp_params.m_crn_color_endpoint_palette_size = color_endpoints;
         comp_params.m_crn_color_selector_palette_size = color_selectors;
         comp_params.m_crn_alpha_endpoint_palette_size = alpha_endpoints;
         comp_params.m_crn_alpha_selector_palette_size = alpha_selectors;
      }

      if (m_params.has_key(L"alphaThreshold"))
      {
         int dxt1a_alpha_threshold = m_params.get_value_as_int(L"alphaThreshold", 0, 128, 0, 255);
         comp_params.m_dxt1a_alpha_threshold = dxt1a_alpha_threshold;
         if (dxt1a_alpha_threshold > 0)
         {
            comp_params.set_flag(cCRNCompFlagDXT1AForTransparency, true);
         }
      }

      comp_params.set_flag(cCRNCompFlagPerceptual, !m_params.get_value_as_bool(L"uniformMetrics"));
      comp_params.set_flag(cCRNCompFlagHierarchical, !m_params.get_value_as_bool(L"noAdaptiveBlocks"));

      if (m_params.has_key(L"helperThreads"))
         comp_params.m_num_helper_threads = m_params.get_value_as_int(L"helperThreads", 0, cCRNMaxHelperThreads, 0, cCRNMaxHelperThreads);
      else if (g_number_of_processors > 1)
         comp_params.m_num_helper_threads = g_number_of_processors - 1;

      dynamic_wstring comp_name;
      if (m_params.get_value_as_string(L"compressor", 0, comp_name))
      {
         uint i;
         for (i = 0; i < cCRNTotalDXTCompressors; i++)
         {
            if (comp_name == get_dxt_compressor_name(static_cast<crn_dxt_compressor_type>(i)))
            {
               comp_params.m_dxt_compressor_type = static_cast<crn_dxt_compressor_type>(i);
               break;
            }
         }
         if (i == cCRNTotalDXTCompressors)
         {
            console::error(L"Invalid compressor: \"%s\"", comp_name.get_ptr());
            return false;
         }
      }

      dynamic_wstring dxt_quality_str;
      if (m_params.get_value_as_string(L"dxtquality", 0, dxt_quality_str))
      {
         uint i;
         for (i = 0; i < cCRNDXTQualityTotal; i++)
         {
            if (dxt_quality_str == crn_get_dxt_quality_string(static_cast<crn_dxt_quality>(i)))
            {
               comp_params.m_dxt_quality = static_cast<crn_dxt_quality>(i);
               break;
            }
         }
         if (i == cCRNDXTQualityTotal)
         {
            console::error(L"Invalid DXT quality: \"%s\"", dxt_quality_str.get_ptr());
            return false;
         }
      }
      else
      {
         comp_params.m_dxt_quality = cCRNDXTQualityUber;
      }

      comp_params.set_flag(cCRNCompFlagDisableEndpointCaching, m_params.get_value_as_bool(L"noendpointcaching"));
      comp_params.set_flag(cCRNCompFlagGrayscaleSampling, m_params.get_value_as_bool(L"grayscalesampling"));
      comp_params.set_flag(cCRNCompFlagUseBothBlockTypes, !m_params.get_value_as_bool(L"forceprimaryencoding"));
      if (comp_params.get_flag(cCRNCompFlagUseBothBlockTypes))
         comp_params.set_flag(cCRNCompFlagUseTransparentIndicesForBlack, m_params.get_value_as_bool(L"usetransparentindicesforblack"));
      else
         comp_params.set_flag(cCRNCompFlagUseTransparentIndicesForBlack, false);

      return true;
   }

   convert_status display_file_info(uint file_index, uint num_files, const wchar_t* pSrc_filename)
   {
      if (num_files > 1)
         console::message(L"[%u/%u] Source texture: \"%s\"", file_index + 1, num_files, pSrc_filename);
      else
         console::message(L"Source texture: \"%s\"", pSrc_filename);

      texture_file_types::format src_file_format = texture_file_types::determine_file_format(pSrc_filename);
      if (src_file_format == texture_file_types::cFormatInvalid)
      {
         console::error(L"Unrecognized file type: %s", pSrc_filename);
         return cCSFailed;
      }

      dds_texture src_tex;
      if (!src_tex.load_from_file(pSrc_filename, src_file_format))
      {
         if (src_tex.get_last_error().is_empty())
            console::error(L"Failed reading source file: \"%s\"", pSrc_filename);
         else
            console::error(L"%s", src_tex.get_last_error().get_ptr());

         return cCSFailed;
      }

      uint64 input_file_size;
      win32_file_utils::get_file_size(pSrc_filename, input_file_size);

      uint total_in_pixels = 0;
      for (uint i = 0; i < src_tex.get_num_levels(); i++)
      {
         uint width = math::maximum<uint>(1, src_tex.get_width() >> i);
         uint height = math::maximum<uint>(1, src_tex.get_height() >> i);
         total_in_pixels += width*height*src_tex.get_num_faces();
      }

      vector<uint8> src_tex_bytes;
      if (!cfile_stream::read_file_into_array(pSrc_filename, src_tex_bytes))
      {
         console::error(L"Failed loading source file: %s", pSrc_filename);
         return cCSFailed;
      }

      if (!src_tex_bytes.size())
      {
         console::warning(L"Source file is empty: %s", pSrc_filename);
         return cCSSkipped;
      }

      uint compressed_size = 0;
      if (m_params.has_key(L"lzmastats"))
      {
         lzma_codec lossless_codec;
         vector<uint8> cmp_tex_bytes;
         if (lossless_codec.pack(src_tex_bytes.get_ptr(), src_tex_bytes.size(), cmp_tex_bytes))
         {
            compressed_size = cmp_tex_bytes.size();
         }
      }
      console::info(L"Source texture dimensions: %ux%u, Levels: %u, Faces: %u, Format: %s",
         src_tex.get_width(),
         src_tex.get_height(),
         src_tex.get_num_levels(),
         src_tex.get_num_faces(),
         pixel_format_helpers::get_pixel_format_string(src_tex.get_format()));

      console::info(L"Total pixels: %u, Source file size: %I64i, Source file bits/pixel: %1.3f",
         total_in_pixels, input_file_size, (input_file_size * 8.0f) / total_in_pixels);
      if (compressed_size)
      {
         console::info(L"LZMA compressed file size: %u bytes, %1.3f bits/pixel",
            compressed_size, compressed_size * 8.0f / total_in_pixels);
      }

      double entropy = math::compute_entropy(src_tex_bytes.get_ptr(), src_tex_bytes.size());
      console::info(L"Source file entropy: %3.6f bits per byte", entropy / src_tex_bytes.size());

      if (src_file_format == texture_file_types::cFormatCRN)
      {
         crnd::crn_texture_info tex_info;
         tex_info.m_struct_size = sizeof(crnd::crn_texture_info);
         crn_bool success = crnd::crnd_get_texture_info(src_tex_bytes.get_ptr(), src_tex_bytes.size(), &tex_info);
         if (!success)
            console::error(L"Failed retrieving CRN texture info!");
         else
         {
            console::info(L"CRN texture info:");

            console::info(L"Width: %u, Height: %u, Levels: %u, Faces: %u\nBytes per block: %u, User0: 0x%08X, User1: 0x%08X, CRN Format: %u",
               tex_info.m_width,
               tex_info.m_height,
               tex_info.m_levels,
               tex_info.m_faces,
               tex_info.m_bytes_per_block,
               tex_info.m_userdata0,
               tex_info.m_userdata1,
               tex_info.m_format);
         }
      }

      return cCSSucceeded;
   }

   void print_stats(texture_conversion::convert_stats &stats, bool force_image_stats = false)
   {
      dynamic_wstring csv_filename;
      const wchar_t *pCSVStatsFilename = m_params.get_value_as_string(L"csvfile", 0, csv_filename) ? csv_filename.get_ptr() : NULL;

      bool image_stats = force_image_stats || m_params.get_value_as_bool(L"imagestats") || m_params.get_value_as_bool(L"mipstats") || (pCSVStatsFilename != NULL);
      bool mip_stats = m_params.get_value_as_bool(L"mipstats");
      bool grayscale_sampling = m_params.get_value_as_bool(L"grayscalesampling");
      if (!stats.print(image_stats, mip_stats, grayscale_sampling, pCSVStatsFilename))
      {
         console::warning(L"Unable to compute/display full output file statistics.");
      }
   }

   convert_status compare_file(uint file_index, uint num_files, const wchar_t* pSrc_filename, const wchar_t* pDst_filename, texture_file_types::format out_file_type)
   {
      if (num_files > 1)
         console::message(L"[%u/%u] Comparing source texture \"%s\" to output texture \"%s\"", file_index + 1, num_files, pSrc_filename, pDst_filename);
      else
         console::message(L"Comparing source texture \"%s\" to output texture \"%s\"", pSrc_filename, pDst_filename);

      texture_file_types::format src_file_format = texture_file_types::determine_file_format(pSrc_filename);
      if (src_file_format == texture_file_types::cFormatInvalid)
      {
         console::error(L"Unrecognized file type: %s", pSrc_filename);
         return cCSFailed;
      }

      dds_texture src_tex;

      if (!src_tex.load_from_file(pSrc_filename, src_file_format))
      {
         if (src_tex.get_last_error().is_empty())
            console::error(L"Failed reading source file: \"%s\"", pSrc_filename);
         else
            console::error(L"%s", src_tex.get_last_error().get_ptr());

         return cCSFailed;
      }

      texture_conversion::convert_stats stats;
      if (!stats.init(pSrc_filename, pDst_filename, src_tex, out_file_type, m_params.has_key(L"lzmastats")))
         return cCSFailed;

      print_stats(stats, true);

      return cCSSucceeded;
   }

   convert_status convert_file(uint file_index, uint num_files, const wchar_t* pSrc_filename, const wchar_t* pDst_filename, texture_file_types::format out_file_type)
   {
      timer tim;

      if (num_files > 1)
         console::message(L"[%u/%u] Reading source texture: \"%s\"", file_index + 1, num_files, pSrc_filename);
      else
         console::message(L"Reading source texture: \"%s\"", pSrc_filename);

      texture_file_types::format src_file_format = texture_file_types::determine_file_format(pSrc_filename);
      if (src_file_format == texture_file_types::cFormatInvalid)
      {
         console::error(L"Unrecognized file type: %s", pSrc_filename);
         return cCSFailed;
      }

      dds_texture src_tex;
      tim.start();
      if (!src_tex.load_from_file(pSrc_filename, src_file_format))
      {
         if (src_tex.get_last_error().is_empty())
            console::error(L"Failed reading source file: \"%s\"", pSrc_filename);
         else
            console::error(L"%s", src_tex.get_last_error().get_ptr());

         return cCSFailed;
      }
      double total_time = tim.get_elapsed_secs();
      console::info(L"Texture successfully loaded in %3.3fs", total_time);

      if (m_params.get_value_as_bool(L"converttoluma"))
         src_tex.convert(image_utils::cConversion_Y_To_RGB);
      if (m_params.get_value_as_bool(L"setalphatoluma"))
         src_tex.convert(image_utils::cConversion_Y_To_A);
      
      texture_conversion::convert_params params;

      params.m_texture_type = src_tex.determine_texture_type();
      params.m_pInput_texture = &src_tex;
      params.m_dst_filename = pDst_filename;
      params.m_dst_file_type = out_file_type;
      params.m_lzma_stats = m_params.has_key(L"lzmastats");
      params.m_write_mipmaps_to_multiple_files = m_params.has_key(L"split");

      if ((!m_params.get_value_as_bool(L"noprogress")) && (!m_params.get_value_as_bool(L"quiet")))
         params.m_pProgress_func = progress_callback_func;

      if (m_params.get_value_as_bool(L"debug"))
      {
         params.m_debugging = true;
         params.m_comp_params.set_flag(cCRNCompFlagDebugging, true);
      }

      if (m_params.get_value_as_bool(L"paramdebug"))
         params.m_param_debugging = true;

      if (m_params.get_value_as_bool(L"quick"))
         params.m_quick = true;

      params.m_no_stats = m_params.get_value_as_bool(L"nostats");

      params.m_dst_format = PIXEL_FMT_INVALID;
      
      for (uint i = 0; i < pixel_format_helpers::get_num_formats(); i++)
      {
         pixel_format trial_fmt = pixel_format_helpers::get_pixel_format_by_index(i);
         if (m_params.has_key(pixel_format_helpers::get_pixel_format_string(trial_fmt)))
         {
            params.m_dst_format = trial_fmt;
            break;
         }
      }

      if (texture_file_types::supports_mipmaps(src_file_format))
      {
         params.m_mipmap_params.m_mode = cCRNMipModeUseSourceMips;
      }

      if (!parse_mipmap_params(params.m_mipmap_params))
         return cCSBadParam;

      if (!parse_comp_params(params.m_dst_file_type, params.m_comp_params))
         return cCSBadParam;

      if (!parse_scale_params(params.m_mipmap_params))
         return cCSBadParam;

      print_texture_info(L"Source texture", params, src_tex);

      if (params.m_texture_type == cTextureTypeNormalMap)
      {
         params.m_comp_params.set_flag(cCRNCompFlagPerceptual, false);
      }

      texture_conversion::convert_stats stats;

      tim.start();
      bool status = texture_conversion::process(params, stats);
      total_time = tim.get_elapsed_secs();

      if (!status)
      {
         if (params.m_error_message.is_empty())
            console::error(L"Failed writing output file: \"%s\"", pDst_filename);
         else
            console::error(params.m_error_message.get_ptr());
         return cCSFailed;
      }

      console::info(L"Texture successfully processed in %3.3fs", total_time);

      if (!m_params.get_value_as_bool(L"nostats"))
         print_stats(stats);

      return cCSSucceeded;
   }
};

//-----------------------------------------------------------------------------------------------------------------------

static bool check_for_option(int argc, wchar_t *argv[], const wchar_t *pOption)
{
   for (int i = 1; i < argc; i++)
   {
      if ((argv[i][0] == '/') || (argv[i][0] == '-'))
      {
         if (_wcsicmp(&argv[i][1], pOption) == 0)
            return true;
      }
   }
   return false;
}

//-----------------------------------------------------------------------------------------------------------------------

#define Q(x) L##x
#define U(x) Q(x)

static void print_title()
{
   console::printf(L"crunch: Advanced DXTn Texture Compressor");
   console::printf(L"Copyright (c) 2010-2011 Tenacious Software LLC");
   console::printf(L"crnlib version v%u.%02u %s Built %s, %s", CRNLIB_VERSION / 100U, CRNLIB_VERSION % 100U, crnlib_is_x64() ? L"x64" : L"x86", U(__DATE__), U(__TIME__));
   console::printf(L"");
}

//-----------------------------------------------------------------------------------------------------------------------

static int wmain_internal(int argc, wchar_t *argv[])
{
   argc;
   argv;

   win32_console::init();
   
   if (check_for_option(argc, argv, L"quiet"))
      console::disable_output();

   print_title();

#if 0
   if (check_for_option(argc, argv, L"exp"))
      return test(argc, argv);
#endif

   crunch converter;

   bool status = converter.convert(GetCommandLineW());

   win32_console::deinit();

   crnlib_print_mem_stats();

   return status ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void pause(void)
{
   console::enable_output();

   console::message(L"\nPress a key to continue.");

   for ( ; ; )
   {
      if (_getch() != -1)
         break;
   }
}

//-----------------------------------------------------------------------------------------------------------------------

#ifdef _MSC_VER
int wmain(int argc, wchar_t *argv[], wchar_t *envp[])
#else
int main(int argc, char *argva[])
#endif
{
#ifndef _MSC_VER
   // FIXME - mingw doesn't support wmain()
   wchar_t *first_arg = (wchar_t*)L"crunch.exe";
   wchar_t* argv[1] = { first_arg };
   argc = 1;
#else
   envp;
#endif

   int status = EXIT_FAILURE;

   if (IsDebuggerPresent())
   {
      status = wmain_internal(argc, argv);
   }
   else
   {
#ifdef _MSC_VER
      __try
      {
         status = wmain_internal(argc, argv);
      }
      __except(EXCEPTION_EXECUTE_HANDLER)
      {
         console::error(L"Uncached exception! crunch command line tool failed!");
      }
#else
      status = wmain_internal(argc, argv);
#endif
   }

   console::printf(L"\nExit status: %i", status);

   if (check_for_option(argc, argv, L"pause"))
   {
      if ((status == EXIT_FAILURE) || (console::get_num_messages(cErrorConsoleMessage)))
         pause();
   }

   return status;
}

