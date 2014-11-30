#include "decodeToIR.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <setjmp.h>
#include <pthread.h>
#endif

#include "VexIR.h"

using namespace std;

#define N_TRANSBUF 5000

#ifndef WIN32
pthread_key_t jBuffTls;
#endif

struct DecodeLibState {
	VexControl	vcon;
	VexArchInfo	vai_guest;
    VexArchInfo vai_host;
	VexAbiInfo	vbi;
	VexArch		guestarch;
	TargetInfo	ti;
	bool        do_throw;
};

static Bool chase_into_not_ok ( void* opaque, Addr64 dst ) { return False; }
#ifdef _MSC_BUILD
typedef IRSB *(__cdecl *vexInstTy)(void *,IRSB *,VexGuestLayout *,VexGuestExtents *,IRType,IRType);
#else
typedef IRSB *(*vexInstTy)(void *,IRSB *,VexGuestLayout *,VexGuestExtents *,IRType,IRType);
#endif

struct TransContext
{
	vector<BlockPtr>	&blocks;

	bool isStmtInCtx(Statement *st) {
		return false;
	}

	bool isBlockInCtx(BlockPtr b) {
		return false;
	}

    TransContext(vector<BlockPtr> &b) : blocks(b) { }

    TargetArch  ta;
};

struct TransContext2
{
	vector<BlockPtr>	&blocks;
	unsigned long   curId;
	TargetArch		targetArch;

	bool isStmtInCtx(Statement *st) {
		return false;
	}

	bool isBlockInCtx(BlockPtr b) {
		return false;
	}

	TransContext2(vector<BlockPtr> &b) : blocks(b) { return; }

    TargetArch  ta;
};


struct OneBlockTransCtx
{
	BlockPtr        b;
	unsigned long   curId;
    TargetArch      ta;
};

class TranslateException {
	std::string	msg;
public:
	TranslateException(std::string m) {
		this->msg = m;
	}
};

#ifndef WIN32
void doThrow() {
    jmp_buf e;
    void *b = pthread_getspecific(jBuffTls);
    memcpy(e, b, sizeof(jmp_buf));
    longjmp(e, 1);
    return;
}

int setThrow() {
    int k;
    jmp_buf *b = (jmp_buf *)malloc(sizeof(jmp_buf));

    k = setjmp(*b);

    if( k == 0 ) {
        pthread_setspecific(jBuffTls, b);
    } else {
        void *b = pthread_getspecific(jBuffTls);
        free(b);
        pthread_setspecific(jBuffTls, NULL);
    }

    return k;
}
#endif

static
#ifndef _MSC_BUILD
__attribute__((noreturn))
#endif
void failure_exit ( void )
{
	fprintf(stdout, "VEX did failure_exit.  Bye.\n");
	exit(1);
}

static
#ifndef _MSC_BUILD
__attribute__((noreturn))
#endif
void failure_throw(void) {
	//throw an exception to get us back to someone that cares
#if 0
    doThrow();
#else
	throw TranslateException("failure_throw called");
#endif
}

static
void log_bytes ( HChar* bytes, Int nbytes )
{
	char b[] = { 'V', 'L', 'O', 'G', '>', ' '};
	fwrite ( b, 1, sizeof(b), stdout );
	fwrite ( bytes, 1, nbytes, stdout );
	return;
}

static
void eat_bytes( HChar *bytes, Int nbytes ) 
{
	return;
}

static UInt needs_self_check ( void* opaque, VexGuestExtents* vge ) {
       return 0;
}

void initVex(DecodeLibState *dls, bool do_throw, bool dbg_spew) {

	LibVEX_default_VexControl(&dls->vcon);
	dls->vcon.iropt_level = dls->ti.opLevel; //max optimization
	dls->vcon.guest_max_insns = dls->ti.maxInstructions; //number of insns in a guest block;
	dls->vcon.guest_chase_thresh = dls->ti.chaseThreshold;

	LibVEX_default_VexArchInfo(&dls->vai_guest);
	LibVEX_default_VexArchInfo(&dls->vai_host);
	dls->vai_guest.hwcaps = dls->ti.guestHWcaps;
	dls->vai_host.hwcaps = dls->ti.hostHWcaps;

	LibVEX_default_VexAbiInfo(&dls->vbi);
    dls->vbi.guest_stack_redzone_size = 128;

	dls->do_throw = do_throw;
	if( do_throw ) {
		if( dbg_spew ) {
			LibVEX_Init(failure_throw, log_bytes, 1, False, &dls->vcon);
		} else {
			LibVEX_Init(failure_throw, eat_bytes, 1, False, &dls->vcon);
		}
	} else {
		if( dbg_spew ){
			LibVEX_Init(failure_exit, log_bytes, 1, False, &dls->vcon);
		} else {
			LibVEX_Init(failure_exit, eat_bytes, 1, False, &dls->vcon);
		}
	}

	return;
}

