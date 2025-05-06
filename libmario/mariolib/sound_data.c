#include <types.h>
#include "audio/data.h"
#include "config.h"

#ifdef ENABLE_SM64_RSP_AUDIO
u8 gSoundDataADSR[] = {
    #include "sounds/sound_data.ctl.inc.c"
};

u8 gSoundDataRaw[] = {
    #include "sounds/sound_data.tbl.inc.c"
};

u8 gMusicData[] = {
    #include "sounds/sequences.bin.inc.c"
};

u8 gBankSetsData[] = {
    #include "sounds/bank_sets.inc.c"
};
#else
u8 gSoundDataADSR[] = { 0 };
u8 gSoundDataRaw[] = { 0 };
u8 gMusicData[] = { 0 };
u8 gBankSetsData[] = { 0 };
#endif


u8 gAudioHeap[DOUBLE_SIZE_ON_64_BIT(AUDIO_HEAP_SIZE)] = { 0 };
