// Core netplay client: the input is the byte stream a netplay server sends. FCEUI_NetplayStart puts
// the core in netplay mode; every emulated frame NetplayUpdate (netplay.cpp) reads 5-byte
// joypad/command records through FCEUD_RecvData and acts on them: simple commands
// (FCEU_DoSimpleCommand: reset/power/FDS/VS coin...), chat text, and zlib-compressed cheat files
// (FetchFile -> FCEU_LoadGameCheats). Frames run until the stream is consumed (capped at 16 frames).
#include "fceux_fuzz_common.h"
#include "netplay.h"

extern const uint8 *fz_net_data;
extern size_t fz_net_len, fz_net_pos;

extern "C" int LLVMFuzzerInitialize(int *, char ***) { fz::init(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  fz::init();
  if (!fz::load_builtin()) abort();
  fz_net_data = data; fz_net_len = size; fz_net_pos = 0;
  FCEUI_NetplayStart(1, 1);
  for (int f = 0; f < 16 && GameInfo && fz_net_pos < fz_net_len; f++) fz::run_frames(1);
  if (FCEUnetplay) FCEUI_NetplayStop();
  fz_net_data = nullptr; fz_net_len = fz_net_pos = 0;
  FCEU_DeleteAllCheats();
  fz::finish();
  return 0;
}
