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

#include "anims.c"

u8 gBorderHeight = 0;

void spawn_wind_particles(s16 pitch, s16 yaw) {}
s32 save_file_get_flags() { return 0; }
void *segmented_to_virtual(const void *addr) { return (void*)addr; }
void *virtual_to_segmented(u32 segment, const void *addr) { return (void*)addr; }

struct MainPoolBlock {
    struct MainPoolBlock *prev;
    struct MainPoolBlock *next;
};
#define ALIGN16(val) (((val) + 0xF) & ~0xF)

#define MAIN_POOL_SIZE 0x8000
u8 sMainPool[MAIN_POOL_SIZE];
u32 sPoolFreeSpace;
u8 *sPoolStart;
u8 *sPoolEnd;
struct MainPoolBlock *sPoolListHeadL;
struct MainPoolBlock *sPoolListHeadR;

void main_pool_init(void *start, void *end) {
    sPoolStart = (u8 *) ALIGN16((uintptr_t) start) + 16;
    sPoolEnd = (u8 *) ALIGN16((uintptr_t) end - 15) - 16;
    sPoolFreeSpace = sPoolEnd - sPoolStart;

    sPoolListHeadL = (struct MainPoolBlock *) (sPoolStart - 16);
    sPoolListHeadR = (struct MainPoolBlock *) sPoolEnd;
    sPoolListHeadL->prev = NULL;
    sPoolListHeadL->next = NULL;
    sPoolListHeadR->prev = NULL;
    sPoolListHeadR->next = NULL;
}

u32 main_pool_available(void) {
    return sPoolFreeSpace - 16;
}

#define ALIGN4(val) (((val) + 0x3) & ~0x3)

void *main_pool_alloc(u32 size, u32 side) {
    struct MainPoolBlock *newListHead;
    void *addr = NULL;

    size = ALIGN16(size) + 16;
    if (size != 0 && sPoolFreeSpace >= size) {
        sPoolFreeSpace -= size;
        if (side == MEMORY_POOL_LEFT) {
            newListHead = (struct MainPoolBlock *) ((u8 *) sPoolListHeadL + size);
            sPoolListHeadL->next = newListHead;
            newListHead->prev = sPoolListHeadL;
            newListHead->next = NULL;
            addr = (u8 *) sPoolListHeadL + 16;
            sPoolListHeadL = newListHead;
        } else {
            newListHead = (struct MainPoolBlock *) ((u8 *) sPoolListHeadR - size);
            sPoolListHeadR->prev = newListHead;
            newListHead->next = sPoolListHeadR;
            newListHead->prev = NULL;
            sPoolListHeadR = newListHead;
            addr = (u8 *) sPoolListHeadR + 16;
        }
    }
    return addr;
}

u32 main_pool_free(void *addr) {
    struct MainPoolBlock *block = (struct MainPoolBlock *) ((u8 *) addr - 16);
    struct MainPoolBlock *oldListHead = (struct MainPoolBlock *) ((u8 *) addr - 16);

    if (oldListHead < sPoolListHeadL) {
        while (oldListHead->next != NULL) {
            oldListHead = oldListHead->next;
        }
        sPoolListHeadL = block;
        sPoolListHeadL->next = NULL;
        sPoolFreeSpace += (uintptr_t) oldListHead - (uintptr_t) sPoolListHeadL;
    } else {
        while (oldListHead->prev != NULL) {
            oldListHead = oldListHead->prev;
        }
        sPoolListHeadR = block->next;
        sPoolListHeadR->prev = NULL;
        sPoolFreeSpace += (uintptr_t) sPoolListHeadR - (uintptr_t) oldListHead;
    }
    return sPoolFreeSpace;
}

struct AllocOnlyPool *alloc_only_pool_init(u32 size, u32 side) {
    void *addr;
    struct AllocOnlyPool *subPool = NULL;

    size = ALIGN4(size);
    addr = main_pool_alloc(size + sizeof(struct AllocOnlyPool), side);
    if (addr != NULL) {
        subPool = (struct AllocOnlyPool *) addr;
        subPool->totalSpace = size;
        subPool->usedSpace = 0;
        subPool->startPtr = (u8 *) addr + sizeof(struct AllocOnlyPool);
        subPool->freePtr = (u8 *) addr + sizeof(struct AllocOnlyPool);
    }
    return subPool;
}

void *alloc_only_pool_alloc(struct AllocOnlyPool *pool, s32 size) {
    void *addr = NULL;

    size = ALIGN4(size);
    if (size > 0 && pool->usedSpace + size <= pool->totalSpace) {
        addr = pool->freePtr;
        pool->freePtr += size;
        pool->usedSpace += size;
    }
    return addr;
}

Gfx *gDisplayListHead;
u8 *gGfxPoolEnd;
struct GfxPool gGfxPools[2];
struct GfxPool *gGfxPool = &gGfxPools[0];
#define ALIGN8(val) (((val) + 0x7) & ~0x7)
void *alloc_display_list(u32 size) {
    void *ptr = NULL;

    size = ALIGN8(size);
    if (gGfxPoolEnd - size >= (u8 *) gDisplayListHead) {
        gGfxPoolEnd -= size;
        ptr = gGfxPoolEnd;
    }
    return ptr;
}

void init_graphics_pool(void) {
    main_pool_init(sMainPool, sMainPool + MAIN_POOL_SIZE);
    gGfxPool = &gGfxPools[0];
    gDisplayListHead = gGfxPool->buffer;
    gGfxPoolEnd = (u8 *)(gGfxPool->buffer + GFX_POOL_SIZE);
}