/* we can get pointer to vector of statements
 * so, create a new statement and append it to the
 * vector if the statement is unique
 */
IRSB *
#ifdef _MSC_BUILD 
__cdecl
#endif
convertCallback(void *ctx, 
				IRSB *bb_in, 
				VexGuestLayout *vgl, 
				VexGuestExtents *vge, 
				IRType girty, 
				IRType hirty ) 
{
	TransContext	*tcxt = (TransContext *)ctx;

	VBlockPtr    b = VBlockPtr(new VBlock(0, tcxt->ta));

    b->buildFromIRSB(bb_in);

	if( !tcxt->isBlockInCtx(b) ) {
	    tcxt->blocks.push_back(boost::static_pointer_cast<Block>(b));
	} 

	return bb_in;
}

IRSB *
#ifdef _MSC_BUILD 
__cdecl
#endif
convertCallback2(void *ctx, 
				IRSB *bb_in, 
				VexGuestLayout *vgl, 
				VexGuestExtents *vge, 
				IRType girty, 
				IRType hirty ) 
{
	TransContext2	*tcxt = (TransContext2 *)ctx;

	VBlockPtr   b = VBlockPtr(new VBlock(tcxt->curId, tcxt->targetArch));

    b->buildFromIRSB(bb_in);

	tcxt->blocks.push_back(boost::static_pointer_cast<Block>(b));

	return bb_in;
}

IRSB *
#ifdef _MSC_BUILD 
__cdecl
#endif
convertToOneBlockCb(void *ctx,
				  IRSB *bb_in,
				  VexGuestLayout *vgl,
				  VexGuestExtents *vge,
				  IRType			girty,
				  IRType			hirty)
{
	//Block **b = (Block **)ctx;
	OneBlockTransCtx	*obtc = (OneBlockTransCtx *)ctx;

    VBlockPtr b = VBlockPtr(new VBlock(obtc->curId, obtc->ta));
    b->buildFromIRSB(bb_in);
    obtc->b = boost::static_pointer_cast<Block>(b);

	return bb_in;
}

/*
 */
bool runVEXOnBlobWithCallback(	DecodeLibState	*dls,
							    unsigned char	*b,
								unsigned long	bufExtents,
								unsigned long long b_addr,
								vexInstTy		func, 
								void			*ctx,
								unsigned long	*readBytes) 
{
	bool				didGood = false;
	VexTranslateArgs	vta;
	VexTranslateResult	vtr;
	VexGuestExtents		vge;
	UChar				transbuf[N_TRANSBUF];
	Int					trans_used;

	memset(&vta, 0, sizeof(VexTranslateArgs));
    memset(&vge, 0, sizeof(VexGuestExtents));	

	vta.instrument1 = func;
	vta.callback_opaque = ctx;

	vta.abiinfo_both    = dls->vbi;
	vta.guest_bytes     = b;
	vta.guest_bytes_addr = (Addr64)b_addr;
	vta.chase_into_ok   = chase_into_not_ok;
	vta.guest_extents   = &vge;
	vta.host_bytes      = transbuf;
	vta.host_bytes_size = N_TRANSBUF;
	vta.host_bytes_used = &trans_used;

	vta.archinfo_guest = dls->vai_guest;
	vta.arch_guest = (VexArch)dls->ti.tarch.ta;

	vta.archinfo_host = dls->vai_host;
#ifdef __i386__
	vta.arch_host = VexArchX86;
#else
    vta.arch_host = VexArchAMD64;
#endif

	//vta.do_self_check = False;
	vta.preamble_function = NULL;
	vta.dispatch_assisted = (void *)0x12345678;
	vta.dispatch_unassisted = (void *)0x12345678;
    vta.needs_self_check = needs_self_check;
	vta.traceflags = 0;
	vta.finaltidy = NULL;

	bool did_throw=false;
	if( dls->do_throw ) {
#if 0
        bool entered = false;
        setThrow();
        if( entered == false ) { 
            entered = true;
            vtr = LibVEX_Translate(&vta);
        } else {
            did_throw = true;
        }
#else
		try {
			vtr = LibVEX_Translate(&vta);
		} catch( TranslateException &te) {
			did_throw = true;
		}
#endif
	} else {
		vtr = LibVEX_Translate(&vta);
	}

	if( !did_throw ) {
		unsigned long read = vge.len[0];
		if( (vtr.status == VexTranslateResult::VexTransOK) && (read > 0)) {
			didGood = true;
		}

		*readBytes = read;
	} else {
		*readBytes = 0;
	}

	return didGood;
}

