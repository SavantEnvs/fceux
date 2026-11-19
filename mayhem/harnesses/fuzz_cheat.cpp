// Cheat file loading: FCEU_LoadGameCheats (cheat.cpp line parser, FCEUI_AddCheat, sub-cheat
// rebuild), then frames so the periodic/compare cheats are applied to emulated memory.
#include "fceux_fuzz_common.h"

extern "C" int LLVMFuzzerInitialize(int *, char ***) { fz::init(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  fz::init();
  if (!fz::load_builtin()) abort();
  std::string p = fz::g_work + "/game.cht";
  fz::write_file(p, data, size);
  if (FILE *f = fopen(p.c_str(), "rb")) {
    FCEU_LoadGameCheats(f, 1);   // an override FILE* stays open
    fclose(f);
    fz::run_frames(2);
  }
  FCEU_DeleteAllCheats();
  fz::finish();
  return 0;
}
