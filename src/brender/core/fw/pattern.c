/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright 2022-2026 rec2 developers
 */

#include "pattern.h"

#define UPPER(c) (((c) >= 'a' && (c) <= 'z') ? ((c) - ('a' - 'A')) : (c))
#define MATCH_CHAR(a, b) (UPPER(a) == UPPER(b))

// FUNCTION: CARMA2_HW 0x00529860
// FUNCTION: CARMA2_SW 0x005243d0
// FUNCTION: CARMA2_CHEAT 0x0052f450
br_boolean C2_HOOK_CDECL BrNamePatternMatch(const char* p, const char* s) {
    const char* cp;

    // A NULL pattern matches everything
    if (p == NULL) {
        return 1;
    }
    // A NULL string never matches
    if (s == NULL) {
        return 0;
    }

    for (;;) {
        switch (*p) {
        case '/':
        case '\0':
            // Empty pattern only matches empty string
            return *s == '\0';

        case '*':
            // Match p+1 in any position from s to end of s
            cp = s;
            do {
                if (BrNamePatternMatch(p + 1, cp)) {
                    return 1;
                }
            } while (*cp++);
            return 0;

        case '?':
            // Match any character followed by matching(p+1, s+1)
            if (*s == '\0') {
                return 0;
            }
            p++;
            s++;
            continue;

        default:
            // Match this character followed by matching(p+1, s+1)
            if (!MATCH_CHAR(*s, *p)) {
                return 0;
            }
            p++;
            s++;
            continue;
        }
    }
}
