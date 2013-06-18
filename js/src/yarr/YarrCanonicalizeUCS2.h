/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 *
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef YarrCanonicalizeUCS2_H
#define YarrCanonicalizeUCS2_H

#include "mozilla/StandardInteger.h"
#include "wtfbridge.h"
#include "assembler/wtf/Assertions.h"

namespace JSC { namespace Yarr {

// This set of data (autogenerated using YarrCanonicalizeUCS2.js into YarrCanonicalizeUCS2.cpp)
// provides information for each UCS2 code point as to the set of code points that it should
// match under the ES5.1 case insensitive RegExp matching rules, specified in 15.10.2.8.
enum UCS2CanonicalizationType {
    CanonicalizeUnique,               // No canonically equal values, e.g. 0x0.
    CanonicalizeSet,                  // Value indicates a set in characterSetInfo.
    CanonicalizeRangeLo,              // Value is positive delta to pair, E.g. 0x41 has value 0x20, -> 0x61.
    CanonicalizeRangeHi,              // Value is positive delta to pair, E.g. 0x61 has value 0x20, -> 0x41.
    CanonicalizeAlternatingAligned,   // Aligned consequtive pair, e.g. 0x1f4,0x1f5.
    CanonicalizeAlternatingUnaligned  // Unaligned consequtive pair, e.g. 0x241,0x242.
};
struct UCS2CanonicalizationRange { uint16_t begin, end, value, type; };
extern const size_t UCS2_CANONICALIZATION_RANGES;
extern const uint16_t* const characterSetInfo[];
extern const UCS2CanonicalizationRange rangeInfo[];

// This table is similar to the full rangeInfo table, however this maps from UCS2 codepoints to
// the set of Latin1 codepoints that could match.
enum LatinCanonicalizationType {
    CanonicalizeLatinSelf,     // This character is in the Latin1 range, but has no canonical equivalent in the range.
    CanonicalizeLatinMask0x20, // One of a pair of characters, under the mask 0x20.
    CanonicalizeLatinOther,    // This character is not in the Latin1 range, but canonicalizes to another that is.
    CanonicalizeLatinInvalid   // Cannot match against Latin1 input.
};
struct LatinCanonicalizationRange { uint16_t begin, end, value, type; };
extern const size_t LATIN_CANONICALIZATION_RANGES;
extern const LatinCanonicalizationRange latinRangeInfo[];

// This searches in log2 time over ~364 entries, so should typically result in 8 compares.
inline const UCS2CanonicalizationRange* rangeInfoFor(UChar ch)
{
    const UCS2CanonicalizationRange* info = rangeInfo;
    size_t entries = UCS2_CANONICALIZATION_RANGES;

    while (true) {
        size_t candidate = entries >> 1;
        const UCS2CanonicalizationRange* candidateInfo = info + candidate;
        if (ch < candidateInfo->begin)
            entries = candidate;
        else if (ch <= candidateInfo->end)
            return candidateInfo;
        else {
            info = candidateInfo + 1;
            entries -= (candidate + 1);
        }
    }
}

// Should only be called for characters that have one canonically matching value.
inline UChar getCanonicalPair(const UCS2CanonicalizationRange* info, UChar ch)
{
    ASSERT(ch >= info->begin && ch <= info->end);
    switch (info->type) {
    case CanonicalizeRangeLo:
        return ch + info->value;
    case CanonicalizeRangeHi:
        return ch - info->value;
    case CanonicalizeAlternatingAligned:
        return ch ^ 1;
    case CanonicalizeAlternatingUnaligned:
        return ((ch - 1) ^ 1) + 1;
    default:
        ASSERT_NOT_REACHED();
    }
    ASSERT_NOT_REACHED();
    return 0;
}

// Returns true if no other UCS2 codepoint can match this value.
inline bool isCanonicallyUnique(UChar ch)
{
    return rangeInfoFor(ch)->type == CanonicalizeUnique;
}

// Returns true if values are equal, under the canonicalization rules.
inline bool areCanonicallyEquivalent(UChar a, UChar b)
{
    const UCS2CanonicalizationRange* info = rangeInfoFor(a);
    switch (info->type) {
    case CanonicalizeUnique:
        return a == b;
    case CanonicalizeSet: {
        for (const uint16_t* set = characterSetInfo[info->value]; (a = *set); ++set) {
            if (a == b)
                return true;
        }
        return false;
    }
    case CanonicalizeRangeLo:
        return (a == b) || (a + info->value == b);
    case CanonicalizeRangeHi:
        return (a == b) || (a - info->value == b);
    case CanonicalizeAlternatingAligned:
        return (a | 1) == (b | 1);
    case CanonicalizeAlternatingUnaligned:
        return ((a - 1) | 1) == ((b - 1) | 1);
    }

    ASSERT_NOT_REACHED();
    return false;
}

} } // JSC::Yarr

#endif
