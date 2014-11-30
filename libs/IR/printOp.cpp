#include "VexIR.h"

std::string Op::printOp(void) {
    std::string r="";

    switch(this->op) {
        case Add: r.append("Add"); break;
        case Sub: r.append("Sub"); break;
        case Mul: r.append("Mul"); break;
        case MulU: r.append("MulU"); break;
        case MulS: r.append("MulS"); break;
        case Or: r.append("Or"); break;
        case And: r.append("And"); break;
        case Xor: r.append("Xor"); break;
        case Shl: r.append("Shl"); break;
        case Shr: r.append("Shr"); break;
        case Sar: r.append("Sar"); break;
        case CmpEQ: r.append("CmpEQ"); break;
        case CmpNE: r.append("CmpNE"); break;
        case CmpLTS: r.append("CmpLTS"); break;
        case CmpLTU: r.append("CmpLTU"); break;
        case CmpLES: r.append("CmpLES"); break;
        case CmpLEU: r.append("CmpLEU"); break;
        case Not: r.append("Not"); break;
        case CF32toF64: r.append("CF32toF64"); break;
        case C64to8: r.append("C64to8"); break;
        case C32to8: r.append("C32to8"); break;
        case C64to16: r.append("C64to16"); break;
        case C64LOto32: r.append("C64LOto32"); break;
        case C64HIto32: r.append("C64HIto32"); break;
        case C32LOto16: r.append("C32LOto16"); break;
        case C32HIto16: r.append("C32HIto16"); break;
        case C16LOto8: r.append("C16LOto8"); break;
        case C16HIto8: r.append("C16HIto8"); break;
        case C16HLto32: r.append("C16HLto32"); break;
        case C1Uto32: r.append("C1Uto32"); break;
        case C1Uto8: r.append("C1Uto8"); break;
        case C8Uto32: r.append("C8Uto32"); break;
        case C8Sto32: r.append("C8Sto32"); break;
        case C8Uto16: r.append("C8Uto16"); break;
        case C8Sto16: r.append("C8Sto16"); break;
        case C8Uto64: r.append("C8Uto64"); break;
        case C16Uto64: r.append("C16Uto64"); break;
        case C16Uto32: r.append("C16Uto32"); break;
        case C16Sto32: r.append("C16Sto32"); break;
        case C32Uto64: r.append("C32Uto64"); break;
        case C32Sto64: r.append("C32Sto64"); break;
        case C32HLto64: r.append("C32HLto64"); break;
        case C32to1: r.append("C32to1"); break;
        case C64to1: r.append("C64to1"); break;
        case DivModS64to32: r.append("DivModS64to32"); break;
        case DivModU64to32: r.append("DivModU64to32"); break;
        case Sad8Ux4: r.append("Sad8Ux4"); break;
        case Add8x8: r.append("Add8x8"); break;
        case Add16x4: r.append("Add16x4"); break;
        case Add32x2: r.append("Add32x2"); break;
        case Add64x1: r.append("Add64x1"); break;
        case QAdd8Sx8: r.append("QAdd8Sx8"); break;
        case QAdd16Sx4: r.append("QAdd16Sx4"); break;
        case QAdd32Sx2: r.append("QAdd32Sx2"); break;
        case QAdd64Sx1: r.append("QAdd64Sx1"); break;
        case QAdd8Ux8: r.append("QAdd8Ux8"); break;
        case QAdd16Ux4: r.append("QAdd16Ux4"); break;
        case QAdd32Ux2: r.append("QAdd32Ux2"); break;
        case QAdd64Ux1: r.append("QAdd64Ux1"); break;
        case Sub8x8: r.append("Sub8x8"); break;
        case Sub16x4: r.append("Sub16x4"); break;
        case Sub32x2: r.append("Sub32x2"); break;
        case QSub8Sx8: r.append("QSub8Sx8"); break;
        case QSub16Sx4: r.append("QSub16Sx4"); break;
        case QSub32Sx2: r.append("QSub32Sx2"); break;
        case QSub64Sx1: r.append("QSub64Sx1"); break;
        case QSub8Ux8: r.append("QSub8Ux8"); break;
        case QSub16Ux4: r.append("QSub16Ux4"); break;
        case QSub32Ux2: r.append("QSub32Ux2"); break;
        case QSub64Ux1: r.append("QSub64Ux1"); break;
        case CmpEQ8x8: r.append("CmpEQ8x8"); break;
        case CmpEQ16x4: r.append("CmpEQ16x4"); break;
        case CmpEQ32x2: r.append("CmpEQ32x2"); break;
        case CmpGT8Ux8: r.append("CmpGT8Ux8"); break;
        case CmpGT16Ux4: r.append("CmpGT16Ux4"); break;
        case CmpGT32Ux2: r.append("CmpGT32Ux2"); break;
        case CmpGT8Sx8: r.append("CmpGT8Sx8"); break;
        case CmpGT16Sx4: r.append("CmpGT16Sx4"); break;
        case CmpGT32Sx2: r.append("CmpGT32Sx2"); break;
        case ShlN8x8: r.append("ShlN8x8"); break;
        case ShlN16x4: r.append("ShlN16x4"); break;
        case ShlN32x2: r.append("ShlN32x2"); break;
        case ShrN8x8: r.append("ShrN8x8"); break;
        case ShrN16x4: r.append("ShrN16x4"); break;
        case ShrN32x2: r.append("ShrN32x2"); break;
        case SarN8x8: r.append("SarN8x8"); break;
        case SarN16x4: r.append("SarN16x4"); break;
        case SarN32x2: r.append("SarN32x2"); break;
        case Mul8x8: r.append("Mul8x8"); break;
        case Mul16x4: r.append("Mul16x4"); break;
        case Mul32x2: r.append("Mul32x2"); break;
        case Mul32Fx2: r.append("Mul32Fx2"); break;
        case MulHi16Ux4: r.append("MulHi16Ux4"); break;
        case MulHi16Sx4: r.append("MulHi16Sx4"); break;
        case PolyMul8x8: r.append("PolyMul8x8"); break;
        case InterleaveHI8x8: r.append("InterleaveHI8x8"); break;
        case InterleaveHI16x4: r.append("InterleaveHI16x4"); break;
        case InterleaveHI32x2: r.append("InterleaveHI32x2"); break;
        case InterleaveLO8x8: r.append("InterleaveLO8x8"); break;
        case InterleaveLO16x4: r.append("InterleaveLO16x4"); break;
        case InterleaveLO32x2: r.append("InterleaveLO32x2"); break;
        case InterleaveOddLanes8x8: r.append("InterleaveOddLanes8x8"); break;
        case InterleaveEvenLanes8x8: r.append("InterleaveEvenLanes8x8"); break;
        case InterleaveOddLanes16x4: r.append("InterleaveOddLanes16x4"); break;
        case InterleaveEvenLanes16x4: r.append("InterleaveEvenLanes16x4"); break;
        case Abs8x16: r.append("Abs8x16"); break;
        case Abs16x8: r.append("Abs16x8"); break;
        case Abs32x4: r.append("Abs32x4"); break;
        case Avg8Ux16: r.append("Avg8Ux16"); break;
        case Avg16Ux8: r.append("Avg16Ux8"); break;
        case Avg32Ux4: r.append("Avg32Ux4"); break;
        case Avg8Sx16: r.append("Avg8Sx16"); break;
        case Avg16Sx8: r.append("Avg16Sx8"); break;
        case Avg32Sx4: r.append("Avg32Sx4"); break;
        case Max8Sx16: r.append("Max8Sx16"); break;
        case Max16Sx8: r.append("Max16Sx8"); break;
        case Max32Sx4: r.append("Max32Sx4"); break;
        case Max8Ux16: r.append("Max8Ux16"); break;
        case Max16Ux8: r.append("Max16Ux8"); break;
        case Max32Ux4: r.append("Max32Ux4"); break;
        case Min8Sx16: r.append("Min8Sx16"); break;
        case Min16Sx8: r.append("Min16Sx8"); break;
        case Min32Sx4: r.append("Min32Sx4"); break;
        case Min8Ux16: r.append("Min8Ux16"); break;
        case Min16Ux8: r.append("Min16Ux8"); break;
        case Min32Ux4: r.append("Min32Ux4"); break;
        case Min8Ux8: r.append("Min8Ux8"); break;
        case Min16Ux4: r.append("Min16Ux4"); break;
        case Min32Ux2: r.append("Min32Ux2"); break;
        case Max8Sx8: r.append("Max8Sx8"); break;
        case Max16Sx4: r.append("Max16Sx4"); break;
        case Max32Sx2: r.append("Max32Sx2"); break;
        case Max8Ux8: r.append("Max8Ux8"); break;
        case Max16Ux4: r.append("Max16Ux4"); break;
        case Max32Ux2: r.append("Max32Ux2"); break;
        case Min8Sx8: r.append("Min8Sx8"); break;
        case Min16Sx4: r.append("Min16Sx4"); break;
        case Min32Sx2: r.append("Min32Sx2"); break;
        case QNarrow16Ux4: r.append("QNarrow16Ux4"); break;
        case QNarrow16Sx4: r.append("QNarrow16Sx4"); break;
        case QNarrow32Sx2: r.append("QNarrow32Sx2"); break;
        case UNSUP: r.append("UNSUP"); break;
    }

    return r;
}

