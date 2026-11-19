// FM2 movie playback: FCEUI_LoadMovie (movie.cpp LoadFM2: header key/values, base64 checksums,
// subtitles, embedded savestate, text/binary input log) then playback frames into input.cpp.
#include "fceux_fuzz_common.h"

extern "C" int LLVMFuzzerInitialize(int *, char ***) { fz::init(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  fz::init();
  if (!fz::load_builtin()) abort();
  std::string p = fz::g_work + "/movie.fm2";
  fz::write_file(p, data, size);
  if (FCEUI_LoadMovie(p.c_str(), true, 0)) fz::run_frames(8);
  FCEUI_StopMovie();
  fz::finish();
  return 0;
}