void select_gfx_pool(void) {
    static s32 whichPool = 0;
    whichPool ^= 1;
    gGfxPool = &gGfxPools[whichPool];
    gDisplayListHead = gGfxPool->buffer;
    gGfxPoolEnd = (u8 *) (gGfxPool->buffer + GFX_POOL_SIZE);
}

void clear_framebuffer(s32 color) {}
void make_viewport_clip_rect(Vp *viewport) {};

void dl_rgba16_begin_cutscene_msg_fade() {}
void dl_rgba16_stop_cutscene_msg_fade(void) {}
void print_text_fmt_int(s32 x, s32 y, const char *str, s32 n) {}
void print_credits_str_ascii(s16 x, s16 y, const char *str) {}
void print_debug_top_down_objectinfo(const char *str, s32 number) {}
void play_transition(s16 transType, s16 time, u8 red, u8 green, u8 blue) {}
void create_dialog_box(s16 dialog) {}
void create_dialog_box_with_response(s16 dialog) {}
void create_dialog_box_with_var(s16 dialog, s32 dialogVar) {}
void create_dialog_inverted_box(s16 dialog) {}
void trigger_cutscene_dialog(s32 trigger) {}
s16 get_dialog_id(void) {}
void fade_into_special_warp(u32 arg, u32 color) {}
s16 cutscene_object_without_dialog(u8 cutscene, struct Object *o) { return 0; }
s16 cutscene_object_with_dialog(u8 cutscene, struct Object *o, s16 dialogID) { return 0; }

void set_camera_mode(struct Camera *c, s16 mode, s16 frames) {}
void set_camera_shake_from_hit(s16 shake) {}
f32 camera_approach_f32_symmetric(f32 current, f32 target, f32 increment) {}
void set_camera_shake_from_point(s16 shake, f32 posX, f32 posY, f32 posZ) {}

void bhv_spawn_star_no_level_exit(u32 sp20) {}
s16 level_trigger_warp(struct MarioState *m, s32 warpOp) { return 0; }
void load_level_init_text(u32 arg) {}
void set_cutscene_message(s16 xOffset, s16 yOffset, s16 msgIndex, s16 msgDuration) {}
s32 save_file_get_total_star_count(s32 fileIndex, s32 minCourse, s32 maxCourse) { return 0; }
void set_menu_mode(s16 mode) {}

void play_music(u8 player, u16 seqArgs, u16 fadeTimer) {}
void raise_background_noise(s32 a) {}
void play_toads_jingle(void) {}
void play_menu_sounds(s16 soundMenuFlags) {}
void stop_sound(u32 soundBits, f32 *pos) {}
void *play_sound(s32 soundBits, f32 *pos) {}
void set_sound_moving_speed(u8 bank, u8 speed) {}
void play_infinite_stairs_music() {}
void lower_background_noise(s32 a) {}
void seq_player_lower_volume(u8 player, u16 fadeDuration, u8 percentage) {}
void seq_player_unlower_volume(u8 player, u16 fadeDuration) {}
void play_cutscene_music(u16 seqArgs) {}
void play_course_clear(void) {}
void sound_banks_enable(UNUSED u8 player, u16 bankMask) {}
void disable_background_sound(void) {}
void enable_background_sound(void) {}
void play_peachs_jingle(void) {}
void cur_obj_play_sound_2(s32 soundMagic) {}
void play_shell_music(void) {}
void stop_shell_music(void) {}
void play_cap_music(void) {}
void stop_cap_music(void) {}
void fadeout_cap_music(void) {}

Gfx *gdm_gettestdl(s32 id) { return NULL; }
void gd_vblank(void) {}
s32 gd_sfx_to_play(void) { return 0; }
void gd_copy_p1_contpad(OSContPad *p1cont) {}

void save_file_do_save(s32 fileIndex) {}
void save_file_set_flags(u32 flags) {}
void save_file_clear_flags(u32 flags) {}
void override_viewport_and_clip(Vp *a, Vp *b, u8 c, u8 d, u8 e) {}
void reset_cutscene_msg_fade(void) {}
u32 get_door_save_file_flag(struct Object *door) { return 0; }
s32 save_file_get_cap_pos(Vec3s capPos) {}

void mario_grab_used_object(struct MarioState *m) {}
void mario_drop_held_object(struct MarioState *m) {}
u32 mario_check_object_grab(struct MarioState *m) { return FALSE; }
void mario_throw_held_object(struct MarioState *m) {}
void mario_stop_riding_object(struct MarioState *m) {}
void mario_stop_riding_and_holding(struct MarioState *m) {}
void mario_blow_off_cap(struct MarioState *m, f32 capSpeed) {}

void spawn_default_star(f32 sp20, f32 sp24, f32 sp28) {}
void spawn_mist_particles_variable(s32 count, s32 offsetY, f32 size) {}
void create_sound_spawner(s32 soundMagic) {}
void spawn_triangle_break_particles(s16 numTris, s16 triModel, f32 triSize, s16 triAnimState) {}
struct Object *create_object(const BehaviorScript *bhvScript) { return NULL; }

s16 gFindFloorIncludeSurfaceIntangible = FALSE;
s8 gDebugLevelSelect = FALSE;
s8 gShowDebugText = FALSE;
s16 gDebugInfo[16][8];
u32 gTimeStopState = 0;
s8 gNeverEnteredCastle = 1;
extern u16 gAreaUpdateCounter;
struct Controller *gPlayer1Controller;
struct DmaHandlerList gMarioAnimsBuf;
float gPaintingMarioYEntry = 0.0f;

