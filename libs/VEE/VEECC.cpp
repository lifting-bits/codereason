#include "VEE.h"

extern "C" {
#include <libvex_basictypes.h>
#include <libvex_guest_offsets.h>
#include <libvex_emnote.h>
#include <libvex.h>
}

using namespace std;

//helpers imported from guest_x86_helpers.c
/* eflags masks */
#define X86G_CC_SHIFT_O   11
#define X86G_CC_SHIFT_S   7
#define X86G_CC_SHIFT_Z   6
#define X86G_CC_SHIFT_A   4
#define X86G_CC_SHIFT_C   0
#define X86G_CC_SHIFT_P   2

#define X86G_CC_MASK_O    (1 << X86G_CC_SHIFT_O)
#define X86G_CC_MASK_S    (1 << X86G_CC_SHIFT_S)
#define X86G_CC_MASK_Z    (1 << X86G_CC_SHIFT_Z)
#define X86G_CC_MASK_A    (1 << X86G_CC_SHIFT_A)
#define X86G_CC_MASK_C    (1 << X86G_CC_SHIFT_C)
#define X86G_CC_MASK_P    (1 << X86G_CC_SHIFT_P)

/* FPU flag masks */
#define X86G_FC_SHIFT_C3   14
#define X86G_FC_SHIFT_C2   10
#define X86G_FC_SHIFT_C1   9
#define X86G_FC_SHIFT_C0   8

#define X86G_FC_MASK_C3    (1 << X86G_FC_SHIFT_C3)
#define X86G_FC_MASK_C2    (1 << X86G_FC_SHIFT_C2)
#define X86G_FC_MASK_C1    (1 << X86G_FC_SHIFT_C1)
#define X86G_FC_MASK_C0    (1 << X86G_FC_SHIFT_C0)

enum {
    X86G_CC_OP_COPY=0,  /* DEP1 = current flags, DEP2 = 0, NDEP = unused */
                        /* just copy DEP1 to output */

    X86G_CC_OP_ADDB,    /* 1 */
    X86G_CC_OP_ADDW,    /* 2 DEP1 = argL, DEP2 = argR, NDEP = unused */
    X86G_CC_OP_ADDL,    /* 3 */

    X86G_CC_OP_SUBB,    /* 4 */
    X86G_CC_OP_SUBW,    /* 5 DEP1 = argL, DEP2 = argR, NDEP = unused */
    X86G_CC_OP_SUBL,    /* 6 */

    X86G_CC_OP_ADCB,    /* 7 */
    X86G_CC_OP_ADCW,    /* 8 DEP1 = argL, DEP2 = argR ^ oldCarry, NDEP = oldCarry */
    X86G_CC_OP_ADCL,    /* 9 */

    X86G_CC_OP_SBBB,    /* 10 */
    X86G_CC_OP_SBBW,    /* 11 DEP1 = argL, DEP2 = argR ^ oldCarry, NDEP = oldCarry */
    X86G_CC_OP_SBBL,    /* 12 */

    X86G_CC_OP_LOGICB,  /* 13 */
    X86G_CC_OP_LOGICW,  /* 14 DEP1 = result, DEP2 = 0, NDEP = unused */
    X86G_CC_OP_LOGICL,  /* 15 */

    X86G_CC_OP_INCB,    /* 16 */
    X86G_CC_OP_INCW,    /* 17 DEP1 = result, DEP2 = 0, NDEP = oldCarry (0 or 1) */
    X86G_CC_OP_INCL,    /* 18 */

    X86G_CC_OP_DECB,    /* 19 */
    X86G_CC_OP_DECW,    /* 20 DEP1 = result, DEP2 = 0, NDEP = oldCarry (0 or 1) */
    X86G_CC_OP_DECL,    /* 21 */
    X86G_CC_OP_SHLB,    /* 22 DEP1 = res, DEP2 = res', NDEP = unused */
    X86G_CC_OP_SHLW,    /* 23 where res' is like res but shifted one bit less */
    X86G_CC_OP_SHLL,    /* 24 */

    X86G_CC_OP_SHRB,    /* 25 DEP1 = res, DEP2 = res', NDEP = unused */
    X86G_CC_OP_SHRW,    /* 26 where res' is like res but shifted one bit less */
    X86G_CC_OP_SHRL,    /* 27 */

    X86G_CC_OP_ROLB,    /* 28 */
    X86G_CC_OP_ROLW,    /* 29 DEP1 = res, DEP2 = 0, NDEP = old flags */
    X86G_CC_OP_ROLL,    /* 30 */

    X86G_CC_OP_RORB,    /* 31 */
    X86G_CC_OP_RORW,    /* 32 DEP1 = res, DEP2 = 0, NDEP = old flags */
    X86G_CC_OP_RORL,    /* 33 */

    X86G_CC_OP_UMULB,   /* 34 */
    X86G_CC_OP_UMULW,   /* 35 DEP1 = argL, DEP2 = argR, NDEP = unused */
    X86G_CC_OP_UMULL,   /* 36 */

    X86G_CC_OP_SMULB,   /* 37 */
    X86G_CC_OP_SMULW,   /* 38 DEP1 = argL, DEP2 = argR, NDEP = unused */
    X86G_CC_OP_SMULL,   /* 39 */

    X86G_CC_OP_NUMBER
};

typedef
   enum {
      X86CondO      = 0,  /* overflow           */
      X86CondNO     = 1,  /* no overflow        */

      X86CondB      = 2,  /* below              */
      X86CondNB     = 3,  /* not below          */

      X86CondZ      = 4,  /* zero               */
      X86CondNZ     = 5,  /* not zero           */

      X86CondBE     = 6,  /* below or equal     */
      X86CondNBE    = 7,  /* not below or equal */

      X86CondS      = 8,  /* negative           */
      X86CondNS     = 9,  /* not negative       */

      X86CondP      = 10, /* parity even        */
      X86CondNP     = 11, /* not parity even    */

      X86CondL      = 12, /* jump less          */
      X86CondNL     = 13, /* not less           */

      X86CondLE     = 14, /* less or equal      */
      X86CondNLE    = 15, /* not less or equal  */

      X86CondAlways = 16  /* HACK */
   }
   X86Condcode;

static const UChar parity_table[256] = {
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0,
    0, X86G_CC_MASK_P, X86G_CC_MASK_P, 0, X86G_CC_MASK_P, 0, 0, X86G_CC_MASK_P,
};

/* generalised left-shifter */
inline static Int lshift ( Int x, Int n )
{
   if (n >= 0)
      return x << n;
   else
      return x >> (-n);
}

/* identity on ULong */
static inline ULong idULong ( ULong x )
{
   return x;
}

#define PREAMBLE(__data_bits)                   \
   /* const */ UInt DATA_MASK                   \
      = __data_bits==8 ? 0xFF                   \
                       : (__data_bits==16 ? 0xFFFF      \
                                          : 0xFFFFFFFF);    \
   /* const */ UInt SIGN_MASK = 1 << (__data_bits - 1);     \
   /* const */ UInt CC_DEP1 = cc_dep1_formal;           \
   /* const */ UInt CC_DEP2 = cc_dep2_formal;           \
   /* const */ UInt CC_NDEP = cc_ndep_formal;           \
   /* Four bogus assignments, which hopefully gcc can     */    \
   /* optimise away, and which stop it complaining about  */    \
   /* unused variables.                                   */    \
   SIGN_MASK = SIGN_MASK;                   \
   DATA_MASK = DATA_MASK;                   \
   CC_DEP2 = CC_DEP2;                       \
   CC_NDEP = CC_NDEP;

#define ACTIONS_ADD(DATA_BITS,DATA_UTYPE)           \
{                               \
   PREAMBLE(DATA_BITS);                     \
   { Int cf, pf, af, zf, sf, of;                \
     Int argL, argR, res;                   \
     argL = CC_DEP1;                        \
     argR = CC_DEP2;                        \
     res  = argL + argR;                    \
     cf = (DATA_UTYPE)res < (DATA_UTYPE)argL;           \
     pf = parity_table[(UChar)res];             \
     af = (res ^ argL ^ argR) & 0x10;               \
     zf = ((DATA_UTYPE)res == 0) << 6;              \
     sf = lshift(res, 8 - DATA_BITS) & 0x80;            \
     of = lshift((argL ^ argR ^ -1) & (argL ^ res),         \
                 12 - DATA_BITS) & X86G_CC_MASK_O;      \
     return cf | pf | af | zf | sf | of;            \
   }                                \
}

