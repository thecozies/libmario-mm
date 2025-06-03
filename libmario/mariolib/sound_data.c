#include <types.h>
// #include "audio/data.h"
#include "config.h"

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

__attribute__((aligned(16))) u8 gSoundBanksHeader[] = {
    #include "sounds/ctl_header.inc.c"
};

__attribute__((aligned(16))) u8 gSampleBanksHeader[] = {
    #include "sounds/tbl_header.inc.c"
};

__attribute__((aligned(16))) u8 gSequencesHeader[] = {
    #include "sounds/sequences_header.inc.c"
};
