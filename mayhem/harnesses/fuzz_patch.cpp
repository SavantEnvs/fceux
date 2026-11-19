// ROM container paths in FCEU_fopen (file.cpp): the IPS patcher (ApplyIPS, auto-applied from
// <rom>.ips next to the ROM) and the bundled minizip archive reader (unzip.cpp, TryUnzip).
// First byte picks the mode; the rest is the IPS patch (over the built-in cartridge) or the archive.
#include "fceux_fuzz_common.h"

extern "C" int LLVMFuzzerInitialize(int *, char ***) { fz::init(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  fz::init();
  if (size < 1) return -1;
  std::string p;
  if (data[0] & 1) {
    std::vector<uint8_t> rom = fz::builtin_rom();
    p = fz::g_work + "/game.nes";
    fz::write_file(p, rom.data(), rom.size());
    fz::write_file(p + ".ips", data + 1, size - 1);
  } else {
    p = fz::g_work + "/game.zip";
    fz::write_file(p, data + 1, size - 1);
  }
  if (FCEUI_LoadGame(p.c_str(), 1, true)) fz::run_frames(2);
  fz::finish();
  return 0;
}
