/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright 2022-2026 rec2 developers
 */

#include "22-replay.h"

#include <stdlib.h>
#include <stdio.h>

#include "globvars.h"
#include "30-opponent.h"
#include "18-graphics2.h"
#include "23-piping.h"
#include "41-utility.h"
#include "59-camera.h"
#include "08-loading1.h"
#include "10-loading2.h"
#include "13-crush1.h"
#include "32-spark.h"
#include "56-aiworld.h"
#include "57-drone.h"
#include "61-pedestrn.h"

// GLOBAL: CARMA2_HW 0x006768fc
// GLOBAL: CARMA2_SW 0x0067106c
int gPaused;

// GLOBAL: CARMA2_HW 0x0058f2d8
// GLOBAL: CARMA2_SW 0x0058a2d8
const int gPow2_array[32] = {
    0x1, 0x2, 0x4, 0x8,
    0x10, 0x20, 0x40, 0x80,
    0x100, 0x200, 0x400, 0x800,
    0x1000, 0x2000, 0x4000, 0x8000,
    0x10000, 0x20000, 0x40000, 0x80000,
    0x100000, 0x200000, 0x400000, 0x800000,
    0x1000000, 0x2000000, 0x4000000, 0x8000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
};

// GLOBAL: CARMA2_HW 0x0058f600
// GLOBAL: CARMA2_SW 0x0058a600
tU8 gCamera_type_allowed_replay[9] = {
    1, 1, 1, 1, 1, 0, 0, 1, 1,
};

// GLOBAL: CARMA2_HW 0x0058f610
// GLOBAL: CARMA2_SW 0x0058a610
tU8 gCamera_type_allowed_gameplay[9] = {
    1, 0, 0, 0, 1, 0, 0, 1, 1,
};

// GLOBAL: CARMA2_HW 0x0058f620
// GLOBAL: CARMA2_SW 0x0058a620
int gAR_camera_type = 2;

// GLOBAL: CARMA2_HW 0x0058f6a4
// GLOBAL: CARMA2_SW 0x0058a6a4
int gCar_car_collisions = 1;

// GLOBAL: CARMA2_HW 0x0058f6b0
// GLOBAL: CARMA2_SW 0x0058a6b0
const float gCar_simplification_factor[2][5] = {
    { 50.0f, 3.0f, 1.5f, 0.75f, 0.0f },
    { 50.0f, 5.0f, 2.5f, 1.5f, 0.0f }
};

