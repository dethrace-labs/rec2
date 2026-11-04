/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright 2022-2026 rec2 developers
 */

#include "49-physics.h"

#include "00-car.h"
#include "16-graphics1.h"
#include "22-replay.h"
#include "28-world3.h"
#include "32-spark.h"
#include "34-finteray.h"
#include "37-brucetrk.h"
#include "52-errors.h"
#include "60-phil.h"
#include "65-hull.h"
#include "globvars.h"
#include "platform.h"
#include "rec2_macros.h"
#include "c2_math.h"
#include "c2_string.h"
#include "c2_stdlib.h"

#include <stddef.h>

#define PHYSICS_BUFFER_PART_SIZE 40000
#define PHYSICS_BUFFER_OTHER_SIZE 50000

// GLOBAL: CARMA2_HW 0x00692dc8
// GLOBAL: CARMA2_SW 0x0068ce20
int gINT_00692dc8;

// GLOBAL: CARMA2_HW 0x0058a768
br_scalar gFLOAT_0058a768 = 0.0f;

// GLOBAL: CARMA2_HW 0x00692dd0
float gFLOAT_00692dd0;

// GLOBAL: CARMA2_HW 0x00692dd4
float gFLOAT_00692dd4;

// GLOBAL: CARMA2_HW 0x0058a798
double gDOUBLE_0058a798 = 0.01;

// GLOBAL: CARMA2_HW 0x006940c0
// GLOBAL: CARMA2_SW 0x0068e118
tPhysicsError_cbfn* gPhysics_error_cb;

// GLOBAL: CARMA2_HW 0x00744818
// GLOBAL: CARMA2_SW 0x0073b2d8
void* gPhysics_buffer1_ptr;

// GLOBAL: CARMA2_HW 0x00744814
// GLOBAL: CARMA2_SW 0x0073b2d4
void* gPhysics_buffer2_ptr;

// GLOBAL: CARMA2_HW 0x00744810
// GLOBAL: CARMA2_SW 0x0073b2d0
int gImmortal_peds;

// GLOBAL: CARMA2_HW 0x00744804
// GLOBAL: CARMA2_SW 0x0073b2c4
int gBOOL_00744804;

// GLOBAL: CARMA2_HW 0x00744808
// GLOBAL: CARMA2_SW 0x0073b2c8
tPedestrian* gPedestrian_array;

// GLOBAL: CARMA2_HW 0x0074a5e0
// GLOBAL: CARMA2_SW 0x007410a0
void* gPhysics_buffer3_ptr;

// GLOBAL: CARMA2_HW 0x0074a5e4
// GLOBAL: CARMA2_SW 0x007410a4
int gPHIL_mechanics_time_sync;

// GLOBAL: CARMA2_HW 0x006940c8
// GLOBAL: CARMA2_SW 0x0068e120
int gPhysics_other_buffer_capacity;

// GLOBAL: CARMA2_HW 0x00694140
// GLOBAL: CARMA2_SW 0x0068e198
tProximity_ray gProximity_rays[20];

// GLOBAL: CARMA2_HW 0x00694280
// GLOBAL: CARMA2_SW 0x0068e2d8
br_actor* gPed_limbed_actors[30];

// GLOBAL: CARMA2_HW 0x006940c4
// GLOBAL: CARMA2_SW 0x0068e11c
void* gPhysics_other_buffer;

// GLOBAL: CARMA2_HW 0x006940bc
// GLOBAL: CARMA2_SW 0x0068e114
void* gPhysics_scratch_ptr;

// GLOBAL: CARMA2_HW 0x006940b0
// GLOBAL: CARMA2_SW 0x0068e108
int gFace_count;

// GLOBAL: CARMA2_HW 0x00692dd8
int gINT_00692dd8;

// count

// FUNCTION: CARMA2_HW 0x004225b0
// FUNCTION: CARMA2_SW 0x00422630
void C2_HOOK_FASTCALL DrMatrix34ApplyLPInverse(br_vector3* dest, const br_vector3* source, const br_matrix34* matrix) {
    br_vector3 relative;

    BrVector3Sub(&relative, source, (br_vector3*)matrix->m[3]);
    BrMatrix34TApplyV(dest, &relative, matrix);
}

#ifndef ABS
#define ABS(a) (((a) < 0) ? -(a) : (a))
#endif
// FUNCTION: CARMA2_HW 0x00422fe0
int C2_HOOK_FASTCALL AddObjectWallForce(float* pWalls, const br_vector3* pPoint, float pDistance, const br_vector3* pNormal, tU32 pUnused, int pCount, tPhysics_object* pObject, float pT, float pRadius, const float* pVector) {
    float* piVar1;
    float fAccum;
    float fVar2;
    float fVar3;
    float fVar4;
    tPhysics_object* pObj;
    int iVar6;
    int iVar8;
    int iVar9;
    float* pfVar7;
    float* pfVar10;
    float* pfVar11;
    float* pfVar12;
    float local_18[6];

    pObj = pObject;
    if ((*(int*)((char*)pObj + 0x228) == 0) &&
        (piVar1 = *(float**)((char*)pObj + 0x18c), piVar1 != NULL) && (*(int*)piVar1 != 0)) {
        iVar8 = 0;
        local_18[3] = pObj->bb1.max.v[0] - (float)piVar1[2];
        local_18[4] = pObj->bb1.max.v[1] - (float)piVar1[3];
        local_18[5] = pObj->bb1.max.v[2] - (float)piVar1[4];
        local_18[0] = pObj->bb1.min.v[0] - (float)piVar1[2];
        local_18[1] = pObj->bb1.min.v[1] - (float)piVar1[3];
        local_18[2] = pObj->bb1.min.v[2] - (float)piVar1[4];
        fAccum = 0.0;
        do {
            fVar2 = ABS(*(float*)((char*)local_18 + iVar8 + 0xc));
            if (ABS(*(float*)((char*)local_18 + iVar8)) <= fVar2) {
                fVar2 = ABS(*(float*)((char*)local_18 + iVar8));
            }
            if (fAccum < fVar2) {
                fAccum = fVar2;
            }
            iVar8 = iVar8 + 4;
        } while (iVar8 < 0xc);
        fAccum = fAccum - -0.01;
        if (((ABS(pPoint->v[0] - (float)piVar1[2]) < fAccum) &&
            (fVar2 = ABS(pPoint->v[1] - (float)piVar1[3]), fVar2 < fAccum)) &&
           ((fVar2 + fVar2 < pObj->bb1.max.v[1] - pObj->bb1.min.v[1]) &&
            (ABS(pPoint->v[2] - (float)piVar1[4]) < fAccum))) {
            return pCount;
        }
    }
    iVar8 = 0;
    if (0 < pCount) {
        pfVar7 = pWalls + 6;
        do {
            fVar4 = pfVar7[-1] - (pPoint->v[0] - pObj->cmpos.v[0]);
            fVar2 = *pfVar7 - (pPoint->v[1] - pObj->cmpos.v[1]);
            fVar3 = pfVar7[1] - (pPoint->v[2] - pObj->cmpos.v[2]);
            fVar2 = fVar3 * fVar3 + fVar2 * fVar2 + fVar4 * fVar4;
            if (fVar2 < 0.0045) {
                fVar3 = pfVar7[5] * pNormal->v[0] + pNormal->v[1] * pfVar7[6] + pfVar7[7] * pNormal->v[2];
                if ((0.95 < fVar3) && (fVar2 < 0.0005)) {
                    return pCount;
                }
                if (fVar3 < -0.95) {
                    return pCount;
                }
            }
            iVar8 = iVar8 + 1;
            pfVar7 = pfVar7 + 0x18;
        } while (iVar8 < pCount);
    }
    iVar8 = pCount;
    if (pCount < 4) {
        iVar8 = pCount + 1;
        pWalls[pCount * 0x18] = 2.0;
    }
    iVar9 = 0;
    pfVar7 = pWalls;
    if (0 < iVar8) {
        while (*pfVar7 <= pDistance) {
            iVar9 = iVar9 + 1;
            pfVar7 = pfVar7 + 0x18;
            if (iVar8 <= iVar9) {
                return iVar8;
            }
        }
        iVar6 = 3;
        if (iVar8 < 4) {
            iVar6 = iVar8;
        }
        if (iVar9 <= iVar6) {
            pCount = (iVar6 - iVar9) + 1;
            pfVar11 = pWalls + iVar6 * 0x18;
            do {
                pfVar10 = pfVar11 + -0x18;
                pfVar12 = pfVar11;
                for (iVar9 = 0x18; iVar9 != 0; iVar9 = iVar9 + -1) {
                    *pfVar12 = *pfVar10;
                    pfVar10 = pfVar10 + 1;
                    pfVar12 = pfVar12 + 1;
                }
                pCount = pCount + -1;
                pfVar11 = pfVar11 + -0x18;
            } while (pCount != 0);
        }
        *pfVar7 = pDistance;
        *(tPhysics_object**)(pfVar7 + 1) = pObj;
        pfVar7[2] = 0.0;
        pfVar7[3] = pRadius;
        pfVar7[0xb] = pNormal->v[0];
        pfVar7[0xc] = pNormal->v[1];
        pfVar7[0xd] = pNormal->v[2];
        pfVar7[5] = pPoint->v[0] - pObj->cmpos.v[0];
        pfVar7[6] = pPoint->v[1] - pObj->cmpos.v[1];
        pfVar7[7] = pPoint->v[2] - pObj->cmpos.v[2];
        if (pVector != NULL) {
            pfVar7[8] = pVector[0];
            pfVar7[9] = pVector[1];
            pfVar7[10] = pVector[2];
        }
        pfVar7[0x17] = pT;
    }
    return iVar8;
}

// FUNCTION: CARMA2_HW 0x004268a0
// FUNCTION: CARMA2_SW 0x00426940
int C2_HOOK_FASTCALL AddObjectObjectForce(tPhysics_joint_force* pForces, const br_vector3* pPoint_a, br_scalar pT, const br_vector3* pPoint_b, const br_vector3* pNormal, int pCount, tPhysics_object* pObject_a, tPhysics_object* pObject_b, int pType, void* pUnused, br_scalar pValue) {
    int i;
    int j;
    tPhysics_joint_force* force;

    (void)pUnused;
    if (pCount < 4) {
        pForces[pCount].field_0x0 = 2.f;
        pCount++;
    }
    for (i = 0, force = pForces; i < pCount; i++, force++) {
        if (pT < force->field_0x0) {
            for (j = MIN(3, pCount); j >= i; j--) {
                pForces[j] = pForces[j - 1];
            }
            force->field_0x10 = pValue;
            force->object_a = pObject_a;
            force->object_b = pObject_b;
            BrVector3Sub(&force->point_a, pPoint_a, &pObject_a->cmpos);
            BrVector3Sub(&force->point_b, pPoint_b, &pObject_b->cmpos);
            BrMatrix34TApplyV(&force->direction_a, pNormal, &pObject_a->transform_matrix);
            BrMatrix34TApplyV(&force->direction_b, pNormal, &pObject_b->transform_matrix);
            force->field_0x0 = pT;
            force->type = pType;
            force->field_0xc = NULL;
            return pCount;
        }
    }
    return pCount;
}

// FUNCTION: CARMA2_HW 0x004225f0
// FUNCTION: CARMA2_SW 0x00422670
tFace_ref* C2_HOOK_FASTCALL RayCastThruFaceList(br_vector3* pPosition, br_vector3* pDirection, br_vector3* pNormal, br_scalar* pT, br_vector3* pCollision_position, tFace_ref* pFaces, int pCount) {
    br_vector3 normal;
    br_vector3 collision_position;
    br_scalar t;
    tFace_ref* best;
    int i;

    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tFace_ref, flags, 0x40);

    *pT = 2.f;
    for (i = 0; i < pCount; i++, pFaces++) {
        if ((pFaces->flags & 0x80) == 0) {
            CheckSingleFace(pFaces, pPosition, pDirection, &normal, &t, &collision_position);
            if (t < *pT) {
                *pT = t;
                BrVector3Copy(pNormal, &normal);
                best = pFaces;
                BrVector3Copy(pCollision_position, &collision_position);
            }
        }
    }
    if (*pT < 2.f) {
        return best;
    } else {
        return NULL;
    }
}

// STUB: CARMA2_HW 0x004226d0
// STUB: CARMA2_SW 0x00422750
tFace_ref* C2_HOOK_FASTCALL RayCastThruFaceList2(br_vector3* pPosition, br_vector3* pDirection, br_vector3* pNormal, br_scalar* pT, br_vector3* pCollision_position, tFace_ref* pFaces, int pCount) {
    NOT_IMPLEMENTED();
}

// RayCastInBox

// FaceOffCount2

// FaceOffCount

// FaceOnCount

// FUNCTION: CARMA2_HW 0x00423320
int C2_HOOK_FASTCALL EliminateFaces(tPhysics_object* pObject, int* pCtx, br_matrix34* pMatrix) {
    float* pfVar1;
    float fVar2;
    float fVar3;
    float fVar4;
    float fVar5;
    float fVar6;
    int iVar7;
    float fVar8;
    float* pfVar9;
    tU32 uVar10;
    int* piVar13;
    int iVar14;
    int iVar15;
    int local_3c;
    float local_24[9];
    float local_30[3];
    float* pfVar11;
    float* pfVar12;

    iVar7 = *(int*)pObject;
    pfVar1 = (float*)((char*)pCtx + 0x1c);
    iVar14 = 0;
    local_3c = 0;
    GetNewBoundingBox((br_bounds*)pfVar1, (br_bounds*)(pCtx + 1), pMatrix);
    pfVar9 = (float*)((char*)pCtx + 0x10);
    iVar15 = 3;
    pfVar11 = pfVar1;
    do {
        if (pfVar9[-3] < *pfVar11) {
            *pfVar11 = pfVar9[-3];
        }
        if (pfVar11[3] < *pfVar9) {
            pfVar11[3] = *pfVar9;
        }
        pfVar11 = pfVar11 + 1;
        pfVar9 = pfVar9 + 1;
        iVar15 = iVar15 + -1;
    } while (iVar15 != 0);
    BrMatrix34ApplyV((br_vector3*)local_30, (br_vector3*)pfVar1, (br_matrix34*)(iVar7 + 0x2c));
    fVar2 = *(float*)(iVar7 + 0x50);
    pfVar11 = local_24 + 1;
    pfVar9 = (float*)(iVar7 + 0x34);
    fVar3 = *(float*)(iVar7 + 0x54);
    iVar15 = 3;
    fVar4 = *(float*)(iVar7 + 0x58);
    pfVar12 = pfVar1;
    do {
        fVar8 = pfVar12[3] - *pfVar12;
        fVar5 = pfVar9[-1];
        fVar6 = *pfVar9;
        pfVar11[-1] = pfVar9[-2] * fVar8;
        *pfVar11 = fVar5 * fVar8;
        pfVar12 = pfVar12 + 1;
        pfVar11 = pfVar11 + 3;
        *(float*)(((int)local_24 - (iVar7 + 0x2c)) + (int)pfVar9) = fVar6 * fVar8;
        pfVar9 = pfVar9 + 3;
        iVar15 = iVar15 + -1;
    } while (iVar15 != 0);
    uVar10 = (tU32)*(tU16*)((char*)pObject + 0x174);
    piVar13 = (int*)((char*)gFace_list__car + uVar10 * 0x50);
    if (*(tU16*)((char*)pObject + 0x176) != uVar10 &&
        -1 < (int)(*(tU16*)((char*)pObject + 0x176) - uVar10)) {
        do {
            pfVar9 = local_24 + 2;
            iVar15 = 3;
            fVar5 = ((local_30[1] + fVar3) - (float)piVar13[2]) * (float)piVar13[0xe] +
                    (float)piVar13[0xf] * ((local_30[2] + fVar4) - (float)piVar13[3]) +
                    (float)piVar13[0xd] * ((local_30[0] + fVar2) - (float)piVar13[1]);
            fVar6 = fVar5;
            do {
                fVar8 = *pfVar9 * (float)piVar13[0xf] +
                        (float)piVar13[0xe] * pfVar9[-1] + (float)piVar13[0xd] * pfVar9[-2];
                if (0.0 <= fVar8) {
                    fVar5 = fVar8 + fVar5;
                }
                else {
                    fVar6 = fVar6 + fVar8;
                }
                pfVar9 = pfVar9 + 3;
                iVar15 = iVar15 + -1;
            } while (iVar15 != 0);
            if ((((0.005 < fVar5) && (0.005 < fVar6)) || ((fVar5 < -0.005 && (fVar6 < -0.005)))) ||
               ((iVar15 = *piVar13, iVar15 != 0 && (**(char**)(iVar15 + 4) == '!')))) {
                piVar13[0x10] = piVar13[0x10] | 0x80;
            }
            else if (**(char**)(iVar15 + 4) == '>') {
                piVar13[0x10] = piVar13[0x10] | 0x80;
            }
            else {
                pfVar9 = (float*)(iVar7 + 0x34);
                iVar15 = 3;
                pfVar11 = pfVar1;
                do {
                    fVar5 = *(float*)(iVar7 + 0x54) * pfVar9[-1] +
                            *(float*)(iVar7 + 0x50) * pfVar9[-2] + *(float*)(iVar7 + 0x58) * *pfVar9;
                    fVar8 = (float)piVar13[2] * pfVar9[-1] +
                            (float)piVar13[1] * pfVar9[-2] + *pfVar9 * (float)piVar13[3];
                    fVar6 = (*pfVar11 - 0.005) + fVar5;
                    if (fVar6 <= fVar8) {
                        fVar5 = (pfVar11[3] + fVar5) - -0.005;
                        if (((fVar5 < fVar8) &&
                            (fVar5 < (float)piVar13[6] * *pfVar9 +
                                     pfVar9[-2] * (float)piVar13[4] + pfVar9[-1] * (float)piVar13[5])) &&
                           (fVar5 < (float)piVar13[9] * *pfVar9 +
                                    pfVar9[-2] * (float)piVar13[7] + pfVar9[-1] * (float)piVar13[8]))
                            goto LAB_00423642;
                    }
                    else if (((float)piVar13[6] * *pfVar9 +
                              pfVar9[-2] * (float)piVar13[4] + pfVar9[-1] * (float)piVar13[5] < fVar6) &&
                             ((float)piVar13[9] * *pfVar9 +
                              pfVar9[-2] * (float)piVar13[7] + pfVar9[-1] * (float)piVar13[8] < fVar6)) {
LAB_00423642:
                        piVar13[0x10] = piVar13[0x10] | 0x80;
                    }
                    pfVar11 = pfVar11 + 1;
                    pfVar9 = pfVar9 + 3;
                    iVar15 = iVar15 - 1;
                } while (iVar15 != 0);
                local_3c = local_3c + 1;
                piVar13[0x10] = piVar13[0x10] & 0xffffff7f;
            }
            iVar14 = iVar14 + 1;
            piVar13 = piVar13 + 0x14;
        } while (iVar14 < (int)((tU32)*(tU16*)((char*)pObject + 0x176) - (tU32)*(tU16*)((char*)pObject + 0x174)));
    }
    return local_3c;
}

// GetShapePoint

// GetObjectObjectEdgeIntersections

// GetFaceEdgeIntersection

// GetSphereToFacePoints

// CheckSingleEdgeWithSphere

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
// STUB: CARMA2_HW 0x00423ce0
int C2_HOOK_FASTCALL CheckSingleEdgeWithSphere(br_vector3* pEdgeStart, br_vector3* pEdgeEnd, tPhysics_object* pObject, float* pSphere, float* pWalls, int pCount, br_vector3* pStart, br_vector3* pEnd, tFace_ref* pFace) {
    NOT_IMPLEMENTED();
    return 0;
}
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif

// FUNCTION: CARMA2_HW 0x00423c00
int C2_HOOK_FASTCALL GetSphereToEdgePoints(tPhysics_object* pObject, int pEdx, int pParam1, int pParam2, int pParam3, int pParam4, int pParam5, int pParam6) {
    int local_4;
    char* puVar1;

    local_4 = (int)*(tU16*)((char*)pObject + 0x174);
    puVar1 = (char*)gFace_list__car + local_4 * 0x50;
    if (local_4 < (int)*(tU16*)((char*)pObject + 0x176)) {
        do {
            if ((*(tU32*)(puVar1 + 0x40) & 0x80) == 0) {
                if ((*(tU32*)(puVar1 + 0x40) & 1) == 0) {
                    pParam2 = CheckSingleEdgeWithSphere((br_vector3*)(puVar1 + 0x4), (br_vector3*)(puVar1 + 0x10), pObject, (float*)pEdx, (float*)pParam1, pParam2, (br_vector3*)pParam4, (br_vector3*)pParam5, (tFace_ref*)puVar1);
                }
                if ((*(tU32*)(puVar1 + 0x40) & 2) == 0) {
                    pParam2 = CheckSingleEdgeWithSphere((br_vector3*)(puVar1 + 0x10), (br_vector3*)(puVar1 + 0x1c), pObject, (float*)pEdx, (float*)pParam1, pParam2, (br_vector3*)pParam4, (br_vector3*)pParam5, (tFace_ref*)puVar1);
                }
                if ((*(tU32*)(puVar1 + 0x40) & 4) == 0) {
                    pParam2 = CheckSingleEdgeWithSphere((br_vector3*)(puVar1 + 0x1c), (br_vector3*)(puVar1 + 0x4), pObject, (float*)pEdx, (float*)pParam1, pParam2, (br_vector3*)pParam4, (br_vector3*)pParam5, (tFace_ref*)puVar1);
                }
            }
            local_4 = local_4 + 1;
            puVar1 = puVar1 + 0x50;
        } while ((int)local_4 < (int)(tU32)*(tU16*)((char*)pObject + 0x176));
    }
    return pParam2;
}

