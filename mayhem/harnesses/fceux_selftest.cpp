// Functional known-answer checks for the headless fceux core (built with normal flags, run by
// mayhem/test.sh, which diffs every printed line against mayhem/tests/golden.txt):
// ROM identification (MD5), deterministic CPU/PPU emulation (RAM + framebuffer CRC after N frames),
// savestate round-trip, Game Genie decoding, mapper-218 rendering, ld65 debug-symbol import.
#include "fceux_fuzz_common.h"
#include "debugsymboltable.h"
#include "utils/crc32.h"
#include "utils/md5.h"
#include "video.h"

extern uint8 *RAM;

static uint32_t ram_crc() { return CalcCRC32(0, RAM, 0x800); }
static uint32_t fb_crc() { return CalcCRC32(0, XBuf, 256 * 240); }

int main() {
  fz::init();
  // 1. built-in NROM: identification + 60 emulated frames
  if (!fz::load_builtin()) { puts("load_builtin FAIL"); return 1; }
  printf("rom_md5 %s\n", md5_asciistr(GameInfo->MD5));
  printf("rom_mapper %d\n", GameInfo->mappernum);
  fz::run_frames(60);
  printf("ram_after_60 %08x zp=%02x%02x%02x\n", ram_crc(), RAM[0], RAM[1], RAM[2]);
  printf("fb_after_60 %08x\n", fb_crc());
  // 2. savestate round trip: save, run 10, reload, run 10 -> identical RAM
  EMUFILE_MEMORY ms;
  printf("state_save %d\n", (int)FCEUSS_SaveMS(&ms, 0));
  fz::run_frames(10);
  uint32_t a = ram_crc();
  ms.fseek(0, SEEK_SET);
  printf("state_load %d\n", (int)FCEUSS_LoadFP(&ms, SSLOADPARAM_NOBACKUP));
  fz::run_frames(10);
  printf("state_roundtrip %08x %s\n", a, a == ram_crc() ? "same" : "DIFF");
  fz::finish();
  // 3. Game Genie codes
  const char *codes[] = {"SXIOPO", "AAEAULPA", "GOSSIP"};
  for (const char *c : codes) {
    int ad = -1, v = -1, cmp = -1;
    int ok = FCEUI_DecodeGG(c, &ad, &v, &cmp);
    printf("gg_%s %d %04x %02x %d\n", c, ok, ad, v, cmp);
  }
  // 4. mapper 218 (Magic Floor: CIRAM as CHR) with each mirroring combination
  for (int f6 = 0; f6 < 16; f6 += 1) {
    if (f6 & 2) continue;  // no battery
    std::vector<uint8_t> rom = fz::builtin_rom(218, (uint8_t)f6);
    std::string p = fz::g_work + "/m218.nes";
    fz::write_file(p, rom.data(), rom.size());
    if (!FCEUI_LoadGame(p.c_str(), 1, true)) { printf("m218_%x load FAIL\n", f6); continue; }
    fz::run_frames(20);
    printf("m218_%x mapper=%d ram=%08x fb=%08x\n", f6, GameInfo->mappernum, ram_crc(), fb_crc());
    fz::finish();
  }
  // 5. ld65 debug file import
  {
    const char dbg[] =
        "version\tmajor=2,minor=0\n"
        "seg\tid=0,name=\"CODE\",start=0x00C000,size=0x4000,addrsize=absolute,type=ro,oname=\"g.nes\",ooffs=16\n"
        "scope\tid=0,name=\"\",mod=0,size=16\n"
        "sym\tid=0,name=\"reset\",addrsize=absolute,size=1,scope=0,def=1,val=0xC000,seg=0,type=lab\n"
        "sym\tid=1,name=\"nmi\",addrsize=absolute,size=1,scope=0,def=2,val=0xC100,seg=0,type=lab\n";
    std::string p = fz::g_work + "/g.dbg";
    fz::write_file(p, (const uint8_t *)dbg, sizeof(dbg) - 1);
    debugSymbolTable_t tbl;
    int r = tbl.ld65LoadDebugFile(p.c_str());
    debugSymbol_t *s = tbl.getSymbolAtAnyBank("reset");
    debugSymbol_t *n = tbl.getSymbolAtAnyBank("nmi");
    printf("ld65 %d %s %s\n", r, s ? s->name().c_str() : "-", n ? n->name().c_str() : "-");
    fz::finish();
  }
  return 0;
}