/*-------------------------------------------------------------*/

#define ACTIONS_SUB(DATA_BITS,DATA_UTYPE)           \
{                               \
   PREAMBLE(DATA_BITS);                     \
   { Int cf, pf, af, zf, sf, of;                \
     Int argL, argR, res;                   \
     argL = CC_DEP1;                        \
     argR = CC_DEP2;                        \
     res  = argL - argR;                    \
     cf = (DATA_UTYPE)argL < (DATA_UTYPE)argR;          \
     pf = parity_table[(UChar)res];             \
     af = (res ^ argL ^ argR) & 0x10;               \
     zf = ((DATA_UTYPE)res == 0) << 6;              \
     sf = lshift(res, 8 - DATA_BITS) & 0x80;            \
     of = lshift((argL ^ argR) & (argL ^ res),          \
                 12 - DATA_BITS) & X86G_CC_MASK_O;      \
     return cf | pf | af | zf | sf | of;            \
   }                                \
}

#define ACTIONS_ADC(DATA_BITS,DATA_UTYPE)           \
{                               \
   PREAMBLE(DATA_BITS);                     \
   { Int cf, pf, af, zf, sf, of;                \
     Int argL, argR, oldC, res;                     \
     oldC = CC_NDEP & X86G_CC_MASK_C;               \
     argL = CC_DEP1;                        \
     argR = CC_DEP2 ^ oldC;                     \
     res  = (argL + argR) + oldC;               \
     if (oldC)                          \
        cf = (DATA_UTYPE)res <= (DATA_UTYPE)argL;       \
     else                           \
        cf = (DATA_UTYPE)res < (DATA_UTYPE)argL;        \
     pf = parity_table[(UChar)res];             \
     af = (res ^ argL ^ argR) & 0x10;               \
     zf = ((DATA_UTYPE)res == 0) << 6;              \
     sf = lshift(res, 8 - DATA_BITS) & 0x80;            \
     of = lshift((argL ^ argR ^ -1) & (argL ^ res),         \
                  12 - DATA_BITS) & X86G_CC_MASK_O;     \
     return cf | pf | af | zf | sf | of;            \
   }                                \
}

#define ACTIONS_SBB(DATA_BITS,DATA_UTYPE)           \
{                               \
   PREAMBLE(DATA_BITS);                     \
   { Int cf, pf, af, zf, sf, of;                \
     Int argL, argR, oldC, res;                     \
     oldC = CC_NDEP & X86G_CC_MASK_C;               \
     argL = CC_DEP1;                        \
     argR = CC_DEP2 ^ oldC;                     \
     res  = (argL - argR) - oldC;               \
     if (oldC)                          \
        cf = (DATA_UTYPE)argL <= (DATA_UTYPE)argR;      \
     else                           \
        cf = (DATA_UTYPE)argL < (DATA_UTYPE)argR;       \
     pf = parity_table[(UChar)res];             \
     af = (res ^ argL ^ argR) & 0x10;               \
     zf = ((DATA_UTYPE)res == 0) << 6;              \
     sf = lshift(res, 8 - DATA_BITS) & 0x80;            \
     of = lshift((argL ^ argR) & (argL ^ res),          \
                 12 - DATA_BITS) & X86G_CC_MASK_O;      \
     return cf | pf | af | zf | sf | of;            \
   }                                \
}

/*-------------------------------------------------------------*/

#define ACTIONS_LOGIC(DATA_BITS,DATA_UTYPE)         \
{                               \
   PREAMBLE(DATA_BITS);                     \
   { Int cf, pf, af, zf, sf, of;                \
     cf = 0;                            \
     pf = parity_table[(UChar)CC_DEP1];             \
     af = 0;                            \
     zf = ((DATA_UTYPE)CC_DEP1 == 0) << 6;          \
     sf = lshift(CC_DEP1, 8 - DATA_BITS) & 0x80;        \
     of = 0;                            \
     return cf | pf | af | zf | sf | of;            \
   }                                \
}

#define ACTIONS_INC(DATA_BITS,DATA_UTYPE)           \
{                               \
   PREAMBLE(DATA_BITS);                     \
   { Int cf, pf, af, zf, sf, of;                \
     Int argL, argR, res;                   \
     res  = CC_DEP1;                        \
     argL = res - 1;                        \
     argR = 1;                          \
     cf = CC_NDEP & X86G_CC_MASK_C;             \
     pf = parity_table[(UChar)res];             \
     af = (res ^ argL ^ argR) & 0x10;               \
     zf = ((DATA_UTYPE)res == 0) << 6;              \
     sf = lshift(res, 8 - DATA_BITS) & 0x80;            \
     of = ((res & DATA_MASK) == SIGN_MASK) << 11;       \
     return cf | pf | af | zf | sf | of;            \
   }                                \
}

/*-------------------------------------------------------------*/

#define ACTIONS_DEC(DATA_BITS,DATA_UTYPE)           \
{                               \
   PREAMBLE(DATA_BITS);                     \
   { Int cf, pf, af, zf, sf, of;                \
     Int argL, argR, res;                   \
     res  = CC_DEP1;                        \
     argL = res + 1;                        \
     argR = 1;                          \
     cf = CC_NDEP & X86G_CC_MASK_C;             \
     pf = parity_table[(UChar)res];             \
     af = (res ^ argL ^ argR) & 0x10;               \
     zf = ((DATA_UTYPE)res == 0) << 6;              \
     sf = lshift(res, 8 - DATA_BITS) & 0x80;            \
     of = ((res & DATA_MASK)                    \
          == ((UInt)SIGN_MASK - 1)) << 11;          \
     return cf | pf | af | zf | sf | of;            \
   }                                \
}

#define ACTIONS_SHL(DATA_BITS,DATA_UTYPE)           \
{                               \
   PREAMBLE(DATA_BITS);                     \
   { Int cf, pf, af, zf, sf, of;                \
     cf = (CC_DEP2 >> (DATA_BITS - 1)) & X86G_CC_MASK_C;    \
     pf = parity_table[(UChar)CC_DEP1];             \
     af = 0; /* undefined */                    \
     zf = ((DATA_UTYPE)CC_DEP1 == 0) << 6;          \
     sf = lshift(CC_DEP1, 8 - DATA_BITS) & 0x80;        \
     /* of is defined if shift count == 1 */            \
     of = lshift(CC_DEP2 ^ CC_DEP1, 12 - DATA_BITS)         \
          & X86G_CC_MASK_O;                 \
     return cf | pf | af | zf | sf | of;            \
   }                                \
}

/*-------------------------------------------------------------*/

#define ACTIONS_SHR(DATA_BITS,DATA_UTYPE)           \
{                               \
   PREAMBLE(DATA_BITS);                     \
   { Int cf, pf, af, zf, sf, of;                \
     cf = CC_DEP2 & 1;                      \
     pf = parity_table[(UChar)CC_DEP1];             \
     af = 0; /* undefined */                    \
     zf = ((DATA_UTYPE)CC_DEP1 == 0) << 6;          \
     sf = lshift(CC_DEP1, 8 - DATA_BITS) & 0x80;        \
     /* of is defined if shift count == 1 */            \
     of = lshift(CC_DEP2 ^ CC_DEP1, 12 - DATA_BITS)     \
          & X86G_CC_MASK_O;                 \
     return cf | pf | af | zf | sf | of;            \
   }                                \
}

#define ACTIONS_ROL(DATA_BITS,DATA_UTYPE)           \
{                               \
   PREAMBLE(DATA_BITS);                     \
   { Int fl                             \
        = (CC_NDEP & ~(X86G_CC_MASK_O | X86G_CC_MASK_C))    \
          | (X86G_CC_MASK_C & CC_DEP1)              \
          | (X86G_CC_MASK_O & (lshift(CC_DEP1,          \
                                      11-(DATA_BITS-1))     \
                     ^ lshift(CC_DEP1, 11)));           \
     return fl;                         \
   }                                \
}

/*-------------------------------------------------------------*/