// FUNCTION: CARMA2_HW 0x00423ed0
int C2_HOOK_FASTCALL GetCornerToFacePoints(tPhysics_object* pObject, int pEdx, tU32 pParam1, int pParam2, int pParam3, char* pParam4, tU32 pParam5) {
    int local_54;
    float local_4c;
    float local_48[3];
    float local_3c[3];
    float local_30[3];
    float local_24[3];
    float local_18[3];
    float local_c[3];
    float fVar1;
    float fVar2;
    float fVar3;
    float fVar4;
    float* pfVar5;
    int* piVar;
    int iVar4;
    int start;
    int count;

    if (3 < pParam3) {
        local_54 = 0;
        if (*(short*)(pParam4 + 0x700) != 0) {
            pfVar5 = (float*)(pParam4 + 0x304);
            piVar = (int*)(pParam4 + 0x600);
            do {
                if (*piVar == 0) {
                    local_48[0] = pfVar5[-1] - pfVar5[-0xc1];
                    local_48[1] = *pfVar5 - pfVar5[-0xc0];
                    local_48[2] = pfVar5[1] - pfVar5[-0xbf];
                    fVar4 = (float)sqrt(local_48[2] * local_48[2] + local_48[1] * local_48[1] + local_48[0] * local_48[0]);
                    if (fVar4 <= 2.3841858e-07f) {
                        local_3c[0] = 1.0f;
                        local_3c[1] = 0.0f;
                        fVar4 = 0.0f;
                    } else {
                        fVar4 = 1.0f / fVar4;
                        local_3c[0] = fVar4 * local_48[0];
                        local_3c[1] = fVar4 * local_48[1];
                        fVar4 = fVar4 * local_48[2];
                    }
                    local_3c[2] = fVar4;
                    local_3c[0] = local_3c[0] * 0.0010434783f;
                    local_3c[1] = local_3c[1] * 0.0010434783f;
                    local_3c[2] = local_3c[2] * 0.0010434783f;
                    local_48[0] = local_3c[0] + local_48[0];
                    local_48[1] = local_3c[1] + local_48[1];
                    local_48[2] = local_3c[2] + local_48[2];
                    start = (int)*(tU16*)((char*)pObject + 0x174);
                    count = (int)*(tU16*)((char*)pObject + 0x176) - start;
                    iVar4 = (int)RayCastThruFaceList((br_vector3*)local_48, (br_vector3*)local_3c, (br_vector3*)local_c, (br_scalar*)&local_4c, (br_vector3*)local_30, (tFace_ref*)((char*)gFace_list__car + start * 0x50), (int)count);
                    if (iVar4 != 0) {
                        BrMatrix34TApplyV((br_vector3*)local_3c, (br_vector3*)local_30, (br_matrix34*)((char*)pObject + 0x8c));
                        fVar1 = local_30[0] * local_48[0];
                        fVar3 = local_30[1] * local_48[1];
                        fVar2 = local_30[2] * local_48[2];
                        local_24[0] = pfVar5[-0xc1] - *(float*)((char*)pObject + 0xb0);
                        local_24[1] = pfVar5[-0xc0] - *(float*)((char*)pObject + 0xb4);
                        local_24[2] = pfVar5[-0xbf] - *(float*)((char*)pObject + 0xb8);
                        BrMatrix34TApplyV((br_vector3*)local_18, (br_vector3*)local_24, (br_matrix34*)((char*)pObject + 0x8c));
                        pParam2 = AddObjectWallForce((float*)pParam4, (br_vector3*)local_18, fVar2 + fVar3 + fVar1, (br_vector3*)local_3c, 0, pParam2, pObject, 1.0f, local_4c, NULL);
                    }
                }
                piVar = piVar + 1;
                local_54 = local_54 + 1;
                pfVar5 = pfVar5 + 3;
            } while (local_54 < (int)(tU32)*(short*)(pParam4 + 0x700));
        }
        if (3 < pParam2) {
            pParam2 = 4;
        }
        return pParam2;
    }
    return 0;
}

// CylinderCheck

// CheckTwoEdges

// WorldPointCollision

// CheckSingleEdgeWithObjectEdges

// GetEdgeToEdgePoints

// SetUpWorkspace

// GetObjectWallForceInfo

// GetObjectCornerToObjectFacePoints

static int notfinite(const float* f) {
    if (f != NULL) {
        return ((~*(tU32*)f) & 0x7f800000) == 0;
    } else {
        return 0;
    }
}

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif

// FUNCTION: CARMA2_HW 0x00425ba0
// STUB: CARMA2_SW 0x00425c40
int C2_HOOK_FASTCALL BoundsOverlapTest__finteray(br_bounds* b1, br_bounds* b2) {
    int i;

    if (b1->min.v[2] > b2->max.v[2]
           || b2->min.v[2] > b1->max.v[2]
           || b1->min.v[1] > b2->max.v[1]
           || b2->min.v[1] > b1->max.v[1]
           || b1->min.v[0] > b2->max.v[0]
           || b2->min.v[0] > b1->max.v[0]) {

        return 0;
    }

    for (i = 0; i < 3; i++) {
        if (notfinite(&b1->min.v[i]) || notfinite(&b1->max.v[i])
                || notfinite(&b2->min.v[i]) || notfinite(&b2->max.v[i])) {
            return 0;
        }
    }
    return 1;
}

#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif

// GetObjectObjectEdgePoints

// GetSphereNonSphereForceInfo

// GetSphereSphereForceInfo

// GetSphereShapeForceInfo

// GetNonSphereNonSphereForceInfo

// GetNonSphereShapeForceInfo

// GetObjectObjectForceInfo

// FUNCTION: CARMA2_HW 0x00428480
// FUNCTION: CARMA2_SW 0x00428510
void C2_HOOK_FASTCALL GetShapePoint(br_vector3* pPoint, tPhysics_shape* pShape, tU32 pIndex) {
    switch (pShape->common.type) {
    case kCollisionShapeType_Box:
        BrVector3Set(pPoint,
            (pIndex & 0x2) ? pShape->common.bb.min.v[0] : pShape->common.bb.max.v[0],
            (pIndex & 0x1) ? pShape->common.bb.min.v[1] : pShape->common.bb.max.v[1],
            (pIndex & 0x4) ? pShape->common.bb.min.v[2] : pShape->common.bb.max.v[2]);
        break;
    case kCollisionShapeType_Wireframe:
        BrVector3Copy(pPoint, &pShape->wireframe.wireframe.points[pIndex]);
        return;
    case kCollisionShapeType_Polyhedron:
    case kCollisionShapeType_Wireframe_Polyhedron:
        BrVector3Copy(pPoint, &pShape->polyhedron.polyhedron.points[pIndex]);
        return;
    default:
        return;
    }
}

// FUNCTION: CARMA2_HW 0x00428cf0
// FUNCTION: CARMA2_SW 0x00428d80
int C2_HOOK_FASTCALL AddOldForces(tPhysics_joint_force* pForces, int pCount, const tPhysics_joint_force* pOld_forces, int pOld_count) {
    int i;

    for (i = 0; i < pOld_count && pCount < 40; i++) {
        if (pOld_forces[i].type != 9) {
            pForces[pCount] = pOld_forces[i];
            pCount++;
        }
    }
    return pCount;
}

// FUNCTION: CARMA2_HW 0x00428d40
// FUNCTION: CARMA2_SW 0x00428dd0
int C2_HOOK_FASTCALL TestObjectOverlap(tPhysics_object* pCollision_1, tPhysics_object* pCollision_2) {
    tPhysics_shape* shape1;
    tPhysics_shape* shape2;

    shape1 = pCollision_1->shape;
    shape2 = pCollision_2->shape;
    if (FlagsAreGo(pCollision_1, pCollision_2)
        && BoundsOverlapTest__finteray(&pCollision_1->field_0xf4, &pCollision_2->field_0xf4)) {
        while (shape1 != NULL) {
            while (shape2 != NULL) {
                if (TestShapeEdgeIntersection(pCollision_1, pCollision_2, shape1, shape2)
                    || TestShapeEdgeIntersection(pCollision_2, pCollision_1, shape2, shape1)) {
                    return 1;
                }
                shape2 = shape2->common.next;
            }
            shape1 = shape1->common.next;
        }
    }
    return 0;
}

// FUNCTION: CARMA2_HW 0x00428dc0
// FUNCTION: CARMA2_SW 0x00428e50
int C2_HOOK_FASTCALL TestShapeEdgeIntersection(tPhysics_object* pCollision_info_1, tPhysics_object* pCollision_info_2, tPhysics_shape* pCollision_shape_1, tPhysics_shape* pCollision_shape_2) {
    br_vector3 points[64];
    br_vector3 transformed;
    br_vector3 point;
    br_bounds bounds;
    br_scalar t;
    tPolyhedron_edge_indexes* edges;
    tPolyhedron_edge_indexes* edge;
    int count_edges;
    int i;

    C2_HOOK_BUG_ON(REC2_ASIZE(gDefault_box_collision_wireframe_edges) != 12);

    switch (pCollision_shape_1->common.type) {
    case kCollisionShapeType_Polyhedron:
    case kCollisionShapeType_Wireframe_Polyhedron:
        for (i = 0; i < pCollision_shape_1->polyhedron.polyhedron.count_points; i++) {
            BrMatrix34ApplyP(&transformed, &pCollision_shape_1->polyhedron.polyhedron.points[i], &pCollision_info_1->transform_matrix);
            DrMatrix34ApplyLPInverse(&points[i], &transformed, &pCollision_info_2->transform_matrix);
        }
        edges = pCollision_shape_1->polyhedron.polyhedron.edges;
        count_edges = pCollision_shape_1->polyhedron.polyhedron.count_edges;
        break;
    case kCollisionShapeType_Box:
    case kCollisionShapeType_Wireframe:
    case kCollisionShapeType_Sphere:
        for (i = 0; i < 8; i++) {
            BrVector3Set(&points[i],
                (i & 0x2) ? pCollision_shape_1->common.bb.min.v[0] : pCollision_shape_1->common.bb.max.v[0],
                (i & 0x1) ? pCollision_shape_1->common.bb.min.v[1] : pCollision_shape_1->common.bb.max.v[1],
                (i & 0x4) ? pCollision_shape_1->common.bb.min.v[2] : pCollision_shape_1->common.bb.max.v[2]);
            BrMatrix34ApplyP(&transformed, &points[i], &pCollision_info_1->transform_matrix);
            DrMatrix34ApplyLPInverse(&points[i], &transformed, &pCollision_info_2->transform_matrix);
        }
        edges = gDefault_box_collision_wireframe_edges;
        count_edges = REC2_ASIZE(gDefault_box_collision_wireframe_edges);
        break;
    }
    switch (pCollision_shape_2->common.type) {
    case kCollisionShapeType_Polyhedron:
    case kCollisionShapeType_Wireframe_Polyhedron:
        for (i = 0, edge = edges; i < count_edges; i++, edge++) {
            if (LinePolyColl(&points[edge->index1], &points[edge->index2], &pCollision_shape_2->polyhedron.polyhedron, &point, 0.005f, &t)) {
                return 1;
            }
        }
        break;
    case kCollisionShapeType_Box:
    case kCollisionShapeType_Wireframe:
    case kCollisionShapeType_Sphere:
        BrVector3Set(&bounds.min,
            pCollision_shape_2->common.bb.min.v[0] - 0.005f,
            pCollision_shape_2->common.bb.min.v[1] - 0.005f,
            pCollision_shape_2->common.bb.min.v[2] - 0.005f);
        BrVector3Set(&bounds.max,
            pCollision_shape_2->common.bb.max.v[0] + 0.005f,
            pCollision_shape_2->common.bb.max.v[1] + 0.005f,
            pCollision_shape_2->common.bb.max.v[2] + 0.005f);
        for (i = 0, edge = edges; i < count_edges; i++, edge++) {
            if (LineBoxColl(&points[edge->index1], &points[edge->index2], &bounds, &point, &t)) {
                return 1;
            }
        }
        break;
    }
    return 0;
}

// TestObjectOverlapIgnoreCloseRelatives

// TestObjectOverlapRecurse

// TestForObjectInSensiblePlace

// HierarchyCalcEnergy

// CheckCollisionList

// MakeJointVelocitiesVector

// ApplySingleForce

// ApplyJointForces

// SkipForces

// ScaleUpObjectMI

// CheckForTwang

// FUNCTION: CARMA2_HW 0x004b8c20
void C2_HOOK_FASTCALL CreateJointForce(tPhysics_object* pObject, tPhysics_object* pPartner, tPhysics_joint_force* pForce, const br_vector3* pDirection) {
    br_vector3 direction;

    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_joint_force, field_0x0, 0x0);
    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_joint_force, object_a, 0x4);
    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_joint_force, object_b, 0x8);
    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_joint_force, field_0xc, 0xc);
    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_joint_force, point_a, 0x14);
    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_joint_force, point_b, 0x20);
    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_joint_force, direction_a, 0x2c);
    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_joint_force, direction_b, 0x38);
    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_joint_force, type, 0x5c);

    pForce->field_0x0 = 0.f;
    pForce->object_a = pObject;
    pForce->object_b = pPartner;
    pForce->field_0xc = NULL;
    BrVector3Sub(&pForce->point_a, &pObject->physics_joint1->field_0x08, &pObject->cmpos);
    BrVector3Sub(&pForce->point_b, &pObject->physics_joint1->field_0x14, &pPartner->cmpos);
    BrVector3Copy(&pForce->direction_a, pDirection);
    BrMatrix34ApplyV(&direction, pDirection, &pObject->transform_matrix);
    BrMatrix34TApplyV(&pForce->direction_b, &direction, &pPartner->transform_matrix);
    pForce->type = 10;
}

// FUNCTION: CARMA2_HW 0x004b8e10
void C2_HOOK_FASTCALL CreateJointTorque(tPhysics_object* pObject, tPhysics_object* pPartner, tPhysics_joint_force* pForce, const br_vector3* pDirection) {
    br_vector3 direction;

    pForce->field_0x0 = 0.f;
    pForce->object_a = pObject;
    pForce->object_b = pPartner;
    BrVector3Sub(&pForce->point_a, &pObject->physics_joint1->field_0x08, &pObject->cmpos);
    BrVector3Sub(&pForce->point_b, &pObject->physics_joint1->field_0x14, &pPartner->cmpos);
    BrVector3Copy(&pForce->direction_a, pDirection);
    BrMatrix34ApplyV(&direction, &pForce->direction_a, &pObject->transform_matrix);
    BrMatrix34TApplyV(&pForce->direction_b, &direction, &pPartner->transform_matrix);
    pForce->type = 11;
}

// AllocateObjectWorkspace

// RedoQuickHingeVelocities

// GetJointForces

// GetUniversalRotationAngle

// GetObjectsParent

// GetJointLimits

// GetListJointLimits

// GetGroupJointLimits

// LimitOmega

// FUNCTION: CARMA2_HW 0x004c1840
void C2_HOOK_FASTCALL CapOmega(tPhysics_object* pObject) {
    br_scalar t;

    t = BrVector3LengthSquared(&pObject->omega) * 0.0016f;
    if (t > 2.4649f) {
        t = (float)sqrt(t) * 0.6369f;
        pObject->omega.v[0] = pObject->omega.v[0] / (2.f * t);
        pObject->omega.v[1] = pObject->omega.v[1] / (2.f * t);
        pObject->omega.v[2] = pObject->omega.v[2] / (2.f * t);
    }
}

// ListLimitOmega

// GroupLimitOmega

// PositionWorldJointedObject

// TranslateAndRotatePartialObject

// TranslateAndRotateJointedObject

// CountJointForces

// MoveJointedObject

// STUB: CARMA2_HW 0x004b7510
void C2_HOOK_FAKE_THISCALL MoveJointedObject(tPhysics_object* pObject, float pDelta_time) {
    NOT_IMPLEMENTED();
}

// TellPhysicsJointsHaveChanged

// CombineBounds

// GetWorldBoundingBoxForObjects

// GetWorldBoundingBoxForObject

// MatricesAreIdentical

// InternalPrepareObject

// PrepareObject

// PrepareForCollisions

// CheckPointVelocities

// FUNCTION: CARMA2_HW 0x004c2a10
// FUNCTION: CARMA2_SW 0x004bf140
void C2_HOOK_FASTCALL WriteNewPositions(tPhysics_object* pObject) {
    tPhysics_object* child;
    tPhysics_object* root;
    br_vector3 parent_point;
    br_vector3 child_point;

    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_object, field_0x1e8, 0x1e8);
    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_object, field_0x230, 0x230);

    root = pObject;
    child = pObject->child;
    if (pObject->flags & 0x400) {
        root = pObject->field_0x230;
    }
    for (; child != NULL; child = child->next) {
        if (!(pObject->flags & 0x400)) {
            BrMatrix34ApplyP(&parent_point, &child->physics_joint1->field_0x14, &pObject->actor->t.t.mat);
            BrMatrix34ApplyP(&child_point, &child->physics_joint1->field_0x08, &child->actor->t.t.mat);
            BrVector3Sub(&parent_point, &parent_point, &child_point);
            BrVector3Accumulate((br_vector3*)child->actor->t.t.mat.m[3], &parent_point);
        } else {
            BrMatrix34Mul(&pObject->actor->t.t.mat, &pObject->field_0x1e8, &root->actor->t.t.mat);
        }
        WriteNewPositions(child);
    }
}

// FUNCTION: CARMA2_HW 0x004b9eb0
// FUNCTION: CARMA2_SW 0x004b65d0
void C2_HOOK_FASTCALL SetListDoingNothingFlag(tPhysics_object* pObject, int pFlag) {
    while (pObject != NULL) {
        pObject->disable_move_rotate = pFlag;
        pObject->field_0x1df = 0;
        if (pObject->child != NULL) {
            SetListDoingNothingFlag(pObject->child, pFlag);
        }
        pObject = pObject->next;
    }
}

// SetGroupDoingNothingFlag

// FUNCTION: CARMA2_HW 0x004b9ef0
// FUNCTION: CARMA2_SW 0x004b6610
void C2_HOOK_FASTCALL SetHierarchyDoingNothingFlag(tPhysics_object* pObject, int pFlag) {
    pObject->disable_move_rotate = pFlag;
    pObject->field_0x1df = 0;
    if (pObject->child != NULL) {
        SetListDoingNothingFlag(pObject->child, pFlag);
    }
}

// CheckDoingNothingFlag

// FUNCTION: CARMA2_HW 0x004bae80
// FUNCTION: CARMA2_SW 0x004b75a0
void C2_HOOK_FASTCALL SetGroup(tPhysics_object* pObject, int pGroup) {
    tPhysics_object* child;

    child = pObject->child;
    pObject->field_0x1dc = (tU8)pGroup;
    for (; child != NULL; child = child->next) {
        if (child->physics_joint1 != NULL && child->physics_joint1->type != eJoint_none) {
            SetGroup(child, pGroup);
        }
    }
}

// GroupTogether

// GetPreCalculatedJointForces

// FUNCTION: CARMA2_HW 0x004b9f40
void C2_HOOK_FASTCALL MakeObjectListDoSomething(tPhysics_object* pObject) {
    while (pObject != NULL) {
        if (pObject->disable_move_rotate) {
            pObject->box_face_ref = gFace_num__car - 2;
            pObject->disable_move_rotate = 0;
        }
        if (pObject->child != NULL) {
            MakeObjectListDoSomething(pObject->child);
        }
        pObject = pObject->next;
    }
}

// CheckForObjectTouchingAnotherObject

// CheckForObjectListTouchingAnotherObject

// CheckForObjectHierachyTouchingAnotherObject

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
#if defined(REC2_MATCHING) && defined(_MSC_VER)
#pragma inline_depth(0)
#endif
// FUNCTION: CARMA2_HW 0x004ba190
void C2_HOOK_FASTCALL CheckForObjectHierachyTouchingObjectList(tPhysics_object* pObject, tPhysics_object* pList, tPhysics_object* pList_original) {
    tPhysics_object* list_cur;
    tPhysics_object* obj;
    tPhysics_object* child;

    for (list_cur = pList; list_cur != NULL; list_cur = list_cur->next) {
        if (list_cur->disable_move_rotate != 0) {
            if (pObject->field_0xf4.min.v[0] - list_cur->field_0xf4.max.v[0] <= gDOUBLE_0058a798
                    && pObject->field_0xf4.min.v[1] - list_cur->field_0xf4.max.v[1] <= gDOUBLE_0058a798
                    && pObject->field_0xf4.min.v[2] - list_cur->field_0xf4.max.v[2] <= gDOUBLE_0058a798
                    && list_cur->field_0xf4.min.v[0] - pObject->field_0xf4.max.v[0] <= gDOUBLE_0058a798
                    && list_cur->field_0xf4.min.v[1] - pObject->field_0xf4.max.v[1] <= gDOUBLE_0058a798
                    && list_cur->field_0xf4.min.v[2] - pObject->field_0xf4.max.v[2] <= gDOUBLE_0058a798) {
                obj = list_cur;
                while (obj->parent != NULL) {
                    if (obj->physics_joint1 == NULL || obj->physics_joint1->type == eJoint_none) {
                        break;
                    }
                    obj = obj->parent;
                }
                obj->box_face_ref = gFace_num__car - 2;
                obj->disable_move_rotate = 0;
                if (obj->child != NULL || obj->physics_joint1 != NULL || obj->physics_joint2 != NULL) {
                    if (obj->field_0x218 == 0) {
                        MoveJointedObject(obj, 0.f);
                    }
                    child = obj->child;
                    while (child != NULL) {
                        if (child->disable_move_rotate) {
                            child->box_face_ref = gFace_num__car - 2;
                            child->disable_move_rotate = 0;
                        }
                        if (child->child != NULL) {
                            MakeObjectListDoSomething(child->child);
                        }
                        child = child->next;
                    }
                }
                CheckForObjectHierachyTouchingObjectList(obj, pList_original, pList_original);
            }
            child = pObject->child;
            if (child != NULL && list_cur->disable_move_rotate != 0) {
                child = pObject->child;
                while (child != NULL) {
                    if (child->field_0xf4.min.v[0] - list_cur->field_0xf4.max.v[0] > gDOUBLE_0058a798
                            || child->field_0xf4.min.v[1] - list_cur->field_0xf4.max.v[1] > gDOUBLE_0058a798
                            || child->field_0xf4.min.v[2] - list_cur->field_0xf4.max.v[2] > gDOUBLE_0058a798
                            || list_cur->field_0xf4.min.v[0] - child->field_0xf4.max.v[0] > gDOUBLE_0058a798
                            || list_cur->field_0xf4.min.v[1] - child->field_0xf4.max.v[1] > gDOUBLE_0058a798
                            || list_cur->field_0xf4.min.v[2] - child->field_0xf4.max.v[2] > gDOUBLE_0058a798) {
                        child = child->next;
                        if (child == NULL) {
                            break;
                        }
                        continue;
                    }
                    obj = list_cur;
                    while (obj->parent != NULL) {
                        if (obj->physics_joint1 == NULL || obj->physics_joint1->type == eJoint_none) {
                            break;
                        }
                        obj = obj->parent;
                    }
                    obj->box_face_ref = gFace_num__car - 2;
                    child = obj->child;
                    obj->disable_move_rotate = 0;
                    if (child != NULL || obj->physics_joint1 != NULL || obj->physics_joint2 != NULL) {
                        if (obj->field_0x218 == 0) {
                            MoveJointedObject(obj, 0.f);
                        }
                        child = obj->child;
                        while (child != NULL) {
                            if (child->disable_move_rotate) {
                                child->box_face_ref = gFace_num__car - 2;
                                child->disable_move_rotate = 0;
                            }
                            if (child->child != NULL) {
                                MakeObjectListDoSomething(child->child);
                            }
                            child = child->next;
                        }
                    }
                    CheckForObjectHierachyTouchingObjectList(obj, pList_original, pList_original);
                    break;
                }
            }
            if (list_cur->child != NULL) {
                CheckForObjectHierachyTouchingObjectList(pObject, list_cur->child, pList_original);
            }
        }
    }
}
#if defined(REC2_MATCHING) && defined(_MSC_VER)
#pragma inline_depth()
#endif
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif

// MakeObjectDoSomething

// GetReducedList

// ReduceReducedList

// FUNCTION: CARMA2_HW 0x004bd9b0
// FUNCTION: CARMA2_SW 0x004ba0d0
void C2_HOOK_FASTCALL SetObjectTrialPositionType(tPhysics_object* pObject, int pType) {
    tPhysics_object* child;

    child = pObject->child;
    pObject->field_0xf0 = pType;
    for (; child != NULL; child = child->next) {
        SetObjectTrialPositionType(child, pType);
    }
}

// FillInTrialWorkspace

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif

// FUNCTION: CARMA2_HW 0x004ba4f0
// FUNCTION: CARMA2_SW 0x004b6c10
int C2_HOOK_FASTCALL FlagsAreGo(tPhysics_object* pCollision_1, tPhysics_object* pCollision_2) {
    int blocked;

    blocked = (pCollision_1->field_0x1a0 & pCollision_2->field_0x1a4)
        || (pCollision_2->field_0x1a0 & pCollision_1->field_0x1a4)
        || (pCollision_1->flags & 4)
        || (pCollision_2->flags & 4);
    return !blocked;
}

#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif

// CollideObjectWithReducedObjectList

// AddObjectToReducedList

// ReducedFindObjectCollisions

// FindObjectCollisions

// DoRigidJoints

// DoInterGroupCollisions

// FUNCTION: CARMA2_HW 0x004ba550
// FUNCTION: CARMA2_SW 0x004b6c70
int C2_HOOK_FASTCALL ScaleCollisionImpulses(tCollision_contact* pContacts, float* pImpulses, int pCount, int* pSmash_indices, int pNum_impacts, float pMax_impulse) {
    extern int gINT_00692dc8;
    extern br_scalar gFLOAT_0058a768;
    int i;
    float total;
    float scale;
    float f;

    total = gFLOAT_0058a768;
    for (i = 0; i < pNum_impacts; i++) {
        total += pImpulses[pSmash_indices[i]];
    }
    if (total <= pMax_impulse) {
        return 0;
    }
    scale = pMax_impulse / total;
    gINT_00692dc8 = 1;
    for (i = 0; i < pCount; i++) {
        pImpulses[i] = -pImpulses[i];
    }
    for (i = 0; i < pNum_impacts; i++) {
        f = pImpulses[pSmash_indices[i]];
        pImpulses[pSmash_indices[i]] = f * (0.f - scale);
        if (pContacts[pSmash_indices[i]].type & 0x100) {
            f = pContacts[pSmash_indices[i]].impulse1.v[0];
            pContacts[pSmash_indices[i]].impulse1.v[0] = f * scale;
            f = pContacts[pSmash_indices[i]].impulse1.v[1];
            pContacts[pSmash_indices[i]].impulse1.v[1] = f * scale;
            f = pContacts[pSmash_indices[i]].impulse1.v[2];
            pContacts[pSmash_indices[i]].impulse1.v[2] = f * scale;
            if (pContacts[pSmash_indices[i]].object2 != NULL) {
                f = pContacts[pSmash_indices[i]].impulse1.v[0];
                pContacts[pSmash_indices[i]].impulse1.v[0] = f * scale;
                f = pContacts[pSmash_indices[i]].impulse1.v[1];
                pContacts[pSmash_indices[i]].impulse1.v[1] = f * scale;
                f = pContacts[pSmash_indices[i]].impulse1.v[2];
                pContacts[pSmash_indices[i]].impulse1.v[2] = f * scale;
            }
        }
    }
    for (i = 0; i < pCount; i++) {
        if (pImpulses[i] < gFLOAT_0058a768) {
            pImpulses[i] = 0.f;
        }
        pContacts[i].type = 9;
    }
    return 1;
}

// DoCollisions

// FUNCTION: CARMA2_HW 0x004baa00
int C2_HOOK_FASTCALL AddDoubleTorqueToMatrix(tPhysics_object* pObject) {
    tPhysics_object* obj;
    int acc;
    int child_acc;
    float X;
    float Y;
    float Z;
    float eAng;
    float eLin;
    float len;
    float nx;
    float ny;
    float nz;
    float d1;
    float d2;
    float bx;
    float by;
    float bz;
    float cx;
    float cy;
    float cz;
    float vx;
    float vy;
    float vz;
    int i;
    int new1df;

    obj = pObject;
    acc = 1;
    if (obj != NULL) {
    for (; obj != NULL; obj = obj->next) {
        if (obj->field_0x21c != 1.0f) {
            obj->I.v[0] /= obj->field_0x21c;
            obj->I.v[1] /= obj->field_0x21c;
            obj->I.v[2] /= obj->field_0x21c;
        }
        if (obj->field_0xf0 != 0) {
            obj->field_0xf0 = 0;
            obj->disable_move_rotate = (tU8)(((tU32)obj->flags >> 5) & 1);
        }
        {
            tU32 f = obj->flags;
            f = (f & ~0x100) | ((f >> 1) & 0x100);
            obj->flags = f;
        }
        new1df = 0;
        if (obj->disable_move_rotate == 0) {
            if (obj->field_0x1df != 0 && obj->field183_0x1d8 == (int)obj) {
                obj->field_0x1df = 0;
            }
            X = ((br_vector3*)obj->field_0x1cc)->v[0] + obj->omega.v[0];
            Y = ((br_vector3*)obj->field_0x1cc)->v[1] + obj->omega.v[1];
            Z = ((br_vector3*)obj->field_0x1cc)->v[2] + obj->omega.v[2];
            eAng = (obj->I.v[0] * X * X + obj->I.v[1] * Y * Y + obj->I.v[2] * Z * Z) / obj->M;
            if (obj->collision_flag != 0 || obj->field_0x1df == 0) {
                eLin = obj->v.v[0] * obj->v.v[0] + obj->v.v[1] * obj->v.v[1] + obj->v.v[2] * obj->v.v[2];
            } else {
                eLin = gFLOAT_0058a768;
            }
            if (eLin < 0.001 && eAng < 0.00009) {
                len = (float)sqrt(obj->field_0x54.v[0] * obj->field_0x54.v[0] + obj->field_0x54.v[1] * obj->field_0x54.v[1] + obj->field_0x54.v[2] * obj->field_0x54.v[2]);
                nx = obj->field_0x54.v[0] / len;
                ny = obj->field_0x54.v[1] / len;
                nz = obj->field_0x54.v[2] / len;
                d1 = -(obj->v.v[0] * nx + obj->v.v[1] * ny + obj->v.v[2] * nz);
                d2 = -(((br_vector3*)obj->field_0x1b4)->v[0] * nx + ((br_vector3*)obj->field_0x1b4)->v[1] * ny + ((br_vector3*)obj->field_0x1b4)->v[2] * nz);
                if (obj->field_0x1df == 0) {
                    if (d1 < 0.012f && (d1 - d2) > 0.9f * len) {
                        bx = obj->omega.v[1] * obj->cmpos.v[2] - obj->omega.v[2] * obj->cmpos.v[1] + obj->velocity_car_space.v[0];
                        by = obj->omega.v[2] * obj->cmpos.v[0] - obj->omega.v[0] * obj->cmpos.v[2] + obj->velocity_car_space.v[1];
                        bz = obj->omega.v[0] * obj->cmpos.v[1] - obj->omega.v[1] * obj->cmpos.v[0] + obj->velocity_car_space.v[2];
                        for (i = 0; i < 8; i++) {
                            cx = (i & 2) ? obj->bb1.min.v[0] : obj->bb1.max.v[0];
                            cy = (i & 1) ? obj->bb1.min.v[1] : obj->bb1.max.v[1];
                            cz = (i & 4) ? obj->bb1.min.v[2] : obj->bb1.max.v[2];
                            vx = obj->omega.v[1] * cz - obj->omega.v[2] * cy + bx;
                            vy = obj->omega.v[2] * cx - obj->omega.v[0] * cz + by;
                            vz = obj->omega.v[0] * cy - obj->omega.v[1] * cx + bz;
                            if (vx * vx + vy * vy + vz * vz > 0.0004f) {
                                goto copy;
                            }
                        }
                    } else {
                        goto copy;
                    }
                }
                if (obj->physics_joint2 == NULL || obj->collision_flag != 0 || obj->transform_matrix.m[1][1] > 0.999) {
                    new1df = 1;
                    BrVector3SetFloat(&obj->v, 0.0f, 0.0f, 0.0f);
                    BrVector3SetFloat(&obj->omega, 0.0f, 0.0f, 0.0f);
                    BrVector3SetFloat(&obj->rotate_omega, 0.0f, 0.0f, 0.0f);
                }
            }
        }
copy:
        obj->field_0x1df = (tU8)new1df;
        BrMatrix34Copy(&obj->transform_matrix, &obj->actor->t.t.mat);
        BrMatrix34ApplyP(&obj->pos, &obj->cmpos, &obj->transform_matrix);
        if (obj->child != NULL) {
            child_acc = AddDoubleTorqueToMatrix(obj->child);
            if (obj->field_0x1df != 0) {
                obj->field_0x1df = (tU8)((child_acc << 1) + obj->field_0x1df);
            }
        } else {
            obj->field_0x1df = (tU8)(obj->field_0x1df * 3);
        }
        acc &= ((tU32)obj->field_0x1df >> 1) & 1;
    }
    }
    return acc;
}

// DRMatrix33Inverse

// SetUpQuickHingeData

// InitQuickHinge

// CalcQuickHingeData

// FUNCTION: CARMA2_HW 0x004be4b0
// FUNCTION: CARMA2_SW 0x004babd0
void C2_HOOK_FASTCALL SetUpForce(br_vector3* pAngular, br_vector3* pLinear, const br_vector3* pPoint, const br_vector3* pDirection, tPhysics_object* pObject) {
    br_scalar inverse_mass;
    int i;

    if (!(pObject->flags & 0x800)) {
        BrVector3Cross(pAngular, pPoint, pDirection);
        for (i = 0; i < 3; i++) {
            pAngular->v[i] /= pObject->I.v[i];
        }
    } else {
        BrVector3Set(pAngular, 0.0f, 0.0f, 0.f);
    }
    inverse_mass = (br_scalar)(1.0 / pObject->M);
    BrVector3Scale(pLinear, pDirection, inverse_mass);
}

// SetUpTorqueHingePartners

// SetUpHingePartners

// MakeMatrix

// FUNCTION: CARMA2_HW 0x004c03f0
void C2_HOOK_FASTCALL RestoreListOmega(tPhysics_object* pObject) {

    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_object, rotate_omega, 0x1c0);

    while (pObject != NULL) {
        BrVector3Copy(&pObject->omega, &pObject->rotate_omega);
        if (pObject->child != NULL) {
            RestoreListOmega(pObject->child);
        }
        pObject = pObject->next;
    }
}

// RestoreGroupOmegas

// MakeVelocitiesVector2

// MakeVelocitiesVector

// NewAddFrictionObjectObject

// FUNCTION: CARMA2_HW 0x004c0ac0
void C2_HOOK_FASTCALL NewAddFriction(tPhysics_joint_force* pContact, tWorld_callbacks* pWorld_callbacks) {
    tPhysics_object* pObject_a;
    tPhysics_object* pObject_b;
    br_vector3 velocity_a;
    br_vector3 velocity_b;
    br_vector3 world;
    br_vector3 normal;
    br_vector3 direction_a;
    br_vector3 direction_b;
    br_vector3 angular_a;
    br_vector3 angular_b;
    br_vector3 cross;
    br_vector3 lever;
    br_scalar coefficient;
    br_scalar maximum;
    br_scalar speed;
    br_scalar inverse_speed;
    br_scalar denominator;
    br_scalar impulse;
    br_scalar scale;
    br_scalar dot;
    br_scalar base;
    br_scalar tangent_x;
    br_scalar tangent_y;
    br_scalar tangent_z;

    pObject_a = pContact->object_a;
    pObject_b = pContact->object_b;

    if (pObject_b != NULL && pContact->type != 14) {
        coefficient = pContact->field_0x0;
        pObject_a->collision_flag |= 2;
        pObject_b->collision_flag |= 2;

        velocity_a.v[0] = pContact->point_a.v[2] * pObject_a->omega.v[1] - pContact->point_a.v[1] * pObject_a->omega.v[2] + pObject_a->velocity_car_space.v[0];
        velocity_a.v[1] = pObject_a->omega.v[2] * pContact->point_a.v[0] - pObject_a->omega.v[0] * pContact->point_a.v[2] + pObject_a->velocity_car_space.v[1];
        velocity_a.v[2] = pObject_a->omega.v[0] * pContact->point_a.v[1] - pObject_a->omega.v[1] * pContact->point_a.v[0] + pObject_a->velocity_car_space.v[2];

        velocity_b.v[0] = pObject_b->omega.v[1] * pContact->point_b.v[2] - pObject_b->omega.v[2] * pContact->point_b.v[1] + pObject_b->velocity_car_space.v[0];
        velocity_b.v[1] = pContact->point_b.v[0] * pObject_b->omega.v[2] - pObject_b->omega.v[0] * pContact->point_b.v[2] + pObject_b->velocity_car_space.v[1];
        velocity_b.v[2] = pObject_b->omega.v[0] * pContact->point_b.v[1] - pObject_b->omega.v[1] * pContact->point_b.v[0] + pObject_b->velocity_car_space.v[2];

        BrMatrix34ApplyV(&world, &velocity_b, &pObject_b->actor->t.t.mat);
        BrMatrix34TApplyV(&velocity_b, &world, &pObject_a->actor->t.t.mat);
        velocity_a.v[0] -= velocity_b.v[0];
        velocity_a.v[1] -= velocity_b.v[1];
        velocity_a.v[2] -= velocity_b.v[2];

        BrMatrix34ApplyV(&world, &pContact->direction_a, &pObject_a->transform_matrix);
        BrMatrix34TApplyV(&normal, &world, &pObject_a->actor->t.t.mat);
        dot = normal.v[2] * velocity_a.v[2] + normal.v[1] * velocity_a.v[1] + normal.v[0] * velocity_a.v[0];
        world.v[0] = normal.v[0] * dot;
        world.v[1] = normal.v[1] * dot;
        world.v[2] = normal.v[2] * dot;
        tangent_x = velocity_a.v[0] - world.v[0];
        tangent_y = velocity_a.v[1] - world.v[1];
        tangent_z = velocity_a.v[2] - world.v[2];
        speed = (br_scalar)sqrt(tangent_z * tangent_z + tangent_y * tangent_y + tangent_x * tangent_x);
        base = coefficient * 0.35f;
        maximum = pObject_a->object_friction * base + base;
        maximum = pObject_b->object_friction * maximum + maximum;

        if (speed >= 0.001f) {
            inverse_speed = 1.0f / -speed;
            direction_a.v[0] = inverse_speed * tangent_x;
            direction_a.v[1] = inverse_speed * tangent_y;
            direction_a.v[2] = inverse_speed * tangent_z;
            BrMatrix34ApplyV(&world, &direction_a, &pObject_a->actor->t.t.mat);
            BrMatrix34TApplyV(&direction_b, &world, &pObject_b->actor->t.t.mat);
            if (!(pObject_a->flags & 0x800)) {
                angular_a.v[0] = (pContact->point_a.v[1] * direction_a.v[2] - pContact->point_a.v[2] * direction_a.v[1]) / pObject_a->I.v[0];
                angular_a.v[1] = (pContact->point_a.v[2] * direction_a.v[0] - pContact->point_a.v[0] * direction_a.v[2]) / pObject_a->I.v[1];
                angular_a.v[2] = (pContact->point_a.v[0] * direction_a.v[1] - pContact->point_a.v[1] * direction_a.v[0]) / pObject_a->I.v[2];
            } else {
                angular_a.v[0] = 0.0f;
                angular_a.v[1] = 0.0f;
                angular_a.v[2] = 0.0f;
            }
            if (!(pObject_b->flags & 0x800)) {
                angular_b.v[0] = (pContact->point_b.v[1] * direction_b.v[2] - pContact->point_b.v[2] * direction_b.v[1]) / pObject_b->I.v[0];
                angular_b.v[1] = (pContact->point_b.v[2] * direction_b.v[0] - pContact->point_b.v[0] * direction_b.v[2]) / pObject_b->I.v[1];
                angular_b.v[2] = (pContact->point_b.v[0] * direction_b.v[1] - pContact->point_b.v[1] * direction_b.v[0]) / pObject_b->I.v[2];
            } else {
                angular_b.v[0] = 0.0f;
                angular_b.v[1] = 0.0f;
                angular_b.v[2] = 0.0f;
            }
            world.v[0] = pContact->point_b.v[2] * angular_b.v[1] - pContact->point_b.v[1] * angular_b.v[2];
            world.v[1] = pContact->point_b.v[0] * angular_b.v[2] - pContact->point_b.v[2] * angular_b.v[0];
            world.v[2] = pContact->point_b.v[1] * angular_b.v[0] - pContact->point_b.v[0] * angular_b.v[1];
            base = (pContact->point_a.v[0] * angular_a.v[2] - pContact->point_a.v[2] * angular_a.v[0]) * direction_a.v[1]
                + (pContact->point_a.v[2] * angular_a.v[1] - pContact->point_a.v[1] * angular_a.v[2]) * direction_a.v[0]
                + (pContact->point_a.v[1] * angular_a.v[0] - pContact->point_a.v[0] * angular_a.v[1]) * direction_a.v[2]
                + 1.0f / pObject_a->M + 1.0f / pObject_b->M;
            denominator = direction_b.v[0] * world.v[0] + direction_b.v[1] * world.v[1] + direction_b.v[2] * world.v[2] + base;
            if (0.0001f < fabs(denominator)) {
                impulse = speed / denominator;
                if (maximum < impulse) {
                    impulse = maximum;
                }
                scale = impulse / pObject_a->M;
                direction_a.v[0] = direction_a.v[0] * scale;
                direction_a.v[1] = direction_a.v[1] * scale;
                direction_a.v[2] = direction_a.v[2] * scale;
                scale = -(impulse / pObject_b->M);
                direction_b.v[0] = direction_b.v[0] * scale;
                direction_b.v[1] = direction_b.v[1] * scale;
                direction_b.v[2] = direction_b.v[2] * scale;
                pContact->field_0x44.v[0] = pObject_a->M * direction_a.v[0];
                pContact->field_0x44.v[1] = pObject_a->M * direction_a.v[1];
                pContact->field_0x44.v[2] = pObject_a->M * direction_a.v[2];
                pContact->field_0x50.v[0] = pObject_b->M * direction_b.v[0];
                pContact->field_0x50.v[1] = pObject_b->M * direction_b.v[1];
                pContact->field_0x50.v[2] = pObject_b->M * direction_b.v[2];
                pContact->type |= 0x100;
                pObject_a->omega.v[0] += impulse * angular_a.v[0];
                pObject_a->omega.v[1] += impulse * angular_a.v[1];
                pObject_a->omega.v[2] += impulse * angular_a.v[2];
                pObject_b->omega.v[0] += -impulse * angular_b.v[0];
                pObject_b->omega.v[1] += -impulse * angular_b.v[1];
                pObject_b->omega.v[2] += -impulse * angular_b.v[2];
                BrMatrix34ApplyV(&world, &direction_a, &pObject_a->actor->t.t.mat);
                pObject_a->v.v[0] += world.v[0];
                pObject_a->v.v[1] += world.v[1];
                pObject_a->v.v[2] += world.v[2];
                pObject_a->velocity_car_space.v[0] += direction_a.v[0];
                pObject_a->velocity_car_space.v[1] += direction_a.v[1];
                pObject_a->velocity_car_space.v[2] += direction_a.v[2];
                BrMatrix34ApplyV(&world, &direction_b, &pObject_b->actor->t.t.mat);
                pObject_b->v.v[0] += world.v[0];
                pObject_b->v.v[1] += world.v[1];
                pObject_b->v.v[2] += world.v[2];
                pObject_b->velocity_car_space.v[0] += direction_b.v[0];
                pObject_b->velocity_car_space.v[1] += direction_b.v[1];
                pObject_b->velocity_car_space.v[2] += direction_b.v[2];
            }
        }
        CapOmega(pObject_a);
        CapOmega(pObject_b);
        return;
    }

    coefficient = pContact->field_0x0;
    pObject_a->collision_flag |= 1;
    if (pContact->type == 1) {
        lever.v[0] = pContact->point_a.v[0] * -0.005f + pContact->point_a.v[0];
        lever.v[1] = pContact->point_a.v[1] * -0.005f + pContact->point_a.v[1];
        lever.v[2] = pContact->point_a.v[2] * -0.005f + pContact->point_a.v[2];
        velocity_a.v[0] = pObject_a->omega.v[1] * lever.v[2] - pObject_a->omega.v[2] * lever.v[1];
        velocity_a.v[1] = pObject_a->omega.v[2] * lever.v[0] - pObject_a->omega.v[0] * lever.v[2];
        velocity_a.v[2] = pObject_a->omega.v[0] * lever.v[1] - pObject_a->omega.v[1] * lever.v[0];
    } else {
        velocity_a.v[0] = pContact->point_a.v[2] * pObject_a->omega.v[1] - pObject_a->omega.v[2] * pContact->point_a.v[1];
        velocity_a.v[1] = pContact->point_a.v[0] * pObject_a->omega.v[2] - pObject_a->omega.v[0] * pContact->point_a.v[2];
        velocity_a.v[2] = pContact->point_a.v[1] * pObject_a->omega.v[0] - pObject_a->omega.v[1] * pContact->point_a.v[0];
    }
    velocity_a.v[0] += pObject_a->velocity_car_space.v[0];
    velocity_a.v[1] += pObject_a->velocity_car_space.v[1];
    velocity_a.v[2] += pObject_a->velocity_car_space.v[2];
    BrMatrix34ApplyV(&world, &pContact->direction_a, &pObject_a->transform_matrix);
    BrMatrix34TApplyV(&normal, &world, &pObject_a->actor->t.t.mat);
    dot = normal.v[2] * velocity_a.v[2] + normal.v[1] * velocity_a.v[1] + normal.v[0] * velocity_a.v[0];
    world.v[0] = dot * normal.v[0];
    world.v[1] = dot * normal.v[1];
    world.v[2] = dot * normal.v[2];
    velocity_a.v[0] -= world.v[0];
    velocity_a.v[1] -= world.v[1];
    velocity_a.v[2] -= world.v[2];
    if (pWorld_callbacks != NULL && pWorld_callbacks->get_friction_from_face != NULL && pContact->field_0xc != NULL) {
        coefficient = pWorld_callbacks->get_friction_from_face(pContact->field_0xc) * coefficient;
    }
    speed = (br_scalar)sqrt(velocity_a.v[2] * velocity_a.v[2] + velocity_a.v[1] * velocity_a.v[1] + velocity_a.v[0] * velocity_a.v[0]);
    base = coefficient * 0.35f;
    maximum = pObject_a->world_friction * base + base;
    if (speed >= 0.001f) {
        inverse_speed = 1.0f / -speed;
        direction_a.v[0] = inverse_speed * velocity_a.v[0];
        direction_a.v[1] = inverse_speed * velocity_a.v[1];
        direction_a.v[2] = inverse_speed * velocity_a.v[2];
        if (!(pObject_a->flags & 0x800)) {
            angular_a.v[0] = (pContact->point_a.v[1] * direction_a.v[2] - pContact->point_a.v[2] * direction_a.v[1]) / pObject_a->I.v[0];
            angular_a.v[1] = (pContact->point_a.v[2] * direction_a.v[0] - pContact->point_a.v[0] * direction_a.v[2]) / pObject_a->I.v[1];
            angular_a.v[2] = (pContact->point_a.v[0] * direction_a.v[1] - pContact->point_a.v[1] * direction_a.v[0]) / pObject_a->I.v[2];
        } else {
            angular_a.v[0] = 0.0f;
            angular_a.v[1] = 0.0f;
            angular_a.v[2] = 0.0f;
        }
        dot = pObject_a->omega.v[0] * pContact->direction_a.v[0] + pContact->direction_a.v[2] * pObject_a->omega.v[2] + pObject_a->omega.v[1] * pContact->direction_a.v[1];
        if (fabs(dot) < 0.1f
                && pObject_a->omega.v[0] * angular_a.v[0] + pObject_a->omega.v[2] * angular_a.v[2] + pObject_a->omega.v[1] * angular_a.v[1] < 0.0f
                && pObject_a->v.v[2] * pObject_a->v.v[2] + pObject_a->v.v[1] * pObject_a->v.v[1] + pObject_a->v.v[0] * pObject_a->v.v[0] < 0.001f) {
            world.v[0] = pContact->direction_a.v[0] * dot;
            world.v[1] = pContact->direction_a.v[1] * dot;
            world.v[2] = pContact->direction_a.v[2] * dot;
            pObject_a->omega.v[0] -= world.v[0];
            pObject_a->omega.v[1] -= world.v[1];
            pObject_a->omega.v[2] -= world.v[2];
            BrVector3SetFloat(&angular_a, 0.0f, 0.0f, 0.0f);
            direction_a.v[0] = pObject_a->velocity_car_space.v[0];
            direction_a.v[1] = pObject_a->velocity_car_space.v[1];
            direction_a.v[2] = pObject_a->velocity_car_space.v[2];
            speed = (br_scalar)sqrt(direction_a.v[2] * direction_a.v[2] + direction_a.v[1] * direction_a.v[1] + direction_a.v[0] * direction_a.v[0]);
            if (speed < 0.001f) {
                return;
            }
            inverse_speed = -speed;
            direction_a.v[0] = direction_a.v[0] / inverse_speed;
            direction_a.v[1] = direction_a.v[1] / inverse_speed;
            direction_a.v[2] = direction_a.v[2] / inverse_speed;
        }
        cross.v[0] = pContact->point_a.v[2] * angular_a.v[1] - pContact->point_a.v[1] * angular_a.v[2];
        cross.v[1] = pContact->point_a.v[0] * angular_a.v[2] - pContact->point_a.v[2] * angular_a.v[0];
        cross.v[2] = pContact->point_a.v[1] * angular_a.v[0] - pContact->point_a.v[0] * angular_a.v[1];
        denominator = direction_a.v[2] * cross.v[2] + direction_a.v[1] * cross.v[1] + direction_a.v[0] * cross.v[0] + 1.0f / pObject_a->M;
        if (0.0001f < fabs(denominator)) {
            impulse = speed / denominator;
            if (maximum < impulse) {
                impulse = maximum;
            }
            angular_a.v[0] = impulse * angular_a.v[0];
            angular_a.v[1] = impulse * angular_a.v[1];
            angular_a.v[2] = impulse * angular_a.v[2];
            scale = impulse / pObject_a->M;
            direction_a.v[0] = direction_a.v[0] * scale;
            direction_a.v[1] = direction_a.v[1] * scale;
            direction_a.v[2] = direction_a.v[2] * scale;
            pContact->field_0x44.v[0] = pObject_a->M * direction_a.v[0];
            pContact->field_0x44.v[1] = pObject_a->M * direction_a.v[1];
            pContact->field_0x44.v[2] = pObject_a->M * direction_a.v[2];
            pContact->type |= 0x100;
            pObject_a->omega.v[0] += angular_a.v[0];
            pObject_a->omega.v[1] += angular_a.v[1];
            pObject_a->omega.v[2] += angular_a.v[2];
            BrMatrix34ApplyV(&world, &direction_a, &pObject_a->actor->t.t.mat);
            pObject_a->v.v[0] += world.v[0];
            pObject_a->v.v[1] += world.v[1];
            pObject_a->v.v[2] += world.v[2];
            pObject_a->velocity_car_space.v[0] += direction_a.v[0];
            pObject_a->velocity_car_space.v[1] += direction_a.v[1];
            pObject_a->velocity_car_space.v[2] += direction_a.v[2];
            CapOmega(pObject_a);
        }
    }
}

