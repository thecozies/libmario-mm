#pragma once

#ifdef I_AM_OOT
    #include "ultra64.h"

    struct SM64Normal {
        /*0x00*/ f32 x;
        /*0x04*/ f32 y;
        /*0x08*/ f32 z;
    };

    struct SurfaceSM64 {
        /*0x00*/ s16 type;
        /*0x02*/ s16 force;
        /*0x04*/ s8 flags;
        /*0x05*/ s8 room;
        /*0x06*/ s16 lowerY;
        /*0x08*/ s16 upperY;
        /*0x0A*/ s16 vertex1[3];
        /*0x10*/ s16 vertex2[3];
        /*0x16*/ s16 vertex3[3];
        /*0x1C*/ struct SM64Normal normal;
        /*0x28*/ f32 originOffset;
        /*0x2C*/ void *object; // TODO: oot translation layer for objects?
    };

    struct ControllerSM64 {
        /*0x00*/ s16 rawStickX;       //
        /*0x02*/ s16 rawStickY;       //
        /*0x04*/ f32 stickX;          // [-64, 64] positive is right
        /*0x08*/ f32 stickY;          // [-64, 64] positive is up
        /*0x0C*/ f32 stickMag;        // distance from center [0, 64]
        /*0x10*/ u16 buttonDown;
        /*0x12*/ u16 buttonPressed;
        /*0x14*/ OSContStatus *statusData;
        /*0x18*/ OSContPad *controllerData;
    };

    struct MarioBodyState {
        /*0x00*/ u32 action;
        /*0x04*/ s8 capState; /// see MarioCapGSCId
        /*0x05*/ s8 eyeState;
        /*0x06*/ s8 handState;
        /*0x07*/ s8 wingFlutter; /// whether Mario's wing cap wings are fluttering
        /*0x08*/ s16 modelState;
        /*0x0A*/ s8 grabPos;
        /*0x0B*/ u8 punchState; /// 2 bits for type of punch, 6 bits for punch animation timer
        /*0x0C*/ s16 torsoAngle[3];
        /*0x12*/ s16 headAngle[3];
        /*0x18*/ f32 heldObjLastPosition[3]; /// also known as HOLP
        // u8 filler[4];
    };

    struct MarioState {
        /*0x00*/ u16 playerID;
        /*0x02*/ u16 input;
        /*0x04*/ u32 flags;
        /*0x08*/ u32 particleFlags;
        /*0x0C*/ u32 action;
        /*0x10*/ u32 prevAction;
        /*0x14*/ u32 terrainSoundAddend;
        /*0x18*/ u16 actionState;
        /*0x1A*/ u16 actionTimer;
        /*0x1C*/ u32 actionArg;
        /*0x20*/ f32 intendedMag;
        /*0x24*/ s16 intendedYaw;
        /*0x26*/ s16 invincTimer;
        /*0x28*/ u8 framesSinceA;
        /*0x29*/ u8 framesSinceB;
        /*0x2A*/ u8 wallKickTimer;
        /*0x2B*/ u8 doubleJumpTimer;
        /*0x2C*/ s16 faceAngle[3];
        /*0x32*/ s16 angleVel[3];
        /*0x38*/ s16 slideYaw;
        /*0x3A*/ s16 twirlYaw;
        /*0x3C*/ f32 pos[3];
        /*0x48*/ f32 vel[3];
        /*0x54*/ f32 forwardVel;
        /*0x58*/ f32 slideVelX;
        /*0x5C*/ f32 slideVelZ;
        /*0x60*/ struct SurfaceSM64 *wall;
        /*0x64*/ struct SurfaceSM64 *ceil;
        /*0x68*/ struct SurfaceSM64 *floor;
        /*0x6C*/ f32 ceilHeight;
        /*0x70*/ f32 floorHeight;
        /*0x74*/ s16 floorYaw;
        /*0x76*/ s16 waterLevel;
        /*0x78*/ void *interactObj;
        /*0x7C*/ void *heldObj;
        /*0x80*/ void *usedObj;
        /*0x84*/ void *riddenObj;
        /*0x88*/ void *marioObj;
        /*0x8C*/ void *spawnInfo;
        /*0x90*/ void *area;
        /*0x94*/ void *statusForCamera;
        /*0x98*/ struct MarioBodyState *marioBodyState;
        /*0x9C*/ struct ControllerSM64 *controller;
        /*0xA0*/ void *animList;
        /*0xA4*/ u32 collidedObjInteractTypes;
        /*0xA8*/ s16 numCoins;
        /*0xAA*/ s16 numStars;
        /*0xAC*/ s8 numKeys; // Unused key mechanic
        /*0xAD*/ s8 numLives;
        /*0xAE*/ s16 health;
        /*0xB0*/ s16 animYTrans;
        /*0xB2*/ u8 hurtCounter;
        /*0xB3*/ u8 healCounter;
        /*0xB4*/ u8 squishTimer;
        /*0xB5*/ u8 fadeWarpOpacity;
        /*0xB6*/ u16 capTimer;
        /*0xB8*/ s16 prevNumStarsForDialog;
        /*0xBC*/ f32 peakHeight;
        /*0xC0*/ f32 quicksandDepth;
        /*0xC4*/ f32 windGravity;

        u8  isDead : 1;
        f32 lastSafePos[3];
        f32 prevPos[3];
        f32 lateralSpeed;
        f32 moveSpeed;
        s16 movePitch;
        s16 moveYaw;
        s16 ceilYaw;
        s16 wallYaw;
    };
    
    struct MarioAnimData {
        f32 rootTranslation[3];
        s16 boneRotations[20][3];
    };

    void getMarioAnimData(struct MarioAnimData *out);
