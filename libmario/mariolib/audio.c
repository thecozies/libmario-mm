#include <PR/ultratypes.h>
#include <types.h>
#include <sm64.h>
#include "funcs.h"
#include "sound_data.h"

u8 *get_sm64_sound_data_ctl(void) {
    return gSoundDataADSR;
}

u8 *get_sm64_sound_data_tbl(void) {
    return gSoundDataRaw;
}

u8 *get_sm64_sequence_data(void) {
    return gMusicData;
}

u8 *get_sm64_bank_sets(void) {
    return gBankSetsData;
}

u8 *get_sm64_ctl_header(void) {
    return gSoundBanksHeader;
}

u8 *get_sm64_tbl_header(void) {
    return gSampleBanksHeader;
}

u8 *get_sm64_sequences_header(void) {
    return gSequencesHeader;
}

typedef struct
{
    u8 *offset;
    s32 len;
    s8 medium;
    s8 magic; // tbl: 0x04, otherwise: 0x03

    // for ctl (else zeros):
    union {
        // unused, just for clarification (big endian)
        struct {
            u8 bank;
            u8 ff;
            u8 numInstruments;
            u8 numDrums;
        } as_u8;

        // used
        struct {
            s16 bankAndFf;
            s16 numInstrumentsAndDrums;
        } as_s16;
    } ctl;
} ALSeqData_local;

typedef struct
{
    s16 seqCount;
    s16 unk2;
    u8 *data;
    ALSeqData_local seqArray[1];
} ALSeqFile_local;

void patch_seq_file(ALSeqFile_local *seqFile, u8 *data, u16 arg2) {
    s32 i;

    seqFile->unk2 = arg2;
    seqFile->data = data;
    for (i = 0; i < seqFile->seqCount; i++) {
        if (seqFile->seqArray[i].len != 0 && seqFile->seqArray[i].medium == 2) {
            seqFile->seqArray[i].offset += (uintptr_t)data;
        }
    }
}

void init_other_tables(void) {
    // gSeqFileHeader = (ALSeqFile_local *) gShindouSequencesHeader;
    // gAlCtlHeader = (ALSeqFile_local *) gShindouSoundBanksHeader;
    // gAlTbl = (ALSeqFile_local *) gShindouSampleBanksHeader;
    patch_seq_file((ALSeqFile_local *)gSequencesHeader, gMusicData, 0);
    patch_seq_file((ALSeqFile_local *)gSoundBanksHeader, gSoundDataADSR, 0);
    patch_seq_file((ALSeqFile_local *)gSampleBanksHeader, gSoundDataRaw, 0);
}