/* ROR: cf' = msb(result).  of' = msb(result) ^ msb-1(result). */
/* DEP1 = result, NDEP = old flags */
#define ACTIONS_ROR(DATA_BITS,DATA_UTYPE)           \
{                               \
   PREAMBLE(DATA_BITS);                     \
   { Int fl                             \
        = (CC_NDEP & ~(X86G_CC_MASK_O | X86G_CC_MASK_C))    \
          | (X86G_CC_MASK_C & (CC_DEP1 >> (DATA_BITS-1)))   \
          | (X86G_CC_MASK_O & (lshift(CC_DEP1,          \
                                      11-(DATA_BITS-1))     \
                     ^ lshift(CC_DEP1, 11-(DATA_BITS-1)+1)));   \
     return fl;                         \
   }                                \
}

#define ACTIONS_UMUL(DATA_BITS, DATA_UTYPE,  NARROWtoU,         \
                                DATA_U2TYPE, NARROWto2U)        \
{                                                               \
   PREAMBLE(DATA_BITS);                                         \
   { Int cf, pf, af, zf, sf, of;                                \
     DATA_UTYPE  hi;                                            \
     DATA_UTYPE  lo                                             \
        = NARROWtoU( ((DATA_UTYPE)CC_DEP1)                      \
                     * ((DATA_UTYPE)CC_DEP2) );                 \
     DATA_U2TYPE rr                                             \
        = NARROWto2U(                                           \
             ((DATA_U2TYPE)((DATA_UTYPE)CC_DEP1))               \
             * ((DATA_U2TYPE)((DATA_UTYPE)CC_DEP2)) );          \
     hi = NARROWtoU(rr >>/*u*/ DATA_BITS);                      \
     cf = (hi != 0);                                            \
     pf = parity_table[(UChar)lo];                              \
     af = 0; /* undefined */                                    \
     zf = (lo == 0) << 6;                                       \
     sf = lshift(lo, 8 - DATA_BITS) & 0x80;                     \
     of = cf << 11;                                             \
     return cf | pf | af | zf | sf | of;                        \
   }                                \
}

#define ACTIONS_SMUL(DATA_BITS, DATA_STYPE,  NARROWtoS,         \
                                DATA_S2TYPE, NARROWto2S)        \
{                                                               \
   PREAMBLE(DATA_BITS);                                         \
   { Int cf, pf, af, zf, sf, of;                                \
     DATA_STYPE  hi;                                            \
     DATA_STYPE  lo                                             \
        = NARROWtoS( ((DATA_STYPE)CC_DEP1)                      \
                     * ((DATA_STYPE)CC_DEP2) );                 \
     DATA_S2TYPE rr                                             \
        = NARROWto2S(                                           \
             ((DATA_S2TYPE)((DATA_STYPE)CC_DEP1))               \
             * ((DATA_S2TYPE)((DATA_STYPE)CC_DEP2)) );          \
     hi = NARROWtoS(rr >>/*s*/ DATA_BITS);                      \
     cf = (hi != (lo >>/*s*/ (DATA_BITS-1)));                   \
     pf = parity_table[(UChar)lo];                              \
     af = 0; /* undefined */                                    \
     zf = (lo == 0) << 6;                                       \
     sf = lshift(lo, 8 - DATA_BITS) & 0x80;                     \
     of = cf << 11;                                             \
     return cf | pf | af | zf | sf | of;                        \
   }                                \
}

UInt x86g_calculate_eflags_all_WRK ( UInt cc_op,
                                     UInt cc_dep1_formal,
                                     UInt cc_dep2_formal,
                                     UInt cc_ndep_formal )
{
   switch (cc_op) {
      case X86G_CC_OP_COPY:
         return cc_dep1_formal
                & (X86G_CC_MASK_O | X86G_CC_MASK_S | X86G_CC_MASK_Z
                   | X86G_CC_MASK_A | X86G_CC_MASK_C | X86G_CC_MASK_P);

      case X86G_CC_OP_ADDB:   ACTIONS_ADD( 8,  UChar  );
      case X86G_CC_OP_ADDW:   ACTIONS_ADD( 16, UShort );
      case X86G_CC_OP_ADDL:   ACTIONS_ADD( 32, UInt   );

      case X86G_CC_OP_ADCB:   ACTIONS_ADC( 8,  UChar  );
      case X86G_CC_OP_ADCW:   ACTIONS_ADC( 16, UShort );
      case X86G_CC_OP_ADCL:   ACTIONS_ADC( 32, UInt   );

      case X86G_CC_OP_SUBB:   ACTIONS_SUB(  8, UChar  );
      case X86G_CC_OP_SUBW:   ACTIONS_SUB( 16, UShort );
      case X86G_CC_OP_SUBL:   ACTIONS_SUB( 32, UInt   );

      case X86G_CC_OP_SBBB:   ACTIONS_SBB(  8, UChar  );
      case X86G_CC_OP_SBBW:   ACTIONS_SBB( 16, UShort );
      case X86G_CC_OP_SBBL:   ACTIONS_SBB( 32, UInt   );

      case X86G_CC_OP_LOGICB: ACTIONS_LOGIC(  8, UChar  );
      case X86G_CC_OP_LOGICW: ACTIONS_LOGIC( 16, UShort );
      case X86G_CC_OP_LOGICL: ACTIONS_LOGIC( 32, UInt   );

      case X86G_CC_OP_INCB:   ACTIONS_INC(  8, UChar  );
      case X86G_CC_OP_INCW:   ACTIONS_INC( 16, UShort );
      case X86G_CC_OP_INCL:   ACTIONS_INC( 32, UInt   );

      case X86G_CC_OP_DECB:   ACTIONS_DEC(  8, UChar  );
      case X86G_CC_OP_DECW:   ACTIONS_DEC( 16, UShort );
      case X86G_CC_OP_DECL:   ACTIONS_DEC( 32, UInt   );

      case X86G_CC_OP_SHLB:   ACTIONS_SHL(  8, UChar  );
      case X86G_CC_OP_SHLW:   ACTIONS_SHL( 16, UShort );
      case X86G_CC_OP_SHLL:   ACTIONS_SHL( 32, UInt   );
     
      case X86G_CC_OP_SHRB:   ACTIONS_SHR(  8, UChar  );
      case X86G_CC_OP_SHRW:   ACTIONS_SHR( 16, UShort );
      case X86G_CC_OP_SHRL:   ACTIONS_SHR( 32, UInt   );

      case X86G_CC_OP_ROLB:   ACTIONS_ROL(  8, UChar  );
      case X86G_CC_OP_ROLW:   ACTIONS_ROL( 16, UShort );
      case X86G_CC_OP_ROLL:   ACTIONS_ROL( 32, UInt   );

      case X86G_CC_OP_RORB:   ACTIONS_ROR(  8, UChar  );
      case X86G_CC_OP_RORW:   ACTIONS_ROR( 16, UShort );
      case X86G_CC_OP_RORL:   ACTIONS_ROR( 32, UInt   );

      case X86G_CC_OP_UMULB:  ACTIONS_UMUL(  8, UChar,  toUChar,
                                                UShort, toUShort );
      case X86G_CC_OP_UMULW:  ACTIONS_UMUL( 16, UShort, toUShort,
                                                UInt,   toUInt );
      case X86G_CC_OP_UMULL:  ACTIONS_UMUL( 32, UInt,   toUInt,
                                                ULong,  idULong );

      case X86G_CC_OP_SMULB:  ACTIONS_SMUL(  8, Char,   toUChar,
                                                Short,  toUShort );
      case X86G_CC_OP_SMULW:  ACTIONS_SMUL( 16, Short,  toUShort,
                                                Int,    toUInt   );
      case X86G_CC_OP_SMULL:  ACTIONS_SMUL( 32, Int,    toUInt,
                                                Long,   idULong );

      default:
        throw StepErr("INVALID STATE");
         /* shouldn't really make these calls from generated code */
         /*vex_printf("x86g_calculate_eflags_all_WRK(X86)"
                    "( %u, 0x%x, 0x%x, 0x%x )\n",
                    cc_op, cc_dep1_formal, cc_dep2_formal, cc_ndep_formal );
         vpanic("x86g_calculate_eflags_all_WRK(X86)");*/
   }

}