// ApplyForces

// DoJointFriction

// ListDoJointFriction

// PostAddFriction

// RotateObjectSecondOrder

// FUNCTION: CARMA2_HW 0x004c1f10
void C2_HOOK_FAKE_THISCALL RotateObjectFirstOrder(tPhysics_object* pObject, undefined4 pUnused, br_scalar pTime) {
    br_matrix34 rotation;
    br_vector3 axis;
    br_vector3 momentum;
    br_vector3 rotated;
    br_scalar length;
    br_scalar angle;
    br_scalar m1;

    length = BrVector3Length(&pObject->omega);
    angle = length * pTime;
    if (angle < 0.0001) {
        return;
    }
    axis.v[0] = pObject->omega.v[0] / length;
    axis.v[1] = pObject->omega.v[1] / length;
    axis.v[2] = pObject->omega.v[2] / length;
    momentum.v[0] = pObject->I.v[0] * pObject->omega.v[0];
    m1 = pObject->omega.v[1];
    momentum.v[1] = m1 * pObject->I.v[1];
    momentum.v[2] = pObject->I.v[2] * pObject->omega.v[2];
    BrMatrix34Rotate(&rotation, BrRadianToAngle(angle), &axis);
    BrMatrix34TApplyV(&rotated, &momentum, &rotation);
    BrMatrix34PreTranslate(&rotation, -pObject->cmpos.v[0], -pObject->cmpos.v[1], -pObject->cmpos.v[2]);
    BrMatrix34PostTranslate(&rotation, pObject->cmpos.v[0], pObject->cmpos.v[1], pObject->cmpos.v[2]);
    BrMatrix34Pre(&pObject->actor->t.t.mat, &rotation);
    pObject->omega.v[0] = rotated.v[0] / pObject->I.v[0];
    pObject->omega.v[1] = rotated.v[1] / pObject->I.v[1];
    pObject->omega.v[2] = rotated.v[2] / pObject->I.v[2];
}

extern tNet_mode gNet_mode;

// FUNCTION: CARMA2_HW 0x004c1be0
void C2_HOOK_FAKE_THISCALL RotateObject(tPhysics_object* pObject, undefined4 pUnused, br_scalar pDelta_time) {
    br_matrix34 rotation;
    br_vector3 axis;
    br_vector3 momentum;
    br_vector3 rotated;
    tFixed_quat packed_rotation;
    br_vector3 translation;
    br_scalar rad_rate;
    br_scalar rad;
    br_scalar rad_squared;
    int steps;
    int i;

    (void)pUnused;
    pObject->rotate_omega.v[0] = pObject->omega.v[0];
    rad_squared = pDelta_time * pDelta_time * BrVector3LengthSquared(&pObject->omega);
    pObject->rotate_omega.v[1] = pObject->omega.v[1];
    pObject->rotate_omega.v[2] = pObject->omega.v[2];
    if (rad_squared < 0.0000001) {
        return;
    }
    if (rad_squared > 0.008f) {
        steps = (int)sqrt(rad_squared * 31.25f) + 1;
        pDelta_time /= steps;
        for (i = 0; i < steps && i < 400; i++) {
            rad_rate = BrVector3Length(&pObject->omega);
            axis.v[0] = pObject->omega.v[0] / rad_rate;
            axis.v[1] = pObject->omega.v[1] / rad_rate;
            axis.v[2] = pObject->omega.v[2] / rad_rate;
            momentum.v[0] = pObject->omega.v[0] * pObject->I.v[0];
            momentum.v[1] = pObject->omega.v[1] * pObject->I.v[1];
            momentum.v[2] = pObject->omega.v[2] * pObject->I.v[2];
            rad = rad_rate * pDelta_time;
            BrMatrix34Rotate(&rotation, (br_angle)(BrRadianToAngle(rad) >> 1), &axis);
            BrMatrix34TApplyV(&rotated, &momentum, &rotation);
            axis.v[0] = rotated.v[0] / pObject->I.v[0];
            axis.v[1] = rotated.v[1] / pObject->I.v[1];
            axis.v[2] = rotated.v[2] / pObject->I.v[2];
            rad_rate = BrVector3Length(&axis);
            axis.v[0] = axis.v[0] / rad_rate;
            axis.v[1] = axis.v[1] / rad_rate;
            axis.v[2] = axis.v[2] / rad_rate;
            rad = rad_rate * pDelta_time;
            BrMatrix34Rotate(&rotation, BrRadianToAngle(rad), &axis);
            BrMatrix34PreTranslate(&rotation, -pObject->cmpos.v[0], -pObject->cmpos.v[1], -pObject->cmpos.v[2]);
            BrMatrix34PostTranslate(&rotation, pObject->cmpos.v[0], pObject->cmpos.v[1], pObject->cmpos.v[2]);
            BrMatrix34Pre(&pObject->actor->t.t.mat, &rotation);
            BrMatrix34TApplyV(&rotated, &momentum, &rotation);
            pObject->omega.v[0] = rotated.v[0] / pObject->I.v[0];
            pObject->omega.v[1] = rotated.v[1] / pObject->I.v[1];
            pObject->omega.v[2] = rotated.v[2] / pObject->I.v[2];
        }
    } else {
        RotateObjectFirstOrder(pObject, pDelta_time, pDelta_time);
    }
    if (gNet_mode != 0) {
        GetOrientationFromMatrix(&packed_rotation, &pObject->actor->t.t.mat);
        translation.v[0] = pObject->actor->t.t.mat.m[3][0];
        translation.v[1] = pObject->actor->t.t.mat.m[3][1];
        translation.v[2] = pObject->actor->t.t.mat.m[3][2];
        GetMatrixFromOrientation(&pObject->actor->t.t.mat, packed_rotation.v);
        pObject->actor->t.t.mat.m[3][0] = translation.v[0];
        pObject->actor->t.t.mat.m[3][1] = translation.v[1];
        pObject->actor->t.t.mat.m[3][2] = translation.v[2];
    }
}

// FUNCTION: CARMA2_HW 0x004c18d0
void C2_HOOK_FASTCALL TranslateObject(tPhysics_object* pObject) {
    tPhysics_joint* joint;
    tPhysics_object* child;
    br_vector3 axis;
    br_vector3 tmp[3];
    br_scalar imp[3];
    br_scalar friction;
    br_scalar friction_sq;
    br_scalar sum_sq;
    br_scalar num;
    br_scalar denom;
    br_scalar scale;
    int i;

    joint = pObject->physics_joint1;
    if (joint != NULL && joint->friction != 0.0f) {
        if (pObject->field_0x230 != NULL) {
            br_vector3* src = (br_vector3*)&pObject->transform_matrix.m[0][0];
            float* impPtr = imp;
            br_vector3* tmpPtr = tmp;
            float* iPtr = &pObject->I.v[0];
            float* oPtr = &pObject->omega.v[0];
            int k = 3;
            friction = joint->friction * 0.04f;
            sum_sq = 0.0f;
            friction_sq = friction * friction;
            do {
                float t;
                float* tp = &tmpPtr->v[0];
                float* ap = &axis.v[0];
                float* ip = &pObject->field_0x230->I.v[0];
                int n = 3;
                BrMatrix34TApplyV(&axis, src, &pObject->field_0x230->transform_matrix);
                num = pObject->field_0x230->omega.v[0] * axis.v[0] + pObject->field_0x230->omega.v[2] * axis.v[2] + pObject->field_0x230->omega.v[1] * axis.v[1] - *oPtr;
                do {
                    *tp = *ap / *ip;
                    tp++;
                    ap++;
                    ip++;
                    n--;
                } while (n != 0);
                denom = 1.0 / *iPtr + (tmpPtr->v[0] * axis.v[0] + tmpPtr->v[2] * axis.v[2] + tmpPtr->v[1] * axis.v[1]);
                t = num / denom;
                sum_sq += t * t;
                *impPtr = t;
                src++;
                impPtr++;
                tmpPtr++;
                iPtr++;
                oPtr++;
                k--;
            } while (k != 0);
            if (friction_sq < sum_sq) {
                scale = sqrt(friction_sq / sum_sq);
                imp[0] *= scale;
                imp[1] *= scale;
                imp[2] *= scale;
            }
            for (i = 0; i < 3; i++) {
                pObject->omega.v[i] += imp[i] / pObject->I.v[i];
                pObject->field_0x230->omega.v[0] -= imp[i] * tmp[i].v[0];
                pObject->field_0x230->omega.v[1] -= imp[i] * tmp[i].v[1];
                pObject->field_0x230->omega.v[2] -= imp[i] * tmp[i].v[2];
            }
        }
    }

    joint = pObject->physics_joint2;
    if (joint != NULL && joint->friction != 0.0f) {
        float* b = &pObject->I.v[0];
        friction = joint->friction * 0.04f;
        friction_sq = friction * friction;
        sum_sq = 0.0f;
        for (i = 0; i < 3; i++) {
            float u = b[0] * b[27];
            sum_sq += u * u;
            imp[i] = u;
            b++;
        }
        if (sum_sq > friction_sq) {
            scale = sqrt(friction_sq / sum_sq);
            imp[0] *= scale;
            imp[1] *= scale;
            imp[2] *= scale;
        }
        for (i = 0; i < 3; i++) {
            pObject->omega.v[i] -= imp[i] / pObject->I.v[i];
        }
    }

    for (child = pObject->child; child != NULL; child = child->next) {
        if (child->disable_move_rotate == 0) {
            TranslateObject(child);
        }
    }
}

// GetFacesNearObject

// FUNCTION: CARMA2_HW 0x004c25a0
// FUNCTION: CARMA2_SW 0x004becd0
void C2_HOOK_FASTCALL ResetObjectAndChildren(tPhysics_object* pObject) {
    tPhysics_object* child;

    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_object, field_0xbc, 0xbc);
    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_object, field_0xf0, 0xf0);

    if (pObject->field_0xf0 == 1) {
        BrMatrix34Copy(&pObject->actor->t.t.mat, &pObject->field_0xbc);
    } else {
        BrMatrix34LPNormalise(&pObject->actor->t.t.mat, &pObject->transform_matrix);
    }
    for (child = pObject->child; child != NULL; child = child->next) {
        ResetObjectAndChildren(child);
    }
}

// FUNCTION: CARMA2_HW 0x004c2600
// FUNCTION: CARMA2_SW 0x004bed30
void C2_HOOK_FASTCALL ResetObjectList(tPhysics_object* pObjects) {

    while (pObjects != NULL) {
        ResetObjectAndChildren(pObjects);
        pObjects = pObjects->next;
    }
}

// RepositionObjectGroup

// SimpleRotate

// InterpolateSingleObject

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
#if defined(REC2_MATCHING) && defined(_MSC_VER)
#pragma inline_depth(0)
#endif
// FUNCTION: CARMA2_HW 0x004c27c0
void C2_HOOK_FAKE_THISCALL InterpolateObjects(tPhysics_object* pObject, float pDt) {
    br_vector3 omega_world;

    pObject->rotate_omega.v[0] = pObject->omega.v[0];
    pObject->rotate_omega.v[1] = pObject->omega.v[1];
    pObject->rotate_omega.v[2] = pObject->omega.v[2];
    BrMatrix34ApplyV(&omega_world, &pObject->omega, &pObject->actor->t.t.mat);
    InterpolateSingleObject(pObject, pDt);
    BrMatrix34TApplyV(&pObject->omega, &omega_world, &pObject->actor->t.t.mat);
}
#if defined(REC2_MATCHING) && defined(_MSC_VER)
#pragma inline_depth()
#endif
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif

// ApplyGravitySingleObject

// ApplyGravity

// SetGravityDeltaV

// TrialPositionChildren

// FUNCTION: CARMA2_HW 0x004c2b20
void C2_HOOK_FASTCALL InternalPositionChildren(tPhysics_object* pParent, tPhysics_object* pRoot) {
    tPhysics_object* child;
    br_vector3 parent_point;
    br_vector3 child_point;

    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_object, field_0x1e8, 0x1e8);

    child = pParent->child;
    if (!(pParent->flags & 0x400)) {
        pRoot = pParent;
    }
    for (; child != NULL; child = child->next) {
        if (!(pParent->flags & 0x400)) {
            BrMatrix34ApplyP(&parent_point, &child->physics_joint1->field_0x14, &pParent->transform_matrix);
            BrMatrix34ApplyP(&child_point, &child->physics_joint1->field_0x08, &child->transform_matrix);
            BrVector3Sub(&parent_point, &parent_point, &child_point);
            BrVector3Accumulate((br_vector3*)child->transform_matrix.m[3], &parent_point);
        } else {
            BrMatrix34Mul(&pParent->transform_matrix, &pParent->field_0x1e8, &pRoot->transform_matrix);
        }
        InternalPositionChildren(child, pRoot);
    }
}

// FUNCTION: CARMA2_HW 0x004c2b10
void C2_HOOK_FASTCALL PositionChildren(tPhysics_object* pCollision_info) {

    InternalPositionChildren(pCollision_info, pCollision_info);
}

// FUNCTION: CARMA2_HW 0x004c2c30
// FUNCTION: CARMA2_SW 0x004bf360
void C2_HOOK_FASTCALL ApplyTorque(tPhysics_object* pObject, const br_vector3* pTorque) {
    br_vector3 torque;
    br_vector3 inertia;

    BrVector3Scale(&torque, pTorque, 1.0f / (WORLD_SCALE * WORLD_SCALE));
    BrVector3Copy(&inertia, &pObject->I);
    pObject->omega.v[0] += torque.v[0] / inertia.v[0];
    pObject->omega.v[1] += torque.v[1] / inertia.v[1];
    pObject->omega.v[2] += torque.v[2] / inertia.v[2];
}

// ApplyDiminishingTorque

// OrderObjectListOnX

// SwapPair

// CheckObjectsPostionInList

// TestForWrongAnswer

// RQIterativeImprovement

// RQExpand

// RQSingleExpand

