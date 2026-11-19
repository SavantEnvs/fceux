// Savestate loading: FCEUSS_LoadFP (state.cpp) — header, zlib inflate, chunk/section reader,
// per-mapper restore callbacks — on a freshly powered-on built-in cartridge, then a few frames
// of emulation from the restored state.
#include "fceux_fuzz_common.h"

extern "C" int LLVMFuzzerInitialize(int *, char ***) { fz::init(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  fz::init();
  if (!fz::load_builtin()) abort();
  EMUFILE_MEMORY is((u8 *)data, (s32)size);
  if (FCEUSS_LoadFP(&is, SSLOADPARAM_NOBACKUP)) fz::run_frames(3);
  fz::finish();
  return 0;
}