unsigned int x86g_calculate_eflags_c ( unsigned int cc_op,
                               unsigned int cc_dep1,
                               unsigned int cc_dep2,
                               unsigned int cc_ndep )
{
  switch (cc_op) {
      case X86G_CC_OP_LOGICL:
      case X86G_CC_OP_LOGICW:
      case X86G_CC_OP_LOGICB:
         return 0;
      case X86G_CC_OP_SUBL:
         return ((UInt)cc_dep1) < ((UInt)cc_dep2)
                   ? X86G_CC_MASK_C : 0;
      case X86G_CC_OP_SUBW:
         return ((UInt)(cc_dep1 & 0xFFFF)) < ((UInt)(cc_dep2 & 0xFFFF))
                   ? X86G_CC_MASK_C : 0;
      case X86G_CC_OP_SUBB:
         return ((UInt)(cc_dep1 & 0xFF)) < ((UInt)(cc_dep2 & 0xFF))
                   ? X86G_CC_MASK_C : 0;
      case X86G_CC_OP_INCL:
      case X86G_CC_OP_DECL:
         return cc_ndep & X86G_CC_MASK_C;
      default:
         break;
   }

    return x86g_calculate_eflags_all_WRK(cc_op,cc_dep1,cc_dep2,cc_ndep)
          & X86G_CC_MASK_C;
}

UInt x86g_calculate_condition ( UInt/*X86Condcode*/ cond,
                                UInt cc_op,
                                UInt cc_dep1,
                                UInt cc_dep2,
                                UInt cc_ndep )
{
   UInt eflags = x86g_calculate_eflags_all_WRK(cc_op, cc_dep1,
                                               cc_dep2, cc_ndep);
   UInt of,sf,zf,cf,pf;
   UInt inv = cond & 1;

   switch (cond) {
      case X86CondNO:
      case X86CondO: /* OF == 1 */
         of = eflags >> X86G_CC_SHIFT_O;
         return 1 & (inv ^ of);

      case X86CondNZ:
      case X86CondZ: /* ZF == 1 */
         zf = eflags >> X86G_CC_SHIFT_Z;
         return 1 & (inv ^ zf);

      case X86CondNB:
      case X86CondB: /* CF == 1 */
         cf = eflags >> X86G_CC_SHIFT_C;
         return 1 & (inv ^ cf);
         break;

      case X86CondNBE:
      case X86CondBE: /* (CF or ZF) == 1 */
         cf = eflags >> X86G_CC_SHIFT_C;
         zf = eflags >> X86G_CC_SHIFT_Z;
         return 1 & (inv ^ (cf | zf));
         break;

      case X86CondNS:
      case X86CondS: /* SF == 1 */
         sf = eflags >> X86G_CC_SHIFT_S;
         return 1 & (inv ^ sf);

      case X86CondNP:
      case X86CondP: /* PF == 1 */
         pf = eflags >> X86G_CC_SHIFT_P;
         return 1 & (inv ^ pf);
      case X86CondNL:
      case X86CondL: /* (SF xor OF) == 1 */
         sf = eflags >> X86G_CC_SHIFT_S;
         of = eflags >> X86G_CC_SHIFT_O;
         return 1 & (inv ^ (sf ^ of));
         break;

      case X86CondNLE:
      case X86CondLE: /* ((SF xor OF) or ZF)  == 1 */
         sf = eflags >> X86G_CC_SHIFT_S;
         of = eflags >> X86G_CC_SHIFT_O;
         zf = eflags >> X86G_CC_SHIFT_Z;
         return 1 & (inv ^ ((sf ^ of) | zf));
         break;

      default:
         /*vex_printf("x86g_calculate_condition( %u, %u, 0x%x, 0x%x, 0x%x )\n",
                    cond, cc_op, cc_dep1, cc_dep2, cc_ndep );
         vpanic("x86g_calculate_condition");*/
        throw StepErr("Unknown state");
   }

}

UInt x86g_calculate_eflags_all ( UInt cc_op,
                                 UInt cc_dep1,
                                 UInt cc_dep2,
                                 UInt cc_ndep )
{
   return
      x86g_calculate_eflags_all_WRK ( cc_op, cc_dep1, cc_dep2, cc_ndep );
}

static UInt calc_parity_8bit ( UInt w32 ) {
   UInt i;
   UInt p = 1;
   for (i = 0; i < 8; i++)
      p ^= (1 & (w32 >> i));
   return p;
}

UInt x86g_calculate_daa_das_aaa_aas ( UInt flags_and_AX, UInt opcode )
{  
   UInt r_AL = (flags_and_AX >> 0) & 0xFF;
   UInt r_AH = (flags_and_AX >> 8) & 0xFF;
   UInt r_O  = (flags_and_AX >> (16 + X86G_CC_SHIFT_O)) & 1;
   UInt r_S  = (flags_and_AX >> (16 + X86G_CC_SHIFT_S)) & 1;
   UInt r_Z  = (flags_and_AX >> (16 + X86G_CC_SHIFT_Z)) & 1;
   UInt r_A  = (flags_and_AX >> (16 + X86G_CC_SHIFT_A)) & 1;
   UInt r_C  = (flags_and_AX >> (16 + X86G_CC_SHIFT_C)) & 1;
   UInt r_P  = (flags_and_AX >> (16 + X86G_CC_SHIFT_P)) & 1;
   UInt result = 0;

   switch (opcode) {
      case 0x27: { /* DAA */
         UInt old_AL = r_AL;
         UInt old_C  = r_C;
         r_C = 0;
         if ((r_AL & 0xF) > 9 || r_A == 1) {
            r_AL = r_AL + 6;
            r_C  = old_C;
            if (r_AL >= 0x100) r_C = 1;
            r_A = 1;
         } else {
            r_A = 0;
         }  
         if (old_AL > 0x99 || old_C == 1) {
            r_AL = r_AL + 0x60;
            r_C  = 1;
         } else {
            r_C = 0;
         }
         /* O is undefined.  S Z and P are set according to the
        result. */
         r_AL &= 0xFF;
         r_O = 0; /* let's say */
         r_S = (r_AL & 0x80) ? 1 : 0;
         r_Z = (r_AL == 0) ? 1 : 0;
         r_P = calc_parity_8bit( r_AL );
         break;
      }
     case 0x2F: { /* DAS */
         UInt old_AL = r_AL;
         UInt old_C  = r_C;
         r_C = 0;
         if ((r_AL & 0xF) > 9 || r_A == 1) {
            Bool borrow = r_AL < 6;
            r_AL = r_AL - 6;
            r_C  = old_C;
            if (borrow) r_C = 1;
            r_A = 1;
         } else {
            r_A = 0;
         }
         if (old_AL > 0x99 || old_C == 1) {
            r_AL = r_AL - 0x60;
            r_C  = 1;
         } else {
            /* Intel docs are wrong: r_C = 0; */
         }
         /* O is undefined.  S Z and P are set according to the
        result. */
         r_AL &= 0xFF;
         r_O = 0; /* let's say */
         r_S = (r_AL & 0x80) ? 1 : 0;
         r_Z = (r_AL == 0) ? 1 : 0;
         r_P = calc_parity_8bit( r_AL );
         break;
      }
      case 0x37: { /* AAA */
         Bool nudge = r_AL > 0xF9;
         if ((r_AL & 0xF) > 9 || r_A == 1) {
            r_AL = r_AL + 6;
            r_AH = r_AH + 1 + (nudge ? 1 : 0);
            r_A  = 1;
            r_C  = 1;
            r_AL = r_AL & 0xF;
         } else {
            r_A  = 0;
            r_C  = 0;
            r_AL = r_AL & 0xF;
         }
         /* O S Z and P are undefined. */
         r_O = r_S = r_Z = r_P = 0; /* let's say */
         break;
      }
      case 0x3F: { /* AAS */
         Bool nudge = r_AL < 0x06;
         if ((r_AL & 0xF) > 9 || r_A == 1) {
            r_AL = r_AL - 6;
            r_AH = r_AH - 1 - (nudge ? 1 : 0);
            r_A  = 1;
            r_C  = 1;
            r_AL = r_AL & 0xF;
         } else {
            r_A  = 0;
            r_C  = 0;
            r_AL = r_AL & 0xF;
         }
         /* O S Z and P are undefined. */
         r_O = r_S = r_Z = r_P = 0; /* let's say */
         break;
      }
      default:
         throw StepErr("invalid state");
   }
   result =   ( (r_O & 1) << (16 + X86G_CC_SHIFT_O) )
            | ( (r_S & 1) << (16 + X86G_CC_SHIFT_S) )
            | ( (r_Z & 1) << (16 + X86G_CC_SHIFT_Z) )
            | ( (r_A & 1) << (16 + X86G_CC_SHIFT_A) )
            | ( (r_C & 1) << (16 + X86G_CC_SHIFT_C) )
            | ( (r_P & 1) << (16 + X86G_CC_SHIFT_P) )
            | ( (r_AH & 0xFF) << 8 )
            | ( (r_AL & 0xFF) << 0 );
   return result;
}