struct MarioState gMarioStates[1];
struct HudDisplay gHudDisplay;
struct MarioState *gMarioState = &gMarioStates[0];
struct Object MarioObjectInstance;
struct Object *gMarioObject = &MarioObjectInstance;
struct SpawnInfo gPlayerSpawnInfos[1];
struct SpawnInfo *gMarioSpawnInfo = &gPlayerSpawnInfos[0];
struct Area *gAreas = gAreaData;
struct Area gCurrentAreaInstance;
struct Area *gCurrentArea = &gCurrentAreaInstance;
struct Object *gCurrentObject = &MarioObjectInstance;
struct Object *gCutsceneFocus = NULL;
struct Object *gSecondCameraFocus = NULL;
s16 gWarpTransDelay = 0;
u32 gFBSetColor = 0;
u32 gWarpTransFBSetColor = 0;
u8 gWarpTransRed = 0;
u8 gWarpTransGreen = 0;
u8 gWarpTransBlue = 0;
s16 gCurrSaveFileNum = 1;
s16 gCurrLevelNum = LEVEL_MIN;
u32 gAudioRandom = 0;
s32 gDialogResponse = 0;

struct Camera *gCamera;

s16 gCurrCourseNum;
s16 gCurrActNum;
s16 gCurrAreaIndex;
s16 gSavedCourseNum;
s16 gPauseScreenMode;
s16 gSaveOptSelectIndex;
s16 gMarioCurrentRoom;

s16 gCurrAreaIndex = 0;
u8 gLastCompletedCourseNum = 1;
u8 gLastCompletedStarNum = 0;
s8 sUnusedGotGlobalCoinHiScore = 0;
u8 gGotFileCoinHiScore = FALSE;
u8 gCurrCourseStarFlags = 0;
u8 gSpecialTripleJump = FALSE;

s16 gCameraMovementFlags = 0;

struct CreditsEntry *gCurrCreditsEntry = NULL;

struct GraphNodeRoot gCurGraphNodeRootSrc;
extern struct GraphNodeRoot *gCurGraphNodeRoot;
extern struct GraphNodeMasterList *gCurGraphNodeMasterList;
extern struct GraphNodePerspective *gCurGraphNodeCamFrustum;
extern struct GraphNodeCamera *gCurGraphNodeCamera;
extern struct GraphNodeObject *gCurGraphNodeObject;
extern struct GraphNodeHeldObject *gCurGraphNodeHeldObject;

struct GraphNode *gGraphNodePointers[MODEL_ID_COUNT];
struct GraphNode **gLoadedGraphNodes = gGraphNodePointers;

Gfx *gDisplayListHeadOpa = NULL;
Gfx *gDisplayListHeadXlu = NULL;

struct GraphNodeMasterList gMasterList = { {}, {{}}, {{}} };

#define GRAPH_NODE_POOL_SIZE 0x8000
u8 gGraphNodePoolBuffer[GRAPH_NODE_POOL_SIZE] = { 0 };
struct AllocOnlyPool gGraphNodePoolSrc = { GRAPH_NODE_POOL_SIZE, 0, gGraphNodePoolBuffer, gGraphNodePoolBuffer };

#define LEVEL_POOL_SIZE 0x8000
u8 gLevelPoolBuffer[LEVEL_POOL_SIZE] = { 0 };
struct AllocOnlyPool sLevelPoolSrc = { LEVEL_POOL_SIZE, 0, gLevelPoolBuffer, gLevelPoolBuffer };
struct AllocOnlyPool *sLevelPool = NULL;

void init_graph_node_system(void) {
    gCurGraphNodeRootSrc.areaIndex = 1;
    gCurGraphNodeMasterList = &gMasterList;
    gMasterList.node.prev = &gMasterList.node;
    gMasterList.node.next = &gMasterList.node;
    gMasterList.node.parent = NULL;
    gMasterList.node.type = GRAPH_NODE_TYPE_MASTER_LIST;
    gMasterList.node.flags = GRAPH_RENDER_ACTIVE | GRAPH_RENDER_Z_BUFFER;
    gMasterList.node.children = &MarioObjectInstance.header.gfx.node;
    gGraphNodePool = &gGraphNodePoolSrc;
    MarioObjectInstance.header.gfx.node.next = &MarioObjectInstance.header.gfx.node;
    MarioObjectInstance.header.gfx.node.prev = &MarioObjectInstance.header.gfx.node;
    MarioObjectInstance.header.gfx.areaIndex = 1;
    sLevelPool = &sLevelPoolSrc;
    gLoadedGraphNodes[MODEL_MARIO] = process_geo_layout(sLevelPool, mario_geo);
    gMarioSpawnInfo->model = gLoadedGraphNodes[MODEL_MARIO];
}

struct PlayerCameraState gPlayerCameraState[2];
struct WarpTransition gWarpTransition;

const BehaviorScript bhvSparkleSpawn[] = { 0 };
const BehaviorScript bhvJumpingBox[] = { 0 };
const BehaviorScript bhvEndToad[] = { 0 };
const BehaviorScript bhvEndPeach[] = { 0 };
const BehaviorScript bhvCelebrationStar[] = { 0 };
const BehaviorScript bhvStaticObject[] = { 0 };
const BehaviorScript bhvTree[] = { 0 };
const BehaviorScript bhvGiantPole[] = { 0 };
const BehaviorScript bhvBowserKeyUnlockDoor[] = { 0 };
const BehaviorScript bhvUnlockDoorStar[] = { 0 };
const BehaviorScript bhvCarrySomethingDropped[] = { 0 };
const BehaviorScript bhvCarrySomethingHeld[] = { 0 };
const BehaviorScript bhvCarrySomethingThrown[] = { 0 };
const BehaviorScript bhvBowserKeyCourseExit[] = { 0 };
const BehaviorScript bhvKoopaShellUnderwater[] = { 0 };
const BehaviorScript bhvMrIBlueCoin[] = { 0 };
const BehaviorScript bhvNormalCap[] = { 0 };
const BehaviorScript bhvBowser[] = { 0 };
const BehaviorScript bhvBlueCoinJumping[] = { 0 };
const BehaviorScript bhvSingleCoinGetsSpawned[] = { 0 };
const BehaviorScript bhvSpawnedStarNoLevelExit[] = { 0 };
const BehaviorScript bhvWhitePuffExplosion[] = { 0 };