#else
    #include "types.h"
    #define SurfaceSM64 Surface
    #define SM64Normal Normal

    struct MarioAnimData {
        f32 rootTranslation[3];
        s16 boneRotations[20][3];
    };
#endif

typedef f32 FindFloorHandler_t(f32 x, f32 y, f32 z, struct SurfaceSM64 *floor, s32 *foundFloor);
typedef f32 FindCeilHandler_t(f32 x, f32 y, f32 z, struct SurfaceSM64 *ceil, s32 *foundCeil);
typedef s32 FindWallHandler_t(f32 x, f32 y, f32 z, f32 offsetY, f32 radius, struct SurfaceSM64 walls[4], f32 posOut[3]);
typedef f32 FindWaterLevelHandler_t(f32 x, f32 z);

extern FindFloorHandler_t *gFloorHandler;
extern FindCeilHandler_t *gCeilHandler;
extern FindWallHandler_t *gWallHandler;
extern FindWaterLevelHandler_t *gWaterLevelHandler;

enum DamageProperties {
    MARIO_DAMAGE_PROPERTIES_NONE = 0,
    MARIO_DAMAGE_PROPERTIES_FIRE = 1 << 0,
    MARIO_DAMAGE_PROPERTIES_FREEZE = 1 << 1,
    MARIO_DAMAGE_PROPERTIES_SHOCK = 1 << 2,
    MARIO_DAMAGE_PROPERTIES_LAVA = 1 << 3,
    MARIO_DAMAGE_PROPERTIES_KNOCKBACK = 1 << 4,
};

typedef enum {
    INTERACT_OBJ_NONE = 0,
    INTERACT_OBJ_HELD,
    INTERACT_OBJ_USED,
    INTERACT_OBJ_RIDDEN,
} InteractObjectType;

void damage_mario(s32 damage, u32 damageProperties, f32 *sourcePos);

void init_libmario(FindFloorHandler_t *floorHandler, FindCeilHandler_t *ceilHandler, FindWallHandler_t *wallHandler, FindWaterLevelHandler_t *waterHandler);
// void step_libmario(s32 buttons, f32 stickX, f32 stickY);
void step_libmario(OSContPad *controllerData, s32 updateAnims);
Gfx *render_mario(Gfx **opa, Gfx **xlu, f32 *scale, u32 transform_id);
extern struct MarioState *gMarioState;
u32 getMarioAction(void);
void setMarioRelativeCamYaw(s16 yaw);
void getMarioVelocity(f32 vel[3]);
void setMarioVelocity(f32 pos[3]);



void getMarioPosition(f32 pos[3]);
void setMarioPosition(f32 pos[3]);
void displaceMarioPosition(f32 pos[3]);
void getMarioVelocity(f32 vel[3]);
void setMarioVelocity(f32 pos[3]);
void getMarioRotation(s16 rot[3]);
void setMarioRotation(s16 rot[3]);
void reset_mario_state(u32 action);

void set_mario_interact_object(InteractObjectType interactObjectType, f32 pos[3], s16 rot[3], f32 radius, f32 height);

/* Returns FALSE (0) if mario can't open a door right now */
s32 mario_interact_door(f32 pos[3], s16 rot[3], s32 shouldPushDoor);