ULong x86g_calculate_RCL ( UInt arg, UInt rot_amt, UInt eflags_in, UInt sz )
{
   UInt tempCOUNT = rot_amt & 0x1F, cf=0, of=0, tempcf;

   switch (sz) {
      case 4:
         cf = (eflags_in >> X86G_CC_SHIFT_C) & 1;
         while (tempCOUNT > 0) {
            tempcf = (arg >> 31) & 1;
            arg    = (arg << 1) | (cf & 1);
            cf     = tempcf;
            tempCOUNT--;
         }
         of = ((arg >> 31) ^ cf) & 1;
         break;
      case 2:
         while (tempCOUNT >= 17) tempCOUNT -= 17;
         cf = (eflags_in >> X86G_CC_SHIFT_C) & 1;
         while (tempCOUNT > 0) {
            tempcf = (arg >> 15) & 1;
            arg    = 0xFFFF & ((arg << 1) | (cf & 1));
            cf     = tempcf;
            tempCOUNT--;
         }
         of = ((arg >> 15) ^ cf) & 1;
         break;
      case 1:
         while (tempCOUNT >= 9) tempCOUNT -= 9;
         cf = (eflags_in >> X86G_CC_SHIFT_C) & 1;
         while (tempCOUNT > 0) {
            tempcf = (arg >> 7) & 1;
            arg    = 0xFF & ((arg << 1) | (cf & 1));
            cf     = tempcf;
            tempCOUNT--;
         }
         of = ((arg >> 7) ^ cf) & 1;
         break;
      default:
        throw StepClientErr("Invalid state");
   }

   cf &= 1;
   of &= 1;
   eflags_in &= ~(X86G_CC_MASK_C | X86G_CC_MASK_O);
   eflags_in |= (cf << X86G_CC_SHIFT_C) | (of << X86G_CC_SHIFT_O);

   return (((ULong)eflags_in) << 32) | ((ULong)arg);
}

ULong x86g_calculate_RCR ( UInt arg, UInt rot_amt, UInt eflags_in, UInt sz )
{  
   UInt tempCOUNT = rot_amt & 0x1F, cf=0, of=0, tempcf;

   switch (sz) {
      case 4:
         cf        = (eflags_in >> X86G_CC_SHIFT_C) & 1;
         of        = ((arg >> 31) ^ cf) & 1;
         while (tempCOUNT > 0) {
            tempcf = arg & 1;
            arg    = (arg >> 1) | (cf << 31);
            cf     = tempcf;
            
         }
         break;
      case 2:
         while (tempCOUNT >= 17) tempCOUNT -= 17;
         cf        = (eflags_in >> X86G_CC_SHIFT_C) & 1;
         of        = ((arg >> 15) ^ cf) & 1;
         while (tempCOUNT > 0) {
            tempcf = arg & 1;
            arg    = ((arg >> 1) & 0x7FFF) | (cf << 15);
            cf     = tempcf;
            tempCOUNT--;
         }
         break;
      case 1:
         while (tempCOUNT >= 9) tempCOUNT -= 9;
         cf        = (eflags_in >> X86G_CC_SHIFT_C) & 1;
         of        = ((arg >> 7) ^ cf) & 1;
         while (tempCOUNT > 0) {
            tempcf = arg & 1;
            arg    = ((arg >> 1) & 0x7F) | (cf << 7);
            cf     = tempcf;
            tempCOUNT--;
         }
         break;
      default:
        throw StepClientErr("Invalid state");
   }

   cf &= 1;
   of &= 1;
   eflags_in &= ~(X86G_CC_MASK_C | X86G_CC_MASK_O);
   eflags_in |= (cf << X86G_CC_SHIFT_C) | (of << X86G_CC_SHIFT_O);

   return (((ULong)eflags_in) << 32) | ((ULong)arg);
}

UInt x86g_create_fpucw ( UInt fpround )
{
       fpround &= 3;
          return 0x037F | (fpround << 10);
}

static inline UChar abdU8 ( UChar xx, UChar yy ) {
   return toUChar(xx>yy ? xx-yy : yy-xx);
}

static inline ULong mk32x2 ( UInt w1, UInt w0 ) {
   return (((ULong)w1) << 32) | ((ULong)w0);
}

static inline UShort sel16x4_3 ( ULong w64 ) {
   UInt hi32 = toUInt(w64 >> 32);
   return toUShort(hi32 >> 16);
}
static inline UShort sel16x4_2 ( ULong w64 ) {
   UInt hi32 = toUInt(w64 >> 32);
   return toUShort(hi32);
}
static inline UShort sel16x4_1 ( ULong w64 ) {
   UInt lo32 = toUInt(w64);
   return toUShort(lo32 >> 16);
}
static inline UShort sel16x4_0 ( ULong w64 ) {
   UInt lo32 = toUInt(w64);
   return toUShort(lo32);
}

static inline UChar sel8x8_7 ( ULong w64 ) {
   UInt hi32 = toUInt(w64 >> 32);
   return toUChar(hi32 >> 24);
}
static inline UChar sel8x8_6 ( ULong w64 ) {
   UInt hi32 = toUInt(w64 >> 32);
   return toUChar(hi32 >> 16);
}
static inline UChar sel8x8_5 ( ULong w64 ) {
   UInt hi32 = toUInt(w64 >> 32);
   return toUChar(hi32 >> 8);
}
static inline UChar sel8x8_4 ( ULong w64 ) {
   UInt hi32 = toUInt(w64 >> 32);
   return toUChar(hi32 >> 0);
}
static inline UChar sel8x8_3 ( ULong w64 ) {
   UInt lo32 = toUInt(w64);
   return toUChar(lo32 >> 24);
}
static inline UChar sel8x8_2 ( ULong w64 ) {
   UInt lo32 = toUInt(w64);
   return toUChar(lo32 >> 16);
}
static inline UChar sel8x8_1 ( ULong w64 ) {
   UInt lo32 = toUInt(w64);
   return toUChar(lo32 >> 8);
}
static inline UChar sel8x8_0 ( ULong w64 ) {
   UInt lo32 = toUInt(w64);
   return toUChar(lo32 >> 0);
}

ULong x86g_check_fldcw ( UInt fpucw )
{  
   /* Decide on a rounding mode.  fpucw[11:10] holds it. */
   /* NOTE, encoded exactly as per enum IRRoundingMode. */
   UInt rmode = (fpucw >> 10) & 3;

   /* Detect any required emulation warnings. */
   VexEmNote ew = EmNote_NONE;

   if ((fpucw & 0x3F) != 0x3F) {
      /* unmasked exceptions! */
      ew = EmWarn_X86_x87exns;
   }
   else
   if (((fpucw >> 8) & 3) != 3) {
      /* unsupported precision */
      ew = EmWarn_X86_x87precision;
   }

   return (((ULong)ew) << 32) | ((ULong)rmode);
}

ULong x86g_calculate_mmx_psadbw ( ULong xx, ULong yy )
{  
   UInt t = 0;
   t += (UInt)abdU8( sel8x8_7(xx), sel8x8_7(yy) );
   t += (UInt)abdU8( sel8x8_6(xx), sel8x8_6(yy) );
   t += (UInt)abdU8( sel8x8_5(xx), sel8x8_5(yy) );
   t += (UInt)abdU8( sel8x8_4(xx), sel8x8_4(yy) );
   t += (UInt)abdU8( sel8x8_3(xx), sel8x8_3(yy) );
   t += (UInt)abdU8( sel8x8_2(xx), sel8x8_2(yy) );
   t += (UInt)abdU8( sel8x8_1(xx), sel8x8_1(yy) );
   t += (UInt)abdU8( sel8x8_0(xx), sel8x8_0(yy) );
   t &= 0xFFFF;
   return (ULong)t;
}

