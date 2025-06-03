
#include <PR/ultratypes.h>
#include <types.h>
#include <sm64.h>
#include "funcs.h"
#include <stdio.h>
#include <string.h>
#include "math.h"
#include "area.h"
#include "level_update.h"
#include "surface_collision.h"
#include "mario.h"
#include "object_list_processor.h"
#include "rendering_graph_node.h"
#include "graph_node.h"
#include "model_data.h"
#include "external.h"
#include "data.h"

extern s32 sGameLoopTicked;
extern s32 initSound;
u64* synthesis_execute_wrap(u64* abiCmdStart, s32* numAbiCmds, s16* aiBufStart, s32 numSamplesPerFrame) {
    if (!initSound) {
        return abiCmdStart;
    }
    
    gAudioFrameCount++;
    if (sGameLoopTicked != 0) {
        update_game_sound();
        sGameLoopTicked = 0;
    }

    u64* curCmd = synthesis_execute((u64 *)abiCmdStart, numAbiCmds, aiBufStart, numSamplesPerFrame);

    gAudioRandom = ((gAudioRandom + gAudioFrameCount) * gAudioFrameCount);
    return curCmd;
}