struct Area gAreaData[8] = { { 0 } };
s8 gDoorAdjacentRooms[60][2];
struct ObjectNode gObjectListArray[16];
struct ObjectNode gFreeObjectList;
struct ObjectNode *gObjectLists = gObjectListArray;
s16 gPrevFrameObjectCount = 0;

f32 gGlobalSoundSource[3] = { 0.0f, 0.0f, 0.0f };


// s16 gCheckingSurfaceCollisionsForCamera;
// s16 gFindFloorIncludeSurfaceIntangible;
// s16 *gEnvironmentRegions;
// s32 gEnvironmentLevels[20];
// s8 gDoorAdjacentRooms[60][2];
// s16 gMarioCurrentRoom;
// s16 D_8035FEE2;
// s16 D_8035FEE4;
// s16 gTHIWaterDrained;
// s16 gTTCSpeedSetting;
// s16 gMarioShotFromCannon;
// s16 gCCMEnteredSlide;
s16 gNumRoomedObjectsInMarioRoom = 0;
s16 gNumRoomedObjectsNotInMarioRoom = 0;
// s16 gWDWWaterLevelChanging;
// s16 gMarioOnMerryGoRound;

void (*gGoddardVblankCallback)(void) = NULL;

s16 Math_Atan2S(f32 y, f32 x);

s16 mario_obj_angle_to_object(struct MarioState *m, struct Object *o) {
    f32 dx = o->oPosX - m->pos[0];
    f32 dz = o->oPosZ - m->pos[2];

    return Math_Atan2S(dz, dx);
}

// void guMtxF2L(float mf[4][4], Mtx *m) {
//     int r, c;
//     s32 tmp1;
//     s32 tmp2;
//     s32 *m1 = &m->m[0][0];
//     s32 *m2 = &m->m[2][0];
//     for (r = 0; r < 4; r++) {
//         for (c = 0; c < 2; c++) {
//             tmp1 = mf[r][2 * c] * 65536.0f;
//             tmp2 = mf[r][2 * c + 1] * 65536.0f;
//             *m1++ = (tmp1 & 0xffff0000) | ((tmp2 >> 0x10) & 0xffff);
//             *m2++ = ((tmp1 << 0x10) & 0xffff0000) | (tmp2 & 0xffff);
//         }
//     }
// }

// void guMtxL2F(float mf[4][4], Mtx *m) {
//     int r, c;
//     u32 tmp1;
//     u32 tmp2;
//     u32 *m1;
//     u32 *m2;
//     s32 stmp1, stmp2;
//     m1 = (u32 *) &m->m[0][0];
//     m2 = (u32 *) &m->m[2][0];
//     for (r = 0; r < 4; r++) {
//         for (c = 0; c < 2; c++) {
//             tmp1 = (*m1 & 0xffff0000) | ((*m2 >> 0x10) & 0xffff);
//             tmp2 = ((*m1++ << 0x10) & 0xffff0000) | (*m2++ & 0xffff);
//             stmp1 = *(s32 *) &tmp1;
//             stmp2 = *(s32 *) &tmp2;
//             mf[r][c * 2 + 0] = stmp1 / 65536.0f;
//             mf[r][c * 2 + 1] = stmp2 / 65536.0f;
//         }
//     }
// }


void check_death_barrier(struct MarioState *m) {
    if (m->pos[1] < m->floorHeight + 2048.0f) {
        if (level_trigger_warp(m, WARP_OP_WARP_FLOOR) == 20 && !(m->flags & MARIO_FALL_SOUND_PLAYED)) {
            play_sound(SOUND_MARIO_WAAAOOOW, m->marioObj->header.gfx.cameraToObject);
        }
    }
}

void update_mario_sound_and_camera(struct MarioState *m);
s32 drop_and_set_mario_action(struct MarioState *m, u32 action, u32 actionArg);

void check_lava_boost(struct MarioState *m) {
    if (!(m->action & ACT_FLAG_RIDING_SHELL) && m->pos[1] < m->floorHeight + 10.0f) {
        if (!(m->flags & MARIO_METAL_CAP)) {
            m->hurtCounter += SCALE_NF((m->flags & MARIO_CAP_ON_HEAD) ? 12 : 18);
        }

        update_mario_sound_and_camera(m);
        drop_and_set_mario_action(m, ACT_LAVA_BOOST, 0);
    }
}

void pss_begin_slide(UNUSED struct MarioState *m) {}
void pss_end_slide(struct MarioState *m) {}

void mario_handle_special_floors(struct MarioState *m) {
    if ((m->action & ACT_GROUP_MASK) == ACT_GROUP_CUTSCENE) {
        return;
    }

    if (m->floor != NULL) {
        s32 floorType = m->floor->type;

        switch (floorType) {
            case SURFACE_DEATH_PLANE:
            case SURFACE_VERTICAL_WIND:
                check_death_barrier(m);
                break;

            case SURFACE_WARP:
                level_trigger_warp(m, WARP_OP_WARP_FLOOR);
                break;

            case SURFACE_TIMER_START:
                pss_begin_slide(m);
                break;

            case SURFACE_TIMER_END:
                pss_end_slide(m);
                break;
        }

        if (!(m->action & ACT_FLAG_AIR) && !(m->action & ACT_FLAG_SWIMMING)) {
            switch (floorType) {
                case SURFACE_BURNING:
                    check_lava_boost(m);
                    break;
            }
        }
    }
}

