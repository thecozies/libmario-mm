#ifndef SM64_AUDIO_H
#define SM64_AUDIO_H

#ifdef I_AM_OOT
    #include "ultra64.h"
#else
    #include "<ultra64>"
#endif

// gSoundDataADSR (ROM) -> gSM64SoundFontData
u8 *get_sm64_sound_data_ctl(void);

// gSoundDataRaw (ROM) -> gSM64SampleBankData
u8 *get_sm64_sound_data_tbl(void);

// gMusicData (ROM) -> gSM64SequenceData
u8 *get_sm64_sequence_data(void);

// gBankSetsData -> gSM64SequenceFontTable
u8 *get_sm64_bank_sets(void);

// gSoundBanksHeader -> gSM64SoundFontTable
u8 *get_sm64_ctl_header(void);

// gSampleBanksHeader -> gSM64SampleBankTable
u8 *get_sm64_tbl_header(void);

// gSequencesHeader -> gSM64SequenceTable
u8 *get_sm64_sequences_header(void);

#endif // SM64_AUDIO_H
