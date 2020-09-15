// File: corpus_gen.h
// This software is in the public domain. Please see license.txt.
#pragma once
#include "crn_command_line_params.h"
#include "crn_image.h"

namespace crnlib
{
   class corpus_gen
   {
   public:
      corpus_gen();

      bool generate(const char* pCmd_line);

   private:
      void sort_blocks(image_u8& img);
   };

} // namespace crnlib