// GLOBAL: CARMA2_HW 0x0058f7f0
// GLOBAL: CARMA2_SW 0x0058a7f0
tPolyhedron_edge_indexes gDefault_box_collision_wireframe_edges[12] = {
    { 0, 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 },
    { 0, 2 }, { 1, 3 }, { 4, 6 }, { 5, 7 },
    { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
};

// GLOBAL: CARMA2_HW 0x006a2388
br_pixelmap* gReplay_pixelmaps[11];


// ActualActionReplayHeadups

// FUNCTION: CARMA2_HW 0x004e6260
// FUNCTION: CARMA2_SW 0x004e2280
void C2_HOOK_FASTCALL DoActionReplayPostSwap(void) {
    RemoveTransientBitmaps(1);
}

// DoZappyActionReplayHeadups

// DoActionReplayHeadups

// PreProcess

// PostProcess

// DoActionReplay

// MoveToEndOfReplay

// MoveToStartOfReplay

// PollActionReplayControls

// CheckReplayTurnOn

// InitializeActionReplayPix

// FreeActionReplayPix

// SynchronizeActionReplay

// TurnOnActionReplay

// TurnOffActionReplay

// FUNCTION: CARMA2_HW 0x004e72d0
// FUNCTION: CARMA2_SW 0x004e3110
void C2_HOOK_FASTCALL AfterActionReplay(void) {
    ForceRebuildActiveCarList();
}

// ToggleReplay

// DisplayMovieStatusPanel

// FUNCTION: CARMA2_HW 0x004022e0
// FUNCTION: CARMA2_SW 0x004022e0
void C2_HOOK_FASTCALL ApplyTime(tU32 pTime) {
    gLast_replay_frame_time = pTime;
}

// FUNCTION: CARMA2_HW 0x004022f0
// FUNCTION: CARMA2_SW 0x004022f0
void C2_HOOK_FASTCALL ARSetPause(int pPause) {
    gPaused = pPause;
}

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
// FUNCTION: CARMA2_HW 0x00408580
tPhysics_object* C2_HOOK_FASTCALL MakeCharacterPhysicsForBone(tPed_character_instance* pCharacter, int pBone_index, undefined4 pArg3, undefined4 pArg4) {
    extern int C2_HOOK_FASTCALL MakeCharacterPhysics(tPed_character_instance* pCharacter, tPhysics_object* pObject, br_actor* pActor, void* pFrame, br_vector3* pBonePos, int pKind, undefined4 pArg3, int pFlag, int pBoneIndex, br_vector3* pBoneVec, br_vector3* pBonePtr);
    tPed_form* form;
    int iVar1;
    int iVar2;

    form = pCharacter->personality->form;
    iVar1 = (int)form->boned_physicing + (tS8)pCharacter->field_0x6 * 8;
    pBone_index *= 4;
    iVar2 = *(int*)((int)form->actor_sets + 4 + (tS8)pCharacter->field_0x4 * 8);
    iVar2 = *(int*)(iVar2 + pBone_index);
    iVar2 = MakeCharacterPhysics(pCharacter, (tPhysics_object*)*(int*)(*(int*)(iVar1 + 4) + pBone_index), (br_actor*)iVar2, (void*)(*(int*)(iVar2 + 0x18) + 0x34), NULL, 3, pArg3, pArg4, 0, NULL, NULL);
    if (iVar2 != 0) {
        return (tPhysics_object*)*(int*)(*(int*)(iVar1 + 4) + pBone_index);
    }
    return NULL;
}
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif
// RESIDUAL 97.44% in work-v4 (reccmp: effective 100): one symmetric SIB encoding differs at 0x4085c5 ([eax+esi] vs [esi+eax]); operand swap in the source flips the return-path load too, so it is the allocator's choice

extern void C2_HOOK_CDECL BrMatrix34ToTransform(br_transform* xform, const br_matrix34* mat);
extern br_uint_8 C2_HOOK_CDECL ActorToActorMatrix34Local(br_matrix34* m, br_actor* a, br_actor* b);

// FUNCTION: CARMA2_HW 0x0040ca40
void C2_HOOK_CDECL RelinkActorKeepWorldTransform(br_actor* pParent, br_actor* pActor) {
    br_matrix34 mat;

    if (pActor->parent != pParent) {
        ActorToActorMatrix34Local(&mat, pActor, pParent);
        BrMatrix34ToTransform(&pActor->t, &mat);
        BrActorAdd(pParent, BrActorRemove(pActor));
    }
}

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
// FUNCTION: CARMA2_HW 0x0040c660
br_uint_8 C2_HOOK_CDECL ActorToActorMatrix34Local(br_matrix34* m, br_actor* a, br_actor* b) {
    br_uint_8 transform_type_combine[7][7] = {
        { 0, 0, 0, 0, 0, 0, 0 },
        { 0, 1, 1, 1, 1, 1, 1 },
        { 0, 1, 1, 1, 1, 2, 2 },
        { 0, 1, 1, 1, 1, 3, 3 },
        { 0, 1, 1, 1, 1, 4, 4 },
        { 0, 1, 2, 3, 4, 5, 5 },
        { 0, 1, 2, 3, 4, 5, 6 },
    };
    br_matrix34 mat_a;
    br_matrix34 mat_b;
    br_matrix34 mat_c;
    br_uint_8 a_type;
    br_uint_8 b_type;

    if (a == b) {
        BrMatrix34Identity(m);
        return BR_TRANSFORM_IDENTITY;
    }
    if (a->parent == b) {
        BrTransformToMatrix34(m, &a->t);
        return (br_uint_8)a->t.type;
    }
    if (b->parent == a) {
        BrTransformToMatrix34(&mat_b, &b->t);
        if (b->t.type != BR_TRANSFORM_MATRIX34) {
            BrMatrix34LPInverse(m, &mat_b);
        } else {
            BrMatrix34Inverse(m, &mat_b);
        }
        return (br_uint_8)b->t.type;
    }
    a_type = BR_TRANSFORM_IDENTITY;
    BrMatrix34Identity(&mat_a);
    b_type = BR_TRANSFORM_IDENTITY;
    BrMatrix34Identity(&mat_b);
    while (a != NULL && b != NULL && a != b) {
        if (a->depth > b->depth) {
            if (a->t.type != BR_TRANSFORM_IDENTITY) {
                BrMatrix34PostTransform(&mat_a, &a->t);
                a_type = transform_type_combine[a_type][a->t.type];
            }
            a = a->parent;
        } else if (b->depth > a->depth) {
            if (b->t.type != BR_TRANSFORM_IDENTITY) {
                BrMatrix34PostTransform(&mat_b, &b->t);
                b_type = transform_type_combine[b_type][b->t.type];
            }
            b = b->parent;
        } else {
            if (a->t.type != BR_TRANSFORM_IDENTITY) {
                BrMatrix34PostTransform(&mat_a, &a->t);
                a_type = transform_type_combine[a_type][a->t.type];
            }
            if (b->t.type != BR_TRANSFORM_IDENTITY) {
                BrMatrix34PostTransform(&mat_b, &b->t);
                b_type = transform_type_combine[b_type][b->t.type];
            }
            b = b->parent;
            a = a->parent;
        }
    }
    if (b_type == BR_TRANSFORM_IDENTITY) {
        BrMatrix34Copy(m, &mat_a);
        return a_type;
    }
    if (a_type == BR_TRANSFORM_IDENTITY) {
        if (b_type != BR_TRANSFORM_MATRIX34) {
            BrMatrix34LPInverse(m, &mat_b);
        } else {
            BrMatrix34Inverse(m, &mat_b);
        }
        return b_type;
    }
    if (b_type != BR_TRANSFORM_MATRIX34) {
        BrMatrix34LPInverse(&mat_c, &mat_b);
    } else {
        BrMatrix34Inverse(&mat_c, &mat_b);
    }
    BrMatrix34Mul(m, &mat_a, &mat_c);
    return transform_type_combine[a_type][b_type];
}
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif


// FUNCTION: CARMA2_HW 0x0043bd70
void C2_HOOK_FASTCALL WorkerCrush(int field_0x4, void* pField_0x8, void* pField_0xe) {
    br_model* pModel;
    br_vector3 offset;
    br_vector3 scaled_offset;
    tU16* pWeights;
    br_scalar scale;
    int i;
    int vertex_offset;

    pModel = (br_model*)field_0x4;
    ExpandVector3(&offset, (const tCompressed_vector3*)pField_0x8, -1.f, 1.f);
    i = 0;
    if (pModel->nvertices > 0) {
        pWeights = (tU16*)pField_0xe;
        vertex_offset = 0;
        do {
            if (*pWeights != 0) {
                scale = DRU16ToScalar(*pWeights, 0.f, 10.f);
                scaled_offset.v[0] = scale * offset.v[0];
                scaled_offset.v[1] = scale * offset.v[1];
                scaled_offset.v[2] = scale * offset.v[2];
                *(br_scalar*)((tU8*)pModel->vertices + vertex_offset) = scaled_offset.v[0] + *(br_scalar*)((tU8*)pModel->vertices + vertex_offset);
                *(br_scalar*)((tU8*)pModel->vertices + vertex_offset + 4) = scaled_offset.v[1] + *(br_scalar*)((tU8*)pModel->vertices + vertex_offset + 4);
                *(br_scalar*)((tU8*)pModel->vertices + vertex_offset + 8) = scaled_offset.v[2] + *(br_scalar*)((tU8*)pModel->vertices + vertex_offset + 8);
            }
            i++;
            pWeights++;
            vertex_offset += sizeof(br_vertex);
        } while (i < pModel->nvertices);
    }
    if ((pModel->flags & BR_MODF_CUSTOM) != 0 && pModel->user != NULL) {
        ((tUser_detail_level_model*)pModel->user)->field_0x0 = 1;
        return;
    }
    pModel->custom = JitModelUpdate;
    pModel->flags |= BR_MODF_CUSTOM;
}
// PARTIAL 86.84% (same in work-v4): x87 keeps one scaled component in memory in a different order ([esp+0x24] holds v2 in the original, v1 here); rest of the loop and both epilogues are identical.

// FUNCTION: CARMA2_HW 0x0043be50
void C2_HOOK_FASTCALL WorkerCrushUndo(int field_0x4, void* pField_0x8, void* pField_0xe) {
    br_model* pModel;
    br_vector3 offset;
    br_vector3 scaled_offset;
    tU16* pWeights;
    br_scalar scale;
    int i;
    int vertex_offset;

    pModel = (br_model*)field_0x4;
    ExpandVector3(&offset, (const tCompressed_vector3*)pField_0x8, -1.f, 1.f);
    i = 0;
    if (pModel->nvertices > 0) {
        pWeights = (tU16*)pField_0xe;
        vertex_offset = 0;
        do {
            if (*pWeights != 0) {
                scale = DRU16ToScalar(*pWeights, 0.f, 10.f);
                BrVector3Scale(&scaled_offset, &offset, -scale);
                BrVector3Accumulate((br_vector3*)((tU8*)pModel->vertices + vertex_offset), &scaled_offset);
            }
            i++;
            pWeights++;
            vertex_offset += sizeof(br_vertex);
        } while (i < pModel->nvertices);
    }
    if ((pModel->flags & BR_MODF_CUSTOM) != 0 && pModel->user != NULL) {
        ((tUser_detail_level_model*)pModel->user)->field_0x0 = 1;
        return;
    }
    pModel->custom = JitModelUpdate;
    pModel->flags |= BR_MODF_CUSTOM;
}
// PARTIAL 89.03%: identical logic; the x87 sequence stores the first accumulated component one instruction earlier than the original (extra `fstp [eax+edi]`, shifted store), and the skip branch is one byte longer.

// FUNCTION: CARMA2_HW 0x004c9070
void C2_HOOK_FASTCALL WorkerSmudge(tPipe_chunk** pChunk, int pScale) {
    tCar_spec* car;
    tU8* a;
    tU32 code;
    int i;
    tU8* esi;

    code = *(tU32*)*pChunk;
    if ((code & 0xffffff00) == 0) {
        car = &gProgram_state.current_car;
    } else {
        car = GetCarSpec(code >> 8, code & 0xff);
    }
    a = *(tU8**)((tU8*)car + 0xe10);
    i = 0;
    if (*(tU16*)((tU8*)*pChunk + 4) <= 0) {
        return;
    }
    esi = *(tU8**)(a + 0x18);
    do {
        tU32* colors = *(tU32**)(*(tU8**)(*(tU8**)(esi + 0x4c) + 0x18) + 0x14);
        tU16 idx = *(tU16*)((tU8*)*pChunk + i * 4 + 0xc);
        tS8 bright = (tS8)((tS8)*(tU8*)((tU8*)*pChunk + i * 4 + 0xe) * (tS8)pScale);
        tU32 alpha24 = ((tU32)(tU8)bright) << 24;
        tU32 c = colors[idx];

        colors[idx] = ((c & 0xff000000) + alpha24) & 0xff000000;
        if (*(tU8*)(esi + 0x20) & 0x80) {
            tU8* q = *(tU8**)(esi + 0x4c);
            tU8* w = *(tU8**)(q + 0x18);
            tU32* colors2 = (tU32*)(*(tU8**)(w + 0x14));
            tU16* idx2 = (tU16*)(*(tU8**)(w + 0x18));
            tU8 dl = (tU8)(colors2[idx] >> 24) + (tU8)bright;
            tU8* arr = *(tU8**)(esi + 8);
            tU16 ix = idx2[idx];

            arr[(tU32)ix * 40 + 0x14] = dl;
        }
        i++;
    } while (i < *(tU16*)((tU8*)*pChunk + 4));
}
// PARTIAL 77.78%: work-v4 measures 78.32% on the same body; register/param-home allocation differs (edi vs esi for pChunk), logic and call sequence identical.

// FUNCTION: CARMA2_HW 0x004ca680
void C2_HOOK_FASTCALL WorkerPedStatus(int field_0x0, tU8 field_0x4, tU8 field_0x5, tU8 field_0x6, tU8 field_0x7, tU8 field_0x9, tU8 field_0xb, br_vector3* p0xc, br_vector3* p0x18, br_vector3* p0x24) {
    extern int C2_HOOK_FASTCALL MakeCharacterRenderable2(tPed_character_instance* pCharacter, int pIndex);
    extern int C2_HOOK_FASTCALL CharacterNoLongerRenderable(tPed_character_instance* pCharacter);
    extern void C2_HOOK_FASTCALL RecalculateOrientationOfRoot(tPed_character_instance* pPed);
    tPedestrian* ped;

    ped = (tPedestrian*)field_0x0;
    if (field_0x5 != 0xff) {
        if (field_0x5 != 0) {
            *(tU8*)&ped->flags |= 1;
            MakeCharacterRenderable2(ped->character, (int)(tS8)field_0x9);
        } else {
            *(tU8*)&ped->flags &= 0xfe;
            CharacterNoLongerRenderable(ped->character);
        }
    }
    if (field_0x7 != 0xff) {
        ped->character->field_0x14 = (int)(tS8)field_0x7;
        ped->character->field_0x5 = field_0xb;
        if ((field_0x6 & 4) != 0 && (field_0x7 & 4) == 0) {
            RecalculateOrientationOfRoot(ped->character);
        }
    }
}
// PARTIAL 77.78% (work-v4 measures the same): logic complete; register allocation and the flags-byte read/write differ.

// FUNCTION: CARMA2_HW 0x004ca6e0
void C2_HOOK_FASTCALL WorkerPedStatusUndo(int field_0x0, tU8 field_0x4, tU8 field_0x5, tU8 field_0x6, tU8 field_0x7, tU8 field_0x8, tU8 field_0xa, br_vector3* p0xc, br_vector3* p0x18, br_vector3* p0x24) {
    extern int C2_HOOK_FASTCALL MakeCharacterRenderable2(tPed_character_instance* pCharacter, int pIndex);
    extern int C2_HOOK_FASTCALL CharacterNoLongerRenderable(tPed_character_instance* pCharacter);
    extern void C2_HOOK_FASTCALL SetCharacterDirection(tPed_character_instance* pPed, const br_vector3* pDir, const br_vector3* pUp);
    tPedestrian* ped;

    ped = (tPedestrian*)field_0x0;
    if (field_0x4 != 0xff) {
        if (field_0x4 != 0) {
            ((tS16*)ped)[4] |= 1;
            MakeCharacterRenderable2(ped->character, (int)(tS8)field_0x8);
        } else {
            ((tS16*)ped)[4] &= ~1;
            CharacterNoLongerRenderable(ped->character);
        }
    }
    if (field_0x6 == 0xff) {
        return;
    }
    if ((field_0x6 & 4) == 0) {
        if ((field_0x7 & 4) != 0) {
            ped->character->field_0x14 = (int)(tS8)field_0x6;
            ped->character->field_0x5 = field_0xa;
            SetCharacterDirection(ped->character, p0xc, p0x18);
            BrMatrix34Copy(&ped->character->field_0x8c, (br_matrix34*)p0x24);
            SetCharacterBonePositions(ped->character, 0, 0);
            return;
        }
    } else if ((field_0x7 & 4) != 0) {
        goto update_state;
    }
    if ((field_0x6 & 4) != 0) {
        BrMatrix34Copy(&ped->character->field_0x8c, (br_matrix34*)p0x24);
        SetCharacterBonePositions(ped->character, 0, 0);
        ped->character->field_0x14 = (int)(tS8)field_0x6;
        ped->character->field_0x5 = field_0xa;
        return;
    }
update_state:
    ped->character->field_0x14 = (int)(tS8)field_0x6;
    ped->character->field_0x5 = field_0xa;
}
// PARTIAL 92.12%: same block structure and early return as the original; residual is in register/x87 allocation around the matrix copy and setters.

// FUNCTION: CARMA2_HW 0x004ca830
void C2_HOOK_FASTCALL WorkerPedPos(int field_0x0, br_vector3* pVec) {
    extern int gBOOL_00744804;
    extern void C2_HOOK_FASTCALL SetCharacterPosition(tPed_character_instance* pCharacter, br_vector3* pPos, int pArg3);
    tPedestrian* ped;

    ped = (tPedestrian*)field_0x0;
    if (gBOOL_00744804 != 0) {
        return;
    }
    if (pVec->v[0] == 0.0f && pVec->v[1] == 0.0f && pVec->v[2] == 0.0f) {
        ped->flags |= 0x80;
        return;
    }
    SetCharacterPosition(ped->character, pVec, -1);
    if (ped->flags & 0x80) {
        ped->flags &= ~0x80;
    }
}
// PARTIAL 86.96% (work-v4 measures 89.55%): zero-vector test and flags clear are correct; register allocation differs.

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
// FUNCTION: CARMA2_HW 0x0040b500
void C2_HOOK_FASTCALL FUN_0040b500(tPed_character_instance* pCharacter, int pBone_index, int pArg3, int pArg4) {
    if (pCharacter->field_0x4 >= 0) {
        pCharacter->field_0xa = (tS8)pArg4;
        pCharacter->personality->form->actor_sets[(tS8)pCharacter->field_0x4].actors[pBone_index]->model = (br_model*)pArg3;
    }
}
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif

// FUNCTION: CARMA2_HW 0x004ca8a0
void C2_HOOK_FASTCALL WorkerPedModelChangeUndo(int field_0x0, int field_0x4, int field_0xc, int field_0xa) {
    extern void C2_HOOK_FASTCALL FUN_0040b500(tPed_character_instance* pCharacter, int pBone_index, int pArg3, int pArg4);
    tPedestrian* ped;

    ped = (tPedestrian*)field_0x0;
    if (field_0x4 < 0) {
        SetCharacterAllBonesModel(ped->character, -1, field_0xa);
    } else {
        FUN_0040b500(ped->character, field_0x4, field_0xc, field_0xa);
    }
}

// FUNCTION: CARMA2_HW 0x004ca900
void C2_HOOK_FASTCALL WorkerPedDismember(int field_0x0, int field_0x4, int field_0x7) {
    extern const int gPow2_array[32];
    extern tPhysics_object* C2_HOOK_FASTCALL MakeCharacterPhysicsForBone(tPed_character_instance* pCharacter, int pBone_index, undefined4 pArg3, undefined4 pArg4);
    tPedestrian* ped;
    tPed_character_instance* character;
    br_actor* actor;

    ped = (tPedestrian*)field_0x0;
    ped->character->field_0xc |= gPow2_array[field_0x4];
    ped->character->field_0x6 = *(tS8*)&field_0x7;
    MakeCharacterPhysicsForBone(ped->character, field_0x4, 1, 0);
    character = ped->character;
    actor = character->personality->form->actor_sets[(tS8)character->field_0x4].actors[field_0x4];
    if (actor->parent == NULL && (field_0x4 == 0 || (gPow2_array[field_0x4] & character->field_0xc) != 0)) {
        BrActorAdd(gNon_track_actor, actor);
    }
}

extern void C2_HOOK_FASTCALL DoGibShower(int pId, int pTime, int pSeed, int pForce, int pSize_index, br_vector3* pArg6, br_vector3* pArg7, br_vector3* pArg8);

// FUNCTION: CARMA2_HW 0x004ccae0
void C2_HOOK_FASTCALL WorkerGibShower(int field_0x0, int field_0xc, int field_0x8, int field_0x10, int field_0x4, br_vector3* pField_0x14, br_vector3* pField_0x20, br_vector3* pField_0x2c) {

    DoGibShower(field_0x0, field_0xc, field_0x8, field_0x10, field_0x4, pField_0x14, pField_0x20, pField_0x2c);
}

