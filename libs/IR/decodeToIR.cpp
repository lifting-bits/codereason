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


// stub functions for the new vex lib
void disp_cp_chain_me_to_slowEP() {}
void disp_cp_chain_me_to_fastEP() {}
void disp_cp_xindir() {}
void disp_cp_xassisted() {}

static Bool chase_into_not_ok ( void* opaque, Addr dst ) { return False; }
//static Bool chase_into_not_ok ( void* opaque, Addr64 dst ) { return False; }
#ifdef _MSC_BUILD
//typedef IRSB *(__cdecl *vexInstTy)(void *,IRSB *,VexGuestLayout *,VexGuestExtents *,IRType,IRType);
typedef IRSB *(__cdecl *vexInstTy)(void *,IRSB *,const VexGuestLayout *,const VexGuestExtents *,const VexArchInfo*, IRType,IRType);
#else
typedef IRSB *(*vexInstTy)(void *,IRSB *,const VexGuestLayout *,const VexGuestExtents *,const VexArchInfo*, IRType,IRType);
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
  unsigned int    maxSize;
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
void log_bytes ( const HChar* bytes, SizeT nbytes )
{
	//char b[] = { 'V', 'L', 'O', 'G', '>', ' '};
	//fwrite ( b, 1, sizeof(b), stdout );
	fwrite ( bytes, 1, nbytes, stdout );
	return;
}

static
void eat_bytes( const HChar *bytes, SizeT nbytes ) 
{
	return;
}

static UInt needs_self_check ( void* opaque, VexRegisterUpdates * vru, const VexGuestExtents* vge ) {
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
			LibVEX_Init(failure_throw, log_bytes, 1, &dls->vcon);
		} else {
			LibVEX_Init(failure_throw, eat_bytes, 1, &dls->vcon);
		}
	} else {
		if( dbg_spew ){
			LibVEX_Init(failure_exit, log_bytes, 1, &dls->vcon);
		} else {
			LibVEX_Init(failure_exit, eat_bytes, 1, &dls->vcon);
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
convertToOneBlockCb(void *ctx,
				  IRSB *bb_in,
				  const VexGuestLayout *vgl,
				  const VexGuestExtents *vge,
				  const VexArchInfo *vgi, // in new valgrind, currently used. TODO see if we should
				  IRType			girty,
				  IRType			hirty)
{
  OneBlockTransCtx	*obtc = (OneBlockTransCtx *)ctx;
  if(bb_in->stmts_used < obtc->maxSize) {
    VBlockPtr b = VBlockPtr(new VBlock(obtc->curId, obtc->ta));
    b->buildFromIRSB(bb_in);
    obtc->b = boost::static_pointer_cast<Block>(b);
  }

	return bb_in;
}

/*
 */
bool runVEXOnBlobWithCallback(	DecodeLibState	*dls,
							    unsigned char	*b,
								unsigned long	bufExtents,
								uint64_t b_addr,
								vexInstTy		func, 
								void			*ctx,
								unsigned long	*readBytes,
								TargetArch arch)
{
	bool				didGood = false;
	VexTranslateArgs	vta;
	VexTranslateResult	vtr;
	VexGuestExtents		vge;
	//UChar				transbuf[N_TRANSBUF];
	//Int					trans_used;

	memset(&vta, 0, sizeof(VexTranslateArgs));
    memset(&vge, 0, sizeof(VexGuestExtents));	

	vta.instrument1 = func;
	vta.callback_opaque = ctx;

	vta.abiinfo_both    = dls->vbi;
	vta.chase_into_ok   = chase_into_not_ok;

  vta.guest_bytes     = b;
  vta.guest_bytes_addr = (Addr64)b_addr;
	vta.guest_extents   = &vge;
	vta.archinfo_guest = dls->vai_guest;
	vta.archinfo_guest.endness = VexEndnessLE;
  vta.arch_guest = (VexArch)dls->ti.tarch.ta;

  assert(arch.ta == dls->ti.tarch.ta);

  vta.host_bytes      = NULL;
  vta.host_bytes_size = 0;
  vta.host_bytes_used = NULL;
  vta.arch_host = vta.arch_guest;
  vta.archinfo_host = dls->vai_guest;
  vta.archinfo_host.endness = vta.archinfo_guest.endness;

	//vta.do_self_check = False;
	vta.preamble_function = NULL;

    vta.needs_self_check = needs_self_check;
	vta.traceflags = 0; //-1;
	vta.finaltidy = NULL;

    vta.disp_cp_chain_me_to_slowEP = (const void *)disp_cp_chain_me_to_slowEP;
    vta.disp_cp_chain_me_to_fastEP = (const void *)disp_cp_chain_me_to_fastEP;
    vta.disp_cp_xindir = (const void *)disp_cp_xindir;
    vta.disp_cp_xassisted = (const void *)disp_cp_xassisted;

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

bool convertToOneBlock( void				  *ctx,
					              unsigned char	*buf,
					              unsigned long	bufLen,
					              uint64_t baseAddr,
					              TargetArch		arch,
                        unsigned int  maxNumStmts,
					              BlockPtr      &blockOut)
{
	bool				result=true;
	DecodeLibState		*dls = (DecodeLibState*)ctx;
	OneBlockTransCtx	obtc;
	unsigned long		nbytes;

	obtc.curId = 0;
  obtc.ta = arch;
  obtc.maxSize = maxNumStmts;

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
                                        &nbytes,
                                        arch);

    if(!(obtc.b)) {
      result = false;
    }
    if( result ) {
        blockOut = obtc.b;
    }

	return result;
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
            VEX_HWCAPS_X86_MMXEXT|VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
        ti.hostHWcaps =
            VEX_HWCAPS_X86_MMXEXT|VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
    } else if( ta.ta == ARM ) {
        ti.guestHWcaps = 5|VEX_HWCAPS_ARM_VFP3;
        ti.hostHWcaps =
            VEX_HWCAPS_X86_MMXEXT|VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
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
