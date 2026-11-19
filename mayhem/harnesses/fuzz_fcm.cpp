// Legacy FCM movie conversion (oldmovie.cpp convert_fcm, what --fcmconvert runs): header,
// metadata, compressed input stream decoder; the converted movie is then serialized as FM2.
#include "fceux_fuzz_common.h"
#include "oldmovie.h"

extern "C" int LLVMFuzzerInitialize(int *, char ***) { fz::init(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  fz::init();
  std::string p = fz::g_work + "/movie.fcm";
  fz::write_file(p, data, size);
  {
    MovieData md;
    if (convert_fcm(md, p) == FCM_CONVERTRESULT_SUCCESS) {
      EMUFILE_MEMORY out;
      md.dump(&out, false);
    }
  }
  fz::finish();
  return 0;
}
