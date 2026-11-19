// Debugger symbol import: debugSymbolTable_t::ld65LoadDebugFile -> ld65dbg.cpp (ld65/cc65 .dbg
// line + key=value parser, scope/segment/symbol id cross references).
#include "fceux_fuzz_common.h"
#include "debugsymboltable.h"

extern "C" int LLVMFuzzerInitialize(int *, char ***) { fz::init(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  fz::init();
  std::string p = fz::g_work + "/game.dbg";
  fz::write_file(p, data, size);
  {
    debugSymbolTable_t tbl;
    tbl.ld65LoadDebugFile(p.c_str());
  }
  fz::finish();
  return 0;
}