/**
 * Find the height of the highest floor below a point.
 */
f32 find_floor_height(f32 x, f32 y, f32 z) {
    struct Surface *floor;

    f32 floorHeight = find_floor(x, y, z, &floor);

    return floorHeight;
}

s32 f32_find_wall_collision(f32 *xPtr, f32 *yPtr, f32 *zPtr, f32 offsetY, f32 radius) {
    struct WallCollisionData collision;
    s32 numCollisions = 0;

    collision.offsetY = offsetY;
    collision.radius = radius;

    collision.x = *xPtr;
    collision.y = *yPtr;
    collision.z = *zPtr;

    collision.numWalls = 0;

    numCollisions = find_wall_collisions(&collision);

    *xPtr = collision.x;
    *yPtr = collision.y;
    *zPtr = collision.z;

    return numCollisions;
}

s32 find_poison_gas_level(s32 x, s32 z) {
    s32 gasLevel = FLOOR_LOWER_LIMIT;
    return gasLevel;
}

u32 gGlobalTimer = 0;
struct Controller gControllers[1];

void copy_mario_state_to_object(void) {
    s32 i = 0;
    // L is real
    if (gCurrentObject != gMarioObject) {
        i += 1;
    }

    gCurrentObject->oVelX = gMarioStates[i].vel[0];
    gCurrentObject->oVelY = gMarioStates[i].vel[1];
    gCurrentObject->oVelZ = gMarioStates[i].vel[2];

    gCurrentObject->oPosX = gMarioStates[i].pos[0];
    gCurrentObject->oPosY = gMarioStates[i].pos[1];
    gCurrentObject->oPosZ = gMarioStates[i].pos[2];

    gCurrentObject->oMoveAnglePitch = gCurrentObject->header.gfx.angle[0];
    gCurrentObject->oMoveAngleYaw = gCurrentObject->header.gfx.angle[1];
    gCurrentObject->oMoveAngleRoll = gCurrentObject->header.gfx.angle[2];

    gCurrentObject->oFaceAnglePitch = gCurrentObject->header.gfx.angle[0];
    gCurrentObject->oFaceAngleYaw = gCurrentObject->header.gfx.angle[1];
    gCurrentObject->oFaceAngleRoll = gCurrentObject->header.gfx.angle[2];

    gCurrentObject->oAngleVelPitch = gMarioStates[i].angleVel[0];
    gCurrentObject->oAngleVelYaw = gMarioStates[i].angleVel[1];
    gCurrentObject->oAngleVelRoll = gMarioStates[i].angleVel[2];
}

extern const struct Animation *gMarioAnims[];
extern u32 gMarioNumAnims;


// #include <time.h>

s32 load_patchable_table(struct DmaHandlerList *list, s32 index) {
    s32 ret = FALSE;
    const struct Animation *addr;

    if (index < gMarioNumAnims) {
        addr = gMarioAnims[index];
        if (list->currentAddr != (u8*)addr) {
            memcpy(list->bufTarget, addr, sizeof(struct Animation));
            list->currentAddr = (u8*)addr;
        }
    }
    return ret;
}

s32 execute_mario_action(struct Object *);

struct Animation gTargetAnim;

void delay(int milliseconds)
{
    // long pause;
    // clock_t now,then;

    // pause = milliseconds*(CLOCKS_PER_SEC/1000);
    // now = then = clock();
    // while( (now-then) < pause )
    //     now = clock();
}

struct Camera areaCamera;

#ifdef _WIN32
#define EXPORT __declspec( dllexport )
#define ADDCALL __cdecl
#else
#define EXPORT // __attribute__ ((visibility ("default")))
#define ADDCALL
#endif

FindFloorHandler_t *gFloorHandler = NULL;
FindCeilHandler_t *gCeilHandler = NULL;
FindWallHandler_t *gWallHandler = NULL;
FindWaterLevelHandler_t *gWaterLevelHandler = NULL;

struct Surface surfacePool[0x100];
s32 surfacesUsed = 0;

f32 find_floor(f32 xPos, f32 yPos, f32 zPos, struct Surface **pfloor) { 
    f32 height = FLOOR_LOWER_LIMIT;
    *pfloor = NULL;

    if (gFloorHandler) {
        struct Surface *curSurface = &surfacePool[surfacesUsed];
        s32 foundFloor;
        height = gFloorHandler(xPos, yPos, zPos, curSurface, &foundFloor);
        if (foundFloor) {
            *pfloor = curSurface;
            surfacesUsed++;
        }
    }

    return height;
}

f32 find_ceil(f32 xPos, f32 yPos, f32 zPos, struct Surface **pceil) {
    f32 height = CELL_HEIGHT_LIMIT;
    *pceil = NULL;

    if (gCeilHandler) {
        struct Surface *curSurface = &surfacePool[surfacesUsed];
        s32 foundCeil;
        height = gCeilHandler(xPos, yPos, zPos, curSurface, &foundCeil);
        if (foundCeil) {
            *pceil = curSurface;
            surfacesUsed++;
        }
    }

    return height;
}