// FUNCTION: CARMA2_HW 0x004c5830
// FUNCTION: CARMA2_SW 0x004c1f60
int C2_HOOK_FASTCALL RQDcmpV(br_scalar* pMatrix, int pDim, br_scalar* pQ) {
    int i;
    int j;
    int k;
    int singular;
    br_scalar scale;
    br_scalar sum;
    br_scalar norm;
    br_scalar divisor;
    br_scalar matrix_sum;
    br_scalar orthogonal_sum;
    br_scalar tau;
    br_scalar tau2;
    br_scalar* row;

    /* Using householder transformations, apply RQ decomposition on pMatrix:
     * - pMatrix is in-place converted into a lower-triangular matrix
     * - pQ is a orthonormal matrix */

    singular = 0;

    /* pQ starts as a identity matrix */
    for (i = 0; i < pDim; i++) {
        for (j = 0; j < pDim; j++) {
            pQ[i * pDim + j] = 0.0f;
        }
        pQ[i * pDim + i] = 1.0f;
    }

    for (i = 0; i < pDim; i++) {
        // Rotate i-th upper-diagonal row onto base vector

        row = &pMatrix[i * pDim];
        scale = 0.0f;
        for (j = i; j < pDim; j++) {
            scale = (br_scalar)MAX(scale, fabs(row[j]));
        }
        if (scale == 0.0) {
            singular = 1;
            continue;
        }
        // Only normalize the upper-vector that must be rotated onto the base vector
        // This is done for numerical stability reasons.
        for (j = i; j < pDim; j++) {
            row[j] /= scale;
        }
        // Calculate norm of the upper-diagonal row
        sum = 0.0f;
        for (j = i; j < pDim; j++) {
            sum += row[j] * row[j];
        }
        // Apply Householder reflection on pQ and the row+matrix
        norm = (br_scalar)(row[i] > 0.0 ? fabs(sqrt(sum)) : -fabs(sqrt(sum)));
        divisor = norm * (norm + row[i]);
        row[i] += norm;
        for (j = 0; j <= i; j++) {
            sum = 0.0f;
            for (k = i; k < pDim; k++) {
                sum += row[k] * pQ[k * pDim + j];
            }
            tau = sum / divisor;
            for (k = i; k < pDim; k++) {
                pQ[k * pDim + j] -= tau * row[k];
            }
        }
        for (j = i + 1; j < pDim; j++) {
            matrix_sum = 0.0f;
            for (k = i; k < pDim; k++) {
                matrix_sum += pMatrix[j * pDim + k] * row[k];
            }
            tau2 = matrix_sum / divisor;
            orthogonal_sum = 0.0f;
            for (k = i; k < pDim; k++) {
                orthogonal_sum += pQ[k * pDim + j] * pMatrix[i * pDim + k];
            }
            tau = orthogonal_sum / divisor;
            for (k = i; k < pDim; k++) {
                pMatrix[j * pDim + k] -= tau2 * row[k];
                pQ[k * pDim + j] -= tau * row[k];
            }
        }
        row[i] = -(norm * scale);
        // Zero upper-dialog values so what remains is lower-diagonal
        for (j = i + 1; j < pDim; j++) {
            row[j] = 0.0f;
        }
    }
    return singular;
}

// MakeIdentity

// SparseRQDcmpV

// MakeMatrixFromIndices

// HasBeenDoneBefore

// RemoveRQSingularities

// AddToMatrix

// RQRemovedOneTooManyForces

// GetInverseM

// RQMatrixSolveNoNegatives

// RQMatrixSolveWithNegatives

// FUNCTION: CARMA2_HW 0x004c5b50
// FUNCTION: CARMA2_SW 0x004c2280
int C2_HOOK_FASTCALL MatrixProcess(br_scalar* pR, br_scalar* pQ, int pDim) {
    return RQDcmpV(pR, pDim, pQ);
}

// FUNCTION: CARMA2_HW 0x004c5b60
void C2_HOOK_FASTCALL RqBackSub(const br_scalar* pR, const br_scalar* pQ, const br_scalar* pRhs, br_scalar* pX, int pDim) {
    br_scalar intermediate[100];
    br_scalar sum;
    int i;
    int j;

    /* Solve Ax = b, with A represented by its RQ decomposition, where:
     * - R is a lower-diagonal matrix
     * - Q is an orthonormal matrix
     * with:
     * - all matrices (A, Q, R) are square of size N x N (with N = pDim)
     * - vectors x, t are of size N x 1
     * A, Q, R matrices are of size [pDim x pDim]
     */

    C2_HOOK_ASSERT(pDim > 0 && pDim <= (int)REC2_ASIZE(intermediate));

    /* Use back substitution to solve Ry = b, with R a lower-diagonal matrix */
    intermediate[0] = pRhs[0] / pR[0 * pDim + 0];             /*  T[0] = B[0] / R[0, 0] */
    for (i = 1; i < pDim; i++) {
        sum = 0.0f;
        for (j = 0; j < i; j++) {
            sum += intermediate[j] * pR[i * pDim + j];        /*  sum += T[j] * R[i, j] */
        }
        intermediate[i] = (pRhs[i] - sum) / pR[i * pDim + i]; /*  T[i] = (B[i] - sum) / R[i, i] */
    }
    /* Solve Qx = T, by multiplying both sides with Q transposed */
    for (i = 0; i < pDim; i++) {
        pX[i] = pQ[0 * pDim + i] * intermediate[0];           /*  X[i]  = Q[0, i] * T[0] */
        for (j = 1; j < pDim; j++) {
            pX[i] += intermediate[j] * pQ[j * pDim + i];      /*  X[i] += Q[j, i] * T[j] */
        }
    }
}

// FUNCTION: CARMA2_HW 0x004c5c70
void* C2_HOOK_FASTCALL GetPhysicsScratchPointer(int pSize) {

    if ((tU8*)gPhysics_scratch_ptr + pSize > (tU8*)gPhysics_other_buffer + gPhysics_other_buffer_capacity) {
        PhysicsError(ePhysicsError_OutOfMemory, NULL);
    }
    return gPhysics_scratch_ptr;
}

// FUNCTION: CARMA2_HW 0x004c5cb0
void C2_HOOK_FASTCALL IncreasePhysicsScratchPointer(int pSize) {

    if ((tU8*)gPhysics_scratch_ptr + pSize > (tU8*)gPhysics_other_buffer + gPhysics_other_buffer_capacity) {
        PhysicsError(ePhysicsError_OutOfMemory, NULL);
    }
    gPhysics_scratch_ptr = (tU8*)gPhysics_scratch_ptr + pSize;
}

// FUNCTION: CARMA2_HW 0x004c5d00
void C2_HOOK_FASTCALL ClearPhysicsScratchSpace(void) {

    gPhysics_scratch_ptr = gPhysics_other_buffer;
}

// FUNCTION: CARMA2_HW 0x004c5d10
// FUNCTION: CARMA2_SW 0x004c2440
tCollision_shape_sphere* C2_HOOK_FASTCALL AllocateShapeSphere(int pType) {
    tCollision_shape_sphere* result;

    result = BrMemAllocate(sizeof(tCollision_shape_sphere), (br_uint_8)pType);
    result->common.type = kCollisionShapeType_Sphere;
    return result;
}

// FUNCTION: CARMA2_HW 0x004c5d70
// FUNCTION: CARMA2_SW 0x004c24a0
tCollision_shape_polyhedron* C2_HOOK_FASTCALL AllocateShapePolyhedron(int pCount_points, int pType) {
    tCollision_shape_polyhedron* result;
    tU8* raw_memory;
    int count_edges;

    count_edges = (pCount_points - 2) * 3;
    result = BrMemAllocate(sizeof(tCollision_shape_polyhedron) + pCount_points * sizeof(br_vector3)
        + count_edges * sizeof(tPolyhedron_edge_indexes) + (pCount_points - 2) * 2 * sizeof(br_vector4), (br_uint_8)pType);
    raw_memory = (tU8*)result + sizeof(tCollision_shape_polyhedron);
    result->polyhedron.points = (br_vector3*)raw_memory;
    raw_memory += pCount_points * sizeof(br_vector3);
    result->polyhedron.edges = (tPolyhedron_edge_indexes*)raw_memory;
    raw_memory += count_edges * sizeof(tPolyhedron_edge_indexes);
    result->polyhedron.planes = (br_vector4*)raw_memory;
    result->polyhedron.count_points = 0;
    result->polyhedron.count_edges = 0;
    result->polyhedron.count_planes = 0;
    result->common.type = kCollisionShapeType_Polyhedron;
    return result;
}

// FUNCTION: CARMA2_HW 0x004c5dc0
tCollision_shape_wireframe* C2_HOOK_FASTCALL AllocateShapeWireFrame(int pCount_points, int pCount_lines, int pType) {
    tCollision_shape_wireframe* result;
    tU8* raw_memory;

    result = BrMemAllocate(sizeof(tCollision_shape_wireframe) + pCount_points * sizeof(br_vector3)
        + pCount_lines * sizeof(tPolyhedron_edge_indexes), (br_uint_8)pType);
    raw_memory = (tU8*)result + sizeof(tCollision_shape_wireframe);
    result->wireframe.points = (br_vector3*)raw_memory;
    raw_memory += pCount_points * sizeof(br_vector3);
    result->wireframe.count_lines = pCount_lines;
    result->wireframe.lines = (tPolyhedron_edge_indexes*)raw_memory;
    result->wireframe.count_points = pCount_points;
    result->common.type = kCollisionShapeType_Wireframe;
    return result;
}

// FUNCTION: CARMA2_HW 0x004c5e00
tCollision_shape_box* C2_HOOK_FASTCALL AllocateShapeBox(int pType) {
    tCollision_shape_box* result;

    result = BrMemAllocate(sizeof(tCollision_shape_box), (br_uint_8)pType);
    result->common.type = kCollisionShapeType_Box;
    return result;
}

// GetJointSize

// FUNCTION: CARMA2_HW 0x004c5e20
// FUNCTION: CARMA2_SW 0x004c2550
tPhysics_joint* C2_HOOK_FASTCALL AllocatePhysicsJoint(int pCount_limits, int pType) {
    return BrMemAllocate(offsetof(tPhysics_joint, limits) + pCount_limits * sizeof(tPhysics_joint_limit), (br_uint_8)pType);
}

// FUNCTION: CARMA2_HW 0x004c5e40
tPhysics_joint* C2_HOOK_FASTCALL ClonePhysicsJoint(const tPhysics_joint* pJoint, int pType) {
    size_t size;
    tPhysics_joint* clone;

    size = offsetof(tPhysics_joint, limits) + pJoint->count_limits * sizeof(tPhysics_joint_limit);
    clone = BrMemAllocate(size, (br_uint_8)pType);
    memcpy(clone, pJoint, size);
    return clone;
}

// FUNCTION: CARMA2_HW 0x004c5e70
// FUNCTION: CARMA2_SW 0x004c25a0
void C2_HOOK_FASTCALL FreeShape(void* pData) {
    BrMemFree(pData);
}

// FUNCTION: CARMA2_HW 0x004c5e80
// FUNCTION: CARMA2_SW 0x004c25b0
void C2_HOOK_FASTCALL FreeShapeList(tPhysics_shape *pShape) {

    if (pShape->common.next != NULL) {
        FreeShapeList(pShape->common.next);
    }
    BrMemFree(pShape);
}

// FUNCTION: CARMA2_HW 0x004c5ea0
// FUNCTION: CARMA2_SW 0x004c25d0
void C2_HOOK_FASTCALL FreePhysicsJoint(tPhysics_joint* pJoint) {

    BrMemFree(pJoint);
}

// FUNCTION: CARMA2_HW 0x004c5eb0
// FUNCTION: CARMA2_SW 0x004c25e0
void C2_HOOK_FASTCALL GetBoundsFromPointList(const br_vector3* pVertices, int pCount_vertices, br_bounds3* pBounds) {
    int i;

    BrVector3Copy(&pBounds->min, &pVertices[0]);
    BrVector3Copy(&pBounds->max, &pVertices[0]);
    for (i = 1; i < pCount_vertices; i++) {
        int j;

        for (j = 0; j < 3; j++) {
            if (pVertices[i].v[j] < pBounds->min.v[j]) {
                pBounds->min.v[j] = pVertices[i].v[j];
            } else if (pVertices[i].v[j] > pBounds->max.v[j]) {
                pBounds->max.v[j] = pVertices[i].v[j];
            }
        }
    }
}

// FUNCTION: CARMA2_HW 0x004c5f20
// FUNCTION: CARMA2_SW 0x004c2650
void C2_HOOK_FASTCALL FillInShape(tPhysics_shape* pShape) {
    int i;
    tPhysicsError error;

    C2_HOOK_BUG_ON(REC2_ASIZE(pShape->common.bb.min.v) != 3);

    switch (pShape->common.type) {
    case kCollisionShapeType_Box:
        break;
    case kCollisionShapeType_Wireframe:
        if (pShape->wireframe.wireframe.count_points == 0) {
            PhysicsError(ePhysicsError_WireFrameHasNoPoints, NULL);
        } else {
            GetBoundsFromPointList(pShape->wireframe.wireframe.points, pShape->wireframe.wireframe.count_points, &pShape->common.bb);
        }
        break;
    case kCollisionShapeType_Polyhedron:
        if (pShape->polyhedron.polyhedron.count_points < 4) {
            PhysicsError(ePhysicsError_PolyhedronHasNoPoints, NULL);
        } else {
            error = ConvexHull3D(&pShape->polyhedron.polyhedron);
            if (error != ePhysicsError_Ok) {
                if (error == ePhysicsError_WireFrameHasNoPoints) {
                    pShape->common.type = kCollisionShapeType_Wireframe_Polyhedron;
                } else {
                    PDFatalError("This polyhedron is too wierd for me");
                }
            }
            GetBoundsFromPointList(pShape->polyhedron.polyhedron.points, pShape->polyhedron.polyhedron.count_points, &pShape->common.bb);
        }
        break;
    case kCollisionShapeType_Wireframe_Polyhedron:
        if (pShape->polyhedron.polyhedron.count_points < 3) {
            PhysicsError(4, NULL);
        } else {
            ConvexHull2D(&pShape->polyhedron.polyhedron);
            GetBoundsFromPointList(pShape->polyhedron.polyhedron.points, pShape->polyhedron.polyhedron.count_points, &pShape->common.bb);
        }
        break;
    case kCollisionShapeType_Sphere:
        BrVector3Copy(&pShape->common.bb.min, &pShape->sphere.sphere.center);
        BrVector3Copy(&pShape->common.bb.max, &pShape->sphere.sphere.center);
        for (i = 0; i < REC2_ASIZE(pShape->common.bb.min.v); i++) {
            pShape->common.bb.min.v[i] -= pShape->sphere.sphere.radius;
            pShape->common.bb.max.v[i] += pShape->sphere.sphere.radius;
        }
        pShape->sphere.sphere.radius_squared = REC2_SQR(pShape->sphere.sphere.radius);
        break;
    default:
        PhysicsError(ePhysicsError_UnknownShapeType, NULL);
        break;
    }
}

// UpdateCollisionObject

// FUNCTION: CARMA2_HW 0x004c61c0
// FUNCTION: CARMA2_SW 0x004c28f0
void C2_NORETURN C2_HOOK_FASTCALL PhysicsError(tPhysicsError pError, const char* pMessage) {

    if (gPhysics_error_cb != NULL) {
        gPhysics_error_cb(pError, pMessage);
    }
    exit(pError);
}

// FUNCTION: CARMA2_HW 0x004c61e0
void C2_HOOK_FASTCALL PhysicsWarning(const char* pMessage) {
    (void)pMessage;
}

// FUNCTION: CARMA2_HW 0x004c2910
// FUNCTION: CARMA2_SW 0x004c2910
void C2_HOOK_FASTCALL PhysicsObjectMoveVelocity(tPhysics_object* pObject) {
    tPhysics_object* pChild;

    if (!pObject->disable_move_rotate) {
        pObject->v.v[0] += pObject->field_0x54.v[0];
        pObject->v.v[1] += pObject->field_0x54.v[1];
        pObject->v.v[2] += pObject->field_0x54.v[2];
    }
    if (pObject->child != NULL) {
        for (pChild = pObject->child; pChild != NULL; pChild = pChild->next) {
            PhysicsObjectMoveVelocity(pChild);
        }
    }
}

// FUNCTION: CARMA2_HW 0x004c61f0
// FUNCTION: CARMA2_SW 0x004c2920
void C2_HOOK_FASTCALL PhysicsSetErrorHandler(tPhysicsError_cbfn* pError_cbfn) {

    gPhysics_error_cb = pError_cbfn;
}

// FUNCTION: CARMA2_HW 0x004c62e0
// FUNCTION: CARMA2_SW 0x004c2a10
void C2_HOOK_FASTCALL RepositionAndOrientateChildren(tPhysics_object* pObject, br_matrix34* pParent_matrix) {
    br_matrix34 inverse;
    br_matrix34 parent_matrix;
    br_matrix34 transform;
    tPhysics_object* child;

    child = pObject->child;
    if (pParent_matrix != NULL) {
        BrMatrix34Mul(&pObject->actor->t.t.mat, &pObject->transform_matrix, pParent_matrix);
    } else {
        BrMatrix34LPInverse(&inverse, &pObject->transform_matrix);
        BrMatrix34Mul(&transform, &inverse, &pObject->actor->t.t.mat);
        BrMatrix34LPNormalise(&parent_matrix, &transform);
        pParent_matrix = &parent_matrix;
    }
    if (child != NULL) {
        BrMatrix34LPInverse(&inverse, &pObject->transform_matrix);
        BrMatrix34Mul(&parent_matrix, &inverse, &pObject->actor->t.t.mat);
        for (; child != NULL; child = child->next) {
            RepositionAndOrientateChildren(child, pParent_matrix);
        }
    }
    BrMatrix34Copy(&pObject->transform_matrix, &pObject->actor->t.t.mat);
}

// FUNCTION: CARMA2_HW 0x004c63b0
// FUNCTION: CARMA2_SW 0x004c2ae0
void C2_HOOK_FASTCALL PhysicsAddObject(tPhysics_object* pParent, tPhysics_object* pChild) {

    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_object, next, 0x220);
    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_object, child, 0x224);
    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_object, parent, 0x228);

    pChild->next = pParent->child;
    pParent->child = pChild;
    pChild->parent = pParent;
}

// FUNCTION: CARMA2_HW 0x004c63d0
// FUNCTION: CARMA2_SW 0x004c2b00
void C2_HOOK_FASTCALL PhysicsAddObjectList(tPhysics_object* pParent, tPhysics_object* pChild) {
    tPhysics_object* last;

    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_object, next, 0x220);
    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_object, child, 0x224);
    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tPhysics_object, parent, 0x228);

    pChild->parent = pParent;
    last = pChild;
    while (last->next != NULL) {
        last = last->next;
        last->parent = pParent;
    }
    last->next = pParent->child;
    pParent->child = pChild;
}

// FUNCTION: CARMA2_HW 0x004c6410
// FUNCTION: CARMA2_SW 0x004c2b40
void C2_HOOK_FASTCALL PhysicsRemoveObject(tPhysics_object* pObject) {
    tPhysics_object* parent;
    tPhysics_object* sibling;

    parent = pObject->parent;
    sibling = parent->child;
    if (sibling == pObject) {
        parent->child = pObject->next;
    } else {
        while (sibling->next != pObject) {
            sibling = sibling->next;
        }
        sibling->next = pObject->next;
    }
    pObject->parent = NULL;
    pObject->next = NULL;
}

// FUNCTION: CARMA2_HW 0x004c6470
// FUNCTION: CARMA2_SW 0x004c2ba0
int C2_HOOK_FASTCALL PhysicsObjectRecurse(tPhysics_object* pObject, tEnumCollision_cbfn* pCallback, void* pContext) {
    int result;
    tPhysics_object* child;

    result = pCallback(pObject, pContext);
    if (result != 0) {
        return result;
    }
    for (child = pObject->child; child != NULL; child = child->next) {
        result = PhysicsObjectRecurse(child, pCallback, pContext);
        if (result != 0) {
            return result;
        }
    }
    return 0;
}

// FUNCTION: CARMA2_HW 0x004c64b0
int C2_HOOK_FASTCALL PhysicsObjectRecurseChildren(tPhysics_object* pObject, tEnumCollision_cbfn* pCallback, void* pContext) {
    tPhysics_object* child;

    for (child = pObject->child; child != NULL; child = child->next) {
        int result;

        result = PhysicsObjectRecurse(child, pCallback, pContext);
        if (result != 0) {
            return result;
        }
    }
    return 0;
}

// FUNCTION: CARMA2_HW 0x004c6510
// FUNCTION: CARMA2_SW 0x004c2c40
void C2_HOOK_FASTCALL InitPhysicsWorkspace(tU8* pBuffer, int pSize) {

    gPhysics_buffer1_ptr = pBuffer;
    pSize -= 3 * PHYSICS_BUFFER_PART_SIZE;
    gPhysics_buffer2_ptr = pBuffer + PHYSICS_BUFFER_PART_SIZE;
    gPhysics_buffer3_ptr = pBuffer + 2 * PHYSICS_BUFFER_PART_SIZE;
    pBuffer += 3 * PHYSICS_BUFFER_PART_SIZE;
    if (pSize < PHYSICS_BUFFER_OTHER_SIZE) {
        PhysicsError(ePhysicsError_InsufficientSizedBuffer, "Physics workspace too small");
    }
    gPhysics_other_buffer_capacity = pSize;
    gPhysics_other_buffer = pBuffer;
}

// FUNCTION: CARMA2_HW 0x004b57d0
int C2_HOOK_FASTCALL FindFacesInBox(tBounds* pBounds, tFace_ref* pFace_list, int pMax_face, tWorld_callbacks* pWorld_callbacks) {
    tU8 cx_min;
    tU8 cz_min;
    tU8 cx_max;
    tU8 cz_max;
    int x;
    int z;
    int emitted;
    tTrack_spec* track_spec;

    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tTrack_spec, ncolumns_x, 0x0);
    C2_HOOK_STATIC_ASSERT_STRUCT_OFFSET(tTrack_spec, ncolumns_z, 0x1);

    emitted = 0;
    track_spec = &gProgram_state.track_spec;
    FillInBounds(pBounds);
    XZToColumnXZ(&cx_min, &cz_min, pBounds->real_bounds.min.v[0], pBounds->real_bounds.min.v[2], track_spec);
    XZToColumnXZ(&cx_max, &cz_max, pBounds->real_bounds.max.v[0], pBounds->real_bounds.max.v[2], track_spec);
    if (cx_min > 0) {
        cx_min--;
    }
    if (cz_min > 0) {
        cz_min--;
    }
    if (cx_max + 1 < track_spec->ncolumns_x) {
        cx_max++;
    }
    if (cz_max + 1 < track_spec->ncolumns_z) {
        cz_max++;
    }
    for (x = cx_min; x <= cx_max; x++) {
        for (z = cz_min; z <= cz_max; z++) {
            if (track_spec->columns[z][x].actor_0x0 != NULL) {
                emitted = pMax_face - ActorBoxPick(pBounds, track_spec->columns[z][x].actor_0x0, NULL, gBlack_material, &pFace_list[emitted], pMax_face - emitted, NULL, pWorld_callbacks);
            }
        }
    }
    if (gAdditional_actors != NULL) {
        emitted = pMax_face - ActorBoxPick(pBounds, gAdditional_actors, NULL, gBlack_material, &pFace_list[emitted], pMax_face - emitted, NULL, pWorld_callbacks);
    }
    return emitted;
}

