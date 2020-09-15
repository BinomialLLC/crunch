// File: crn_zeng.h
// This software is in the public domain. Please see license.txt.

namespace crnlib
{
   typedef float (*zeng_similarity_func)(uint index_a, uint index_b, void* pContext);
   
   void create_zeng_reorder_table(uint n, uint num_indices, const uint* pIndices, crnlib::vector<uint>& remap_table, zeng_similarity_func pFunc, void* pContext, float similarity_func_weight);
   
} // namespace crnlib
