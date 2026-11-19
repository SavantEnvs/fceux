// ROM loading + emulation through the public core API: FCEUI_LoadGame() sniffs the image
// (iNES/NES 2.0 -> NSF -> UNIF -> FDS) and dispatches to ines.cpp / nsf.cpp / unif.cpp / fds.cpp,
// then the selected mapper board, CPU, PPU and APU run for a few frames. One source, built once per
// format (ROM_EXT) so each target keeps its own seed corpus; the loader itself is the same.
#include "fceux_fuzz_common.h"
#ifndef ROM_EXT
#define ROM_EXT ".nes"
#endif

extern "C" int LLVMFuzzerInitialize(int *, char ***) { fz::init(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  fz::init();
  std::string p = fz::g_work + "/game" ROM_EXT;
  fz::write_file(p, data, size);
  if (FCEUI_LoadGame(p.c_str(), 1, true)) {
    for (int i = 0; i < 3 && GameInfo; i++) {
      fz::run_frames(2);
      if (GameInfo && GameInfo->type == GIT_NSF) FCEUI_NSFChange(1);
      if (GameInfo && GameInfo->type == GIT_FDS) { FCEUI_FDSSelect(); FCEUI_FDSInsert(); }
    }
  }
  fz::finish();
  return 0;
}
