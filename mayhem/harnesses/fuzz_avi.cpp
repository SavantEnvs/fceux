// AVI RIFF reader behind the Qt "AVI RIFF viewer" (src/drivers/Qt/avi/gwavi.cpp): openIn,
// riffwalk over RIFF/LIST/chunk headers, and getChunkData for each chunk exactly as the viewer
// does (header + payload). Frontend-independent; no Qt needed.
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>
#include "drivers/Qt/avi/gwavi.h"

static std::string g_path;
static gwavi_t *g_avi;

static int walk_cb(int type, long long int fpos, const char *fourcc, size_t size, void *) {
  if (type == gwavi_t::CHUNK_START && size < (1u << 20)) {
    std::vector<unsigned char> buf(size + 8);
    g_avi->getChunkData(fpos, buf.data(), size + 8);
  }
  return 0;
}

extern "C" int LLVMFuzzerInitialize(int *, char ***) {
  const char *t = getenv("TMPDIR");
  char tmpl[4096];
  snprintf(tmpl, sizeof tmpl, "%s/fceux-avi-XXXXXX", t && *t ? t : "/tmp");
  int fd = mkstemp(tmpl);
  if (fd < 0) abort();
  close(fd);
  g_path = tmpl;
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (g_path.empty()) LLVMFuzzerInitialize(nullptr, nullptr);
  FILE *f = fopen(g_path.c_str(), "wb");
  if (!f) abort();
  fwrite(data, 1, size, f);
  fclose(f);
  gwavi_t avi;
  g_avi = &avi;
  if (avi.openIn(g_path.c_str()) == 0) {
    avi.setRiffWalkCallback(walk_cb, nullptr);
    avi.riffwalk();
  }
  g_avi = nullptr;
  return 0;
}