UInt x86g_calculate_aad_aam ( UInt flags_and_AX, UInt opcode )
{  
   UInt r_AL = (flags_and_AX >> 0) & 0xFF;
   UInt r_AH = (flags_and_AX >> 8) & 0xFF;
   UInt r_O  = (flags_and_AX >> (16 + X86G_CC_SHIFT_O)) & 1;
   UInt r_S  = (flags_and_AX >> (16 + X86G_CC_SHIFT_S)) & 1;
   UInt r_Z  = (flags_and_AX >> (16 + X86G_CC_SHIFT_Z)) & 1;
   UInt r_A  = (flags_and_AX >> (16 + X86G_CC_SHIFT_A)) & 1;
   UInt r_C  = (flags_and_AX >> (16 + X86G_CC_SHIFT_C)) & 1;
   UInt r_P  = (flags_and_AX >> (16 + X86G_CC_SHIFT_P)) & 1;
   UInt result = 0;

   switch (opcode) {
      case 0xD4: { /* AAM */
         r_AH = r_AL / 10;
         r_AL = r_AL % 10;
         break;
      }
      case 0xD5: { /* AAD */
         r_AL = ((r_AH * 10) + r_AL) & 0xff;
         r_AH = 0;
         break;
      }
      default:
        throw StepErr("Bad state");
   }
   r_O = 0; /* let's say (undefined) */
   r_C = 0; /* let's say (undefined) */
   r_A = 0; /* let's say (undefined) */
   r_S = (r_AL & 0x80) ? 1 : 0;
   r_Z = (r_AL == 0) ? 1 : 0;
   r_P = calc_parity_8bit( r_AL );

   result =   ( (r_O & 1) << (16 + X86G_CC_SHIFT_O) )
            | ( (r_S & 1) << (16 + X86G_CC_SHIFT_S) )
            | ( (r_Z & 1) << (16 + X86G_CC_SHIFT_Z) )
            | ( (r_A & 1) << (16 + X86G_CC_SHIFT_A) )
            | ( (r_C & 1) << (16 + X86G_CC_SHIFT_C) )
            | ( (r_P & 1) << (16 + X86G_CC_SHIFT_P) )
            | ( (r_AH & 0xFF) << 8 )
            | ( (r_AL & 0xFF) << 0 );
   return result;
}

ULong x86g_calculate_mmx_pmaddwd ( ULong xx, ULong yy )
{  
   return
      mk32x2(
         (((Int)(Short)sel16x4_3(xx)) * ((Int)(Short)sel16x4_3(yy)))
            + (((Int)(Short)sel16x4_2(xx)) * ((Int)(Short)sel16x4_2(yy))),
         (((Int)(Short)sel16x4_1(xx)) * ((Int)(Short)sel16x4_1(yy)))
            + (((Int)(Short)sel16x4_0(xx)) * ((Int)(Short)sel16x4_0(yy)))
      );
}
//ARM stuff

typedef
   enum {
      ARMCondEQ     = 0,  /* equal                         : Z=1 */
      ARMCondNE     = 1,  /* not equal                     : Z=0 */

      ARMCondHS     = 2,  /* >=u (higher or same)          : C=1 */
      ARMCondLO     = 3,  /* <u  (lower)                   : C=0 */

      ARMCondMI     = 4,  /* minus (negative)              : N=1 */
      ARMCondPL     = 5,  /* plus (zero or +ve)            : N=0 */

      ARMCondVS     = 6,  /* overflow                      : V=1 */
      ARMCondVC     = 7,  /* no overflow                   : V=0 */

      ARMCondHI     = 8,  /* >u   (higher)                 : C=1 && Z=0 */
      ARMCondLS     = 9,  /* <=u  (lower or same)          : C=0 || Z=1 */

      ARMCondGE     = 10, /* >=s (signed greater or equal) : N=V */
      ARMCondLT     = 11, /* <s  (signed less than)        : N!=V */

      ARMCondGT     = 12, /* >s  (signed greater)          : Z=0 && N=V */
      ARMCondLE     = 13, /* <=s (signed less or equal)    : Z=1 || N!=V */

      ARMCondAL     = 14, /* always (unconditional)        : 1 */
      ARMCondNV     = 15  /* never (unconditional):        : 0 */
      /* NB: ARM have deprecated the use of the NV condition code.
         You are now supposed to use MOV R0,R0 as a noop rather than
         MOVNV R0,R0 as was previously recommended.  Future processors
         may have the NV condition code reused to do other things.  */
   }
   ARMCondcode;

enum {
   ARMG_CC_OP_COPY=0,  /* DEP1 = NZCV in 31:28, DEP2 = 0, DEP3 = 0
                          just copy DEP1 to output */

   ARMG_CC_OP_ADD,     /* DEP1 = argL (Rn), DEP2 = argR (shifter_op),
                          DEP3 = 0 */

   ARMG_CC_OP_SUB,     /* DEP1 = argL (Rn), DEP2 = argR (shifter_op),
                          DEP3 = 0 */

   ARMG_CC_OP_ADC,     /* DEP1 = argL (Rn), DEP2 = arg2 (shifter_op),
                          DEP3 = oldC (in LSB) */

   ARMG_CC_OP_SBB,     /* DEP1 = argL (Rn), DEP2 = arg2 (shifter_op),
                          DEP3 = oldC (in LSB) */

   ARMG_CC_OP_LOGIC,   /* DEP1 = result, DEP2 = shifter_carry_out (in LSB),
                          DEP3 = old V flag (in LSB) */
  
   ARMG_CC_OP_MUL,     /* DEP1 = result, DEP2 = 0, DEP3 = oldC:old_V
                          (in bits 1:0) */

   ARMG_CC_OP_MULL,    /* DEP1 = resLO32, DEP2 = resHI32, DEP3 = oldC:old_V
                          (in bits 1:0) */

   ARMG_CC_OP_NUMBER
};

#define ARMG_CC_SHIFT_N  31
#define ARMG_CC_SHIFT_Z  30
#define ARMG_CC_SHIFT_C  29
#define ARMG_CC_SHIFT_V  28
#define ARMG_CC_SHIFT_Q  27

#define ARMG_CC_MASK_N    (1 << ARMG_CC_SHIFT_N)
#define ARMG_CC_MASK_Z    (1 << ARMG_CC_SHIFT_Z)
#define ARMG_CC_MASK_C    (1 << ARMG_CC_SHIFT_C)
#define ARMG_CC_MASK_V    (1 << ARMG_CC_SHIFT_V)
#define ARMG_CC_MASK_Q    (1 << ARMG_CC_SHIFT_Q)

static
UInt armg_calculate_flag_z ( UInt cc_op, UInt cc_dep1,
                             UInt cc_dep2, UInt cc_dep3 )
{
   switch (cc_op) {
      case ARMG_CC_OP_COPY: {
         /* (nzcv:28x0, unused, unused) */
         UInt zf   = (cc_dep1 >> ARMG_CC_SHIFT_Z) & 1;
         return zf;
      }
      case ARMG_CC_OP_ADD: {
         /* (argL, argR, unused) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt res  = argL + argR;
         UInt zf   = res == 0;
         return zf;
      }
      case ARMG_CC_OP_SUB: {
         /* (argL, argR, unused) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt res  = argL - argR;
         UInt zf   = res == 0;
         return zf;
      }
      case ARMG_CC_OP_ADC: {
         /* (argL, argR, oldC) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt oldC = cc_dep3;
         assert((oldC & ~1) == 0);
         UInt res  = argL + argR + oldC;
         UInt zf   = res == 0;
         return zf;
      }
      case ARMG_CC_OP_SBB: {
         /* (argL, argR, oldC) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt oldC = cc_dep3;
         assert((oldC & ~1) == 0);
         UInt res  = argL - argR - (oldC ^ 1);
         UInt zf   = res == 0;
         return zf;
      }
      case ARMG_CC_OP_LOGIC: {
         /* (res, shco, oldV) */
         UInt res  = cc_dep1;
         UInt zf   = res == 0;
         return zf;
      }
      case ARMG_CC_OP_MUL: {
         /* (res, unused, oldC:oldV) */
         UInt res  = cc_dep1;
         UInt zf   = res == 0;
         return zf;
      }
      case ARMG_CC_OP_MULL: {
         /* (resLo32, resHi32, oldC:oldV) */
         UInt resLo32 = cc_dep1;
         UInt resHi32 = cc_dep2;
         UInt zf      = (resHi32|resLo32) == 0;
         return zf;
      }
      default:
         /* shouldn't really make these calls from generated code */
         assert(!"armg_calculate_flags_z");
   }
}


