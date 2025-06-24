#include <types.h>
#include "audio/data.h"
#include "config.h"

#ifdef ENABLE_SM64_RSP_AUDIO

#define SOUND_CTL_SIZE 97856
#define SOUND_TBL_SIZE 2216704
#define SEQ_DATA_SIZE 114112
#define BANK_SETS_SIZE 160

#ifdef EMBED_SM64_RSP_AUDIO_C
// in order to embed the data, these need to be extracted from the us sound build process
__attribute__((aligned(16))) u8 gSoundDataADSR[] = {
    #include "sounds/sound_data.ctl.inc.c"
};

__attribute__((aligned(16))) u8 gSoundDataRaw[] = {
    #include "sounds/sound_data.tbl.inc.c"
};

__attribute__((aligned(16))) u8 gMusicData[] = {
    #include "sounds/sequences.bin.inc.c"
};

__attribute__((aligned(16))) u8 gBankSetsData[] = {
    #include "sounds/bank_sets.inc.c"
};

// Peace of mind that the sizes are correct when not embedding C data
_Static_assert(sizeof(gSoundDataADSR) == SOUND_CTL_SIZE, "gSoundDataADSR size mismatch");
_Static_assert(sizeof(gSoundDataRaw) == SOUND_TBL_SIZE, "gSoundDataRaw size mismatch");
_Static_assert(sizeof(gMusicData) == SEQ_DATA_SIZE, "gMusicData size mismatch");
_Static_assert(sizeof(gBankSetsData) == BANK_SETS_SIZE, "gBankSetsData size mismatch");

#else

u8 gSoundDataADSR[SOUND_CTL_SIZE] = { 0 };
u8 gSoundDataRaw[SOUND_TBL_SIZE] = { 0 };
u8 gMusicData[SEQ_DATA_SIZE] = { 0 };
u8 gBankSetsData[BANK_SETS_SIZE] = { 0 };
#endif

#endif


__attribute__((aligned(16))) u8 gAudioHeap[DOUBLE_SIZE_ON_64_BIT(AUDIO_HEAP_SIZE)] = { 0 };
