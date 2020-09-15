// File: crn_value.cpp
// This software is in the public domain. Please see license.txt.
#include "crn_core.h"
#include "crn_value.h"

namespace crnlib
{
   const char* gValueDataTypeStrings[cDTTotal + 1] =
   {
      "invalid",
      "string",
      "bool",
      "int",
      "uint",
      "float",
      "vec3f",
      "vec3i",

      NULL,
   };

} // namespace crnlib