s32 find_wall_collisions(struct WallCollisionData *colData) {
    s32 numWalls = 0;

    if (gWallHandler) {
        struct Surface *curSurface = &surfacePool[surfacesUsed];
        Vec3f posOut;
        numWalls = gWallHandler(colData->x, colData->y, colData->z, colData->offsetY, colData->radius, curSurface, posOut);
        if (numWalls > 0) {
            for (int i = 0; i < numWalls; i++) {
                colData->walls[i] = &curSurface[i];
            }
            colData->x = posOut[0];
            colData->y = posOut[1];
            colData->z = posOut[2];
        }
        surfacesUsed += (numWalls > 4 ? 4 : numWalls);
    }

    colData->numWalls = numWalls;
    return numWalls;
}

s32 find_water_level(s32 x, s32 z) {
    s32 waterHeight = FLOOR_LOWER_LIMIT;
    if (gWaterLevelHandler) {
        waterHeight = gWaterLevelHandler(x, z);
    }
    return waterHeight;
}

struct Object *mario_get_collided_object(struct MarioState *m, u32 interactType) {
    return NULL;
}



#include "math_util.h"
#undef sins
#undef coss

s16 sins(u16);
s16 coss(u16);

void check_kick_or_punch_wall(struct MarioState *m) {
    if (m->flags & (MARIO_PUNCHING | MARIO_KICKING | MARIO_TRIPPING)) {
        struct WallCollisionData detector;
        detector.x = m->pos[0] + 50.0f * sins(m->faceAngle[1]);
        detector.z = m->pos[2] + 50.0f * coss(m->faceAngle[1]);
        detector.y = m->pos[1];
        detector.offsetY = 80.0f;
        detector.radius = 5.0f;

        if (find_wall_collisions(&detector) > 0) {
            if (m->action != ACT_MOVE_PUNCHING || m->forwardVel >= 0.0f) {
                if (m->action == ACT_PUNCHING) {
                    m->action = ACT_MOVE_PUNCHING;
                }

                mario_set_forward_vel(m, -48.0f);
                play_sound(SOUND_ACTION_HIT_2, m->marioObj->header.gfx.cameraToObject);
                m->particleFlags |= PARTICLE_TRIANGLE;
            } else if (m->action & ACT_FLAG_AIR) {
                mario_set_forward_vel(m, -16.0f);
                play_sound(SOUND_ACTION_HIT_2, m->marioObj->header.gfx.cameraToObject);
                m->particleFlags |= PARTICLE_TRIANGLE;
            }
        }
    }
}

void mario_process_interactions(struct MarioState *m) {
    check_kick_or_punch_wall(m);
    m->flags &= ~MARIO_PUNCHING & ~MARIO_KICKING & ~MARIO_TRIPPING;
}

f32 find_room_floor(f32 x, f32 y, f32 z, struct Surface **pfloor) {
    return find_floor(x, y, z, pfloor);
}

/**
 * Collides with walls and returns the most recent wall.
 */
void resolve_and_return_wall_collisions(Vec3f pos, f32 offset, f32 radius, struct WallCollisionData *collisionData) {
    collisionData->x = pos[0];
    collisionData->y = pos[1];
    collisionData->z = pos[2];
    collisionData->radius = radius;
    collisionData->offsetY = offset;

    find_wall_collisions(collisionData);

    pos[0] = collisionData->x;
    pos[1] = collisionData->y;
    pos[2] = collisionData->z;
}

EXPORT void ADDCALL init_libmario(FindFloorHandler_t *floorHandler, FindCeilHandler_t *ceilHandler, FindWallHandler_t *wallHandler, FindWaterLevelHandler_t *waterHandler) {
    gFloorHandler = floorHandler;
    gCeilHandler = ceilHandler;
    gWallHandler = wallHandler;
    gWaterLevelHandler = waterHandler;
    // memset(gMarioState, 0, sizeof(struct MarioState));
    bzero(gMarioState, sizeof(struct MarioState));
    gMarioState->pos[0] = 0.0f;
    gMarioState->pos[1] = 100.0f;
    gMarioState->pos[2] = 0.0f;
    gMarioState->action = ACT_FREEFALL;
    gMarioState->marioObj = gMarioObject;
    gMarioState->marioBodyState = &gBodyStates[0];
    gMarioState->controller = &gControllers[0];
    gMarioState->area = &gAreaData[0];
    gMarioState->area->index = 1;
    gMarioState->animList = &gMarioAnimsBuf;
    gMarioState->animList->bufTarget = &gTargetAnim;
    gMarioState->statusForCamera = &gPlayerCameraState[0];
    gMarioState->health = 0x880;
    gMarioState->marioObj->header.gfx.animInfo.curAnim = gMarioState->animList->bufTarget;
    gMarioState->marioObj->header.gfx.animInfo.curAnim->loopEnd = 100;
    gMarioState->marioObj->header.gfx.animInfo.animFrame = 0;
    gMarioState->marioObj->header.gfx.animInfo.animFrameAccelAssist = 0;
    gMarioState->marioObj->header.gfx.animInfo.animAccel = 0x10000;
    gMarioState->marioObj->header.gfx.animInfo.animTimer = 0;
    gMarioObject->header.gfx.node.flags |= GRAPH_RENDER_HAS_ANIMATION;
    gMarioState->area->camera = &areaCamera;
    areaCamera.yaw = 0;
    init_graphics_pool();
    recomp_printf("init_libmario: init_graphics_pool\n");
    init_graph_node_system();
    recomp_printf("init_libmario: init_graph_node_system\n");
    gMarioObject->header.gfx.node.parent = &gMasterList.node;
    geo_make_first_child(&gMarioObject->header.gfx.node);
    recomp_printf("init_libmario: geo_make_first_child\n");
    geo_obj_init_spawninfo(&gMarioObject->header.gfx, gMarioSpawnInfo);
    gMarioObject->header.gfx.node.flags |= GRAPH_RENDER_ACTIVE | GRAPH_RENDER_Z_BUFFER;
    gMarioObject->header.gfx.node.type = GRAPH_NODE_TYPE_OBJECT;
    recomp_printf("init_libmario: gMarioObject->header.gfx.node %p\n", gMarioObject->header.gfx.node);
    recomp_printf("init_libmario: geo_obj_init_spawninfo\n");
}