std::string VOp::printOp(void) {
	std::string r="";
	IROp base=Iop_INVALID;
	switch(this->OpCode) {
		case Iop_Add8:
		case Iop_Add16:
		case Iop_Add32:
		case Iop_Add64:
		  r.append("Add"); base = Iop_Add8; break;
		case Iop_Sub8:
		case Iop_Sub16:
		case Iop_Sub32:
		case Iop_Sub64:
		  r.append("Sub"); base = Iop_Sub8; break;
		case Iop_Mul8:
		case Iop_Mul16:
		case Iop_Mul32:
		case Iop_Mul64:
		  r.append("Mul"); base = Iop_Mul8; break;
		case Iop_Or8:
		case Iop_Or16:
		case Iop_Or32:
		case Iop_Or64:
		  r.append("Or"); base = Iop_Or8; break;
		case Iop_And8:
		case Iop_And16:
		case Iop_And32:
		case Iop_And64:
		  r.append("And"); base = Iop_And8; break;
		case Iop_Xor8: 
		case Iop_Xor16:
		case Iop_Xor32:
		case Iop_Xor64:
		  r.append("Xor"); base = Iop_Xor8; break;
		case Iop_Shl8: 
		case Iop_Shl16:
		case Iop_Shl32:
		case Iop_Shl64:
		  r.append( "Shl"); base = Iop_Shl8; break;
		case Iop_Shr8: 
		case Iop_Shr16:
		case Iop_Shr32:
		case Iop_Shr64:
		  r.append( "Shr"); base = Iop_Shr8; break;
		case Iop_Sar8: 
		case Iop_Sar16:
		case Iop_Sar32:
		case Iop_Sar64:
		  r.append( "Sar"); base = Iop_Sar8; break;
		case Iop_CmpEQ8: 
		case Iop_CmpEQ16:
		case Iop_CmpEQ32:
		case Iop_CmpEQ64:
		  r.append( "CmpEQ"); base = Iop_CmpEQ8; break;
		case Iop_CmpNE8: 
		case Iop_CmpNE16:
		case Iop_CmpNE32:
		case Iop_CmpNE64:
		  r.append("CmpNE"); base = Iop_CmpNE8; break;
		case Iop_CasCmpEQ8: 
		case Iop_CasCmpEQ16:
		case Iop_CasCmpEQ32:
		case Iop_CasCmpEQ64:
		  r.append( "CasCmpEQ"); base = Iop_CasCmpEQ8; break;
		case Iop_CasCmpNE8: 
		case Iop_CasCmpNE16:
		case Iop_CasCmpNE32:
		case Iop_CasCmpNE64:
		  r.append("CasCmpNE"); base = Iop_CasCmpNE8; break;
		case Iop_Not8: 
		case Iop_Not16:
		case Iop_Not32:
		case Iop_Not64:
		  r.append("Not"); base = Iop_Not8; break;
		case Iop_8Uto16:   r.append("8Uto16");  break;
		case Iop_8Uto32:   r.append("8Uto32");  break;
		case Iop_16Uto32:  r.append("16Uto32"); break;
		case Iop_8Sto16:   r.append("8Sto16");  break;
		case Iop_8Sto32:   r.append("8Sto32");  break;
		case Iop_16Sto32:  r.append("16Sto32"); break;
		case Iop_32Sto64:  r.append("32Sto64"); break;
		case Iop_32Uto64:  r.append("32Uto64"); break;
		case Iop_32to8:    r.append("32to8");   break;
		case Iop_16Uto64:  r.append("16Uto64"); break;
		case Iop_16Sto64:  r.append("16Sto64"); break;
		case Iop_8Uto64:   r.append("8Uto64"); break;
		case Iop_8Sto64:   r.append("8Sto64"); break;
		case Iop_64to16:   r.append("64to16"); break;
		case Iop_64to8:    r.append("64to8");  break;

		case Iop_Not1:     r.append("Not1");    break;
		case Iop_32to1:    r.append("32to1");   break;
		case Iop_64to1:    r.append("64to1");   break;
		case Iop_1Uto8:    r.append("1Uto8");   break;
		case Iop_1Uto32:   r.append("1Uto32");  break;
		case Iop_1Uto64:   r.append("1Uto64");  break;
		case Iop_1Sto8:    r.append("1Sto8");  break;
		case Iop_1Sto16:   r.append("1Sto16");  break;
		case Iop_1Sto32:   r.append("1Sto32");  break;
		case Iop_1Sto64:   r.append("1Sto64");  break;

		case Iop_MullS8:   r.append("MullS8");  break;
		case Iop_MullS16:  r.append("MullS16"); break;
		case Iop_MullS32:  r.append("MullS32"); break;
		case Iop_MullS64:  r.append("MullS64"); break;
		case Iop_MullU8:   r.append("MullU8");  break;
		case Iop_MullU16:  r.append("MullU16"); break;
		case Iop_MullU32:  r.append("MullU32"); break;
		case Iop_MullU64:  r.append("MullU64"); break;

		case Iop_Clz64:    r.append("Clz64"); break;
		case Iop_Clz32:    r.append("Clz32"); break;
		case Iop_Ctz64:    r.append("Ctz64"); break;
		case Iop_Ctz32:    r.append("Ctz32"); break;

		case Iop_CmpLT32S: r.append("CmpLT32S"); break;
		case Iop_CmpLE32S: r.append("CmpLE32S"); break;
		case Iop_CmpLT32U: r.append("CmpLT32U"); break;
		case Iop_CmpLE32U: r.append("CmpLE32U"); break;

		case Iop_CmpLT64S: r.append("CmpLT64S"); break;
		case Iop_CmpLE64S: r.append("CmpLE64S"); break;
		case Iop_CmpLT64U: r.append("CmpLT64U"); break;
		case Iop_CmpLE64U: r.append("CmpLE64U"); break;

		case Iop_CmpNEZ8:  r.append("CmpNEZ8"); break;
		case Iop_CmpNEZ16: r.append("CmpNEZ16"); break;
		case Iop_CmpNEZ32: r.append("CmpNEZ32"); break;
		case Iop_CmpNEZ64: r.append("CmpNEZ64"); break;

		case Iop_CmpwNEZ32: r.append("CmpwNEZ32"); break;
		case Iop_CmpwNEZ64: r.append("CmpwNEZ64"); break;

		case Iop_Left8:  r.append("Left8"); break;
		case Iop_Left16: r.append("Left16"); break;
		case Iop_Left32: r.append("Left32"); break;
		case Iop_Left64: r.append("Left64"); break;
		case Iop_Max32U: r.append("Max32U"); break;

		case Iop_CmpORD32U: r.append("CmpORD32U"); break;
		case Iop_CmpORD32S: r.append("CmpORD32S"); break;

		case Iop_CmpORD64U: r.append("CmpORD64U"); break;
		case Iop_CmpORD64S: r.append("CmpORD64S"); break;

		case Iop_DivU32: r.append("DivU32"); break;
		case Iop_DivS32: r.append("DivS32"); break;
		case Iop_DivU64: r.append("DivU64"); break;
		case Iop_DivS64: r.append("DivS64"); break;

		case Iop_DivModU64to32: r.append("DivModU64to32"); break;
		case Iop_DivModS64to32: r.append("DivModS64to32"); break;

		case Iop_DivModU128to64: r.append("DivModU128to64"); break;
		case Iop_DivModS128to64: r.append("DivModS128to64"); break;

		case Iop_DivModS64to64: r.append("DivModS64to64"); break;

		case Iop_16HIto8:  r.append("16HIto8"); break;
		case Iop_16to8:    r.append("16to8");   break;
		case Iop_8HLto16:  r.append("8HLto16"); break;

		case Iop_32HIto16: r.append("32HIto16"); break;
		case Iop_32to16:   r.append("32to16");   break;
		case Iop_16HLto32: r.append("16HLto32"); break;

		case Iop_64HIto32: r.append("64HIto32"); break;
		case Iop_64to32:   r.append("64to32");   break;
		case Iop_32HLto64: r.append("32HLto64"); break;

		case Iop_128HIto64: r.append("128HIto64"); break;
		case Iop_128to64:   r.append("128to64");   break;
		case Iop_64HLto128: r.append("64HLto128"); break;

		case Iop_CmpF32:    r.append("CmpF32");    break;
		case Iop_F32toI16S: r.append("F32toI16S");  break;
		case Iop_F32toI32S: r.append("F32toI32S");  break;
		case Iop_F32toI64S: r.append("F32toI64S");  break;
		case Iop_I16StoF32: r.append("I16StoF32");  break;
		case Iop_I32StoF32: r.append("I32StoF32");  break;
		case Iop_I64StoF32: r.append("I64StoF32");  break;

		case Iop_AddF64:    r.append("AddF64"); break;
		case Iop_SubF64:    r.append("SubF64"); break;
		case Iop_MulF64:    r.append("MulF64"); break;
		case Iop_DivF64:    r.append("DivF64"); break;
		case Iop_AddF64r32: r.append("AddF64r32"); break;
		case Iop_SubF64r32: r.append("SubF64r32"); break;
		case Iop_MulF64r32: r.append("MulF64r32"); break;
		case Iop_DivF64r32: r.append("DivF64r32"); break;
		case Iop_AddF32:    r.append("AddF32"); break;
		case Iop_SubF32:    r.append("SubF32"); break;
		case Iop_MulF32:    r.append("MulF32"); break;
		case Iop_DivF32:    r.append("DivF32"); break;

		  /* 128 bit floating point */
		case Iop_AddF128:   r.append("AddF128");  break;
		case Iop_SubF128:   r.append("SubF128");  break;
		case Iop_MulF128:   r.append("MulF128");  break;
		case Iop_DivF128:   r.append("DivF128");  break;
		case Iop_AbsF128:   r.append("AbsF128");  break;
		case Iop_NegF128:   r.append("NegF128");  break;
		case Iop_SqrtF128:  r.append("SqrtF128"); break;
		case Iop_CmpF128:   r.append("CmpF128");  break;

		case Iop_F64HLtoF128: r.append("F64HLtoF128"); break;
		case Iop_F128HItoF64: r.append("F128HItoF64"); break;
		case Iop_F128LOtoF64: r.append("F128LOtoF64"); break;
		case Iop_I32StoF128: r.append("I32StoF128"); break;
		case Iop_I64StoF128: r.append("I64StoF128"); break;
		case Iop_F128toI32S: r.append("F128toI32S"); break;
		case Iop_F128toI64S: r.append("F128toI64S"); break;
		case Iop_F32toF128:  r.append("F32toF128");  break;
		case Iop_F64toF128:  r.append("F64toF128");  break;
		case Iop_F128toF64:  r.append("F128toF64");  break;
		case Iop_F128toF32:  r.append("F128toF32");  break;

		  /* s390 specific */
		case Iop_MAddF32:    r.append("s390_MAddF32"); break;
		case Iop_MSubF32:    r.append("s390_MSubF32"); break;

		case Iop_ScaleF64:      r.append("ScaleF64"); break;
		case Iop_AtanF64:       r.append("AtanF64"); break;
		case Iop_Yl2xF64:       r.append("Yl2xF64"); break;
		case Iop_Yl2xp1F64:     r.append("Yl2xp1F64"); break;
		case Iop_PRemF64:       r.append("PRemF64"); break;
		case Iop_PRemC3210F64:  r.append("PRemC3210F64"); break;
		case Iop_PRem1F64:      r.append("PRem1F64"); break;
		case Iop_PRem1C3210F64: r.append("PRem1C3210F64"); break;
		case Iop_NegF64:        r.append("NegF64"); break;
		case Iop_AbsF64:        r.append("AbsF64"); break;
		case Iop_NegF32:        r.append("NegF32"); break;
		case Iop_AbsF32:        r.append("AbsF32"); break;
		case Iop_SqrtF64:       r.append("SqrtF64"); break;
		case Iop_SqrtF32:       r.append("SqrtF32"); break;
		case Iop_SinF64:    r.append("SinF64"); break;
		case Iop_CosF64:    r.append("CosF64"); break;
		case Iop_TanF64:    r.append("TanF64"); break;
		case Iop_2xm1F64:   r.append("2xm1F64"); break;

		case Iop_MAddF64:    r.append("MAddF64"); break;
		case Iop_MSubF64:    r.append("MSubF64"); break;
		case Iop_MAddF64r32: r.append("MAddF64r32"); break;
		case Iop_MSubF64r32: r.append("MSubF64r32"); break;

		case Iop_Est5FRSqrt:    r.append("Est5FRSqrt"); break;
		case Iop_RoundF64toF64_NEAREST: r.append("RoundF64toF64_NEAREST"); break;
		case Iop_RoundF64toF64_NegINF: r.append("RoundF64toF64_NegINF"); break;
		case Iop_RoundF64toF64_PosINF: r.append("RoundF64toF64_PosINF"); break;
		case Iop_RoundF64toF64_ZERO: r.append("RoundF64toF64_ZERO"); break;

		case Iop_TruncF64asF32: r.append("TruncF64asF32"); break;
		case Iop_CalcFPRF:      r.append("CalcFPRF"); break;

		case Iop_Add16x2:   r.append("Add16x2"); break;
		case Iop_Sub16x2:   r.append("Sub16x2"); break;
		case Iop_QAdd16Sx2: r.append("QAdd16Sx2"); break;
		case Iop_QAdd16Ux2: r.append("QAdd16Ux2"); break;
		case Iop_QSub16Sx2: r.append("QSub16Sx2"); break;
		case Iop_QSub16Ux2: r.append("QSub16Ux2"); break;
		case Iop_HAdd16Ux2: r.append("HAdd16Ux2"); break;
		case Iop_HAdd16Sx2: r.append("HAdd16Sx2"); break;
		case Iop_HSub16Ux2: r.append("HSub16Ux2"); break;
		case Iop_HSub16Sx2: r.append("HSub16Sx2"); break;

		case Iop_Add8x4:   r.append("Add8x4"); break;
		case Iop_Sub8x4:   r.append("Sub8x4"); break;
		case Iop_QAdd8Sx4: r.append("QAdd8Sx4"); break;
		case Iop_QAdd8Ux4: r.append("QAdd8Ux4"); break;
		case Iop_QSub8Sx4: r.append("QSub8Sx4"); break;
		case Iop_QSub8Ux4: r.append("QSub8Ux4"); break;
		case Iop_HAdd8Ux4: r.append("HAdd8Ux4"); break;
		case Iop_HAdd8Sx4: r.append("HAdd8Sx4"); break;
		case Iop_HSub8Ux4: r.append("HSub8Ux4"); break;
		case Iop_HSub8Sx4: r.append("HSub8Sx4"); break;
		case Iop_Sad8Ux4:  r.append("Sad8Ux4"); break;

		case Iop_CmpNEZ16x2: r.append("CmpNEZ16x2"); break;
		case Iop_CmpNEZ8x4:  r.append("CmpNEZ8x4"); break;

		case Iop_CmpF64:    r.append("CmpF64"); break;

		case Iop_F64toI16S: r.append("F64toI16S"); break;
		case Iop_F64toI32S: r.append("F64toI32S"); break;
		case Iop_F64toI64S: r.append("F64toI64S"); break;

		case Iop_F64toI32U: r.append("F64toI32U"); break;

		case Iop_I16StoF64: r.append("I16StoF64"); break;
		case Iop_I32StoF64: r.append("I32StoF64"); break;
		case Iop_I64StoF64: r.append("I64StoF64"); break;
		case Iop_I64UtoF64: r.append("I64UtoF64"); break;
		case Iop_I64UtoF32: r.append("I64UtoF32"); break;

		case Iop_I32UtoF64: r.append("I32UtoF64"); break;

		case Iop_F32toF64: r.append("F32toF64"); break;
		case Iop_F64toF32: r.append("F64toF32"); break;

		case Iop_RoundF64toInt: r.append("RoundF64toInt"); break;
		case Iop_RoundF32toInt: r.append("RoundF32toInt"); break;
		case Iop_RoundF64toF32: r.append("RoundF64toF32"); break;

		case Iop_ReinterpF64asI64: r.append("ReinterpF64asI64"); break;
		case Iop_ReinterpI64asF64: r.append("ReinterpI64asF64"); break;
		case Iop_ReinterpF32asI32: r.append("ReinterpF32asI32"); break;
		case Iop_ReinterpI32asF32: r.append("ReinterpI32asF32"); break;

		case Iop_I32UtoFx4: r.append("I32UtoFx4"); break;
		case Iop_I32StoFx4: r.append("I32StoFx4"); break;

		case Iop_F32toF16x4: r.append("F32toF16x4"); break;
		case Iop_F16toF32x4: r.append("F16toF32x4"); break;

		case Iop_Rsqrte32Fx4: r.append("VRsqrte32Fx4"); break;
		case Iop_Rsqrte32x4:  r.append("VRsqrte32x4"); break;
		case Iop_Rsqrte32Fx2: r.append("VRsqrte32Fx2"); break;
		case Iop_Rsqrte32x2:  r.append("VRsqrte32x2"); break;

		case Iop_QFtoI32Ux4_RZ: r.append("QFtoI32Ux4_RZ"); break;
		case Iop_QFtoI32Sx4_RZ: r.append("QFtoI32Sx4_RZ"); break;

		case Iop_FtoI32Ux4_RZ: r.append("FtoI32Ux4_RZ"); break;
		case Iop_FtoI32Sx4_RZ: r.append("FtoI32Sx4_RZ"); break;

		case Iop_I32UtoFx2: r.append("I32UtoFx2"); break;
		case Iop_I32StoFx2: r.append("I32StoFx2"); break;

		case Iop_FtoI32Ux2_RZ: r.append("FtoI32Ux2_RZ"); break;
		case Iop_FtoI32Sx2_RZ: r.append("FtoI32Sx2_RZ"); break;

		case Iop_RoundF32x4_RM: r.append("RoundF32x4_RM"); break;
		case Iop_RoundF32x4_RP: r.append("RoundF32x4_RP"); break;
		case Iop_RoundF32x4_RN: r.append("RoundF32x4_RN"); break;
		case Iop_RoundF32x4_RZ: r.append("RoundF32x4_RZ"); break;

		case Iop_Abs8x8: r.append("Abs8x8"); break;
		case Iop_Abs16x4: r.append("Abs16x4"); break;
		case Iop_Abs32x2: r.append("Abs32x2"); break;
		case Iop_Add8x8: r.append("Add8x8"); break;
		case Iop_Add16x4: r.append("Add16x4"); break;
		case Iop_Add32x2: r.append("Add32x2"); break;
		case Iop_QAdd8Ux8: r.append("QAdd8Ux8"); break;
		case Iop_QAdd16Ux4: r.append("QAdd16Ux4"); break;
		case Iop_QAdd32Ux2: r.append("QAdd32Ux2"); break;
		case Iop_QAdd64Ux1: r.append("QAdd64Ux1"); break;
		case Iop_QAdd8Sx8: r.append("QAdd8Sx8"); break;
		case Iop_QAdd16Sx4: r.append("QAdd16Sx4"); break;
		case Iop_QAdd32Sx2: r.append("QAdd32Sx2"); break;
		case Iop_QAdd64Sx1: r.append("QAdd64Sx1"); break;
		case Iop_PwAdd8x8: r.append("PwAdd8x8"); break;
		case Iop_PwAdd16x4: r.append("PwAdd16x4"); break;
		case Iop_PwAdd32x2: r.append("PwAdd32x2"); break;
		case Iop_PwAdd32Fx2: r.append("PwAdd32Fx2"); break;
		case Iop_PwAddL8Ux8: r.append("PwAddL8Ux8"); break;
		case Iop_PwAddL16Ux4: r.append("PwAddL16Ux4"); break;
		case Iop_PwAddL32Ux2: r.append("PwAddL32Ux2"); break;
		case Iop_PwAddL8Sx8: r.append("PwAddL8Sx8"); break;
		case Iop_PwAddL16Sx4: r.append("PwAddL16Sx4"); break;
		case Iop_PwAddL32Sx2: r.append("PwAddL32Sx2"); break;
		case Iop_Sub8x8: r.append("Sub8x8"); break;
		case Iop_Sub16x4: r.append("Sub16x4"); break;
		case Iop_Sub32x2: r.append("Sub32x2"); break;
		case Iop_QSub8Ux8: r.append("QSub8Ux8"); break;
		case Iop_QSub16Ux4: r.append("QSub16Ux4"); break;
		case Iop_QSub32Ux2: r.append("QSub32Ux2"); break;
		case Iop_QSub64Ux1: r.append("QSub64Ux1"); break;
		case Iop_QSub8Sx8: r.append("QSub8Sx8"); break;
		case Iop_QSub16Sx4: r.append("QSub16Sx4"); break;
		case Iop_QSub32Sx2: r.append("QSub32Sx2"); break;
		case Iop_QSub64Sx1: r.append("QSub64Sx1"); break;
		case Iop_Mul8x8: r.append("Mul8x8"); break;
		case Iop_Mul16x4: r.append("Mul16x4"); break;
		case Iop_Mul32x2: r.append("Mul32x2"); break;
		case Iop_Mul32Fx2: r.append("Mul32Fx2"); break;
		case Iop_PolynomialMul8x8: r.append("PolynomialMul8x8"); break;
		case Iop_MulHi16Ux4: r.append("MulHi16Ux4"); break;
		case Iop_MulHi16Sx4: r.append("MulHi16Sx4"); break;
		case Iop_QDMulHi16Sx4: r.append("QDMulHi16Sx4"); break;
		case Iop_QDMulHi32Sx2: r.append("QDMulHi32Sx2"); break;
		case Iop_QRDMulHi16Sx4: r.append("QRDMulHi16Sx4"); break;
		case Iop_QRDMulHi32Sx2: r.append("QRDMulHi32Sx2"); break;
		case Iop_QDMulLong16Sx4: r.append("QDMulLong16Sx4"); break;
		case Iop_QDMulLong32Sx2: r.append("QDMulLong32Sx2"); break;
		case Iop_Avg8Ux8: r.append("Avg8Ux8"); break;
		case Iop_Avg16Ux4: r.append("Avg16Ux4"); break;
		case Iop_Max8Sx8: r.append("Max8Sx8"); break;
		case Iop_Max16Sx4: r.append("Max16Sx4"); break;
		case Iop_Max32Sx2: r.append("Max32Sx2"); break;
		case Iop_Max8Ux8: r.append("Max8Ux8"); break;
		case Iop_Max16Ux4: r.append("Max16Ux4"); break;
		case Iop_Max32Ux2: r.append("Max32Ux2"); break;
		case Iop_Min8Sx8: r.append("Min8Sx8"); break;
		case Iop_Min16Sx4: r.append("Min16Sx4"); break;
		case Iop_Min32Sx2: r.append("Min32Sx2"); break;
		case Iop_Min8Ux8: r.append("Min8Ux8"); break;
		case Iop_Min16Ux4: r.append("Min16Ux4"); break;
		case Iop_Min32Ux2: r.append("Min32Ux2"); break;
		case Iop_PwMax8Sx8: r.append("PwMax8Sx8"); break;
		case Iop_PwMax16Sx4: r.append("PwMax16Sx4"); break;
		case Iop_PwMax32Sx2: r.append("PwMax32Sx2"); break;
		case Iop_PwMax8Ux8: r.append("PwMax8Ux8"); break;
		case Iop_PwMax16Ux4: r.append("PwMax16Ux4"); break;
		case Iop_PwMax32Ux2: r.append("PwMax32Ux2"); break;
		case Iop_PwMin8Sx8: r.append("PwMin8Sx8"); break;
		case Iop_PwMin16Sx4: r.append("PwMin16Sx4"); break;
		case Iop_PwMin32Sx2: r.append("PwMin32Sx2"); break;
		case Iop_PwMin8Ux8: r.append("PwMin8Ux8"); break;
		case Iop_PwMin16Ux4: r.append("PwMin16Ux4"); break;
		case Iop_PwMin32Ux2: r.append("PwMin32Ux2"); break;
		case Iop_CmpEQ8x8: r.append("CmpEQ8x8"); break;
		case Iop_CmpEQ16x4: r.append("CmpEQ16x4"); break;
		case Iop_CmpEQ32x2: r.append("CmpEQ32x2"); break;
		case Iop_CmpGT8Ux8: r.append("CmpGT8Ux8"); break;
		case Iop_CmpGT16Ux4: r.append("CmpGT16Ux4"); break;
		case Iop_CmpGT32Ux2: r.append("CmpGT32Ux2"); break;
		case Iop_CmpGT8Sx8: r.append("CmpGT8Sx8"); break;
		case Iop_CmpGT16Sx4: r.append("CmpGT16Sx4"); break;
		case Iop_CmpGT32Sx2: r.append("CmpGT32Sx2"); break;
		case Iop_Cnt8x8: r.append("Cnt8x8"); break;
		case Iop_Clz8Sx8: r.append("Clz8Sx8"); break;
		case Iop_Clz16Sx4: r.append("Clz16Sx4"); break;
		case Iop_Clz32Sx2: r.append("Clz32Sx2"); break;
		case Iop_Cls8Sx8: r.append("Cls8Sx8"); break;
		case Iop_Cls16Sx4: r.append("Cls16Sx4"); break;
		case Iop_Cls32Sx2: r.append("Cls32Sx2"); break;
		case Iop_ShlN8x8: r.append("ShlN8x8"); break;
		case Iop_ShlN16x4: r.append("ShlN16x4"); break;
		case Iop_ShlN32x2: r.append("ShlN32x2"); break;
		case Iop_ShrN8x8: r.append("ShrN8x8"); break;
		case Iop_ShrN16x4: r.append("ShrN16x4"); break;
		case Iop_ShrN32x2: r.append("ShrN32x2"); break;
		case Iop_SarN8x8: r.append("SarN8x8"); break;
		case Iop_SarN16x4: r.append("SarN16x4"); break;
		case Iop_SarN32x2: r.append("SarN32x2"); break;
		/*case Iop_QNarrow16Ux4: r.append("QNarrow16Ux4"); break;
		case Iop_QNarrow16Sx4: r.append("QNarrow16Sx4"); break;
		case Iop_QNarrow32Sx2: r.append("QNarrow32Sx2"); break;*/
		case Iop_InterleaveHI8x8: r.append("InterleaveHI8x8"); break;
		case Iop_InterleaveHI16x4: r.append("InterleaveHI16x4"); break;
		case Iop_InterleaveHI32x2: r.append("InterleaveHI32x2"); break;
		case Iop_InterleaveLO8x8: r.append("InterleaveLO8x8"); break;
		case Iop_InterleaveLO16x4: r.append("InterleaveLO16x4"); break;
		case Iop_InterleaveLO32x2: r.append("InterleaveLO32x2"); break;
		case Iop_CatOddLanes8x8: r.append("CatOddLanes8x8"); break;
		case Iop_CatOddLanes16x4: r.append("CatOddLanes16x4"); break;
		case Iop_CatEvenLanes8x8: r.append("CatEvenLanes8x8"); break;
		case Iop_CatEvenLanes16x4: r.append("CatEvenLanes16x4"); break;
		case Iop_InterleaveOddLanes8x8: r.append("InterleaveOddLanes8x8"); break;
		case Iop_InterleaveOddLanes16x4: r.append("InterleaveOddLanes16x4"); break;
		case Iop_InterleaveEvenLanes8x8: r.append("InterleaveEvenLanes8x8"); break;
		case Iop_InterleaveEvenLanes16x4: r.append("InterleaveEvenLanes16x4"); break;
		case Iop_Shl8x8: r.append("Shl8x8"); break;
		case Iop_Shl16x4: r.append("Shl16x4"); break;
		case Iop_Shl32x2: r.append("Shl32x2"); break;
		case Iop_Shr8x8: r.append("Shr8x8"); break;
		case Iop_Shr16x4: r.append("Shr16x4"); break;
		case Iop_Shr32x2: r.append("Shr32x2"); break;
		case Iop_QShl8x8: r.append("QShl8x8"); break;
		case Iop_QShl16x4: r.append("QShl16x4"); break;
		case Iop_QShl32x2: r.append("QShl32x2"); break;
		case Iop_QShl64x1: r.append("QShl64x1"); break;
		case Iop_QSal8x8: r.append("QSal8x8"); break;
		case Iop_QSal16x4: r.append("QSal16x4"); break;
		case Iop_QSal32x2: r.append("QSal32x2"); break;
		case Iop_QSal64x1: r.append("QSal64x1"); break;
		case Iop_QShlN8x8: r.append("QShlN8x8"); break;
		case Iop_QShlN16x4: r.append("QShlN16x4"); break;
		case Iop_QShlN32x2: r.append("QShlN32x2"); break;
		case Iop_QShlN64x1: r.append("QShlN64x1"); break;
		case Iop_QShlN8Sx8: r.append("QShlN8Sx8"); break;
		case Iop_QShlN16Sx4: r.append("QShlN16Sx4"); break;
		case Iop_QShlN32Sx2: r.append("QShlN32Sx2"); break;
		case Iop_QShlN64Sx1: r.append("QShlN64Sx1"); break;
		case Iop_QSalN8x8: r.append("QSalN8x8"); break;
		case Iop_QSalN16x4: r.append("QSalN16x4"); break;
		case Iop_QSalN32x2: r.append("QSalN32x2"); break;
		case Iop_QSalN64x1: r.append("QSalN64x1"); break;
		case Iop_Sar8x8: r.append("Sar8x8"); break;
		case Iop_Sar16x4: r.append("Sar16x4"); break;
		case Iop_Sar32x2: r.append("Sar32x2"); break;
		case Iop_Sal8x8: r.append("Sal8x8"); break;
		case Iop_Sal16x4: r.append("Sal16x4"); break;
		case Iop_Sal32x2: r.append("Sal32x2"); break;
		case Iop_Sal64x1: r.append("Sal64x1"); break;
		case Iop_Perm8x8: r.append("Perm8x8"); break;
		case Iop_Reverse16_8x8: r.append("Reverse16_8x8"); break;
		case Iop_Reverse32_8x8: r.append("Reverse32_8x8"); break;
		case Iop_Reverse32_16x4: r.append("Reverse32_16x4"); break;
		case Iop_Reverse64_8x8: r.append("Reverse64_8x8"); break;
		case Iop_Reverse64_16x4: r.append("Reverse64_16x4"); break;
		case Iop_Reverse64_32x2: r.append("Reverse64_32x2"); break;
		case Iop_Abs32Fx2: r.append("Abs32Fx2"); break;

		case Iop_CmpNEZ32x2: r.append("CmpNEZ32x2"); break;
		case Iop_CmpNEZ16x4: r.append("CmpNEZ16x4"); break;
		case Iop_CmpNEZ8x8:  r.append("CmpNEZ8x8"); break;

		case Iop_Add32Fx4:  r.append("Add32Fx4"); break;
		case Iop_Add32Fx2:  r.append("Add32Fx2"); break;
		case Iop_Add32F0x4: r.append("Add32F0x4"); break;
		case Iop_Add64Fx2:  r.append("Add64Fx2"); break;
		case Iop_Add64F0x2: r.append("Add64F0x2"); break;

		case Iop_Div32Fx4:  r.append("Div32Fx4"); break;
		case Iop_Div32F0x4: r.append("Div32F0x4"); break;
		case Iop_Div64Fx2:  r.append("Div64Fx2"); break;
		case Iop_Div64F0x2: r.append("Div64F0x2"); break;

		case Iop_Max32Fx4:  r.append("Max32Fx4"); break;
		case Iop_Max32Fx2:  r.append("Max32Fx2"); break;
		case Iop_PwMax32Fx4:  r.append("PwMax32Fx4"); break;
		case Iop_PwMax32Fx2:  r.append("PwMax32Fx2"); break;
		case Iop_Max32F0x4: r.append("Max32F0x4"); break;
		case Iop_Max64Fx2:  r.append("Max64Fx2"); break;
		case Iop_Max64F0x2: r.append("Max64F0x2"); break;

		case Iop_Min32Fx4:  r.append("Min32Fx4"); break;
		case Iop_Min32Fx2:  r.append("Min32Fx2"); break;
		case Iop_PwMin32Fx4:  r.append("PwMin32Fx4"); break;
		case Iop_PwMin32Fx2:  r.append("PwMin32Fx2"); break;
		case Iop_Min32F0x4: r.append("Min32F0x4"); break;
		case Iop_Min64Fx2:  r.append("Min64Fx2"); break;
		case Iop_Min64F0x2: r.append("Min64F0x2"); break;

		case Iop_Mul32Fx4:  r.append("Mul32Fx4"); break;
		case Iop_Mul32F0x4: r.append("Mul32F0x4"); break;
		case Iop_Mul64Fx2:  r.append("Mul64Fx2"); break;
		case Iop_Mul64F0x2: r.append("Mul64F0x2"); break;

		case Iop_Recip32x2: r.append("Recip32x2"); break;
		case Iop_Recip32Fx2:  r.append("Recip32Fx2"); break;
		case Iop_Recip32Fx4:  r.append("Recip32Fx4"); break;
		case Iop_Recip32x4:  r.append("Recip32x4"); break;
		case Iop_Recip32F0x4: r.append("Recip32F0x4"); break;
		case Iop_Recip64Fx2:  r.append("Recip64Fx2"); break;
		case Iop_Recip64F0x2: r.append("Recip64F0x2"); break;
		case Iop_Recps32Fx2:  r.append("VRecps32Fx2"); break;
		case Iop_Recps32Fx4:  r.append("VRecps32Fx4"); break;
		case Iop_Abs32Fx4:  r.append("Abs32Fx4"); break;
		case Iop_Rsqrts32Fx4:  r.append("VRsqrts32Fx4"); break;
		case Iop_Rsqrts32Fx2:  r.append("VRsqrts32Fx2"); break;

		case Iop_RSqrt32Fx4:  r.append("RSqrt32Fx4"); break;
		case Iop_RSqrt32F0x4: r.append("RSqrt32F0x4"); break;
		case Iop_RSqrt64Fx2:  r.append("RSqrt64Fx2"); break;
		case Iop_RSqrt64F0x2: r.append("RSqrt64F0x2"); break;

		case Iop_Sqrt32Fx4:  r.append("Sqrt32Fx4"); break;
		case Iop_Sqrt32F0x4: r.append("Sqrt32F0x4"); break;
		case Iop_Sqrt64Fx2:  r.append("Sqrt64Fx2"); break;
		case Iop_Sqrt64F0x2: r.append("Sqrt64F0x2"); break;

		case Iop_Sub32Fx4:  r.append("Sub32Fx4"); break;
		case Iop_Sub32Fx2:  r.append("Sub32Fx2"); break;
		case Iop_Sub32F0x4: r.append("Sub32F0x4"); break;
		case Iop_Sub64Fx2:  r.append("Sub64Fx2"); break;
		case Iop_Sub64F0x2: r.append("Sub64F0x2"); break;

		case Iop_CmpEQ32Fx4: r.append("CmpEQ32Fx4"); break;
		case Iop_CmpLT32Fx4: r.append("CmpLT32Fx4"); break;
		case Iop_CmpLE32Fx4: r.append("CmpLE32Fx4"); break;
		case Iop_CmpGT32Fx4: r.append("CmpGT32Fx4"); break;
		case Iop_CmpGE32Fx4: r.append("CmpGE32Fx4"); break;
		case Iop_CmpUN32Fx4: r.append("CmpUN32Fx4"); break;
		case Iop_CmpEQ64Fx2: r.append("CmpEQ64Fx2"); break;
		case Iop_CmpLT64Fx2: r.append("CmpLT64Fx2"); break;
		case Iop_CmpLE64Fx2: r.append("CmpLE64Fx2"); break;
		case Iop_CmpUN64Fx2: r.append("CmpUN64Fx2"); break;
		case Iop_CmpGT32Fx2: r.append("CmpGT32Fx2"); break;
		case Iop_CmpEQ32Fx2: r.append("CmpEQ32Fx2"); break;
		case Iop_CmpGE32Fx2: r.append("CmpGE32Fx2"); break;

		case Iop_CmpEQ32F0x4: r.append("CmpEQ32F0x4"); break;
		case Iop_CmpLT32F0x4: r.append("CmpLT32F0x4"); break;
		case Iop_CmpLE32F0x4: r.append("CmpLE32F0x4"); break;
		case Iop_CmpUN32F0x4: r.append("CmpUN32F0x4"); break;
		case Iop_CmpEQ64F0x2: r.append("CmpEQ64F0x2"); break;
		case Iop_CmpLT64F0x2: r.append("CmpLT64F0x2"); break;
		case Iop_CmpLE64F0x2: r.append("CmpLE64F0x2"); break;
		case Iop_CmpUN64F0x2: r.append("CmpUN64F0x2"); break;

		case Iop_Neg32Fx4: r.append("Neg32Fx4"); break;
		case Iop_Neg32Fx2: r.append("Neg32Fx2"); break;

		case Iop_V128to64:   r.append("V128to64");   break;
		case Iop_V128HIto64: r.append("V128HIto64"); break;
		case Iop_64HLtoV128: r.append("64HLtoV128"); break;

		case Iop_64UtoV128:   r.append("64UtoV128"); break;
		case Iop_SetV128lo64: r.append("SetV128lo64"); break;

		case Iop_32UtoV128:   r.append("32UtoV128"); break;
		case Iop_V128to32:    r.append("V128to32"); break;
		case Iop_SetV128lo32: r.append("SetV128lo32"); break;

		case Iop_Dup8x16: r.append("Dup8x16"); break;
		case Iop_Dup16x8: r.append("Dup16x8"); break;
		case Iop_Dup32x4: r.append("Dup32x4"); break;
		case Iop_Dup8x8: r.append("Dup8x8"); break;
		case Iop_Dup16x4: r.append("Dup16x4"); break;
		case Iop_Dup32x2: r.append("Dup32x2"); break;

		case Iop_NotV128:    r.append("NotV128"); break;
		case Iop_AndV128:    r.append("AndV128"); break;
		case Iop_OrV128:     r.append("OrV128");  break;
		case Iop_XorV128:    r.append("XorV128"); break;

		case Iop_CmpNEZ8x16: r.append("CmpNEZ8x16"); break;
		case Iop_CmpNEZ16x8: r.append("CmpNEZ16x8"); break;
		case Iop_CmpNEZ32x4: r.append("CmpNEZ32x4"); break;
		case Iop_CmpNEZ64x2: r.append("CmpNEZ64x2"); break;

		case Iop_Abs8x16: r.append("Abs8x16"); break;
		case Iop_Abs16x8: r.append("Abs16x8"); break;
		case Iop_Abs32x4: r.append("Abs32x4"); break;

		case Iop_Add8x16:   r.append("Add8x16"); break;
		case Iop_Add16x8:   r.append("Add16x8"); break;
		case Iop_Add32x4:   r.append("Add32x4"); break;
		case Iop_Add64x2:   r.append("Add64x2"); break;
		case Iop_QAdd8Ux16: r.append("QAdd8Ux16"); break;
		case Iop_QAdd16Ux8: r.append("QAdd16Ux8"); break;
		case Iop_QAdd32Ux4: r.append("QAdd32Ux4"); break;
		case Iop_QAdd8Sx16: r.append("QAdd8Sx16"); break;
		case Iop_QAdd16Sx8: r.append("QAdd16Sx8"); break;
		case Iop_QAdd32Sx4: r.append("QAdd32Sx4"); break;
		case Iop_QAdd64Ux2: r.append("QAdd64Ux2"); break;
		case Iop_QAdd64Sx2: r.append("QAdd64Sx2"); break;
		case Iop_PwAdd8x16: r.append("PwAdd8x16"); break;
		case Iop_PwAdd16x8: r.append("PwAdd16x8"); break;
		case Iop_PwAdd32x4: r.append("PwAdd32x4"); break;
		case Iop_PwAddL8Ux16: r.append("PwAddL8Ux16"); break;
		case Iop_PwAddL16Ux8: r.append("PwAddL16Ux8"); break;
		case Iop_PwAddL32Ux4: r.append("PwAddL32Ux4"); break;
		case Iop_PwAddL8Sx16: r.append("PwAddL8Sx16"); break;
		case Iop_PwAddL16Sx8: r.append("PwAddL16Sx8"); break;
		case Iop_PwAddL32Sx4: r.append("PwAddL32Sx4"); break;

		case Iop_Sub8x16:   r.append("Sub8x16"); break;
		case Iop_Sub16x8:   r.append("Sub16x8"); break;
		case Iop_Sub32x4:   r.append("Sub32x4"); break;
		case Iop_Sub64x2:   r.append("Sub64x2"); break;
		case Iop_QSub8Ux16: r.append("QSub8Ux16"); break;
		case Iop_QSub16Ux8: r.append("QSub16Ux8"); break;
		case Iop_QSub32Ux4: r.append("QSub32Ux4"); break;
		case Iop_QSub8Sx16: r.append("QSub8Sx16"); break;
		case Iop_QSub16Sx8: r.append("QSub16Sx8"); break;
		case Iop_QSub32Sx4: r.append("QSub32Sx4"); break;
		case Iop_QSub64Ux2: r.append("QSub64Ux2"); break;
		case Iop_QSub64Sx2: r.append("QSub64Sx2"); break;

		case Iop_Mul8x16:    r.append("Mul8x16"); break;
		case Iop_Mul16x8:    r.append("Mul16x8"); break;
		case Iop_Mul32x4:    r.append("Mul32x4"); break;
		case Iop_Mull8Ux8:    r.append("Mull8Ux8"); break;
		case Iop_Mull8Sx8:    r.append("Mull8Sx8"); break;
		case Iop_Mull16Ux4:    r.append("Mull16Ux4"); break;
		case Iop_Mull16Sx4:    r.append("Mull16Sx4"); break;
		case Iop_Mull32Ux2:    r.append("Mull32Ux2"); break;
		case Iop_Mull32Sx2:    r.append("Mull32Sx2"); break;
		case Iop_PolynomialMul8x16: r.append("PolynomialMul8x16"); break;
		case Iop_PolynomialMull8x8: r.append("PolynomialMull8x8"); break;
		case Iop_MulHi16Ux8: r.append("MulHi16Ux8"); break;
		case Iop_MulHi32Ux4: r.append("MulHi32Ux4"); break;
		case Iop_MulHi16Sx8: r.append("MulHi16Sx8"); break;
		case Iop_MulHi32Sx4: r.append("MulHi32Sx4"); break;
		case Iop_QDMulHi16Sx8: r.append("QDMulHi16Sx8"); break;
		case Iop_QDMulHi32Sx4: r.append("QDMulHi32Sx4"); break;
		case Iop_QRDMulHi16Sx8: r.append("QRDMulHi16Sx8"); break;
		case Iop_QRDMulHi32Sx4: r.append("QRDMulHi32Sx4"); break;

		case Iop_MullEven8Ux16: r.append("MullEven8Ux16"); break;
		case Iop_MullEven16Ux8: r.append("MullEven16Ux8"); break;
		case Iop_MullEven8Sx16: r.append("MullEven8Sx16"); break;
		case Iop_MullEven16Sx8: r.append("MullEven16Sx8"); break;

		case Iop_Avg8Ux16: r.append("Avg8Ux16"); break;
		case Iop_Avg16Ux8: r.append("Avg16Ux8"); break;
		case Iop_Avg32Ux4: r.append("Avg32Ux4"); break;
		case Iop_Avg8Sx16: r.append("Avg8Sx16"); break;
		case Iop_Avg16Sx8: r.append("Avg16Sx8"); break;
		case Iop_Avg32Sx4: r.append("Avg32Sx4"); break;

		case Iop_Max8Sx16: r.append("Max8Sx16"); break;
		case Iop_Max16Sx8: r.append("Max16Sx8"); break;
		case Iop_Max32Sx4: r.append("Max32Sx4"); break;
		case Iop_Max8Ux16: r.append("Max8Ux16"); break;
		case Iop_Max16Ux8: r.append("Max16Ux8"); break;
		case Iop_Max32Ux4: r.append("Max32Ux4"); break;

		case Iop_Min8Sx16: r.append("Min8Sx16"); break;
		case Iop_Min16Sx8: r.append("Min16Sx8"); break;
		case Iop_Min32Sx4: r.append("Min32Sx4"); break;
		case Iop_Min8Ux16: r.append("Min8Ux16"); break;
		case Iop_Min16Ux8: r.append("Min16Ux8"); break;
		case Iop_Min32Ux4: r.append("Min32Ux4"); break;

		case Iop_CmpEQ8x16:  r.append("CmpEQ8x16"); break;
		case Iop_CmpEQ16x8:  r.append("CmpEQ16x8"); break;
		case Iop_CmpEQ32x4:  r.append("CmpEQ32x4"); break;
		case Iop_CmpGT8Sx16: r.append("CmpGT8Sx16"); break;
		case Iop_CmpGT16Sx8: r.append("CmpGT16Sx8"); break;
		case Iop_CmpGT32Sx4: r.append("CmpGT32Sx4"); break;
		case Iop_CmpGT64Sx2: r.append("CmpGT64Sx2"); break;
		case Iop_CmpGT8Ux16: r.append("CmpGT8Ux16"); break;
		case Iop_CmpGT16Ux8: r.append("CmpGT16Ux8"); break;
		case Iop_CmpGT32Ux4: r.append("CmpGT32Ux4"); break;

		case Iop_Cnt8x16: r.append("Cnt8x16"); break;
		case Iop_Clz8Sx16: r.append("Clz8Sx16"); break;
		case Iop_Clz16Sx8: r.append("Clz16Sx8"); break;
		case Iop_Clz32Sx4: r.append("Clz32Sx4"); break;
		case Iop_Cls8Sx16: r.append("Cls8Sx16"); break;
		case Iop_Cls16Sx8: r.append("Cls16Sx8"); break;
		case Iop_Cls32Sx4: r.append("Cls32Sx4"); break;

		case Iop_ShlV128: r.append("ShlV128"); break;
		case Iop_ShrV128: r.append("ShrV128"); break;

		case Iop_ShlN8x16: r.append("ShlN8x16"); break;
		case Iop_ShlN16x8: r.append("ShlN16x8"); break;
		case Iop_ShlN32x4: r.append("ShlN32x4"); break;
		case Iop_ShlN64x2: r.append("ShlN64x2"); break;
		case Iop_ShrN8x16: r.append("ShrN8x16"); break;
		case Iop_ShrN16x8: r.append("ShrN16x8"); break;
		case Iop_ShrN32x4: r.append("ShrN32x4"); break;
		case Iop_ShrN64x2: r.append("ShrN64x2"); break;
		case Iop_SarN8x16: r.append("SarN8x16"); break;
		case Iop_SarN16x8: r.append("SarN16x8"); break;
		case Iop_SarN32x4: r.append("SarN32x4"); break;
		case Iop_SarN64x2: r.append("SarN64x2"); break;

		case Iop_Shl8x16: r.append("Shl8x16"); break;
		case Iop_Shl16x8: r.append("Shl16x8"); break;
		case Iop_Shl32x4: r.append("Shl32x4"); break;
		case Iop_Shl64x2: r.append("Shl64x2"); break;
		case Iop_QSal8x16: r.append("QSal8x16"); break;
		case Iop_QSal16x8: r.append("QSal16x8"); break;
		case Iop_QSal32x4: r.append("QSal32x4"); break;
		case Iop_QSal64x2: r.append("QSal64x2"); break;
		case Iop_QShl8x16: r.append("QShl8x16"); break;
		case Iop_QShl16x8: r.append("QShl16x8"); break;
		case Iop_QShl32x4: r.append("QShl32x4"); break;
		case Iop_QShl64x2: r.append("QShl64x2"); break;
		case Iop_QSalN8x16: r.append("QSalN8x16"); break;
		case Iop_QSalN16x8: r.append("QSalN16x8"); break;
		case Iop_QSalN32x4: r.append("QSalN32x4"); break;
		case Iop_QSalN64x2: r.append("QSalN64x2"); break;
		case Iop_QShlN8x16: r.append("QShlN8x16"); break;
		case Iop_QShlN16x8: r.append("QShlN16x8"); break;
		case Iop_QShlN32x4: r.append("QShlN32x4"); break;
		case Iop_QShlN64x2: r.append("QShlN64x2"); break;
		case Iop_QShlN8Sx16: r.append("QShlN8Sx16"); break;
		case Iop_QShlN16Sx8: r.append("QShlN16Sx8"); break;
		case Iop_QShlN32Sx4: r.append("QShlN32Sx4"); break;
		case Iop_QShlN64Sx2: r.append("QShlN64Sx2"); break;
		case Iop_Shr8x16: r.append("Shr8x16"); break;
		case Iop_Shr16x8: r.append("Shr16x8"); break;
		case Iop_Shr32x4: r.append("Shr32x4"); break;
		case Iop_Shr64x2: r.append("Shr64x2"); break;
		case Iop_Sar8x16: r.append("Sar8x16"); break;
		case Iop_Sar16x8: r.append("Sar16x8"); break;
		case Iop_Sar32x4: r.append("Sar32x4"); break;
		case Iop_Sar64x2: r.append("Sar64x2"); break;
		case Iop_Sal8x16: r.append("Sal8x16"); break;
		case Iop_Sal16x8: r.append("Sal16x8"); break;
		case Iop_Sal32x4: r.append("Sal32x4"); break;
		case Iop_Sal64x2: r.append("Sal64x2"); break;
		case Iop_Rol8x16: r.append("Rol8x16"); break;
		case Iop_Rol16x8: r.append("Rol16x8"); break;
		case Iop_Rol32x4: r.append("Rol32x4"); break;

		/*case Iop_Narrow16x8:   r.append("Narrow16x8"); break;
		case Iop_Narrow32x4:   r.append("Narrow32x4"); break;
		case Iop_QNarrow16Ux8: r.append("QNarrow16Ux8"); break;
		case Iop_QNarrow32Ux4: r.append("QNarrow32Ux4"); break;
		case Iop_QNarrow16Sx8: r.append("QNarrow16Sx8"); break;
		case Iop_QNarrow32Sx4: r.append("QNarrow32Sx4"); break;
		case Iop_Shorten16x8: r.append("Shorten16x8"); break;
		case Iop_Shorten32x4: r.append("Shorten32x4"); break;
		case Iop_Shorten64x2: r.append("Shorten64x2"); break;*/
		/*case Iop_QShortenU16Ux8: r.append("QShortenU16Ux8"); break;
		case Iop_QShortenU32Ux4: r.append("QShortenU32Ux4"); break;
		case Iop_QShortenU64Ux2: r.append("QShortenU64Ux2"); break;
		case Iop_QShortenS16Sx8: r.append("QShortenS16Sx8"); break;
		case Iop_QShortenS32Sx4: r.append("QShortenS32Sx4"); break;
		case Iop_QShortenS64Sx2: r.append("QShortenS64Sx2"); break;
		case Iop_QShortenU16Sx8: r.append("QShortenU16Sx8"); break;
		case Iop_QShortenU32Sx4: r.append("QShortenU32Sx4"); break;
		case Iop_QShortenU64Sx2: r.append("QShortenU64Sx2"); break;
		case Iop_Longen8Ux8: r.append("Longen8Ux8"); break;
		case Iop_Longen16Ux4: r.append("Longen16Ux4"); break;
		case Iop_Longen32Ux2: r.append("Longen32Ux2"); break;
		case Iop_Longen8Sx8: r.append("Longen8Sx8"); break;
		case Iop_Longen16Sx4: r.append("Longen16Sx4"); break;
		case Iop_Longen32Sx2: r.append("Longen32Sx2"); break;*/

		case Iop_InterleaveHI8x16: r.append("InterleaveHI8x16"); break;
		case Iop_InterleaveHI16x8: r.append("InterleaveHI16x8"); break;
		case Iop_InterleaveHI32x4: r.append("InterleaveHI32x4"); break;
		case Iop_InterleaveHI64x2: r.append("InterleaveHI64x2"); break;
		case Iop_InterleaveLO8x16: r.append("InterleaveLO8x16"); break;
		case Iop_InterleaveLO16x8: r.append("InterleaveLO16x8"); break;
		case Iop_InterleaveLO32x4: r.append("InterleaveLO32x4"); break;
		case Iop_InterleaveLO64x2: r.append("InterleaveLO64x2"); break;

		case Iop_CatOddLanes8x16: r.append("CatOddLanes8x16"); break;
		case Iop_CatOddLanes16x8: r.append("CatOddLanes16x8"); break;
		case Iop_CatOddLanes32x4: r.append("CatOddLanes32x4"); break;
		case Iop_CatEvenLanes8x16: r.append("CatEvenLanes8x16"); break;
		case Iop_CatEvenLanes16x8: r.append("CatEvenLanes16x8"); break;
		case Iop_CatEvenLanes32x4: r.append("CatEvenLanes32x4"); break;

		case Iop_InterleaveOddLanes8x16: r.append("InterleaveOddLanes8x16"); break;
		case Iop_InterleaveOddLanes16x8: r.append("InterleaveOddLanes16x8"); break;
		case Iop_InterleaveOddLanes32x4: r.append("InterleaveOddLanes32x4"); break;
		case Iop_InterleaveEvenLanes8x16: r.append("InterleaveEvenLanes8x16"); break;
		case Iop_InterleaveEvenLanes16x8: r.append("InterleaveEvenLanes16x8"); break;
		case Iop_InterleaveEvenLanes32x4: r.append("InterleaveEvenLanes32x4"); break;

		case Iop_GetElem8x16: r.append("GetElem8x16"); break;
		case Iop_GetElem16x8: r.append("GetElem16x8"); break;
		case Iop_GetElem32x4: r.append("GetElem32x4"); break;
		case Iop_GetElem64x2: r.append("GetElem64x2"); break;

		case Iop_GetElem8x8: r.append("GetElem8x8"); break;
		case Iop_GetElem16x4: r.append("GetElem16x4"); break;
		case Iop_GetElem32x2: r.append("GetElem32x2"); break;
		case Iop_SetElem8x8: r.append("SetElem8x8"); break;
		case Iop_SetElem16x4: r.append("SetElem16x4"); break;
		case Iop_SetElem32x2: r.append("SetElem32x2"); break;

		case Iop_Extract64: r.append("Extract64"); break;
		case Iop_ExtractV128: r.append("ExtractV128"); break;

		case Iop_Perm8x16: r.append("Perm8x16"); break;
		case Iop_Reverse16_8x16: r.append("Reverse16_8x16"); break;
		case Iop_Reverse32_8x16: r.append("Reverse32_8x16"); break;
		case Iop_Reverse32_16x8: r.append("Reverse32_16x8"); break;
		case Iop_Reverse64_8x16: r.append("Reverse64_8x16"); break;
		case Iop_Reverse64_16x8: r.append("Reverse64_16x8"); break;
		case Iop_Reverse64_32x4: r.append("Reverse64_32x4"); break;

		case Iop_F32ToFixed32Ux4_RZ: r.append("F32ToFixed32Ux4_RZ"); break;
		case Iop_F32ToFixed32Sx4_RZ: r.append("F32ToFixed32Sx4_RZ"); break;
		case Iop_Fixed32UToF32x4_RN: r.append("Fixed32UToF32x4_RN"); break;
		case Iop_Fixed32SToF32x4_RN: r.append("Fixed32SToF32x4_RN"); break;
		case Iop_F32ToFixed32Ux2_RZ: r.append("F32ToFixed32Ux2_RZ"); break;
		case Iop_F32ToFixed32Sx2_RZ: r.append("F32ToFixed32Sx2_RZ"); break;
		case Iop_Fixed32UToF32x2_RN: r.append("Fixed32UToF32x2_RN"); break;
		case Iop_Fixed32SToF32x2_RN: r.append("Fixed32SToF32x2_RN"); break;
        case Iop_INVALID: r.append("INVALID"); break;
        default: r.append("UNK"); break;
	}

	switch (this->OpCode - base) {
	  case 0: r.append("8"); break;
	  case 1: r.append("16"); break;
	  case 2: r.append("32"); break;
	  case 3: r.append("64"); break;
	}

	return r;
}