// FUNCTION: CARMA2_HW 0x004b5970
float C2_HOOK_FASTCALL GetFrictionFromFace(const tFace_ref* pFace) {
    int material_index;

    C2_HOOK_BUG_ON(REC2_ASIZE(gCurrent_race.material_modifiers) != 11);

    material_index = pFace->material->identifier[0] - '/';
    if (material_index < 0 || material_index >= (int)REC2_ASIZE(gCurrent_race.material_modifiers)) {
        material_index = 0;
    }
    return gCurrent_race.material_modifiers[material_index].car_wall_friction;
}

// FUNCTION: CARMA2_HW 0x004b5990
// FUNCTION: CARMA2_SW 0x004b20b0
void C2_HOOK_FASTCALL DoPhysicsError(tPhysicsError pError, const char* pMessage) {
    char s[256];

    (void)pMessage;
    switch (pError) {
    case ePhysicsError_UnknownShapeType:
        strcpy(s, "Unknown shape type");
        break;
    case ePhysicsError_WireFrameHasNoPoints:
        strcpy(s, "Wire frame shape has no points");
        break;
    case ePhysicsError_PolyhedronHasNoPoints:
        strcpy(s, "Polyhedron shape has no points");
        break;
    case ePhysicsError_UnknownHingeType:
        strcpy(s, "Unknown hinge type");
        /* The original overwrites this message with the code. */
#ifndef REC2_MATCHING
        break;
#endif
        /* fall through */
    default:
        sprintf(s, "%d ", pError);
        break;
    }
    FatalError(1000 + pError, s);
}

// FUNCTION: CARMA2_HW 0x004b5c00
void C2_HOOK_FASTCALL ResetMan2(tPhysics_object* pObject) {

    while (pObject != NULL) {
        BrMatrix34Translate(&pObject->transform_matrix, 0.f, 0.f, 0.f);
        BrVector3SetFloat(&pObject->v, 0.f, 0.f, 0.f);
        BrVector3SetFloat(&pObject->omega, 0.f, 0.f, 0.f);
        BrVector3SetFloat(&pObject->rotate_omega, 0.f, 0.f, 0.f);
        BrVector3SetFloat(&pObject->velocity_car_space, 0.f, 0.f, 0.f);
        pObject->disable_move_rotate = 1;
        if (pObject->child != NULL) {
            ResetMan2(pObject->child);
        }
        pObject = pObject->next;
    }
}

// FUNCTION: CARMA2_HW 0x004b5aa0
void C2_HOOK_FASTCALL SetUpTestObjects(void) {

}

// ResetObjectList

// ResetMan

#if defined(REC2_MATCHING) && defined(_MSC_VER)
#pragma inline_depth(0)
#endif

// FUNCTION: CARMA2_HW 0x004b5ca0
// FUNCTION: CARMA2_SW 0x004b23c0
void C2_HOOK_FASTCALL InitPhysics(void) {

    PhysicsSetErrorHandler(DoPhysicsError);
    InitPhysicsWorkspace(gTemporary_physics_render_buffer, sizeof(gTemporary_physics_render_buffer));
}

#if defined(REC2_MATCHING) && defined(_MSC_VER)
#pragma inline_depth()
#endif

// PointOutOfSightNotAR

// CalcCollObjIDFromCarSpec

// CalcCollObjID

// GetCarSpecFromCollObjID

// GetCollObjFromID


// STUB: CARMA2_HW 0x004c63d0
void C2_HOOK_FASTCALL AddCollisionInfoChild(tPhysics_object* pParent, tPhysics_object* pChild) {
    NOT_IMPLEMENTED();
}

// STUB: CARMA2_HW 0x004b9ef0
void C2_HOOK_FASTCALL SetCollisionInfoChildsDoNothing(tPhysics_object *pCollision_info, tU32 pDisable) {
    NOT_IMPLEMENTED();
}

// STUB: CARMA2_HW 0x004c2060
void C2_HOOK_FASTCALL GetFacesInBox(tPhysics_object* pCollision, tWorld_callbacks* pWorld_callbacks) {
    NOT_IMPLEMENTED();
}

// STUB: CARMA2_HW 0x004c2970
void C2_HOOK_FASTCALL PhysicsObjectMoveVelocityList(tPhysics_object* pObject) {
    NOT_IMPLEMENTED();
}

// GLOBAL: CARMA2_HW 0x006793d0
int gUNK_006793d0;