UInt armg_calculate_flag_c ( UInt cc_op, UInt cc_dep1,
                             UInt cc_dep2, UInt cc_dep3 )
{
   switch (cc_op) {
      case ARMG_CC_OP_COPY: {
         /* (nzcv:28x0, unused, unused) */
         UInt cf   = (cc_dep1 >> ARMG_CC_SHIFT_C) & 1;
         return cf;
      }
      case ARMG_CC_OP_ADD: {
         /* (argL, argR, unused) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt res  = argL + argR;
         UInt cf   = res < argL;
         return cf;
      }
      case ARMG_CC_OP_SUB: {
         /* (argL, argR, unused) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt cf   = argL >= argR;
         return cf;
      }
      case ARMG_CC_OP_ADC: {
         /* (argL, argR, oldC) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt oldC = cc_dep3;
         assert((oldC & ~1) == 0);
         UInt res  = argL + argR + oldC;
         UInt cf   = oldC ? (res <= argL) : (res < argL);
         return cf;
      }
      case ARMG_CC_OP_SBB: {
         /* (argL, argR, oldC) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt oldC = cc_dep3;
         assert((oldC & ~1) == 0);
         UInt cf   = oldC ? (argL >= argR) : (argL > argR);
         return cf;
      }
      case ARMG_CC_OP_LOGIC: {
         /* (res, shco, oldV) */
         UInt shco = cc_dep2;
         assert((shco & ~1) == 0);
         UInt cf   = shco;
         return cf;
      }
      case ARMG_CC_OP_MUL: {
         /* (res, unused, oldC:oldV) */
         UInt oldC = (cc_dep3 >> 1) & 1;
         assert((cc_dep3 & ~3) == 0);
         UInt cf   = oldC;
         return cf;
      }
      case ARMG_CC_OP_MULL: {
         /* (resLo32, resHi32, oldC:oldV) */
         UInt oldC    = (cc_dep3 >> 1) & 1;
         assert((cc_dep3 & ~3) == 0);
         UInt cf      = oldC;
         return cf;
      }
      default:
         /* shouldn't really make these calls from generated code */
         assert(!"armg_calculate_flag_c");
    }
}

UInt armg_calculate_flag_n ( UInt cc_op, UInt cc_dep1,
                             UInt cc_dep2, UInt cc_dep3 )
{
   switch (cc_op) {
      case ARMG_CC_OP_COPY: {
         /* (nzcv:28x0, unused, unused) */
         UInt nf   = (cc_dep1 >> ARMG_CC_SHIFT_N) & 1;
         return nf;
      }
      case ARMG_CC_OP_ADD: {
         /* (argL, argR, unused) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt res  = argL + argR;
         UInt nf   = res >> 31;
         return nf;
      }
      case ARMG_CC_OP_SUB: {
         /* (argL, argR, unused) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt res  = argL - argR;
         UInt nf   = res >> 31;
         return nf;
      }
      case ARMG_CC_OP_ADC: {
         /* (argL, argR, oldC) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt oldC = cc_dep3;
         assert((oldC & ~1) == 0);
         UInt res  = argL + argR + oldC;
         UInt nf   = res >> 31;
         return nf;
      }
      case ARMG_CC_OP_SBB: {
         /* (argL, argR, oldC) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt oldC = cc_dep3;
         assert((oldC & ~1) == 0);
         UInt res  = argL - argR - (oldC ^ 1);
         UInt nf   = res >> 31;
         return nf;
      }
      case ARMG_CC_OP_LOGIC: {
         /* (res, shco, oldV) */
         UInt res  = cc_dep1;
         UInt nf   = res >> 31;
         return nf;
      }
      case ARMG_CC_OP_MUL: {
         /* (res, unused, oldC:oldV) */
         UInt res  = cc_dep1;
         UInt nf   = res >> 31;
         return nf;
      }
      case ARMG_CC_OP_MULL: {
         /* (resLo32, resHi32, oldC:oldV) */
         UInt resHi32 = cc_dep2;
         UInt nf      = resHi32 >> 31;
         return nf;
      }
      default:
         /* shouldn't really make these calls from generated code */
         assert(!"armg_calculate_flags_n");
}
}

UInt armg_calculate_flag_v ( UInt cc_op, UInt cc_dep1,
                             UInt cc_dep2, UInt cc_dep3 )
{
   switch (cc_op) {
      case ARMG_CC_OP_COPY: {
         /* (nzcv:28x0, unused, unused) */
         UInt vf   = (cc_dep1 >> ARMG_CC_SHIFT_V) & 1;
         return vf;
      }
      case ARMG_CC_OP_ADD: {
         /* (argL, argR, unused) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt res  = argL + argR;
         UInt vf   = ((res ^ argL) & (res ^ argR)) >> 31;
         return vf;
      }
      case ARMG_CC_OP_SUB: {
         /* (argL, argR, unused) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt res  = argL - argR;
         UInt vf   = ((argL ^ argR) & (argL ^ res)) >> 31;
         return vf;
      }
      case ARMG_CC_OP_ADC: {
         /* (argL, argR, oldC) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt oldC = cc_dep3;
         assert((oldC & ~1) == 0);
         UInt res  = argL + argR + oldC;
         UInt vf   = ((res ^ argL) & (res ^ argR)) >> 31;   
         return vf;
      }
      case ARMG_CC_OP_SBB: {
         /* (argL, argR, oldC) */
         UInt argL = cc_dep1;
         UInt argR = cc_dep2;
         UInt oldC = cc_dep3;
         assert((oldC & ~1) == 0);
         UInt res  = argL - argR - (oldC ^ 1);
         UInt vf   = ((argL ^ argR) & (argL ^ res)) >> 31;
         return vf;
      }
      case ARMG_CC_OP_LOGIC: {
         /* (res, shco, oldV) */
         UInt oldV = cc_dep3;
         assert((oldV & ~1) == 0);
         UInt vf   = oldV;
         return vf;
      }
      case ARMG_CC_OP_MUL: {
         /* (res, unused, oldC:oldV) */
         UInt oldV = (cc_dep3 >> 0) & 1;
         assert((cc_dep3 & ~3) == 0);
         UInt vf   = oldV;
         return vf;
      }
      case ARMG_CC_OP_MULL: {
         /* (resLo32, resHi32, oldC:oldV) */
         UInt oldV    = (cc_dep3 >> 0) & 1;
         assert((cc_dep3 & ~3) == 0);
         UInt vf      = oldV;
         return vf;
      }
      default:
         /* shouldn't really make these calls from generated code */
         assert(!"armg_calculate_flag_v");
   }
}


UInt armg_calculate_condition ( UInt cond_n_op /* (ARMCondcode << 4) | cc_op */,
                                UInt cc_dep1,
                                UInt cc_dep2, UInt cc_dep3 )
{
   UInt cond  = cond_n_op >> 4;
   UInt cc_op = cond_n_op & 0xF;
   UInt nf, zf, vf, cf, inv;
   //   vex_printf("XXXXXXXX %x %x %x %x\n", 
   //              cond_n_op, cc_dep1, cc_dep2, cc_dep3);

   // skip flags computation in this case
   if (cond == ARMCondAL) return 1;

   inv  = cond & 1;

   switch (cond) {
      case ARMCondEQ:    // Z=1         => z
      case ARMCondNE:    // Z=0
         zf = armg_calculate_flag_z(cc_op, cc_dep1, cc_dep2, cc_dep3);
         return inv ^ zf;

      case ARMCondHS:    // C=1         => c
      case ARMCondLO:    // C=0
         cf = armg_calculate_flag_c(cc_op, cc_dep1, cc_dep2, cc_dep3);
         return inv ^ cf;

      case ARMCondMI:    // N=1         => n
      case ARMCondPL:    // N=0
         nf = armg_calculate_flag_n(cc_op, cc_dep1, cc_dep2, cc_dep3);
         return inv ^ nf;

      case ARMCondVS:    // V=1         => v
      case ARMCondVC:    // V=0
         vf = armg_calculate_flag_v(cc_op, cc_dep1, cc_dep2, cc_dep3);
         return inv ^ vf;

      case ARMCondHI:    // C=1 && Z=0   => c & ~z
      case ARMCondLS:    // C=0 || Z=1
         cf = armg_calculate_flag_c(cc_op, cc_dep1, cc_dep2, cc_dep3);
         zf = armg_calculate_flag_z(cc_op, cc_dep1, cc_dep2, cc_dep3);
         return inv ^ (cf & ~zf);

      case ARMCondGE:    // N=V          => ~(n^v)
      case ARMCondLT:    // N!=V
         nf = armg_calculate_flag_n(cc_op, cc_dep1, cc_dep2, cc_dep3);
         vf = armg_calculate_flag_v(cc_op, cc_dep1, cc_dep2, cc_dep3);
         return inv ^ (1 & ~(nf ^ vf));

      case ARMCondGT:    // Z=0 && N=V   => ~z & ~(n^v)  =>  ~(z | (n^v))
      case ARMCondLE:    // Z=1 || N!=V
         nf = armg_calculate_flag_n(cc_op, cc_dep1, cc_dep2, cc_dep3);
         vf = armg_calculate_flag_v(cc_op, cc_dep1, cc_dep2, cc_dep3);
         zf = armg_calculate_flag_z(cc_op, cc_dep1, cc_dep2, cc_dep3);
         return inv ^ (1 & ~(zf | (nf ^ vf)));

      case ARMCondAL: // handled above
      case ARMCondNV: // should never get here: Illegal instr
      default:
         /* shouldn't really make these calls from generated code */
         assert(!"armg_calculate_condition(ARM)");
   }
}