void setMarioRelativeCamYaw(s16 yaw) {
    gMarioState->area->camera->yaw = yaw;
}

Gfx *geo_mario_hand_foot_scaler(s32 callContext, struct GraphNode *node, UNUSED Mat4 *c);
void adjust_analog_stick(struct Controller *controller) {
    // Reset the controller's x and y floats.
    controller->stickX = 0;
    controller->stickY = 0;

    // Modulate the rawStickX and rawStickY to be the new f32 values by adding/subtracting 6.
    if (controller->rawStickX <= -8) {
        controller->stickX = controller->rawStickX + 6;
    }

    if (controller->rawStickX >= 8) {
        controller->stickX = controller->rawStickX - 6;
    }

    if (controller->rawStickY <= -8) {
        controller->stickY = controller->rawStickY + 6;
    }

    if (controller->rawStickY >= 8) {
        controller->stickY = controller->rawStickY - 6;
    }

    // Calculate f32 magnitude from the center by vector length.
    controller->stickMag =
        sqrtf(controller->stickX * controller->stickX + controller->stickY * controller->stickY);

    // Magnitude cannot exceed 64.0f: if it does, modify the values
    // appropriately to flatten the values down to the allowed maximum value.
    if (controller->stickMag > 64) {
        controller->stickX *= 64 / controller->stickMag;
        controller->stickY *= 64 / controller->stickMag;
        controller->stickMag = 64;
    }
}

EXPORT void ADDCALL step_libmario(OSContPad *controllerData, s32 updateAnims) {
    struct GraphNodeGenerated handFootScalerNode;
    struct GraphNodeScale scaleNode;
    handFootScalerNode.fnNode.node.next = (struct GraphNode*)&scaleNode;

    gControllers[0].rawStickX = controllerData->stick_x;
    gControllers[0].rawStickY = controllerData->stick_y;
    gControllers[0].buttonPressed = controllerData->button
                                & (controllerData->button ^ gControllers[0].buttonDown);
    // 0.5x A presses are a good meme
    gControllers[0].buttonDown = controllerData->button;
    adjust_analog_stick(&gControllers[0]);

    surfacesUsed = 0;

    execute_mario_action(gCurrentObject);
    copy_mario_state_to_object();

    if (updateAnims) {
        handFootScalerNode.parameter = 1;
        geo_mario_hand_foot_scaler(GEO_CONTEXT_RENDER, (struct GraphNode*)&handFootScalerNode, NULL);
        handFootScalerNode.parameter = 0;
        geo_mario_hand_foot_scaler(GEO_CONTEXT_RENDER, (struct GraphNode*)&handFootScalerNode, NULL);
        handFootScalerNode.parameter = 2;
        geo_mario_hand_foot_scaler(GEO_CONTEXT_RENDER, (struct GraphNode*)&handFootScalerNode, NULL);
    
        s32 hasAnimation = (gMarioObject->header.gfx.node.flags & GRAPH_RENDER_HAS_ANIMATION) != 0;
        if (gMarioObject->header.gfx.animInfo.curAnim != NULL) {
            geo_set_animation_globals(&gMarioObject->header.gfx.animInfo, hasAnimation);
        }
    }

    gGlobalTimer++;
    gAreaUpdateCounter++;
}
extern struct AllocOnlyPool *gDisplayListHeap;
Gfx *render_mario(Gfx **opa, Gfx **xlu) {
    recomp_printf("render_mario: opa %p xlu %p\n", opa, xlu);
    select_gfx_pool();
    gDisplayListHeadOpa = *opa;
    gDisplayListHeadXlu = *xlu;

    recomp_printf("render_mario: gDisplayListHeadOpa %p gDisplayListHeadXlu %p\n", gDisplayListHeadOpa, gDisplayListHeadXlu);
    gCurGraphNodeMasterList = NULL;
    gCurGraphNodeRoot = &gCurGraphNodeRootSrc;
    gMasterList.node.flags |= GRAPH_RENDER_ACTIVE;
    gCurGraphNodeRoot->areaIndex = gMarioObject->header.gfx.areaIndex = gMarioState->area->index;
    gMatStackIndex = 0;
    recomp_printf("render_mario: gMatStackIndex %d\n", gMatStackIndex);

    // Mtx *initialMatrix;
    // initialMatrix = alloc_display_list(sizeof(*initialMatrix));
    // recomp_printf("render_mario: initialMatrix %p\n", initialMatrix);
    // mtxf_identity(gMatStack[gMatStackIndex]);
    // recomp_printf("render_mario: gMatStack[%d] %p\n", gMatStackIndex, gMatStack[gMatStackIndex]);
    // mtxf_to_mtx(initialMatrix, gMatStack[gMatStackIndex]);
    // gMatStackFixed[gMatStackIndex] = initialMatrix;
    // recomp_printf("render_mario: gMatStackFixed[%d] %p\n", gMatStackIndex, gMatStackFixed[gMatStackIndex]);
    gDisplayListHeap = alloc_only_pool_init(main_pool_available() - sizeof(struct AllocOnlyPool), MEMORY_POOL_LEFT);
    geo_process_master_list(&gMasterList);
    main_pool_free(gDisplayListHeap);
    recomp_printf("render_mario: geo_process_master_list done\n");
    *opa = gDisplayListHeadOpa;
    *xlu = gDisplayListHeadXlu;

    return gDisplayListHead;
}