/* sequentially scan through a block of data
 * treat the data as code unilaterally
 * convert the code into VEX IR, and then, our statements
 */
bool convertBlobToBlocks(void               *ctx,
						 unsigned char      *b, 
						 unsigned long      bufLen, 
						 vector<BlockPtr>   &blocks) 
{
	unsigned long	totalBytesRead=0;
	unsigned long	maxBytesLeft=bufLen;
	unsigned long   baseAddr = 0x1000;
	TransContext	*tcxt = new TransContext(blocks);
	bool			result = true;
	DecodeLibState	*dls = (DecodeLibState *)ctx;

	do
	{
		unsigned long bytesRead=0;
		bool c = runVEXOnBlobWithCallback(	dls,
											b,
											maxBytesLeft,
											baseAddr,
											convertCallback, 
											tcxt, 
											&bytesRead);
		if( !c || bytesRead == 0 ) {
			//result = false;
			//break;
			//these bytes were bad, skip ahead at least one
			bytesRead = 1;
		}

		totalBytesRead += bytesRead;
		baseAddr += bytesRead;
		b += bytesRead;
		maxBytesLeft -= bytesRead;
	} while( totalBytesRead < bufLen );

	return result;
}

void convertToBlockVec(void				    *ctx,
					   unsigned char	    *buf,
					   unsigned long	    bufLen,
					   unsigned long	    baseAddress,
					   vector<BlockPtr>	    &blocksOut,
					   TargetArch		    arch) 
{
	DecodeLibState	*dls = (DecodeLibState *)ctx;

	unsigned char	*b = buf;
	unsigned long	totalBytesRead=0;
	unsigned long	maxBytesLeft=bufLen;
	TransContext2	*tcxt = new TransContext2(blocksOut);

	tcxt->blocks = blocksOut;
	tcxt->targetArch = arch;
	tcxt->curId = 0;

	do
	{
		unsigned long bytesRead=0;

		bool c = runVEXOnBlobWithCallback(	dls,
			b,
			maxBytesLeft,
			baseAddress,
			convertCallback2, 
			tcxt, 
			&bytesRead);

		if( !c || bytesRead == 0 ) {
			bytesRead = 1;
		} else {
			tcxt->curId = tcxt->curId + 1;
		}

		totalBytesRead += 1;
		baseAddress += 1;
		b += 1;
		maxBytesLeft -= 1;
	} while( totalBytesRead < bufLen );

	return;
}
bool convertToOneBlock(void				*ctx,
					   unsigned char	*buf,
					   unsigned long	bufLen,
					   unsigned long	baseAddr,
					   TargetArch		arch,
					   BlockPtr         &blockOut)
{
	bool				result=true;
	DecodeLibState		*dls = (DecodeLibState*)ctx;
	OneBlockTransCtx	obtc;
	unsigned long		nbytes;

	obtc.curId = 0;
    obtc.ta = arch;

    //if this is ARM and it is THUMB, we need to perpetrate a hack
    if( arch.ta == ARM && arch.tm == THUMB ) {
        //bump baseAddr
        baseAddr |= 1;
        //bump the buf addr
        buf = (uint8_t *)( ((ptrdiff_t)buf) | 1 ); 
    }

	result = runVEXOnBlobWithCallback(  dls, 
                                        buf, 
                                        bufLen, 
                                        baseAddr,
                                        convertToOneBlockCb, 
                                        &obtc, 
                                        &nbytes);

    if( result ) {
        blockOut = obtc.b;
    }

	return result;
}

/* convert the buffer into a flow (a linked series of blocks)
 * we need to do some more complicated stuff here, namely, 
 * resolution of jmp targets
 */
