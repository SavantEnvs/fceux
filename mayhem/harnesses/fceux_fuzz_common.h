// Shared helpers for the fceux harnesses: one-time core init with a private scratch base
// directory, input staging as files (the core's loaders are path-based), per-input cleanup of
// everything the core writes there (battery saves, cheats, states), and a tiny built-in NROM
// cartridge for targets that need a game loaded first.
#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <ftw.h>
#include <unistd.h>
#include <sys/stat.h>

#include "types.h"
#include "driver.h"
#include "fceu.h"
#include "emufile.h"
#include "state.h"
#include "movie.h"
#include "cheat.h"

namespace fz {

static std::string g_base;     // FCEUI_SetBaseDirectory() target (disksys.rom lives here)
static std::string g_work;     // per-input staging directory, wiped after every input

static void write_file(const std::string &p, const uint8_t *d, size_t n) {
  FILE *f = fopen(p.c_str(), "wb");
  if (!f) abort();
  if (n && fwrite(d, 1, n, f) != n) abort();
  fclose(f);
}

static int rm_cb(const char *p, const struct stat *, int, struct FTW *ftw) {
  if (ftw->level == 0) return 0;           // keep the directory itself
  remove(p);
  return 0;
}
static void wipe_dir(const std::string &d) { nftw(d.c_str(), rm_cb, 16, FTW_DEPTH | FTW_PHYS); }

// Remove everything the core may have written below the base directory (sav/, cheats/, fcs/ ...)
// and the staged inputs, so no input observes a previous one. disksys.rom is re-created.
static void make_bios();
static void reset_dirs() {
  wipe_dir(g_base);
  make_bios();
  mkdir(g_work.c_str(), 0700);
}

// Dummy 8 KiB FDS BIOS (the real one is not redistributable): every vector points at a JMP-to-self.
static void make_bios() {
  std::vector<uint8_t> b(8192, 0xEA);
  b[0] = 0x4C; b[1] = 0x00; b[2] = 0xE0;              // $E000: JMP $E000
  for (int v = 0x1FFA; v < 0x2000; v += 2) { b[v] = 0x00; b[v + 1] = 0xE0; }
  write_file(g_base + "/disksys.rom", b.data(), b.size());
}

// A 16 KiB-PRG / 8 KiB-CHR NROM image: enables rendering, then increments zero page in a loop and
// mirrors the counter into the palette/scroll so frames differ. Deterministic by construction.
static std::vector<uint8_t> builtin_rom(uint8_t mapper = 0, uint8_t flags6 = 0) {
  std::vector<uint8_t> r(16 + 16384 + 8192, 0);
  const uint8_t hdr[16] = {'N', 'E', 'S', 0x1A, 1, 1, (uint8_t)((mapper << 4) | flags6),
                           (uint8_t)(mapper & 0xF0), 0, 0, 0, 0, 0, 0, 0, 0};
  memcpy(r.data(), hdr, 16);
  uint8_t *prg = &r[16];
  memset(prg, 0xEA, 16384);
  const uint8_t code[] = {
      0x78, 0xD8, 0xA2, 0xFF, 0x9A,             // SEI CLD LDX #$FF TXS
      0xA9, 0x80, 0x8D, 0x00, 0x20,             // LDA #$80 STA $2000 (NMI on)
      0xA9, 0x1E, 0x8D, 0x01, 0x20,             // LDA #$1E STA $2001 (render on)
      0xE6, 0x00,                               // loop: INC $00
      0xA5, 0x00, 0x8D, 0x05, 0x20,             // LDA $00 STA $2005
      0x65, 0x01, 0x85, 0x01,                   // ADC $01 STA $01
      0x4C, 0x0F, 0xC0,                         // JMP loop ($C00F)
  };
  memcpy(prg, code, sizeof(code));
  prg[0x100] = 0xE6; prg[0x101] = 0x02; prg[0x102] = 0x40;   // NMI ($C100): INC $02; RTI
  prg[0x3FFA] = 0x00; prg[0x3FFB] = 0xC1;   // NMI
  prg[0x3FFC] = 0x00; prg[0x3FFD] = 0xC0;   // RESET
  prg[0x3FFE] = 0x02; prg[0x3FFF] = 0xC1;   // IRQ -> RTI
  uint8_t *chr = &r[16 + 16384];
  for (int i = 0; i < 8192; i++) chr[i] = (uint8_t)(i * 7 + (i >> 4));
  return r;
}

static void init() {
  static bool done = false;
  if (done) return;
  done = true;
  const char *t = getenv("TMPDIR");
  std::string tmpl = std::string(t && *t ? t : "/tmp") + "/fceux-fuzz-XXXXXX";
  std::vector<char> buf(tmpl.begin(), tmpl.end());
  buf.push_back(0);
  if (!mkdtemp(buf.data())) abort();
  g_base = buf.data();
  g_work = g_base + "/in";
  if (!FCEUI_Initialize()) abort();
  FCEUI_SetBaseDirectory(g_base);
  FCEUI_Sound(44100);
  reset_dirs();
}

static void run_frames(int n) {
  for (int i = 0; i < n && GameInfo; i++) {
    uint8 *gfx = nullptr; int32 *snd = nullptr; int32 ssize = 0;
    FCEUI_Emulate(&gfx, &snd, &ssize, 0);
  }
}

// Stage the built-in cartridge and load it (fresh power-on state for each input).
static bool load_builtin() {
  std::vector<uint8_t> rom = builtin_rom();
  std::string p = g_work + "/builtin.nes";
  write_file(p, rom.data(), rom.size());
  return FCEUI_LoadGame(p.c_str(), 1, true) != nullptr;
}

static void finish() {
  if (GameInfo) FCEUI_CloseGame();
  reset_dirs();
}

}  // namespace fz
