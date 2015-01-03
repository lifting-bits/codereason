#ifndef _DECODE_LIB_H
#define _DECODE_LIB_H

#include "BasicIR.h"

enum OptLevel {
	NoOpt,
	SomeOpt,
	FullOpt
};

struct TargetInfo {
	OptLevel	opLevel;
	unsigned long maxInstructions;
	unsigned long chaseThreshold;//default to 10
	unsigned long guestHWcaps;
    unsigned long hostHWcaps;
	TargetArch	tarch;
};

void * initDecodeLib(TargetInfo, bool do_throw, bool dbg_spew);

void * initDecodeLib2(TargetArch, bool do_throw, bool dbg_spew);

bool convertToOneBlock(void				*ctx,
					   unsigned char	*buf,
					   unsigned long	bufLen,
					   unsigned long	baseAddr,
					   TargetArch		arch,
             unsigned int maxStatements,
					   BlockPtr			&blockOut);

void finiDecodeLib(void *ctx);

void initLibState(void);

#endif
