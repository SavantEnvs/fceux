// Debugger name-list import: debugSymbolTable_t::loadGameSymbols -> loadFileNL (the line-oriented
// `$ADDR[/LEN]#name#comment` parser with `\` continuation lines). The debugger reads these from
// `<rom>.ram.nl` (RAM, bank -1) and `<rom>.<bank>.nl` (PRG banks) next to the loaded ROM.
// Input byte 0 bit 0 picks which of the two files the rest of the input becomes; the built-in
// cartridge has one 16 KiB PRG bank (debuggerPageSize 14), so the bank file is `<rom>.0.nl`.
#include "fceux_fuzz_common.h"
#include "debugsymboltable.h"

extern char LoadedRomFName[4096];

extern "C" int LLVMFuzzerInitialize(int *, char ***) { fz::init(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size < 1) return -1;
  fz::init();
  if (fz::load_builtin()) {
    std::string p = std::string(LoadedRomFName) + ((data[0] & 1) ? ".ram.nl" : ".0.nl");
    fz::write_file(p, data + 1, size - 1);
    debugSymbolTable_t tbl;
    tbl.loadGameSymbols();
    (void)tbl.numSymbols();
  }
  fz::finish();
  return 0;
}