ConstantValue VexExecutionEngine::stepCCall(ExCCallPtr cc) {
    ConstantValue   v;
    vector<ConstantValue> constVec;

    v.valueIsKnown = false;
    v.width = 32;
    v.valueType = ConstantValue::T_I32;

    getConstArgs(cc->getArgs(), constVec);
    string fn  = cc->getTarget();

    if( fn == "x86g_calculate_condition" ) {
        //to correctly do this one we need 5 known values
        if( constVec.size() == 5 ) {
            //okay do it            
            ConstantValue   v1 = constVec[0];
            ConstantValue   v2 = constVec[1];
            ConstantValue   v3 = constVec[2];
            ConstantValue   v4 = constVec[3];
            ConstantValue   v5 = constVec[4];

            v.U32 = x86g_calculate_condition(v1.U32,
                                             v2.U32,
                                             v3.U32,
                                             v4.U32,
                                             v5.U32);
        }
    } else if( fn == "x86g_calculate_eflags_c" ) {
        if( constVec.size() == 4 ) {
            //okay do it
            ConstantValue   v1 = constVec[0];
            ConstantValue   v2 = constVec[1];
            ConstantValue   v3 = constVec[2];
            ConstantValue   v4 = constVec[3];

            v.U32 = x86g_calculate_eflags_c(v1.U32, v2.U32, v3.U32, v4.U32);
            v.valueIsKnown = true;
        }
    } else if( fn == "x86g_calculate_eflags_all" ) {
        if( constVec.size() == 4 ) {
            ConstantValue   v1 = constVec[0];
            ConstantValue   v2 = constVec[1];
            ConstantValue   v3 = constVec[2];
            ConstantValue   v4 = constVec[3];

            v.U32 = x86g_calculate_eflags_all(v1.U32, v2.U32, v3.U32, v4.U32);
            v.valueIsKnown = true;
        }
    } else if( fn == "x86g_calculate_daa_das_aaa_aas" ) {
        if( constVec.size() == 2 ) {
            ConstantValue   v1 = constVec[0];
            ConstantValue   v2 = constVec[1];

            v.valueIsKnown = true;
            v.U32 = x86g_calculate_daa_das_aaa_aas(v1.U32, v2.U32); 
        }
    } else if( fn == "x86g_use_seg_selector" ) {
        if( constVec.size() == 4 ) {
            //TODO: segment selection
            throw StepErr("We don't carry enough info to deal with this now"); 
        }
    } else if( fn == "x86g_calculate_RCL" ) {
        if( constVec.size() == 4 ) {
            ConstantValue   v1 = constVec[0];
            ConstantValue   v2 = constVec[1];
            ConstantValue   v3 = constVec[2];
            ConstantValue   v4 = constVec[3];

            v.valueIsKnown = true;
			v.width = 64;
			v.valueType = ConstantValue::T_I64;
            v.U64 = x86g_calculate_RCL(v1.U32, v2.U32, v3.U32, v4.U32);
        }
    } else if( fn == "x86g_calculate_RCR" ) {
        if( constVec.size() == 4 ) {
            ConstantValue   v1 = constVec[0];
            ConstantValue   v2 = constVec[1];
            ConstantValue   v3 = constVec[2];
            ConstantValue   v4 = constVec[3];

            v.valueIsKnown = true;
			v.width = 64;
			v.valueType = ConstantValue::T_I64;
            v.U64 = x86g_calculate_RCR(v1.U32, v2.U32, v3.U32, v4.U32);
        }
    } else if( fn == "x86g_create_fpucw" ) {
        if( constVec.size() == 1 ) {
            ConstantValue   v1 = constVec[0];

            v.valueIsKnown = true;
            v.U32 = x86g_create_fpucw(v1.U32);
        }
    } else if( fn == "x86g_calculate_mmx_psadbw" ) {
        if( constVec.size() == 2 ) {
            ConstantValue   v1 = constVec[0];
            ConstantValue   v2 = constVec[1];

            v.valueIsKnown = true;
			v.width = 64;
			v.valueType = ConstantValue::T_I64;
            v.U64 = x86g_calculate_mmx_psadbw(v1.U32, v2.U32);
        }
    } else if( fn == "x86g_check_fldcw" ) {
        if( constVec.size() == 1 ) {
            ConstantValue   v1 = constVec[0];

            v.valueIsKnown = true;
			
			v.width = 64;
			v.valueType = ConstantValue::T_I64;
            v.U64 = x86g_check_fldcw(v1.U32);
        }
    } else if( fn == "x86g_calculate_aad_aam" ) {
        if( constVec.size() == 2 ) {
            ConstantValue   v1 = constVec[0];
            ConstantValue   v2 = constVec[1];

            v.valueIsKnown = true;

            v.U32 = x86g_calculate_aad_aam(v1.U32, v2.U32);
        }
    } else if( fn == "x86g_calculate_mmx_pmaddwd" ) {
        if( constVec.size() == 2 ) {
            ConstantValue   v1 = constVec[0];
            ConstantValue   v2 = constVec[1];

            v.width = 64;
            v.valueType = ConstantValue::T_I64;

            v.valueIsKnown = true;

            v.U64 = x86g_calculate_mmx_pmaddwd(v1.U64, v2.U64);
        }
    } else if( fn == "armg_calculate_condition" ) {
        if( constVec.size() == 4 ) {
            ConstantValue   v1 = constVec[0];
            ConstantValue   v2 = constVec[1];
            ConstantValue   v3 = constVec[2];
            ConstantValue   v4 = constVec[3];

            v.width = 32;
            v.valueType = ConstantValue::T_I32;
            v.valueIsKnown = true;

            v.U32 = armg_calculate_condition(v1.U32, v2.U32, v3.U32, v4.U32);
        }
    } else if( fn == "armg_calculate_flag_v" ) {
        if( constVec.size() == 4 ) {
            ConstantValue   v1 = constVec[0];
            ConstantValue   v2 = constVec[1];
            ConstantValue   v3 = constVec[2];
            ConstantValue   v4 = constVec[3];

            v.width = 32;
            v.valueType = ConstantValue::T_I32;
            v.valueIsKnown = true;

            v.U32 = armg_calculate_flag_v(v1.U32, v2.U32, v3.U32, v4.U32);
        }
    } else if( fn == "armg_calculate_flag_c" ) {
        if( constVec.size() == 4 ) {
            ConstantValue   v1 = constVec[0];
            ConstantValue   v2 = constVec[1];
            ConstantValue   v3 = constVec[2];
            ConstantValue   v4 = constVec[3];

            v.width = 32;
            v.valueType = ConstantValue::T_I32;
            v.valueIsKnown = true;

            v.U32 = armg_calculate_flag_c(v1.U32, v2.U32, v3.U32, v4.U32);
        }
    } else {
        throw StepErr("Unsupported CCall");
    }

    return v;
}


