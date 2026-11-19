// Headless driver for the fceux core: the FCEUD_* callbacks a frontend (Qt/SDL/Win32) normally
// provides, implemented as no-ops (no video/sound/archive/network output). Shared by every harness
// and by the test runner.
#include "types.h"
#include "driver.h"
#include "fceu.h"
#include "emufile.h"
#include "state.h"
#include "movie.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
int closeFinishedMovie=0, dendy=0, eoptions=0, KillFCEUXonFrame=0, pal_emulation=0;
bool paldeemphswap=false, swapDuty=false, turbo=false;
void FCEUD_AviRecordTo(){} void FCEUD_AviStop(){} void FCEUD_DebugBreakpoint(int){} void FCEUD_FlushTrace(){}
const char *FCEUD_GetCompilerString(){return "x";}
static uint8 pr[256],pg[256],pb[256];
void FCEUD_GetPalette(uint8 i,uint8*r,uint8*g,uint8*b){*r=pr[i];*g=pg[i];*b=pb[i];}
void FCEUD_SetPalette(uint8 i,uint8 r,uint8 g,uint8 b){pr[i]=r;pg[i]=g;pb[i]=b;}
uint64 FCEUD_GetTime(){return 0;} uint64 FCEUD_GetTimeFreq(){return 1000;}
void FCEUD_HideMenuToggle(){} void FCEUD_LoadStateFrom(){} void FCEUD_Message(const char*){} void FCEUD_MovieRecordTo(){} void FCEUD_MovieReplayFrom(){}
void FCEUD_NetplayText(uint8*){} void FCEUD_NetworkClose(){}
FCEUFILE* FCEUD_OpenArchive(ArchiveScanRecord&, std::string&, std::string*, int*){return 0;}
FCEUFILE* FCEUD_OpenArchiveIndex(ArchiveScanRecord&, std::string&, int, int*){return 0;}
ArchiveScanRecord FCEUD_ScanArchive(std::string){return ArchiveScanRecord();}
bool FCEUD_PauseAfterPlayback(){return false;} void FCEUD_PrintError(const char*){}
// Netplay transport: the fuzz_netplay harness points these at its input; elsewhere they stay empty,
// so FCEUD_RecvData reports a closed connection exactly like the stub it replaces.
const uint8 *fz_net_data = nullptr; size_t fz_net_len = 0, fz_net_pos = 0;
int FCEUD_RecvData(void*d,uint32 n){ if(!fz_net_data || fz_net_len-fz_net_pos < n) return 0; memcpy(d,fz_net_data+fz_net_pos,n); fz_net_pos+=n; return 1; } int FCEUD_SendData(void*,uint32){return 1;}
void FCEUD_SaveStateAs(){} void FCEUD_SetEmulationSpeed(int){} void FCEUD_SetInput(bool,bool,ESI,ESI,ESIFC){}
bool FCEUD_ShouldDrawInputAids(){return false;} int FCEUD_ShowStatusIcon(){return 0;} void FCEUD_SoundToggle(){} void FCEUD_SoundVolumeAdjust(int){}
void FCEUD_ToggleStatusIcon(){} void FCEUD_TurboOff(){} void FCEUD_TurboOn(){} void FCEUD_TurboToggle(){}
void FCEUD_UpdateNTView(int,bool){} void FCEUD_UpdatePPUView(int,int){}
FILE *FCEUD_UTF8fopen(const char*f,const char*m){return fopen(f,m);}
EMUFILE_FILE* FCEUD_UTF8_fstream(const char*n,const char*m){EMUFILE_FILE*p=new EMUFILE_FILE(n,m); if(!p->is_open()){delete p;return 0;} return p;}
void FCEUD_VideoChanged(){} bool fceu_file_exists(const char*f){FILE*p=fopen(f,"rb"); if(p)fclose(p); return p!=0;}
bool FCEUI_AviDisableMovieMessages(){return true;} bool FCEUI_AviEnableHUDrecording(){return false;} bool FCEUI_AviIsRecording(){return false;}
void FCEUI_AviVideoUpdate(const unsigned char*){} void FCEUI_UseInputPreset(int){}
static unsigned int kb[256]; unsigned int *GetKeyboard(){return kb;} void GetMouseData(uint32 (&md)[3]){md[0]=md[1]=md[2]=0;}
uint8 *ModernDeemphColorMap(const uint8*src,const uint8*,int){return (uint8*)src;}
void RefreshThrottleFPS(){}