bool convertToFlow(void             *ctx,
				   unsigned char	*buf,
				   unsigned long	bufLen,
				   unsigned long	entryOffset,
				   unsigned long	baseAddress,
				   FlowPtr		    &f) 
{
#if 0
	DecodeLibState	*dls = (DecodeLibState *)ctx;
	bool res = true;
	vector<unsigned long long>	blockAddrs;

	blockAddrs.push_back(entryOffset+baseAddress);

	*f = new Flow();
	Flow *lf = *f;

	//cout << "convertToFlow enter" << endl;
	unsigned long blockID = 0;
	while( blockAddrs.size() > 0 ) {
		unsigned long long	newBlock = blockAddrs[blockAddrs.size()-1];
		unsigned char		*decodeHere = (unsigned char *)((newBlock-baseAddress)+buf);
		unsigned long nbytes;
		Block *newB;
		blockAddrs.pop_back();
		OneBlockTransCtx	obtc;
		obtc.b = NULL;
		obtc.curId = blockID;

		//TODO: check that we are not traversing off the edge of the buffer
		//but we're not scanning linearly, so there's no point in keeping track
		//of the number of bytes we've traversed
		bool r;

		r = runVEXOnBlobWithCallback(dls, decodeHere, bufLen, newBlock, convertToOneBlock, &obtc, &nbytes);

		if( !r ) {
			res = false;
			break;
		}

		newB = obtc.b;
		//VEX should have given us a block
		assert(newB != NULL);

		//cout << "new block " << endl;
		//cout << newB->printBlock() << endl;

		//does the new block go anywhere interesting?
		std::set<unsigned long long>	tgts = newB->getHardTargets();

		//if the new block flows to a new-to-the-flow address, 
		//add those addresses to the decode list
		if( tgts.size() > 0 ) {
			//do we have any of these targets in the flow already?
			std::set<unsigned long long>::iterator	it = tgts.begin();
			std::set<unsigned long long>::iterator	e = tgts.end();

			while( it != e ) {
				unsigned long long	addr = *it;

				//this BB SHOULD be unique in our flow
				if( !lf->isAddrInFlowCode(addr) && addr != newB->getBlockBase() ) {
					//we don't have this already, put it into the blockAddrs list
					blockAddrs.push_back(addr);
				}
				++it;
			}
		}

		//if we don't have this block already, add it
		//TODO, we need to deal with block splitting
		if( !lf->isAddrInFlowCode(newB->getBlockBase()) && !lf->isAddrInFlowCode(newB->getBlockEnd()) ) {
			lf->addBlockToFlow(newB);
			blockID++;
		} else if ( !lf->isAddrInFlowCode(newB->getBlockBase()) && lf->isAddrInFlowCode(newB->getBlockEnd()) ) {
			//there is an overlap on one end
		}
	}

	/*if( res ) {
		lf->complete();
	}*/

	//cout << "convertToFlow end " << endl;

	return res;
#endif
    return false;//do this later
}

void * initDecodeLib(TargetInfo	ti, bool do_throw, bool dbg_spew) {
	DecodeLibState	*dls = new DecodeLibState();
	dls->ti = ti;
	initVex(dls, do_throw, dbg_spew);
	return dls;
}

void * initDecodeLib2(TargetArch ta, bool do_throw, bool dbg_spew) {
	DecodeLibState	*dls = new DecodeLibState();
    TargetInfo      ti;

    if( ta.ta == X86 ) {
        ti.guestHWcaps =
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
        ti.hostHWcaps =
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
    } else if( ta.ta == ARM ) {
        ti.guestHWcaps = 5|VEX_HWCAPS_ARM_VFP3;
        ti.hostHWcaps =
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
    } else {
        ti.guestHWcaps = 0;
        ti.hostHWcaps = 0;
    }

    ti.maxInstructions = 99;
    ti.chaseThreshold = 10;
    ti.opLevel = FullOpt;
    ti.tarch = ta;
	dls->ti = ti;
	initVex(dls, do_throw, dbg_spew);
	return dls;
}

void finiDecodeLib(void *ctx) {
	DecodeLibState	*dls = (DecodeLibState *)ctx;
	delete dls;
	return;
}

void initLibState(void) {
#ifndef WIN32
    //pthread_key_create(&jBuffTls, NULL);
#endif
    return;
}
