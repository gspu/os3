/*
 *  exe.h       DOS executable (MZ) format.
 *
 *                          Open Watcom Project
 *
 * Copyright (c) 2004-2006 The Open Watcom Contributors. All Rights Reserved.
 *
 *    This file is automatically generated. Do not edit directly.
 *
 * =========================================================================
*/

#ifndef __EXE__
#define __EXE__

#if !defined(_ENABLE_AUTODEPEND)
  #pragma read_only_file;
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define EXEID   0x5a4d

#ifndef BYTE
typedef unsigned char   BYTE;
#endif

#ifndef WORD
typedef unsigned short  WORD;
#endif

typedef WORD    bbmask;

struct exe {
    WORD    eid;
    WORD    elast;
    WORD    epagsiz;
    WORD    erelcnt;
    WORD    ehdrsiz;
    WORD    eminfre;
    WORD    emaxfre;
    WORD    eiSS;
    WORD    eiSP;
    WORD    enegsum;
    WORD    eiIP;
    WORD    eiCS;
    WORD    ereloff;
    WORD    eovlnum;
    WORD    ever;
    WORD    dumy;
    bbmask  ebb;
    WORD    dumy2[7];
};

#ifdef __cplusplus
} /* End of extern "C" */
#endif

#endif