// STUB: CARMA2_HW 0x004b9e40
void C2_HOOK_FASTCALL SetCollisionInfoDoNothingChain(tPhysics_object* pCollision_info, tU32 pDisable) {
    NOT_IMPLEMENTED();
// FUNCTION: CARMA2_HW 0x00421fd0
tCollision_shape_polyhedron_data* C2_HOOK_FASTCALL FUN_00421fd0(int pCount_points) {
    tCollision_shape_polyhedron_data* result;
    int count_minus_two;
    int count_plus_two;
    int count_edges;

    count_minus_two = pCount_points - 2;
    count_plus_two = pCount_points + 2;
    count_edges = count_minus_two * 3;
    result = BrMemAllocate(2 * (count_edges + count_minus_two * 16 + count_plus_two * 6), 0x80);
    if (result == NULL) {
        return NULL;
    }
    result->points = (br_vector3*)((tU8*)result + sizeof(*result));
    result->count_points = 0;
    result->count_edges = 0;
    result->edges = (tPolyhedron_edge_indexes*)(result->points + pCount_points);
    result->count_planes = 0;
    result->planes = (br_vector4*)(result->edges + count_edges);
    return result;
}


#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
// FUNCTION: CARMA2_HW 0x00422030
int C2_HOOK_FASTCALL FUN_00422030(br_model* pModel, int pCount_vertices, int* pVertex_indices) {
    float* distances;
    int removed[512];
    int count_vertices;
    int distance_offset;
    int remove_count;
    int i;

    count_vertices = pModel->nvertices;
    if (count_vertices > 512) {
        count_vertices = 512;
    }
    if (count_vertices <= pCount_vertices) {
        return 3;
    }
    distances = BrMemAllocate(count_vertices * count_vertices / 2 * sizeof(float), 0x80);
    if (distances == NULL) {
        return 4;
    }

    distance_offset = 0;
    i = 0;
    if (count_vertices != 0) {
        int vertex_offset;

        vertex_offset = 0;
        do {
            if (vertex_offset > 0) {
                float* row;
                int j;
                int inner_offset;

                row = distances + distance_offset;
                distance_offset += i;
                j = i;
                inner_offset = 0;
                do {
                    row[0] =
                        (*(float*)((char*)pModel->vertices + vertex_offset + 8) - *(float*)((char*)pModel->vertices + inner_offset + 8)) * (*(float*)((char*)pModel->vertices + vertex_offset + 8) - *(float*)((char*)pModel->vertices + inner_offset + 8))
                        + (*(float*)((char*)pModel->vertices + vertex_offset + 4) - *(float*)((char*)pModel->vertices + inner_offset + 4)) * (*(float*)((char*)pModel->vertices + vertex_offset + 4) - *(float*)((char*)pModel->vertices + inner_offset + 4))
                        + (*(float*)((char*)pModel->vertices + vertex_offset) - *(float*)((char*)pModel->vertices + inner_offset)) * (*(float*)((char*)pModel->vertices + vertex_offset) - *(float*)((char*)pModel->vertices + inner_offset));
                    row++;
                    inner_offset += 0x28;
                    j--;
                } while (j != 0);
            }
            i++;
            vertex_offset += 0x28;
        } while (i < count_vertices);
    }

    memset(removed, 0, count_vertices * sizeof(removed[0]));
    remove_count = count_vertices - pCount_vertices;
    do {
        int index1;
        int index2;
        float min_distance;
        float sum1;
        float sum2;

        min_distance = 3.4028235e+38f;
        for (i = 0; i < count_vertices; i++) {
            if (removed[i] == 0) {
                int j;

                for (j = 0; j < i; j++) {
                    if (removed[j] == 0) {
                        int distance_index;
                        float distance;

                        if (j < i) {
                            distance_index = ((i - 1) * i >> 1) + j;
                        } else {
                            distance_index = ((j - 1) * j >> 1) + i;
                        }
                        distance = distances[distance_index];
                        if (distance < min_distance) {
                            index1 = i;
                            index2 = j;
                            min_distance = distance;
                        }
                    }
                }
            }
        }

        sum1 = 0.0f;
        sum2 = 0.0f;
        for (i = 0; i < count_vertices; i++) {
            if (removed[i] == 0 && i != index1 && i != index2) {
                int distance_index;

                if (i < index1) {
                    distance_index = ((index1 - 1) * index1 >> 1) + i;
                } else {
                    distance_index = ((i - 1) * i >> 1) + index1;
                }
                sum1 = (float)sqrt(distances[distance_index]) + sum1;

                if (i < index2) {
                    distance_index = ((index2 - 1) * index2 >> 1) + i;
                } else {
                    distance_index = ((i - 1) * i >> 1) + index2;
                }
                sum2 = (float)sqrt(distances[distance_index]) + sum2;
            }
        }
        if (sum1 >= sum2) {
            removed[index2] = 1;
        } else {
            removed[index1] = 1;
        }
        remove_count--;
    } while (remove_count != 0);

    for (i = 0; i < count_vertices; i++) {
        if (removed[i] == 0) {
            *pVertex_indices = i;
            pVertex_indices++;
        }
    }
    BrMemFree(distances);
    return 0;
}
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif

// FUNCTION: CARMA2_HW 0x00422400
tPhysicsError C2_HOOK_FASTCALL FUN_00422400(tCollision_shape_polyhedron_data* pPolyhedron, int* pVertex_indices, br_model* pModel, int pCount_vertices) {
    int i;
    int result;

    if (pModel->nvertices > pCount_vertices) {
        result = FUN_00422030(pModel, pCount_vertices, pVertex_indices);
        if (result != ePhysicsError_Ok) {
            return result;
        }
        pPolyhedron->count_points = pCount_vertices;
        if (pCount_vertices > 0) {
            for (i = 0; i < pCount_vertices; i++) {
                pPolyhedron->points[i].v[0] = pModel->vertices[pVertex_indices[i]].p.v[0];
                pPolyhedron->points[i].v[1] = pModel->vertices[pVertex_indices[i]].p.v[1];
                pPolyhedron->points[i].v[2] = pModel->vertices[pVertex_indices[i]].p.v[2];
            }
            return ConvexHull3D(pPolyhedron);
        }
    } else {
        pPolyhedron->count_points = pModel->nvertices;
        for (i = 0; i < pModel->nvertices; i++) {
            pVertex_indices[i] = i;
            pPolyhedron->points[i].v[0] = pModel->vertices[i].p.v[0];
            pPolyhedron->points[i].v[1] = pModel->vertices[i].p.v[1];
            pPolyhedron->points[i].v[2] = pModel->vertices[i].p.v[2];
        }
    }
    return ConvexHull3D(pPolyhedron);
}


// FUNCTION: CARMA2_HW 0x00422500
int C2_HOOK_FASTCALL FUN_00422500(tCollision_shape_polyhedron_data* pPolyhedron, br_model* pModel, int pMin_point_count, int pMax_point_count, int* pVertex_indices) {
    int result;
    int count_points;
    int point_count;

    if (pModel == NULL) {
        return 5;
    }
    if (pMax_point_count < 3) {
        return 6;
    }
    if (pMax_point_count > (int)pModel->nvertices) {
        pMax_point_count = pModel->nvertices;
    }
    if (pMin_point_count < 3) {
        pMin_point_count = 3;
    } else if (pMin_point_count > pMax_point_count) {
        pMin_point_count = pMax_point_count;
    }
    point_count = pMax_point_count;
    result = FUN_00422400(pPolyhedron, pVertex_indices, pModel, point_count);
    count_points = pPolyhedron->count_points;
    while (count_points != 0
            && ((result == 0 || result == 2)
                && (point_count = point_count * pMin_point_count / count_points, count_points != pMin_point_count))
            && point_count < pMax_point_count) {
        result = FUN_00422400(pPolyhedron, pVertex_indices, pModel, point_count);
        count_points = pPolyhedron->count_points;
    }
    return result;
}


#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
#if defined(REC2_MATCHING) && defined(_MSC_VER)
#pragma inline_depth(0)
#endif
// FUNCTION: CARMA2_HW 0x004b73a0
void C2_HOOK_FASTCALL FUN_004b73a0(tPhysics_object* pObject, const br_matrix34* pParent_matrix, const br_matrix34* pRoot_matrix) {
    br_matrix34 inverse;
    br_matrix34 normalised;
    br_vector3 transformed;
    tPhysics_joint* joint;
    int i;

    for (; pObject != NULL; pObject = pObject->next) {
        if ((pObject->flags & 0x400) != 0) {
            BrMatrix34LPInverse(&inverse, &pObject->parent->transform_matrix);
            BrMatrix34LPNormalise(&normalised, &inverse);
            BrMatrix34Mul(&inverse, &pObject->transform_matrix, &normalised);
            BrMatrix34Mul(&pObject->field_0x1e8, &inverse, &pObject->parent->field_0x1e8);
        }
        joint = pObject->physics_joint1;
        if (joint != NULL) {
            if ((pObject->flags & 0x400) != 0) {
                FUN_004b73a0(pObject->child, pParent_matrix, pRoot_matrix);
            } else {
                BrMatrix34ApplyP(&transformed, &joint->field_0x14, pParent_matrix);
                DrMatrix34ApplyLPInverse(&joint->field_0x14, &transformed, pRoot_matrix);
                if (joint->type == 1 || joint->type == 4) {
                    BrMatrix34ApplyV(&transformed, &joint->parent_bone_axis, pParent_matrix);
                    BrMatrix34TApplyV(&joint->parent_bone_axis, &transformed, pRoot_matrix);
                }
            }
            for (i = 0; i < joint->count_limits; i++) {
                BrMatrix34ApplyV(&transformed, &joint->limits[i].parent, pParent_matrix);
                BrMatrix34TApplyV(&joint->limits[i].parent, &transformed, pRoot_matrix);
            }
        }
    }
}
#if defined(REC2_MATCHING) && defined(_MSC_VER)
#pragma inline_depth()
#endif
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif


#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
#if defined(REC2_MATCHING) && defined(_MSC_VER)
#pragma inline_depth(0)
#endif
// FUNCTION: CARMA2_HW 0x004b7310
void C2_HOOK_FASTCALL FUN_004b7310(tPhysics_object* pObject) {
    tPhysics_object* parent;
    br_matrix34 inverse;
    br_matrix34 normalised;

    parent = pObject->parent;
    pObject->flags |= 0x400;
    while (parent != NULL && (parent->flags & 0x400) != 0) {
        parent = parent->parent;
    }
    BrMatrix34LPInverse(&inverse, &parent->transform_matrix);
    BrMatrix34LPNormalise(&normalised, &inverse);
    BrMatrix34Mul(&pObject->field_0x1e8, &pObject->transform_matrix, &normalised);
    FUN_004b73a0(pObject->child, &pObject->transform_matrix, &parent->transform_matrix);
}
#if defined(REC2_MATCHING) && defined(_MSC_VER)
#pragma inline_depth()
#endif
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif


// STUB: CARMA2_HW 0x004bf490
void C2_HOOK_FASTCALL FUN_004bf490(br_scalar* pMatrix, int pStride, tPhysics_joint_force** pRecords, int pCount) {
    NOT_IMPLEMENTED();
}

// FUNCTION: CARMA2_HW 0x004b77b0
void C2_HOOK_FASTCALL FUN_004b77b0(br_scalar* pVelocity, tPhysics_joint_force** pRecords, int pCount) {
    tPhysics_joint_force* record;
    tPhysics_object* object1;
    tPhysics_object* object2;
    float angular_delta[3];
    int offset;

    if (pCount <= 0) {
        return;
    }
    offset = (int)pVelocity - (int)pRecords;
    do {
        record = *pRecords;
        object1 = record->object_a;
        if (record->type == eJoint_limit_11) {
            *(br_scalar*)((tU8*)pRecords + offset) = -(record->direction_a.v[1] * object1->omega.v[1]
                + record->direction_a.v[2] * object1->omega.v[2]
                + record->direction_a.v[0] * object1->omega.v[0]);
            record = *pRecords;
            object2 = record->object_b;
            if (object2 != NULL) {
                *(br_scalar*)((tU8*)pRecords + offset) = record->direction_b.v[1] * object2->omega.v[1]
                    + record->direction_b.v[0] * object2->omega.v[0]
                    + record->direction_b.v[2] * object2->omega.v[2]
                    + *(br_scalar*)((tU8*)pRecords + offset);
            }
        } else {
            angular_delta[0] = record->point_a.v[2] * object1->omega.v[1] - record->point_a.v[1] * object1->omega.v[2] + object1->velocity_car_space.v[0];
            angular_delta[1] = record->point_a.v[0] * object1->omega.v[2] - record->point_a.v[2] * object1->omega.v[0] + object1->velocity_car_space.v[1];
            angular_delta[2] = record->point_a.v[1] * object1->omega.v[0] - record->point_a.v[0] * object1->omega.v[1] + object1->velocity_car_space.v[2];
            *(br_scalar*)((tU8*)pRecords + offset) = -(record->direction_a.v[1] * angular_delta[1]
                + record->direction_a.v[2] * angular_delta[2]
                + record->direction_a.v[0] * angular_delta[0]);
            record = *pRecords;
            object2 = record->object_b;
            if (object2 != NULL) {
                angular_delta[0] = record->point_b.v[2] * object2->omega.v[1] - record->point_b.v[1] * object2->omega.v[2] + object2->velocity_car_space.v[0];
                angular_delta[1] = record->point_b.v[0] * object2->omega.v[2] - record->point_b.v[2] * object2->omega.v[0] + object2->velocity_car_space.v[1];
                angular_delta[2] = record->point_b.v[1] * object2->omega.v[0] - record->point_b.v[0] * object2->omega.v[1] + object2->velocity_car_space.v[2];
                *(br_scalar*)((tU8*)pRecords + offset) = record->direction_b.v[0] * angular_delta[0]
                    + record->direction_b.v[1] * angular_delta[1]
                    + record->direction_b.v[2] * angular_delta[2]
                    + *(br_scalar*)((tU8*)pRecords + offset);
            }
        }
        pRecords++;
    } while (--pCount != 0);
}

// FUNCTION: CARMA2_HW 0x004b7960
void C2_HOOK_FASTCALL FUN_004b7960(tPhysics_joint_force* pForce, br_scalar pImpulse) {
    /* The seven vectors occupy the whole 0x54-byte frame in this exact order
       (linear_delta at the lowest address, linear_accel at the highest). */
    struct {
        br_vector3 linear_delta;
        br_vector3 angular_delta;
        br_vector3 linear_delta_other;
        br_vector3 angular_delta_other;
        br_vector3 world_delta;
        br_vector3 impulse;
        br_vector3 linear_accel;
    } delta;
    tPhysics_object* pObject_a;
    tPhysics_object* pObject_b;
    tPhysics_object* pPartner;
    br_scalar inverse_dt;

    if (pForce->type == 9 || pImpulse == 0.f) {
        return;
    }
    pObject_a = pForce->object_a;
    pObject_b = pForce->object_b;
    if (pForce->type == 0xb) {
        pPartner = (tPhysics_object*)FUN_004be550(delta.angular_delta.v, delta.linear_delta.v, delta.angular_delta_other.v, delta.linear_delta_other.v, &pForce->direction_a, pObject_a);
        delta.angular_delta.v[0] *= pImpulse;
        delta.angular_delta.v[1] *= pImpulse;
        delta.angular_delta.v[2] *= pImpulse;
        pObject_a->omega.v[0] += delta.angular_delta.v[0];
        pObject_a->omega.v[1] += delta.angular_delta.v[1];
        pObject_a->omega.v[2] += delta.angular_delta.v[2];
        if (pPartner != NULL) {
            delta.linear_delta.v[0] *= pImpulse;
            delta.linear_delta.v[1] *= pImpulse;
            delta.linear_delta.v[2] *= pImpulse;
            pObject_a->velocity_car_space.v[0] += delta.linear_delta.v[0];
            pObject_a->velocity_car_space.v[1] += delta.linear_delta.v[1];
            pObject_a->velocity_car_space.v[2] += delta.linear_delta.v[2];
            BrMatrix34ApplyV(&pObject_a->v, &pObject_a->velocity_car_space, &pObject_a->transform_matrix);
            delta.linear_delta_other.v[0] *= pImpulse;
            delta.linear_delta_other.v[1] *= pImpulse;
            delta.linear_delta_other.v[2] *= pImpulse;
            pPartner->velocity_car_space.v[0] += delta.linear_delta_other.v[0];
            pPartner->velocity_car_space.v[1] += delta.linear_delta_other.v[1];
            pPartner->velocity_car_space.v[2] += delta.linear_delta_other.v[2];
            BrMatrix34ApplyV(&pPartner->v, &pPartner->velocity_car_space, &pPartner->transform_matrix);
            delta.angular_delta_other.v[0] *= pImpulse;
            delta.angular_delta_other.v[1] *= pImpulse;
            delta.angular_delta_other.v[2] *= pImpulse;
            pPartner->omega.v[0] += delta.angular_delta_other.v[0];
            pPartner->omega.v[1] += delta.angular_delta_other.v[1];
            pPartner->omega.v[2] += delta.angular_delta_other.v[2];
        }
        if (pObject_b != NULL) {
            pPartner = (tPhysics_object*)FUN_004be550(delta.angular_delta.v, delta.linear_delta.v, delta.angular_delta_other.v, delta.linear_delta_other.v, &pForce->direction_b, pObject_b);
            pImpulse = -pImpulse;
            delta.angular_delta.v[0] *= pImpulse;
            delta.angular_delta.v[1] *= pImpulse;
            delta.angular_delta.v[2] *= pImpulse;
            pObject_b->omega.v[0] += delta.angular_delta.v[0];
            pObject_b->omega.v[1] += delta.angular_delta.v[1];
            pObject_b->omega.v[2] += delta.angular_delta.v[2];
            if (pPartner != NULL) {
                delta.linear_delta.v[0] *= pImpulse;
                delta.linear_delta.v[1] *= pImpulse;
                delta.linear_delta.v[2] *= pImpulse;
                pObject_b->velocity_car_space.v[0] += delta.linear_delta.v[0];
                pObject_b->velocity_car_space.v[1] += delta.linear_delta.v[1];
                pObject_b->velocity_car_space.v[2] += delta.linear_delta.v[2];
                BrMatrix34ApplyV(&pObject_b->v, &pObject_b->velocity_car_space, &pObject_b->transform_matrix);
                delta.linear_delta_other.v[0] *= pImpulse;
                delta.linear_delta_other.v[1] *= pImpulse;
                delta.linear_delta_other.v[2] *= pImpulse;
                pPartner->velocity_car_space.v[0] += delta.linear_delta_other.v[0];
                pPartner->velocity_car_space.v[1] += delta.linear_delta_other.v[1];
                pPartner->velocity_car_space.v[2] += delta.linear_delta_other.v[2];
                BrMatrix34ApplyV(&pPartner->v, &pPartner->velocity_car_space, &pPartner->transform_matrix);
                delta.angular_delta_other.v[0] *= pImpulse;
                delta.angular_delta_other.v[1] *= pImpulse;
                delta.angular_delta_other.v[2] *= pImpulse;
                pPartner->omega.v[0] += delta.angular_delta_other.v[0];
                pPartner->omega.v[1] += delta.angular_delta_other.v[1];
                pPartner->omega.v[2] += delta.angular_delta_other.v[2];
            }
        }
        return;
    }
    if (*(int*)((tU8*)pObject_a + 0x1e4) != 0) {
        SetUpForce((br_vector3*)delta.angular_delta.v, (br_vector3*)delta.linear_delta.v, &pForce->point_a, &pForce->direction_a, pObject_a);
        pPartner = (tPhysics_object*)FUN_004becf0(delta.angular_delta.v, delta.linear_delta.v, delta.angular_delta_other.v, delta.linear_delta_other.v, pObject_a);
        delta.angular_delta.v[0] *= pImpulse;
        delta.angular_delta.v[1] *= pImpulse;
        delta.angular_delta.v[2] *= pImpulse;
        pObject_a->omega.v[0] += delta.angular_delta.v[0];
        pObject_a->omega.v[1] += delta.angular_delta.v[1];
        pObject_a->omega.v[2] += delta.angular_delta.v[2];
        delta.angular_delta_other.v[0] *= pImpulse;
        delta.angular_delta_other.v[1] *= pImpulse;
        delta.angular_delta_other.v[2] *= pImpulse;
        pPartner->omega.v[0] += delta.angular_delta_other.v[0];
        pPartner->omega.v[1] += delta.angular_delta_other.v[1];
        pPartner->omega.v[2] += delta.angular_delta_other.v[2];
        delta.linear_delta.v[0] *= pImpulse;
        delta.linear_delta.v[1] *= pImpulse;
        delta.linear_delta.v[2] *= pImpulse;
        pObject_a->velocity_car_space.v[0] += delta.linear_delta.v[0];
        pObject_a->velocity_car_space.v[1] += delta.linear_delta.v[1];
        pObject_a->velocity_car_space.v[2] += delta.linear_delta.v[2];
        BrMatrix34ApplyV(&delta.world_delta, &delta.linear_delta, &pObject_a->transform_matrix);
        pObject_a->v.v[0] += delta.world_delta.v[0];
        pObject_a->v.v[1] += delta.world_delta.v[1];
        pObject_a->v.v[2] += delta.world_delta.v[2];
        delta.linear_delta_other.v[0] *= pImpulse;
        delta.linear_delta_other.v[1] *= pImpulse;
        delta.linear_delta_other.v[2] *= pImpulse;
        pPartner->velocity_car_space.v[0] += delta.linear_delta_other.v[0];
        pPartner->velocity_car_space.v[1] += delta.linear_delta_other.v[1];
        pPartner->velocity_car_space.v[2] += delta.linear_delta_other.v[2];
        BrMatrix34ApplyV(&delta.world_delta, &delta.linear_delta_other, &pPartner->transform_matrix);
        pPartner->v.v[0] += delta.world_delta.v[0];
        pPartner->v.v[1] += delta.world_delta.v[1];
        pPartner->v.v[2] += delta.world_delta.v[2];
    } else {
        delta.impulse.v[0] = pForce->direction_a.v[0] * pImpulse;
        delta.linear_accel.v[0] = delta.impulse.v[0] / pObject_a->M;
        delta.impulse.v[1] = pForce->direction_a.v[1] * pImpulse;
        delta.linear_accel.v[1] = delta.impulse.v[1] / pObject_a->M;
        delta.impulse.v[2] = pForce->direction_a.v[2] * pImpulse;
        delta.linear_accel.v[2] = delta.impulse.v[2] / pObject_a->M;
        pObject_a->velocity_car_space.v[0] += delta.linear_accel.v[0];
        pObject_a->velocity_car_space.v[1] += delta.linear_accel.v[1];
        pObject_a->velocity_car_space.v[2] += delta.linear_accel.v[2];
        BrMatrix34ApplyV(&delta.world_delta, &delta.linear_accel, &pObject_a->transform_matrix);
        pObject_a->v.v[0] += delta.world_delta.v[0];
        pObject_a->v.v[1] += delta.world_delta.v[1];
        pObject_a->v.v[2] += delta.world_delta.v[2];
        if ((pObject_a->flags & 0x800) == 0) {
            delta.angular_delta.v[0] = delta.impulse.v[2] * pForce->point_a.v[1] - delta.impulse.v[1] * pForce->point_a.v[2];
            delta.angular_delta.v[1] = delta.impulse.v[0] * pForce->point_a.v[2] - delta.impulse.v[2] * pForce->point_a.v[0];
            delta.angular_delta.v[2] = delta.impulse.v[1] * pForce->point_a.v[0] - delta.impulse.v[0] * pForce->point_a.v[1];
            pObject_a->omega.v[0] += delta.angular_delta.v[0] / pObject_a->I.v[0];
            pObject_a->omega.v[1] += delta.angular_delta.v[1] / pObject_a->I.v[1];
            pObject_a->omega.v[2] += delta.angular_delta.v[2] / pObject_a->I.v[2];
        }
    }
    if (pObject_b != NULL && pForce->type != 0xe) {
        if (*(int*)((tU8*)pObject_b + 0x1e4) != 0) {
            SetUpForce((br_vector3*)delta.angular_delta.v, (br_vector3*)delta.linear_delta.v, &pForce->point_b, &pForce->direction_b, pObject_b);
            pPartner = (tPhysics_object*)FUN_004becf0(delta.angular_delta.v, delta.linear_delta.v, delta.angular_delta_other.v, delta.linear_delta_other.v, pObject_b);
            pImpulse = -pImpulse;
            delta.angular_delta.v[0] *= pImpulse;
            delta.angular_delta.v[1] *= pImpulse;
            delta.angular_delta.v[2] *= pImpulse;
            pObject_b->omega.v[0] += delta.angular_delta.v[0];
            pObject_b->omega.v[1] += delta.angular_delta.v[1];
            pObject_b->omega.v[2] += delta.angular_delta.v[2];
            delta.angular_delta_other.v[0] *= pImpulse;
            delta.angular_delta_other.v[1] *= pImpulse;
            delta.angular_delta_other.v[2] *= pImpulse;
            pPartner->omega.v[0] += delta.angular_delta_other.v[0];
            pPartner->omega.v[1] += delta.angular_delta_other.v[1];
            pPartner->omega.v[2] += delta.angular_delta_other.v[2];
            delta.linear_delta.v[0] *= pImpulse;
            delta.linear_delta.v[1] *= pImpulse;
            delta.linear_delta.v[2] *= pImpulse;
            pObject_b->velocity_car_space.v[0] += delta.linear_delta.v[0];
            pObject_b->velocity_car_space.v[1] += delta.linear_delta.v[1];
            pObject_b->velocity_car_space.v[2] += delta.linear_delta.v[2];
            BrMatrix34ApplyV(&delta.world_delta, &delta.linear_delta, &pObject_b->transform_matrix);
            pObject_b->v.v[0] += delta.world_delta.v[0];
            pObject_b->v.v[1] += delta.world_delta.v[1];
            pObject_b->v.v[2] += delta.world_delta.v[2];
            delta.linear_delta_other.v[0] *= pImpulse;
            delta.linear_delta_other.v[1] *= pImpulse;
            delta.linear_delta_other.v[2] *= pImpulse;
            pPartner->velocity_car_space.v[0] += delta.linear_delta_other.v[0];
            pPartner->velocity_car_space.v[1] += delta.linear_delta_other.v[1];
            pPartner->velocity_car_space.v[2] += delta.linear_delta_other.v[2];
            BrMatrix34ApplyV(&delta.world_delta, &delta.linear_delta_other, &pPartner->transform_matrix);
            pPartner->v.v[0] += delta.world_delta.v[0];
            pPartner->v.v[1] += delta.world_delta.v[1];
            pPartner->v.v[2] += delta.world_delta.v[2];
        } else {
            inverse_dt = -pImpulse;
            delta.impulse.v[0] = pForce->direction_b.v[0] * inverse_dt;
            delta.linear_accel.v[0] = delta.impulse.v[0] / pObject_b->M;
            delta.impulse.v[1] = pForce->direction_b.v[1] * inverse_dt;
            delta.linear_accel.v[1] = delta.impulse.v[1] / pObject_b->M;
            delta.impulse.v[2] = pForce->direction_b.v[2] * inverse_dt;
            delta.linear_accel.v[2] = delta.impulse.v[2] / pObject_b->M;
            pObject_b->velocity_car_space.v[0] += delta.linear_accel.v[0];
            pObject_b->velocity_car_space.v[1] += delta.linear_accel.v[1];
            pObject_b->velocity_car_space.v[2] += delta.linear_accel.v[2];
            BrMatrix34ApplyV(&delta.world_delta, &delta.linear_accel, &pObject_b->transform_matrix);
            pObject_b->v.v[0] += delta.world_delta.v[0];
            pObject_b->v.v[1] += delta.world_delta.v[1];
            pObject_b->v.v[2] += delta.world_delta.v[2];
            if ((pObject_b->flags & 0x800) == 0) {
                delta.angular_delta.v[0] = delta.impulse.v[2] * pForce->point_b.v[1] - delta.impulse.v[1] * pForce->point_b.v[2];
                delta.angular_delta.v[1] = delta.impulse.v[0] * pForce->point_b.v[2] - delta.impulse.v[2] * pForce->point_b.v[0];
                delta.angular_delta.v[2] = delta.impulse.v[1] * pForce->point_b.v[0] - delta.impulse.v[0] * pForce->point_b.v[1];
                pObject_b->omega.v[0] += delta.angular_delta.v[0] / pObject_b->I.v[0];
                pObject_b->omega.v[1] += delta.angular_delta.v[1] / pObject_b->I.v[1];
                pObject_b->omega.v[2] += delta.angular_delta.v[2] / pObject_b->I.v[2];
            }
        }
    }
}

// FUNCTION: CARMA2_HW 0x004be350
void C2_HOOK_FASTCALL FUN_004be350(float* pDestination, const float* pSource) {
    float fVar1;
    float fVar2;
    float fVar3;
    float fVar4;
    float fVar5;

    pDestination[0] = -pSource[5] * pSource[7] + pSource[8] * pSource[4];
    pDestination[1] = -(-pSource[2] * pSource[7] + pSource[1] * pSource[8]);
    fVar1 = -pSource[2] * pSource[4] + pSource[1] * pSource[5];
    pDestination[2] = fVar1;
    fVar5 = 1.0 / (pSource[1] * pDestination[1] + pSource[2] * fVar1 + pSource[0] * pDestination[0]);
    pDestination[0] = pDestination[0] * fVar5;
    pDestination[3] = -((-pSource[5] * pSource[6] + pSource[8] * pSource[3]) * fVar5);
    fVar1 = pSource[4];
    fVar2 = pSource[3];
    fVar3 = pSource[7];
    fVar4 = pSource[6];
    pDestination[1] = pDestination[1] * fVar5;
    pDestination[6] = (-fVar1 * fVar4 + fVar3 * fVar2) * fVar5;
    pDestination[4] = (-pSource[2] * pSource[6] + pSource[8] * pSource[0]) * fVar5;
    fVar1 = pSource[1];
    fVar2 = pSource[7];
    fVar3 = pSource[6];
    fVar4 = pSource[0];
    pDestination[2] = pDestination[2] * fVar5;
    pDestination[7] = -((-fVar1 * fVar3 + fVar2 * fVar4) * fVar5);
    pDestination[5] = -((-pSource[2] * pSource[3] + pSource[5] * pSource[0]) * fVar5);
    pDestination[8] = (-pSource[1] * pSource[3] + pSource[4] * pSource[0]) * fVar5;
}

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
#if defined(REC2_MATCHING) && defined(_MSC_VER)
#pragma inline_depth(0)
#endif
// FUNCTION: CARMA2_HW 0x004b8cf0
void C2_HOOK_FASTCALL FUN_004b8cf0(tPhysics_object* pObject) {
    int uVar1;
    int uVar2;
    int i;
    int* piVar4;
    char* puVar3;
    int local_174[6];
    float local_15c[15];
    tPhysics_joint_force rows[3];
    tPhysics_object* pObject2;
    tPhysics_object* pOther;

    pObject2 = pObject->field_0x230;
    pOther = (tPhysics_object*)*(int*)(*(int*)((tU8*)pObject + 0x1e4) + 4);
    if (pOther != pObject2) {
        pObject2 = pObject;
        pObject = pOther;
    }
    CreateJointForce(pObject, pObject2, &rows[0], (br_vector3*)((char*)pObject->physics_joint1 + 0x38));
    CreateJointTorque(pObject, pObject2, &rows[1], (br_vector3*)((char*)pObject->physics_joint1 + 0x20));
    CreateJointTorque(pObject, pObject2, &rows[2], (br_vector3*)((char*)pObject->physics_joint1 + 0x2c));
    piVar4 = local_174;
    puVar3 = (char*)rows;
    i = 3;
    do {
        *piVar4 = (int)puVar3;
        puVar3 += 0x60;
        piVar4++;
        i--;
    } while (i != 0);
    uVar1 = *(int*)&pObject2->field_0x1e0[4];
    uVar2 = *(int*)&pObject->field_0x1e0[4];
    *(int*)&pObject->field_0x1e0[4] = 0;
    *(int*)&pObject2->field_0x1e0[4] = 0;
    FUN_004bf490(&local_15c[3], 3, (tPhysics_joint_force**)local_174, 3);
    FUN_004be350((float*)(uVar2 + 8), &local_15c[3]);
    FUN_004b77b0((br_scalar*)&local_174[3], (tPhysics_joint_force**)local_174, 3);
    BrMatrix34ApplyV((br_vector3*)&local_15c[0], (br_vector3*)&local_174[3], (br_matrix34*)(uVar2 + 8));
    i = 0;
    do {
        FUN_004b7960((tPhysics_joint_force*)*(int*)((char*)local_174 + i), *(float*)((char*)local_15c + i));
        i += 4;
    } while (i < 0xc);
    *(int*)&pObject->field_0x1e0[4] = uVar2;
    *(int*)&pObject2->field_0x1e0[4] = uVar1;
}
#if defined(REC2_MATCHING) && defined(_MSC_VER)
#pragma inline_depth()
#endif
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif


#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
// FUNCTION: CARMA2_HW 0x004b9ce0
void C2_HOOK_FASTCALL MergeBoundsIntoParent(tPhysics_object* pObject, br_bounds3* pBox) {
    br_bounds3 local_box;
    float* pSrc;
    tPhysics_object* child;
    int i;

    GetNewBoundingBox(&pObject->field_0xf4, &pObject->bb1, &pObject->transform_matrix);
    GetNewBoundingBox(&local_box, &pObject->bb1, &pObject->actor->t.t.mat);
    for (i = 0; i < 3; i++) {
        if (pObject->field_0xf4.min.v[i] > local_box.min.v[i]) {
            pObject->field_0xf4.min.v[i] = local_box.min.v[i];
        }
        if (pObject->field_0xf4.max.v[i] < local_box.max.v[i]) {
            pObject->field_0xf4.max.v[i] = local_box.max.v[i];
        }
        pObject->field_0xf4.min.v[i] = pObject->field_0xf4.min.v[i] - 0.0025;
        pObject->field_0xf4.max.v[i] = pObject->field_0xf4.max.v[i] - -0.0025;
    }
    pObject->field_0x10c = pObject->field_0xf4;
    for (child = pObject->child; child != NULL; child = child->next) {
        MergeBoundsIntoParent(child, &pObject->field_0x10c);
    }
    if (pBox != NULL) {
        pSrc = &pObject->field_0x10c.min.v[0];
        for (i = 0; i < 3; i++) {
            if (pSrc[i] < pBox->min.v[i]) {
                pBox->min.v[i] = pSrc[i];
            }
            if (pSrc[i + 3] > pBox->max.v[i]) {
                pBox->max.v[i] = pSrc[i + 3];
            }
        }
    } else {
        if (gFLOAT_00692dd0 <= pObject->field_0x10c.max.v[0] - pObject->transform_matrix.m[3][0]) {
            gFLOAT_00692dd0 = pObject->field_0x10c.max.v[0] - pObject->transform_matrix.m[3][0];
        }
        if (gFLOAT_00692dd4 >= pObject->field_0x10c.min.v[0] - pObject->transform_matrix.m[3][0]) {
            gFLOAT_00692dd4 = pObject->field_0x10c.min.v[0] - pObject->transform_matrix.m[3][0];
        }
    }
}
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif


/* MSVC5 has no __declspec(noinline); keep the SetListDoingNothingFlag call out-of-line. */
#if defined(REC2_MATCHING) && defined(_MSC_VER)
#pragma inline_depth(0)
#endif

// FUNCTION: CARMA2_HW 0x004b9e40
void C2_HOOK_FASTCALL SetCollisionInfoDoNothingChain(tPhysics_object* pCollision_info, tU32 pDisable) {
    tPhysics_object* cur;
    tPhysics_object* sibling;

    pCollision_info->disable_move_rotate = pDisable;
    cur = pCollision_info;
    do {
        cur->disable_move_rotate = pDisable;
        cur->field_0x1df = 0;
        if (cur->child != NULL) {
            sibling = cur->child;
            while (sibling != NULL) {
                sibling->disable_move_rotate = pDisable;
                sibling->field_0x1df = 0;
                if (sibling->child != NULL) {
                    SetListDoingNothingFlag(sibling->child, pDisable);
                }
                sibling = sibling->next;
            }
        }
        cur = (tPhysics_object*)cur->field183_0x1d8;
    } while (cur != NULL && cur != pCollision_info);
}

#if defined(REC2_MATCHING) && defined(_MSC_VER)
#pragma inline_depth(1)
#endif


// FUNCTION: CARMA2_HW 0x004bd830
tPhysics_object** C2_HOOK_FASTCALL AppendOverlappingObjects(tPhysics_object* pObject, tPhysics_object* pList, tPhysics_object** pObjects) {
    tPhysics_object** pLink;
    tPhysics_object** pChild_link;
    tPhysics_object* pCurrent;
    tPhysics_object* child_head;
    float x;

    pLink = pObjects;
    if (pList->parent != NULL) {
        for (pCurrent = pList; pCurrent != NULL; pCurrent = pCurrent->next) {
            if ((pCurrent == pObject && (pObject->flags & 8) == 0)
                    || BoundsOverlapTest__finteray(&pObject->field_0x10c, &pCurrent->field_0x10c)) {
                *pLink = pCurrent;
                pLink = (tPhysics_object**)&pCurrent->field_0x234;
            }
        }
    } else {
        x = pObject->field_0x10c.min.v[0] - gFLOAT_00692dd0;
        for (pCurrent = pObject->prev; pCurrent != NULL && pCurrent->transform_matrix.m[3][0] >= x; pCurrent = pCurrent->prev) {
            if (BoundsOverlapTest__finteray(&pObject->field_0x10c, &pCurrent->field_0x10c)) {
                *pLink = pCurrent;
                pLink = (tPhysics_object**)&pCurrent->field_0x234;
            }
        }
        x = pObject->field_0x10c.max.v[0] - gFLOAT_00692dd4;
        for (pCurrent = pObject->next; pCurrent != NULL && pCurrent->transform_matrix.m[3][0] <= x; pCurrent = pCurrent->next) {
            if (BoundsOverlapTest__finteray(&pObject->field_0x10c, &pCurrent->field_0x10c)) {
                *pLink = pCurrent;
                pLink = (tPhysics_object**)&pCurrent->field_0x234;
            }
        }
        if ((pObject->flags & 8) == 0) {
            *pLink = pObject;
            pLink = (tPhysics_object**)&pObject->field_0x234;
        }
    }
    *pLink = NULL;

    child_head = NULL;
    pCurrent = *pObjects;
    pChild_link = &child_head;
    while (pCurrent != NULL) {
        if (pCurrent->child != NULL) {
            pChild_link = AppendOverlappingObjects(pObject, pCurrent->child, pChild_link);
        }
        pCurrent = (tPhysics_object*)pCurrent->field_0x234;
    }
    if (child_head != NULL) {
        *pChild_link = NULL;
        *pLink = child_head;
        return pChild_link;
    }
    return pLink;
}

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif

// FUNCTION: CARMA2_HW 0x004bca10
int C2_HOOK_FASTCALL CollectJointLimitsIntoBuffer(tPhysics_object* pObject, tPhysics_joint_force* pContacts, int pCount, int pMax_count) {
    tPhysics_object* partner;
    int i;
    br_vector3 cross;
    br_vector3 vec;
    br_scalar dot;
    br_scalar dot2;
    tPhysics_joint_force* rec;

    if (pObject->physics_joint1 != NULL) {
        partner = pObject->field_0x230;
        if (partner == NULL) {
            PhysicsError((tPhysicsError)11, "");
        }
        i = 0;
        if (pObject->physics_joint1->count_limits > 0) {
            rec = &pContacts[pCount];
            do {
                if (pCount >= pMax_count) {
                    break;
                }
                switch (pObject->physics_joint1->limits[i].type) {
                case eJoint_limit_plane: {
                    br_vector3 vec2;
                    br_vector3 vec3;

                    BrMatrix34ApplyV(&vec, &pObject->physics_joint1->limits[i].child, &pObject->actor->t.t.mat);
                    BrMatrix34TApplyV(&vec2, &vec, &partner->actor->t.t.mat);
                    dot = pObject->physics_joint1->limits[i].parent.v[0] * vec2.v[0]
                        + pObject->physics_joint1->limits[i].parent.v[1] * vec2.v[1]
                        + pObject->physics_joint1->limits[i].parent.v[2] * vec2.v[2];
                    if (dot < 0.001) {
                        BrMatrix34ApplyV(&vec, &pObject->physics_joint1->limits[i].child, &pObject->transform_matrix);
                        BrMatrix34TApplyV(&vec3, &vec, &partner->transform_matrix);
                        dot2 = pObject->physics_joint1->limits[i].parent.v[0] * vec3.v[0]
                            + pObject->physics_joint1->limits[i].parent.v[1] * vec3.v[1]
                            + pObject->physics_joint1->limits[i].parent.v[2] * vec3.v[2];
                        if (0.0f <= dot2 && (0.001 <= dot2 || pObject->field_0xf0 != 0)) {
                            rec->field_0x0 = dot - dot2;
                            rec->type = 11;
                            rec->object_a = pObject;
                            rec->object_b = partner;
                            cross.v[0] = pObject->physics_joint1->limits[i].parent.v[2] * vec3.v[1]
                                - pObject->physics_joint1->limits[i].parent.v[1] * vec3.v[2];
                            cross.v[1] = pObject->physics_joint1->limits[i].parent.v[0] * vec3.v[2]
                                - pObject->physics_joint1->limits[i].parent.v[2] * vec3.v[0];
                            cross.v[2] = pObject->physics_joint1->limits[i].parent.v[1] * vec3.v[0]
                                - pObject->physics_joint1->limits[i].parent.v[0] * vec3.v[1];
                            dot2 = (br_scalar)sqrt(cross.v[0] * cross.v[0] + cross.v[1] * cross.v[1] + cross.v[2] * cross.v[2]);
                            if (dot2 <= 1e-06) {
                                BrMatrix34ApplyV(&cross, &pObject->omega, &pObject->transform_matrix);
                                BrMatrix34TApplyV(&rec->direction_b, &cross, &partner->transform_matrix);
                                rec->direction_b.v[0] = rec->direction_b.v[0] - partner->omega.v[0];
                                rec->direction_b.v[1] = rec->direction_b.v[1] - partner->omega.v[1];
                                rec->direction_b.v[2] = rec->direction_b.v[2] - partner->omega.v[2];
                                dot2 = (br_scalar)sqrt(rec->direction_b.v[0] * rec->direction_b.v[0]
                                    + rec->direction_b.v[1] * rec->direction_b.v[1]
                                    + rec->direction_b.v[2] * rec->direction_b.v[2]);
                                if (dot2 <= 2.3841858e-07f) {
                                    rec->direction_b.v[0] = 1.0f;
                                    rec->direction_b.v[1] = 0.0f;
                                    rec->direction_b.v[2] = 0.0f;
                                } else {
                                    rec->direction_b.v[0] = rec->direction_b.v[0] * (1.0 / dot2);
                                    rec->direction_b.v[1] = rec->direction_b.v[1] * (1.0 / dot2);
                                    rec->direction_b.v[2] = rec->direction_b.v[2] * (1.0 / dot2);
                                }
                            } else {
                                rec->direction_b.v[0] = cross.v[0] / dot2;
                                rec->direction_b.v[1] = cross.v[1] / dot2;
                                rec->direction_b.v[2] = cross.v[2] / dot2;
                            }
                            BrMatrix34ApplyV(&cross, &rec->direction_b, &partner->transform_matrix);
                            BrMatrix34TApplyV(&rec->direction_a, &cross, &pObject->transform_matrix);
                            rec->point_a.v[0] = pObject->physics_joint1->field_0x08.v[0];
                            rec->point_a.v[1] = pObject->physics_joint1->field_0x08.v[1];
                            rec->point_a.v[2] = pObject->physics_joint1->field_0x08.v[2];
                            rec->point_b.v[0] = pObject->physics_joint1->field_0x14.v[0];
                            rec->point_b.v[1] = pObject->physics_joint1->field_0x14.v[1];
                            rec->point_b.v[2] = pObject->physics_joint1->field_0x14.v[2];
                            pCount++;
                            rec++;
                        }
                    }
                    break;
                }
                case eJoint_limit_universal: {
                    br_vector3 out1;
                    br_vector3 out2;

                    FUN_004bd4f0(&pObject->actor->t.t.mat, &partner->actor->t.t.mat,
                                 &pObject->physics_joint1->limits[i].child, &pObject->physics_joint1->limits[i].parent,
                                 &pObject->physics_joint1->limits[i + 1].child, &pObject->physics_joint1->limits[i + 1].parent,
                                 &out1, &cross);
                    dot = pObject->physics_joint1->limits[i + 1].parent.v[0] * out1.v[0]
                        + pObject->physics_joint1->limits[i + 1].parent.v[2] * out1.v[2]
                        + pObject->physics_joint1->limits[i + 1].parent.v[1] * out1.v[1];
                    if (dot < pObject->physics_joint1->limits[i].value - -0.001) {
                        FUN_004bd4f0(&pObject->transform_matrix, &partner->transform_matrix,
                                     &pObject->physics_joint1->limits[i].child, &pObject->physics_joint1->limits[i].parent,
                                     &pObject->physics_joint1->limits[i + 1].child, &pObject->physics_joint1->limits[i + 1].parent,
                                     &out2, &rec->direction_b);
                        dot2 = pObject->physics_joint1->limits[i + 1].parent.v[2] * out2.v[2]
                            + pObject->physics_joint1->limits[i + 1].parent.v[1] * out2.v[1]
                            + pObject->physics_joint1->limits[i + 1].parent.v[0] * out2.v[0];
                        if (pObject->physics_joint1->limits[i].value < dot2) {
                            cross.v[0] = out1.v[1] * out2.v[2] - out1.v[2] * out2.v[1];
                            cross.v[1] = out1.v[2] * out2.v[0] - out1.v[0] * out2.v[2];
                            cross.v[2] = out1.v[0] * out2.v[1] - out1.v[1] * out2.v[0];
                            if (rec->direction_b.v[0] * cross.v[0] + rec->direction_b.v[2] * cross.v[2] + rec->direction_b.v[1] * cross.v[1] < 0.0f) {
                                rec->direction_b.v[0] = -rec->direction_b.v[0];
                                rec->direction_b.v[1] = -rec->direction_b.v[1];
                                rec->direction_b.v[2] = -rec->direction_b.v[2];
                            }
                            BrMatrix34ApplyV(&cross, &rec->direction_b, &partner->transform_matrix);
                            BrMatrix34TApplyV(&rec->direction_a, &cross, &pObject->transform_matrix);
                            rec->field_0x0 = dot - dot2;
                            rec->type = 11;
                            rec->object_a = pObject;
                            rec->object_b = partner;
                            pCount++;
                            rec++;
                        }
                    }
                    i++;
                    break;
                }
                default:
                    PhysicsError((tPhysicsError)8, "");
                    break;
                }
                i++;
            } while (i < pObject->physics_joint1->count_limits);
        }
    }
    if (pObject->physics_joint2 == NULL || pObject->physics_joint2->count_limits < 1) {
        return pCount;
    }
    i = 0;
    do {
        switch (pObject->physics_joint2->limits[i].type) {
        case eJoint_limit_plane:
            BrMatrix34ApplyV(&vec, &pObject->physics_joint2->limits[i].child, &pObject->actor->t.t.mat);
            dot = pObject->physics_joint2->limits[i].parent.v[0] * vec.v[0]
                + pObject->physics_joint2->limits[i].parent.v[1] * vec.v[1]
                + pObject->physics_joint2->limits[i].parent.v[2] * vec.v[2];
            if (dot < 0.001) {
                if (pCount == pMax_count) {
                    return pCount;
                }
                pContacts[pCount].field_0x0 = dot;
                pContacts[pCount].type = 11;
                pContacts[pCount].object_a = pObject;
                pContacts[pCount].object_b = NULL;
                cross.v[0] = pObject->physics_joint2->limits[i].parent.v[2] * vec.v[1]
                    - pObject->physics_joint2->limits[i].parent.v[1] * vec.v[2];
                cross.v[1] = pObject->physics_joint2->limits[i].parent.v[0] * vec.v[2]
                    - pObject->physics_joint2->limits[i].parent.v[2] * vec.v[0];
                cross.v[2] = pObject->physics_joint2->limits[i].parent.v[1] * vec.v[0]
                    - pObject->physics_joint2->limits[i].parent.v[0] * vec.v[1];
                dot2 = (br_scalar)sqrt(cross.v[0] * cross.v[0] + cross.v[1] * cross.v[1] + cross.v[2] * cross.v[2]);
                if (1e-06 < dot2) {
                    pContacts[pCount].direction_b.v[0] = cross.v[0] / dot2;
                    pContacts[pCount].direction_b.v[1] = cross.v[1] / dot2;
                    pContacts[pCount].direction_b.v[2] = cross.v[2] / dot2;
                }
            }
            break;
        case eJoint_limit_10: {
            int n;
            int m;
            if (pObject->field_0x1e0[0] != 0) {
                gINT_00692dd8 = 1;
                for (n = 0; n < pCount; n++) {
                    if (pContacts[n].object_a == pObject) {
                        if (pContacts[n].object_b == NULL) {
                            if (n < pCount) {
                                m = pCount - n;
                                rec = &pContacts[n];
                                do {
                                    *rec = rec[1];
                                    rec++;
                                } while (--m != 0);
                            }
                            pCount--;
                            n--;
                        } else {
                            rec = &pContacts[n];
                            cross.v[0] = -rec->direction_a.v[0];
                            cross.v[1] = -rec->direction_a.v[1];
                            cross.v[2] = -rec->direction_a.v[2];
                            rec->direction_a.v[0] = -rec->direction_b.v[0];
                            rec->direction_a.v[1] = -rec->direction_b.v[1];
                            rec->direction_a.v[2] = -rec->direction_b.v[2];
                            rec->direction_b.v[0] = cross.v[0];
                            rec->direction_b.v[1] = cross.v[1];
                            rec->direction_b.v[2] = cross.v[2];
                            cross = rec->point_a;
                            rec->point_a = rec->point_b;
                            rec->point_b = cross;
                            rec->object_a = rec->object_b;
                            rec->object_b = pObject;
                            rec->type = 14;
                        }
                    } else if (pContacts[n].object_b == pObject) {
                        pContacts[n].type = 14;
                    }
                }
            }
            break;
        }
        case eJoint_limit_11: {
            br_scalar t0;
            br_scalar len;
            br_vector3 vecB;
            BrMatrix34ApplyV(&vecB, &pObject->physics_joint2->hinge_axis, &pObject->transform_matrix);
            dot = pObject->field_0x17c.v[0] * vecB.v[0]
                + pObject->field_0x17c.v[2] * vecB.v[2]
                + pObject->field_0x17c.v[1] * vecB.v[1];
            t0 = pObject->physics_joint2->limits[i].value + dot;
            dot2 = pObject->actor->t.t.mat.m[3][0] * vecB.v[0]
                + pObject->actor->t.t.mat.m[3][1] * vecB.v[1]
                + pObject->actor->t.t.mat.m[3][2] * vecB.v[2];
            len = pObject->transform_matrix.m[3][2] * vecB.v[2]
                + pObject->transform_matrix.m[3][1] * vecB.v[1]
                + pObject->transform_matrix.m[3][0] * vecB.v[0];
            if (dot2 <= t0 || t0 - -0.001 <= len) {
                if (dot2 < dot - 0.001 && dot - 0.002 < len) {
                    if (pCount == pMax_count) {
                        return pCount;
                    }
                    pContacts[pCount].field_0x0 = (dot - len) / (dot2 - len);
                    pContacts[pCount].field_0xc = NULL;
                    pContacts[pCount].type = 2;
                    pContacts[pCount].object_a = pObject;
                    pContacts[pCount].object_b = NULL;
                    pContacts[pCount].direction_a = pObject->physics_joint2->hinge_axis;
                    BrVector3SetFloat(&pContacts[pCount].point_a, 0.0f, 0.0f, 0.0f);
                    pCount++;
                }
            } else {
                if (pCount == pMax_count) {
                    return pCount;
                }
                pContacts[pCount].field_0x0 = (t0 - len) / (dot2 - len);
                pContacts[pCount].field_0xc = NULL;
                pContacts[pCount].type = 2;
                pContacts[pCount].object_a = pObject;
                pContacts[pCount].object_b = NULL;
                pContacts[pCount].direction_a.v[0] = -pObject->physics_joint2->hinge_axis.v[0];
                pContacts[pCount].direction_a.v[1] = -pObject->physics_joint2->hinge_axis.v[1];
                pContacts[pCount].direction_a.v[2] = -pObject->physics_joint2->hinge_axis.v[2];
                BrVector3SetFloat(&pContacts[pCount].point_a, 0.0f, 0.0f, 0.0f);
                pCount++;
            }
            break;
        }
        default:
            PhysicsError((tPhysicsError)8, "");
            break;
        }
        i++;
    } while (i < pObject->physics_joint2->count_limits);
    return pCount;
}

#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif


#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
// FUNCTION: CARMA2_HW 0x004bd6d0
int C2_HOOK_FASTCALL CollectJointLimitsOfTree(tPhysics_object* pObject, tPhysics_joint_force* pContacts, int pCount, int pMax_count) {
    if (pObject != NULL) {
        do {
            pCount = CollectJointLimitsIntoBuffer(pObject, pContacts, pCount, pMax_count);
            if (pObject->child != NULL) {
                pCount = CollectJointLimitsOfTree(pObject->child, pContacts, pCount, pMax_count);
            }
            pObject = pObject->next;
        } while (pObject != NULL);
    }
    return pCount;
}
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif

// STUB: CARMA2_HW 0x004bd720
int C2_HOOK_FASTCALL ClampObjectSpin(tPhysics_object* pObject) {
    NOT_IMPLEMENTED();
}

#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif


#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
// FUNCTION: CARMA2_HW 0x004bd7f0
int C2_HOOK_FASTCALL ClampObjectSpinTree(tPhysics_object* pObject) {
    int count;

    count = 0;
    for (; pObject != NULL; pObject = pObject->next) {
        count += ClampObjectSpin(pObject);
        if (pObject->child != NULL) {
            count += ClampObjectSpinTree(pObject->child);
        }
    }
    return count;
}
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif


#if defined(REC2_MATCHING) && defined(_MSC_VER)
#pragma inline_depth(0)
#endif
#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
// FUNCTION: CARMA2_HW 0x004be0e0
void C2_HOOK_FASTCALL FUN_004be0e0(tPhysics_object* pObject, tPhysics_object* pList) {
    tPhysics_object* child;
    tPhysics_object* item;

    if (pObject->disable_move_rotate == 0) {
        return;
    }
    while (pObject->parent != NULL && pObject->physics_joint1 != NULL && pObject->physics_joint1->type != eJoint_none) {
        pObject = pObject->parent;
    }
    pObject->box_face_ref = gFace_num__car - 2;
    pObject->disable_move_rotate = 0;
    if (pObject->child != NULL || pObject->physics_joint1 != NULL || pObject->physics_joint2 != NULL) {
        if (pObject->field_0x218 == 0) {
            MoveJointedObject(pObject, 0.f);
        }
        for (child = pObject->child; child != NULL; child = child->next) {
            if (child->disable_move_rotate != 0) {
                child->box_face_ref = gFace_num__car - 2;
                child->disable_move_rotate = 0;
            }
            if (child->child != NULL) {
                MakeObjectListDoSomething(child->child);
            }
        }
    }
    for (item = pList; item != NULL; item = item->next) {
        if (item->disable_move_rotate == 0) {
            continue;
        }
        if (pObject->field_0xf4.min.v[0] - item->field_0xf4.max.v[0] <= gDOUBLE_0058a798
                && pObject->field_0xf4.min.v[1] - item->field_0xf4.max.v[1] <= gDOUBLE_0058a798
                && pObject->field_0xf4.min.v[2] - item->field_0xf4.max.v[2] <= gDOUBLE_0058a798
                && item->field_0xf4.min.v[0] - pObject->field_0xf4.max.v[0] <= gDOUBLE_0058a798
                && item->field_0xf4.min.v[1] - pObject->field_0xf4.max.v[1] <= gDOUBLE_0058a798
                && item->field_0xf4.min.v[2] - pObject->field_0xf4.max.v[2] <= gDOUBLE_0058a798) {
            FUN_004be0e0(item, pList);
        }
        if (pObject->child != NULL && item->disable_move_rotate != 0) {
            child = pObject->child;
            if (child != NULL) {
                do {
                    if (child->field_0xf4.min.v[0] - item->field_0xf4.max.v[0] <= gDOUBLE_0058a798
                        && child->field_0xf4.min.v[1] - item->field_0xf4.max.v[1] <= gDOUBLE_0058a798
                        && child->field_0xf4.min.v[2] - item->field_0xf4.max.v[2] <= gDOUBLE_0058a798
                        && item->field_0xf4.min.v[0] - child->field_0xf4.max.v[0] <= gDOUBLE_0058a798
                        && item->field_0xf4.min.v[1] - child->field_0xf4.max.v[1] <= gDOUBLE_0058a798
                        && item->field_0xf4.min.v[2] - child->field_0xf4.max.v[2] <= gDOUBLE_0058a798) {
                        FUN_004be0e0(item, pList);
                        break;
                    }
                    child = child->next;
                } while (child != NULL);
            }
        }
        if (item->child != NULL) {
            CheckForObjectHierachyTouchingObjectList(pObject, item->child, pList);
        }
    }
}
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif

// STUB: CARMA2_HW 0x004c2670
void C2_HOOK_FAKE_THISCALL InterpolateSingleObject(tPhysics_object* pObject, br_scalar pDt) {
    NOT_IMPLEMENTED();
}

#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif


// FUNCTION: CARMA2_HW 0x004c2970
void C2_HOOK_FASTCALL PhysicsObjectMoveVelocityList(tPhysics_object* pObject) {

    while (pObject != NULL) {
        if (!pObject->disable_move_rotate) {
            pObject->v.v[0] += pObject->field_0x54.v[0];
            pObject->v.v[1] += pObject->field_0x54.v[1];
            pObject->v.v[2] += pObject->field_0x54.v[2];
        }
        if (pObject->child != NULL) {
            PhysicsObjectMoveVelocityList(pObject->child);
        }
        pObject = pObject->next;
    }
}


// FUNCTION: CARMA2_HW 0x004c29d0
void C2_HOOK_FASTCALL PhysicsObjectSetImpulse(tPhysics_object* pObject, br_vector3* pImpulse) {
    tPhysics_object* pChild;

    pObject->field_0x54 = *pImpulse;
    for (pChild = pObject->child; pChild != NULL; pChild = pChild->next) {
        PhysicsObjectSetImpulse(pChild, pImpulse);
    }
}

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
// STUB: CARMA2_HW 0x004269c0
int C2_HOOK_FASTCALL FUN_004269c0(tPhysics_object* pObject1, tPhysics_object* pObject2, tPhysics_shape* pShape1, tPhysics_shape* pShape2, const br_vector3* pVertices1, const br_vector3* pVertices2, int pCount1, int pCount2, tPhysics_joint_force* pRecords, int pRecord_count, int pRemaining) {
    NOT_IMPLEMENTED();
}
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
// STUB: CARMA2_HW 0x00427870
int C2_HOOK_FASTCALL FUN_00427870(tPhysics_object* pObject1, tPhysics_object* pObject2, tPhysics_shape* pShape1, tPhysics_shape* pShape2, const br_vector3* pVertices1, const br_vector3* pVertices2, int pCount1, int pCount2, tPhysics_joint_force* pRecords, int pRecord_count, int pRemaining) {
    NOT_IMPLEMENTED();
}
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
// STUB: CARMA2_HW 0x0045d5e0
void C2_HOOK_FASTCALL FUN_0045d5e0(tCollision_shape_sphere_data* pSphere1, tCollision_shape_sphere_data* pSphere2, br_vector3* pStart, br_vector3* pEnd, br_vector3* pNormal, float* pFactor) {
    NOT_IMPLEMENTED();
}
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
// STUB: CARMA2_HW 0x004b9230
void C2_HOOK_FASTCALL FUN_004b9230(tPhysics_object* pObject, tPhysics_object** ppChanged_object, float pDelta_time) {
    NOT_IMPLEMENTED();
}
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
// STUB: CARMA2_HW 0x004be550
int C2_HOOK_FASTCALL FUN_004be550(br_scalar* pOut1, br_scalar* pOut2, br_scalar* pOut3, br_scalar* pOut4, const br_vector3* pDirection, tPhysics_object* pObject) {
    NOT_IMPLEMENTED();
}
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif

#ifdef REC2_MATCHING
#pragma auto_inline(off)
#endif
// STUB: CARMA2_HW 0x004becf0
int C2_HOOK_FASTCALL FUN_004becf0(br_scalar* pOut1, br_scalar* pOut2, br_scalar* pOut3, br_scalar* pOut4, tPhysics_object* pObject) {
    NOT_IMPLEMENTED();
}
#ifdef REC2_MATCHING
#pragma auto_inline(on)
#endif

extern br_quat* C2_HOOK_CDECL BrQuatNormalise(br_quat* q, const br_quat* qq);
extern br_matrix34* C2_HOOK_CDECL BrQuatToMatrix34(br_matrix34* mat, const br_quat* q);

// FUNCTION: CARMA2_HW 0x0049c000
void C2_HOOK_FASTCALL GetMatrixFromOrientation(br_matrix34* pMatrix, const tU16* pOrientation) {
    br_quat quaternion;
    br_scalar x;
    br_scalar y;
    br_scalar z;
    br_scalar w;

    x = (br_scalar)pOrientation[0] * 1.5259255e-05f;
    y = (br_scalar)pOrientation[1] * 1.5259255e-05f;
    z = (br_scalar)pOrientation[2] * 1.5259255e-05f;
    w = (br_scalar)pOrientation[3] * 1.5259255e-05f;
    quaternion.x = (x + x) - 1.0f;
    quaternion.y = (y + y) - 1.0f;
    quaternion.z = (z + z) - 1.0f;
    quaternion.w = (w + w) - 1.0f;
    BrQuatNormalise(&quaternion, &quaternion);
    BrQuatToMatrix34(pMatrix, &quaternion);
}