void getMarioPosition(f32 pos[3]) {
    pos[0] = gMarioObject->header.gfx.pos[0];
    pos[1] = gMarioObject->header.gfx.pos[1];
    pos[2] = gMarioObject->header.gfx.pos[2];
}

u32 getMarioAction(void) {
    return gMarioState->action;
}

void setMarioPosition(f32 pos[3]) {
    gMarioState->pos[0] = pos[0];
    gMarioState->pos[1] = pos[1];
    gMarioState->pos[2] = pos[2];
}

void getMarioVelocity(f32 vel[3]) {
    vel[0] = gMarioState->vel[0];
    vel[1] = gMarioState->vel[1];
    vel[2] = gMarioState->vel[2];
}

void setMarioVelocity(f32 pos[3]) {
    gMarioState->vel[0] = pos[0];
    gMarioState->vel[1] = pos[1];
    gMarioState->vel[2] = pos[2];
}

void getMarioRotation(s16 rot[3]) {
    rot[0] = gMarioObject->header.gfx.angle[0];
    rot[1] = gMarioObject->header.gfx.angle[1];
    rot[2] = gMarioObject->header.gfx.angle[2];
}

// EXPORT void ADDCALL getMarioRotation(Vec3f rot) {
//     rot[0] = gMarioObject->header.gfx.angle[0] * (180.0f / 32768.0f);
//     rot[1] = gMarioObject->header.gfx.angle[1] * (180.0f / 32768.0f);
//     rot[2] = gMarioObject->header.gfx.angle[2] * (180.0f / 32768.0f);
// }

void setMarioRotation(s16 rot[3]) {
    gMarioState->faceAngle[0] = rot[0];
    gMarioState->faceAngle[1] = rot[1];
    gMarioState->faceAngle[2] = rot[2];
}

// EXPORT void ADDCALL setMarioRotation(Vec3f rot) {
//     gMarioState->faceAngle[0] = (s16)(rot[0] * (32768.0f / 180.0f));
//     gMarioState->faceAngle[1] = (s16)(rot[1] * (32768.0f / 180.0f));
//     gMarioState->faceAngle[2] = (s16)(rot[2] * (32768.0f / 180.0f));
// }

EXPORT void ADDCALL getMarioScale(Vec3f scale) {
    scale[0] = gMarioObject->header.gfx.scale[0];
    scale[1] = gMarioObject->header.gfx.scale[1];
    scale[2] = gMarioObject->header.gfx.scale[2];
}

EXPORT void ADDCALL getMarioTorsoRotation(Vec3f out) {
    struct MarioBodyState *bodyState = &gBodyStates[0];
    s32 action = gMarioState->action;
    if (action != ACT_BUTT_SLIDE &&
        action != ACT_HOLD_BUTT_SLIDE &&
        action != ACT_WALKING &&
        action != ACT_RIDING_SHELL_GROUND) {
        vec3s_copy(bodyState->torsoAngle, gVec3sZero);
    }
    out[0] = (180.0f / 32768.0f) * bodyState->torsoAngle[1];
    out[1] = (180.0f / 32768.0f) * bodyState->torsoAngle[2];
    out[2] = (180.0f / 32768.0f) * bodyState->torsoAngle[0];
}

EXPORT struct MarioState *ADDCALL getMarioState() {
    return gMarioState;
}

EXPORT s32 ADDCALL getMarioAnimIndex() {
    return gMarioObject->header.gfx.animInfo.animID;
}

EXPORT s32 ADDCALL getMarioAnimFrame() {
    return gMarioObject->header.gfx.animInfo.animFrame;
}

EXPORT void ADDCALL setMarioHealth(s32 health) {
    gMarioState->health = health;
}

EXPORT void ADDCALL setCameraYaw(f32 angle) {
    gMarioState->area->camera->yaw = angle * (32768.0f / 180.0f);
}

struct AnimData {
    Vec3f rootTranslation;
    Vec3f boneRotations[20];
};

EXPORT void ADDCALL getMarioAnimData(struct AnimData *out) {
    if (gMarioObject->header.gfx.animInfo.curAnim != NULL) {
        u16 *animIndices = (u16*)gMarioObject->header.gfx.animInfo.curAnim->index;
        s16 *animValues = (s16*)gMarioObject->header.gfx.animInfo.curAnim->values;
        s32 curFrame = gMarioObject->header.gfx.animInfo.animFrame;
        f32 animTranslationScale = 1;
        if (gMarioObject->header.gfx.animInfo.curAnim->animYTransDivisor != 0) {
            animTranslationScale = gMarioObject->header.gfx.animInfo.animYTrans / (f32)gMarioObject->header.gfx.animInfo.curAnim->animYTransDivisor;
        }

        int i;
        if (animIndices != NULL && animValues != NULL) {
            out->rootTranslation[0] = animTranslationScale * animValues[retrieve_animation_index(curFrame, &animIndices)];
            out->rootTranslation[1] = animTranslationScale * animValues[retrieve_animation_index(curFrame, &animIndices)];
            out->rootTranslation[2] = animTranslationScale * animValues[retrieve_animation_index(curFrame, &animIndices)];
            for (i = 0; i < 20; i++) {
                out->boneRotations[i][0] = (180.0f / 32768.0f) * animValues[retrieve_animation_index(curFrame, &animIndices)];
                out->boneRotations[i][1] = (180.0f / 32768.0f) * animValues[retrieve_animation_index(curFrame, &animIndices)];
                out->boneRotations[i][2] = (180.0f / 32768.0f) * animValues[retrieve_animation_index(curFrame, &animIndices)];
            }
        }
    }
}
