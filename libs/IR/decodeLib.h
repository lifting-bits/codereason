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

bool convertBlobToBlocks(   void *ctx, 
                            unsigned char *b, 
                            unsigned long bufLen, 
                            std::vector<BlockPtr> &blocks);

void * initDecodeLib(TargetInfo, bool do_throw, bool dbg_spew);

void * initDecodeLib2(TargetArch, bool do_throw, bool dbg_spew);

bool convertToFlow(void             *ctx,
				   unsigned char	*buf,
				   unsigned long	bufLen,
				   unsigned long	entryOffset,
				   unsigned long	baseAddress,
				   FlowPtr		    &f);

void convertToBlockVec(void				*ctx,
					   unsigned char	*buf,
					   unsigned long	bufLen,
					   unsigned long	baseAddress,
					   std::vector<BlockPtr>	&blocksOut,
					   TargetArch		arch);

bool convertToOneBlock(void				*ctx,
					   unsigned char	*buf,
					   unsigned long	bufLen,
					   unsigned long	baseAddr,
					   TargetArch		arch,
					   BlockPtr			&blockOut);

void finiDecodeLib(void *ctx);

void initLibState(void);

#endif
