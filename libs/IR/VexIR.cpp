#include "VexIR.h"

#define OFFB_CC_OP     offsetof(VexGuestX86State,guest_CC_OP)
#define OFFB_CC_DEP1   offsetof(VexGuestX86State,guest_CC_DEP1)
#define OFFB_CC_DEP2   offsetof(VexGuestX86State,guest_CC_DEP2)
#define OFFB_CC_NDEP   offsetof(VexGuestX86State,guest_CC_NDEP)

/* need defines like this but for AMD64 as well */
#define AMD64_OFFB_CC_OP     offsetof(VexGuestAMD64State,guest_CC_OP)
#define AMD64_OFFB_CC_DEP1   offsetof(VexGuestAMD64State,guest_CC_DEP1)
#define AMD64_OFFB_CC_DEP2   offsetof(VexGuestAMD64State,guest_CC_DEP2)
#define AMD64_OFFB_CC_NDEP   offsetof(VexGuestAMD64State,guest_CC_NDEP)


#define OFFB_FPREGS    offsetof(VexGuestX86State,guest_FPREG[0])
#define OFFB_FPTAGS    offsetof(VexGuestX86State,guest_FPTAG[0])
#define OFFB_DFLAG     offsetof(VexGuestX86State,guest_DFLAG)
#define OFFB_IDFLAG    offsetof(VexGuestX86State,guest_IDFLAG)
#define OFFB_ACFLAG    offsetof(VexGuestX86State,guest_ACFLAG)
#define OFFB_FTOP      offsetof(VexGuestX86State,guest_FTOP)
#define OFFB_FC3210    offsetof(VexGuestX86State,guest_FC3210)
#define OFFB_FPROUND   offsetof(VexGuestX86State,guest_FPROUND)

#define OFFB_CS        offsetof(VexGuestX86State,guest_CS)
#define OFFB_DS        offsetof(VexGuestX86State,guest_DS)
#define OFFB_ES        offsetof(VexGuestX86State,guest_ES)
#define OFFB_FS        offsetof(VexGuestX86State,guest_FS)
#define OFFB_GS        offsetof(VexGuestX86State,guest_GS)
#define OFFB_SS        offsetof(VexGuestX86State,guest_SS)
#define OFFB_LDT       offsetof(VexGuestX86State,guest_LDT)
#define OFFB_GDT       offsetof(VexGuestX86State,guest_GDT)

using namespace boost;

unsigned long VBlock::getPCOff(void) {
    switch(this->CodeTarget.ta) {
        case X86:
            return OFFSET_x86_EIP;
            break;
        case AMD64:
            return OFFSET_amd64_RIP;
            break;
        case ARM:
            return OFFSET_arm_R15T;
            break;
        default:
            assert(!"UNKNOWN TARGET");
    }
    return 0;
}

unsigned long getWidthFromTy(IRType ty) {
	unsigned long w = 0;
	switch(ty) {
		case Ity_INVALID:
			break;
		case Ity_I1:
			w = 1;
			break;
		case Ity_I8:
			w = 8;
			break;
		case Ity_I16:
			w = 16;
			break;
		case Ity_I32:
			w = 32;
			break;
		case Ity_I64:
			w = 64;
			break;
		case Ity_I128:
			w = 128;
			break;
		case Ity_F32:
			w = 32;
			break;
		case Ity_F64:
			w = 64;
			break;
		case Ity_F128:
			w = 128;
			break;
		case Ity_V128:
			w = 128;
			break;
	}

    assert(w != 0);
	return w;
}

Register guestOffsetToRegister(int guestOffset, unsigned long width, TargetArch arch) {
    Register    reg;
    assert(width == 8 || width == 16 || width == 32 || width == 64 || width == 128);
    assert(arch.ta != INVALID);

	reg.width = width;
	reg.arch = arch;
	switch( arch.ta ) {
        case S390X:
        case PPC32:
        case PPC64:
        case INVALID:
            assert(false && "Arch not supported yet!");
            break;
        case AMD64:
            reg.regclass = GenericRegister;
            reg.width = width;
            switch(width) {
              case 8:
                switch(guestOffset) {
                  case OFFSET_amd64_RAX:
                    reg.Reg8 = AL;
                    break;
                  case OFFSET_amd64_RAX+1:
                    reg.Reg8 = AH;
                    break;
                  case OFFSET_amd64_RBX:
                    reg.Reg8 = BL;
                    break;
                  case OFFSET_amd64_RBX+1:
                    reg.Reg8 = BH;
                    break;
                  case OFFSET_amd64_RCX:
                    reg.Reg8 = CL;
                    break;
                  case OFFSET_amd64_RCX+1:
                    reg.Reg8 = CH;
                    break;
                  case OFFSET_amd64_RDX:
                    reg.Reg8 = DL;
                    break;
                  case OFFSET_amd64_RDX+1:
                    reg.Reg8 = DH;
                    break;
                  default:
                    reg.width = 0;
                    break;
                }
                break;
              case 16:
                switch(guestOffset) {
                  case OFFSET_amd64_RAX:
                    reg.Reg16 = AX;
                    break;
                  case OFFSET_amd64_RBX:
                    reg.Reg16 = BX;
                    break;
                  case OFFSET_amd64_RCX:
                    reg.Reg16 = CX;
                    break;
                  case OFFSET_amd64_RDX:
                    reg.Reg16 = DX;
                    break;
                  case OFFSET_amd64_RSI:
                    reg.Reg16 = SI;
                    break;
                  case OFFSET_amd64_RDI:
                    reg.Reg16 = DI;
                    break;
                  case OFFSET_amd64_RSP:
                    reg.Reg16 = SP;
                    reg.regclass = StackPointer;
                    break;
                  case OFFSET_amd64_RBP:
                    reg.Reg16 = BP;
                    break;
                  default:
                    reg.width = 0;
                    break;
                }
                break;
              case 32:
                switch(guestOffset) {
                  case AMD64_OFFB_CC_OP:
                    reg.regclass = Flags;
                    reg.Reg32 = OP;
                    break;
                  case AMD64_OFFB_CC_DEP1:
                    reg.regclass = Flags;
                    reg.Reg32 = DEP1;
                    break;
                  case AMD64_OFFB_CC_DEP2:
                    reg.regclass = Flags;
                    reg.Reg32 = DEP2;
                    break;
                  case AMD64_OFFB_CC_NDEP:
                    reg.regclass = Flags;
                    reg.Reg32 = NDEP;
                    break;
                  case OFFSET_amd64_RAX:
                    reg.Reg32 = EAX;
                    break;
                  case OFFSET_amd64_RBX:
                    reg.Reg32 = EBX;
                    break;
                  case OFFSET_amd64_RCX:
                    reg.Reg32 = ECX;
                    break;
                  case OFFSET_amd64_RDX:
                    reg.Reg32 = EDX;
                    break;
                  case OFFSET_amd64_RSI:
                    reg.Reg32 = ESI;
                    break;
                  case OFFSET_amd64_RDI:
                    reg.Reg32 = EDI;
                    break;
                  case OFFSET_amd64_RSP:
                    reg.Reg32 = ESP;
                    reg.regclass = StackPointer;
                    break;
                  case OFFSET_amd64_RBP:
                    reg.Reg32 = EBP;
                    break;
                  case OFFSET_amd64_RIP:
                    reg.Reg32 = EIP;
                    reg.regclass = ProgramCounter;
                    reg.regclass = ProgramCounter;
                    break;
                  default:
                    reg.width = 0;
                    break;
                }
                break;
              case 64:
                switch(guestOffset) {
                  case OFFSET_amd64_RAX:
                    reg.Reg64 = RAX;
                    break;
                  case OFFSET_amd64_RBX:
                    reg.Reg64 = RBX;
                    break;
                  case OFFSET_amd64_RCX:
                    reg.Reg64 = RAX;
                    break;
                  case OFFSET_amd64_RDX:
                    reg.Reg64 = RDX;
                    break;
                  case OFFSET_amd64_RSI:
                    reg.Reg64 = RSI;
                    break;
                  case OFFSET_amd64_RDI:
                    reg.Reg64 = RDI;
                    break;
                  case OFFSET_amd64_RSP:
                    reg.Reg64 = RSP;
                    reg.regclass = StackPointer;
                    break;
                  case OFFSET_amd64_RBP:
                    reg.Reg64 = RBP;
                    break;
                  case OFFSET_amd64_R8:
                    reg.Reg64 = R8;
                    break;
                  case OFFSET_amd64_R9:
                    reg.Reg64 = R9;
                    break;
                  case OFFSET_amd64_R10:
                    reg.Reg64 = R10;
                    break;
                  case OFFSET_amd64_R11:
                    reg.Reg64 = R11;
                    break;
                  case OFFSET_amd64_R12:
                    reg.Reg64 = R12;
                    break;
                  case OFFSET_amd64_R13:
                    reg.Reg64 = R13;
                    break;
                  case OFFSET_amd64_R14:
                    reg.Reg64 = R14;
                    break;
                  case OFFSET_amd64_R15:
                    reg.Reg64 = R15;
                    break;
                  case OFFSET_amd64_RIP:
                    reg.Reg64 = RIP;
                    reg.regclass = ProgramCounter;
                    break;
                  default:
                    reg.width = 0;
                    break;
                }
                break;
              default:
                reg.width = 0;
                break;
            }
            break;
        case ARM:
            reg.regclass = GenericRegister;
            reg.width = 32;
            switch(guestOffset) {
                case OFFSET_arm_R0:
                    reg.RegArm = AR1;
                    break;
                case OFFSET_arm_R1:
                    reg.RegArm = AR1;
                    break;
                case OFFSET_arm_R2:
                    reg.RegArm = AR2;
                    break;
                case OFFSET_arm_R3:
                    reg.RegArm = AR3;
                    break;
                case OFFSET_arm_R4:
                    reg.RegArm = AR4;
                    break;
                case OFFSET_arm_R5:
                    reg.RegArm = AR5;
                    break;
                case OFFSET_arm_R5+4:
                    reg.RegArm = AR6;
                    break;
                case OFFSET_arm_R7:
                    reg.RegArm = AR7;
                    break;
                case OFFSET_arm_R7+4:
                    reg.RegArm = AR8;
                    break;
                case OFFSET_arm_R7+8:
                    reg.RegArm = AR9;
                    break;
                case OFFSET_arm_R7+12:
                    reg.RegArm = AR10;
                    break;
                case OFFSET_arm_R7+16:
                    reg.RegArm = AR11;
                    break;
                case OFFSET_arm_R7+20:
                    reg.RegArm = AR12;
                    break;
                case OFFSET_arm_R13:
                    reg.RegArm = AR13;
                    break;
                case OFFSET_arm_R14:
                    reg.RegArm = AR14;
                    break;
                case OFFSET_arm_R15T:
                    reg.RegArm = AR15;
                    break;
                default:
                    reg.RegArm = ARUnknown;
                    reg.regclass = GenericRegister;
            }
            break;
		case X86:
      reg.regclass = GenericRegister;
			switch(width) {
				case 8:
					switch(guestOffset) {
						case OFFSET_x86_EAX:
							reg.Reg8 = AL;
							break;
						case OFFSET_x86_EAX+1:
							reg.Reg8 = AH;
							break;
						case OFFSET_x86_EBX:
							reg.Reg8 = BL;
							break;
						case OFFSET_x86_EBX+1:
							reg.Reg8 = BH;
							break;
						case OFFSET_x86_ECX:
							reg.Reg8 = CL;
							break;
						case OFFSET_x86_ECX+1:
							reg.Reg8 = CH;
							break;
						case OFFSET_x86_EDX:
							reg.Reg8 = DL;
							break;
						case OFFSET_x86_EDX+1:
							reg.Reg8 = DH;
							break;
                        default:
                            reg.width = 0;
                            break;
					}
					break;
				case 16:
					switch(guestOffset) {
						case OFFSET_x86_EAX:
							reg.Reg16 = AX;
							break;
						case OFFSET_x86_EBX:
							reg.Reg16 = BX;
							break;
						case OFFSET_x86_ECX:
							reg.Reg16 = CX;
							break;
						case OFFSET_x86_EDX:
							reg.Reg16 = DX;
							break;
						case OFFSET_x86_ESI:
							reg.Reg16 = SI;
							break;
						case OFFSET_x86_EDI:
							reg.Reg16 = DI;
							break;
						case OFFSET_x86_ESP:
							reg.Reg16 = SP;
                            reg.regclass = StackPointer;
							break;
						case OFFSET_x86_EBP:
							reg.Reg16 = BP;
							break;
            default:
              reg.width = 0;
              break;
					}
					break;
				case 32:
					switch(guestOffset) {
            case OFFB_CC_OP:
              reg.regclass = Flags;
              reg.Reg32 = OP;
              break;
            case OFFB_CC_DEP1:
              reg.regclass = Flags;
              reg.Reg32 = DEP1;
              break;
            case OFFB_CC_DEP2:
              reg.regclass = Flags;
              reg.Reg32 = DEP2;
              break;
            case OFFB_CC_NDEP:
              reg.regclass = Flags;
              reg.Reg32 = NDEP;
              break;
            case OFFB_DFLAG:
              reg.regclass = Flags;
              reg.Reg32 = DFLAG;
              break;
            case OFFB_IDFLAG:
              reg.regclass = Flags;
              reg.Reg32 = IDFLAG;
              break;
            case OFFB_ACFLAG:
              reg.regclass = Flags;
              reg.Reg32 = ACFLAG;
              break;
						case OFFSET_x86_EAX:
							reg.Reg32 = EAX;
							break;
						case OFFSET_x86_EBX:
							reg.Reg32 = EBX;
							break;
						case OFFSET_x86_ECX:
							reg.Reg32 = ECX;
							break;
						case OFFSET_x86_EDX:
							reg.Reg32 = EDX;
							break;
						case OFFSET_x86_ESI:
							reg.Reg32 = ESI;
							break;
						case OFFSET_x86_EDI:
							reg.Reg32 = EDI;
							break;
						case OFFSET_x86_ESP:
							reg.Reg32 = ESP;
              reg.regclass = StackPointer;
							break;
						case OFFSET_x86_EBP:
							reg.Reg32 = EBP;
							break;
						case OFFSET_x86_EIP:
							reg.Reg32 = EIP;
              reg.regclass = ProgramCounter;
							break;
							// TODO: flags

							// TODO: these ones
						case OFFB_LDT:
                            reg.Reg32 = LDT32;
                            break;
						case OFFB_GDT:
                            reg.Reg32 = GDT32;
                            break;
						case offsetof(VexGuestX86State, guest_EMNOTE):
							reg.Reg32 = EMNOTE32;
							break;

							// TODO: floating point
						case OFFB_FC3210:
						case OFFB_FPROUND:
							break;

							// TODO:mach operations
						case offsetof(VexGuestX86State, guest_SC_CLASS):
						case offsetof(VexGuestX86State, guest_IP_AT_SYSCALL):
							break;

            case OFFB_FTOP:
              reg.Reg32 = FTOP;
              reg.regclass = Flags;
              break;
            default:
              reg.width = 0;
              break;
					}
					break;
				case 64:
					switch(guestOffset) {
                        case offsetof(VexGuestX86State, guest_LDT):
                            reg.Reg64 = LDT;
                            break;
                        
                        case offsetof(VexGuestX86State, guest_GDT):
                            reg.Reg64 = GDT;
                            break;

                        case offsetof(VexGuestX86State, guest_CMSTART):
                            reg.Reg64 = TI;
                            break;

                        case offsetof(VexGuestX86State, guest_EMNOTE):
                            reg.Reg64 = EMNOTE;
                            break;

                        case offsetof(VexGuestX86State, guest_NRADDR):
                            break;
                        
                        case offsetof(VexGuestX86State, guest_CMLEN):
                            break;
					}
					break;
			}
	}
	return reg;
}

Op::Ops VOp::convertOp(IROp op) {
    Ops o = UNSUP;

    switch( op ) {
        case Iop_INVALID:
        case Iop_Clz64:
        case Iop_Clz32:
        case Iop_Ctz64:
        case Iop_Ctz32:
        case Iop_CmpNEZ8:
        case Iop_CmpNEZ16:
        case Iop_CmpNEZ32:
        case Iop_CmpNEZ64:
        case Iop_CmpwNEZ32:
        case Iop_CmpwNEZ64:
        case Iop_Left8:
        case Iop_Left16:
        case Iop_Left32:
        case Iop_Left64:
        case Iop_Max32U:
        case Iop_CmpORD32U:
        case Iop_CmpORD64U:
        case Iop_CmpORD32S:
        case Iop_CmpORD64S:
        case Iop_DivU32:
        case Iop_DivS32:
        case Iop_DivU64:
        case Iop_DivS64:
        case Iop_DivModU128to64:
        case Iop_DivModS128to64:
        case Iop_DivModS64to64:
        case Iop_8Sto64:
        case Iop_16Sto64:
        case Iop_8HLto16:
        case Iop_128to64:
        case Iop_128HIto64:
        case Iop_64HLto128:
        case Iop_1Uto64:
        case Iop_1Sto8:
        case Iop_1Sto16:
        case Iop_1Sto32:
        case Iop_1Sto64:
        case Iop_AddF64:
        case Iop_SubF64:
        case Iop_MulF64:
        case Iop_DivF64:
        case Iop_AddF32:
        case Iop_SubF32:
        case Iop_MulF32:
        case Iop_DivF32:
        case Iop_AddF64r32:
        case Iop_SubF64r32:
        case Iop_MulF64r32:
        case Iop_DivF64r32:
        case Iop_NegF64:
        case Iop_AbsF64:
        case Iop_NegF32:
        case Iop_AbsF32:
        case Iop_SqrtF64:
        //case Iop_SqrtF64r32:
        case Iop_SqrtF32:
        case Iop_CmpF64:
        case Iop_CmpF32:
        case Iop_CmpF128:
        case Iop_F64toI16S:
        case Iop_F64toI32S:
        case Iop_F64toI64S:
        case Iop_F64toI32U:
        //case Iop_I16StoF64:
        case Iop_I32StoF64:
        case Iop_I64StoF64:
        case Iop_I64UtoF64:
        case Iop_I64UtoF32:
        case Iop_I32UtoF64:
        //case Iop_F32toI16S:
        case Iop_F32toI32S:
        case Iop_F32toI64S:
        //case Iop_I16StoF32:
        case Iop_I32StoF32:
        case Iop_I64StoF32:
        case Iop_F64toF32:
        case Iop_ReinterpF64asI64:
        case Iop_ReinterpI64asF64:
        case Iop_ReinterpF32asI32:
        case Iop_ReinterpI32asF32:
        case Iop_F64HLtoF128:
        case Iop_F128HItoF64:
        case Iop_F128LOtoF64:
        case Iop_AddF128:
        case Iop_SubF128:
        case Iop_MulF128:
        case Iop_DivF128:
        case Iop_NegF128:
        case Iop_AbsF128:
        case Iop_SqrtF128:
        case Iop_I32StoF128:
        case Iop_I64StoF128:
        case Iop_F32toF128:
        case Iop_F64toF128:
        case Iop_F128toI32S:
        case Iop_F128toI64S:
        case Iop_F128toF64:
        case Iop_F128toF32:
        case Iop_AtanF64:
        case Iop_Yl2xF64:
        case Iop_Yl2xp1F64:
        case Iop_PRemF64:
        case Iop_PRemC3210F64:
        case Iop_PRem1F64:
        case Iop_PRem1C3210F64:
        case Iop_ScaleF64:
        case Iop_SinF64:
        case Iop_CosF64:
        case Iop_TanF64:
        case Iop_2xm1F64:
        case Iop_RoundF64toInt:
        case Iop_RoundF32toInt:
        case Iop_MAddF32:
        case Iop_MSubF32:
        case Iop_MAddF64:
        case Iop_MSubF64:
        case Iop_MAddF64r32:
        case Iop_MSubF64r32:
        //case Iop_Est5FRSqrt:
        case Iop_RoundF64toF64_NEAREST:
        case Iop_RoundF64toF64_NegINF:
        case Iop_RoundF64toF64_PosINF:
        case Iop_RoundF64toF64_ZERO:
        case Iop_TruncF64asF32:
        case Iop_RoundF64toF32:
        //case Iop_CalcFPRF:
        case Iop_Add16x2:
        case Iop_Sub16x2:
        case Iop_QAdd16Sx2:
        case Iop_QAdd16Ux2:
        case Iop_QSub16Sx2:
        case Iop_QSub16Ux2:
        case Iop_HAdd16Ux2:
        case Iop_HAdd16Sx2:
        case Iop_HSub16Ux2:
        case Iop_HSub16Sx2:
        case Iop_Add8x4:
        case Iop_Sub8x4:
        case Iop_QAdd8Sx4:
        case Iop_QAdd8Ux4:
        case Iop_QSub8Sx4:
        case Iop_QSub8Ux4:
        case Iop_HAdd8Ux4:
        case Iop_HAdd8Sx4:
        case Iop_HSub8Ux4:
        case Iop_HSub8Sx4:
        case Iop_CmpNEZ16x2:
        case Iop_CmpNEZ8x4:
        case Iop_I32UtoFx2:
        case Iop_I32StoFx2:
        case Iop_FtoI32Ux2_RZ:
        case Iop_FtoI32Sx2_RZ:
        case Iop_F32ToFixed32Ux2_RZ:
        case Iop_F32ToFixed32Sx2_RZ:
        case Iop_Fixed32UToF32x2_RN:
        case Iop_Fixed32SToF32x2_RN:
        case Iop_Max32Fx2:
        case Iop_Min32Fx2:
        case Iop_PwMax32Fx2:
        case Iop_PwMin32Fx2:
        case Iop_CmpEQ32Fx2:
        case Iop_CmpGT32Fx2:
        case Iop_CmpGE32Fx2:
        //case Iop_Recip32Fx2:
        //case Iop_Recps32Fx2:
        //case Iop_Rsqrte32Fx2:
        //case Iop_Rsqrts32Fx2:
        case Iop_Neg32Fx2:
        case Iop_Abs32Fx2:
        case Iop_CmpNEZ8x8:
        case Iop_CmpNEZ16x4:
        case Iop_CmpNEZ32x2:
        case Iop_PwAdd8x8:
        case Iop_PwAdd16x4:
        case Iop_PwAdd32x2:
        case Iop_PwMax8Sx8:
        case Iop_PwMax16Sx4:
        case Iop_PwMax32Sx2:
        case Iop_PwMax8Ux8:
        case Iop_PwMax16Ux4:
        case Iop_PwMax32Ux2:
        case Iop_PwMin8Sx8:
        case Iop_PwMin16Sx4:
        case Iop_PwMin32Sx2:
        case Iop_PwMin8Ux8:
        case Iop_PwMin16Ux4:
        case Iop_PwMin32Ux2:
        case Iop_PwAddL8Ux8:
        case Iop_PwAddL16Ux4:
        case Iop_PwAddL32Ux2:
        case Iop_PwAddL8Sx8:
        case Iop_PwAddL16Sx4:
        case Iop_PwAddL32Sx2:
        case Iop_Abs8x8:
        case Iop_Abs16x4:
        case Iop_Abs32x2:
        case Iop_QDMulHi16Sx4:
        case Iop_QDMulHi32Sx2:
        case Iop_QRDMulHi16Sx4:
        case Iop_QRDMulHi32Sx2:
        case Iop_Avg8Ux8:
        case Iop_Avg16Ux4:
        case Iop_Cnt8x8:
        //case Iop_Clz8Sx8:
        //case Iop_Clz16Sx4:
        //case Iop_Clz32Sx2:
        //case Iop_Cls8Sx8:
        //case Iop_Cls16Sx4:
        //case Iop_Cls32Sx2:
        case Iop_Shl8x8:
        case Iop_Shl16x4:
        case Iop_Shl32x2:
        case Iop_Shr8x8:
        case Iop_Shr16x4:
        case Iop_Shr32x2:
        case Iop_Sar8x8:
        case Iop_Sar16x4:
        case Iop_Sar32x2:
        case Iop_Sal8x8:
        case Iop_Sal16x4:
        case Iop_Sal32x2:
        case Iop_Sal64x1:
        case Iop_QShl8x8:
        case Iop_QShl16x4:
        case Iop_QShl32x2:
        case Iop_QShl64x1:
        case Iop_QSal8x8:
        case Iop_QSal16x4:
        case Iop_QSal32x2:
        case Iop_QSal64x1:
        //case Iop_QShlN8Sx8:
        //case Iop_QShlN16Sx4:
        //case Iop_QShlN32Sx2:
        //case Iop_QShlN64Sx1:
        //case Iop_QShlN8x8:
        //case Iop_QShlN16x4:
        //case Iop_QShlN32x2:
        //case Iop_QShlN64x1:
        //case Iop_QSalN8x8:
        //case Iop_QSalN16x4:
        //case Iop_QSalN32x2:
        //case Iop_QSalN64x1:
        case Iop_CatOddLanes8x8:
        case Iop_CatOddLanes16x4:
        case Iop_CatEvenLanes8x8:
        case Iop_CatEvenLanes16x4:
        case Iop_GetElem8x8:
        case Iop_GetElem16x4:
        case Iop_GetElem32x2:
        case Iop_SetElem8x8:
        case Iop_SetElem16x4:
        case Iop_SetElem32x2:
        case Iop_Dup8x8:
        case Iop_Dup16x4:
        case Iop_Dup32x2:
        //case Iop_Extract64:
        //case Iop_Reverse16_8x8:
        //case Iop_Reverse32_8x8:
        //case Iop_Reverse32_16x4:
        //case Iop_Reverse64_8x8:
        //case Iop_Reverse64_16x4:
        //case Iop_Reverse64_32x2:
        case Iop_Perm8x8:
        //case Iop_Recip32x2:
        //case Iop_Rsqrte32x2:
        case Iop_Add32Fx4:
        case Iop_Sub32Fx4:
        case Iop_Mul32Fx4:
        case Iop_Div32Fx4:
        case Iop_Max32Fx4:
        case Iop_Min32Fx4:
        case Iop_Add32Fx2:
        case Iop_Sub32Fx2:
        case Iop_CmpEQ32Fx4:
        case Iop_CmpLT32Fx4:
        case Iop_CmpLE32Fx4:
        case Iop_CmpUN32Fx4:
        case Iop_CmpGT32Fx4:
        case Iop_CmpGE32Fx4:
        case Iop_Abs32Fx4:
        case Iop_PwMax32Fx4:
        case Iop_PwMin32Fx4:
        case Iop_Sqrt32Fx4:
        //case Iop_RSqrt32Fx4:
        case Iop_Neg32Fx4:
        //case Iop_Recip32Fx4:
        //case Iop_Recps32Fx4:
        //case Iop_Rsqrte32Fx4:
        //case Iop_Rsqrts32Fx4:
        case Iop_I32UtoFx4:
        case Iop_I32StoFx4:
        case Iop_FtoI32Ux4_RZ:
        case Iop_FtoI32Sx4_RZ:
        case Iop_QFtoI32Ux4_RZ:
        case Iop_QFtoI32Sx4_RZ:
        case Iop_RoundF32x4_RM:
        case Iop_RoundF32x4_RP:
        case Iop_RoundF32x4_RN:
        case Iop_RoundF32x4_RZ:
        case Iop_F32ToFixed32Ux4_RZ:
        case Iop_F32ToFixed32Sx4_RZ:
        case Iop_Fixed32UToF32x4_RN:
        case Iop_Fixed32SToF32x4_RN:
        case Iop_F32toF16x4:
        case Iop_F16toF32x4:
        case Iop_Add32F0x4:
        case Iop_Sub32F0x4:
        case Iop_Mul32F0x4:
        case Iop_Div32F0x4:
        case Iop_Max32F0x4:
        case Iop_Min32F0x4:
        case Iop_CmpEQ32F0x4:
        case Iop_CmpLT32F0x4:
        case Iop_CmpLE32F0x4:
        case Iop_CmpUN32F0x4:
        //case Iop_Recip32F0x4:
        case Iop_Sqrt32F0x4:
        //case Iop_RSqrt32F0x4:
        case Iop_Add64Fx2:
        case Iop_Sub64Fx2:
        case Iop_Mul64Fx2:
        case Iop_Div64Fx2:
        case Iop_Max64Fx2:
        case Iop_Min64Fx2:
        case Iop_CmpEQ64Fx2:
        case Iop_CmpLT64Fx2:
        case Iop_CmpLE64Fx2:
        case Iop_CmpUN64Fx2:
        //case Iop_Recip64Fx2:
        case Iop_Sqrt64Fx2:
        //case Iop_RSqrt64Fx2:
        case Iop_Add64F0x2:
        case Iop_Sub64F0x2:
        case Iop_Mul64F0x2:
        case Iop_Div64F0x2:
        case Iop_Max64F0x2:
        case Iop_Min64F0x2:
        case Iop_CmpEQ64F0x2:
        case Iop_CmpLT64F0x2:
        case Iop_CmpLE64F0x2:
        case Iop_CmpUN64F0x2:
        //case Iop_Recip64F0x2:
        case Iop_Sqrt64F0x2:
        //case Iop_RSqrt64F0x2:
        case Iop_V128to64:
        case Iop_V128HIto64:
        case Iop_64HLtoV128:
        case Iop_64UtoV128:
        case Iop_SetV128lo64:
        case Iop_32UtoV128:
        case Iop_V128to32:
        case Iop_SetV128lo32:
        case Iop_NotV128:
        case Iop_AndV128:
        case Iop_OrV128:
        case Iop_XorV128:
        case Iop_ShlV128:
        case Iop_ShrV128:
        case Iop_CmpNEZ8x16:
        case Iop_CmpNEZ16x8:
        case Iop_CmpNEZ32x4:
        case Iop_CmpNEZ64x2:
        case Iop_Add8x16:
        case Iop_Add16x8:
        case Iop_Add32x4:
        case Iop_Add64x2:
        case Iop_QAdd8Ux16:
        case Iop_QAdd16Ux8:
        case Iop_QAdd32Ux4:
        case Iop_QAdd64Ux2:
        case Iop_QAdd8Sx16:
        case Iop_QAdd16Sx8:
        case Iop_QAdd32Sx4:
        case Iop_QAdd64Sx2:
        case Iop_Sub8x16:
        case Iop_Sub16x8:
        case Iop_Sub32x4:
        case Iop_Sub64x2:
        case Iop_QSub8Ux16:
        case Iop_QSub16Ux8:
        case Iop_QSub32Ux4:
        case Iop_QSub64Ux2:
        case Iop_QSub8Sx16:
        case Iop_QSub16Sx8:
        case Iop_QSub32Sx4:
        case Iop_QSub64Sx2:
        case Iop_Mul8x16:
        case Iop_Mul16x8:
        case Iop_Mul32x4:
        case Iop_MulHi16Ux8:
        case Iop_MulHi32Ux4:
        case Iop_MulHi16Sx8:
        case Iop_MulHi32Sx4:
        case Iop_MullEven8Ux16:
        case Iop_MullEven16Ux8:
        case Iop_MullEven8Sx16:
        case Iop_MullEven16Sx8:
        case Iop_Mull8Ux8:
        case Iop_Mull8Sx8:
        case Iop_Mull16Ux4:
        case Iop_Mull16Sx4:
        case Iop_Mull32Ux2:
        case Iop_Mull32Sx2:
        case Iop_QDMulHi16Sx8:
        case Iop_QDMulHi32Sx4:
        case Iop_QRDMulHi16Sx8:
        case Iop_QRDMulHi32Sx4:
        //case Iop_QDMulLong16Sx4:
        //case Iop_QDMulLong32Sx2:
        case Iop_PolynomialMul8x16:
        case Iop_PolynomialMull8x8:
        case Iop_PwAdd8x16:
        case Iop_PwAdd16x8:
        case Iop_PwAdd32x4:
        case Iop_PwAdd32Fx2:
        case Iop_PwAddL8Ux16:
        case Iop_PwAddL16Ux8:
        case Iop_PwAddL32Ux4:
        case Iop_PwAddL8Sx16:
        case Iop_PwAddL16Sx8:
        case Iop_PwAddL32Sx4:
        case Iop_CmpEQ8x16:
        case Iop_CmpEQ16x8:
        case Iop_CmpEQ32x4:
        case Iop_CmpGT8Sx16:
        case Iop_CmpGT16Sx8:
        case Iop_CmpGT32Sx4:
        case Iop_CmpGT64Sx2:
        case Iop_CmpGT8Ux16:
        case Iop_CmpGT16Ux8:
        case Iop_CmpGT32Ux4:
        case Iop_Cnt8x16:
        //case Iop_Clz8Sx16:
        //case Iop_Clz16Sx8:
        //case Iop_Clz32Sx4:
        //case Iop_Cls8Sx16:
        //case Iop_Cls16Sx8:
        //case Iop_Cls32Sx4:
        case Iop_ShlN8x16:
        case Iop_ShlN16x8:
        case Iop_ShlN32x4:
        case Iop_ShlN64x2:
        case Iop_ShrN8x16:
        case Iop_ShrN16x8:
        case Iop_ShrN32x4:
        case Iop_ShrN64x2:
        case Iop_SarN8x16:
        case Iop_SarN16x8:
        case Iop_SarN32x4:
        case Iop_SarN64x2:
        case Iop_Shl8x16:
        case Iop_Shl16x8:
        case Iop_Shl32x4:
        case Iop_Shl64x2:
        case Iop_Shr8x16:
        case Iop_Shr16x8:
        case Iop_Shr32x4:
        case Iop_Shr64x2:
        case Iop_Sar8x16:
        case Iop_Sar16x8:
        case Iop_Sar32x4:
        case Iop_Sar64x2:
        case Iop_Sal8x16:
        case Iop_Sal16x8:
        case Iop_Sal32x4:
        case Iop_Sal64x2:
        case Iop_Rol8x16:
        case Iop_Rol16x8:
        case Iop_Rol32x4:
        case Iop_QShl8x16:
        case Iop_QShl16x8:
        case Iop_QShl32x4:
        case Iop_QShl64x2:
        case Iop_QSal8x16:
        case Iop_QSal16x8:
        case Iop_QSal32x4:
        case Iop_QSal64x2:
        //case Iop_QShlN8Sx16:
        //case Iop_QShlN16Sx8:
        //case Iop_QShlN32Sx4:
        //case Iop_QShlN64Sx2:
        //case Iop_QShlN8x16:
        //case Iop_QShlN16x8:
        //case Iop_QShlN32x4:
        //case Iop_QShlN64x2:
        //case Iop_QSalN8x16:
        //case Iop_QSalN16x8:
        //case Iop_QSalN32x4:
        //case Iop_QSalN64x2:
        /*case Iop_QNarrow16Ux8:
        case Iop_QNarrow32Ux4:
        case Iop_QNarrow16Sx8:
        case Iop_QNarrow32Sx4:
        case Iop_Narrow16x8:
        case Iop_Narrow32x4:
        case Iop_Shorten16x8:
        case Iop_Shorten32x4:
        case Iop_Shorten64x2:
        case Iop_QShortenS16Sx8:
        case Iop_QShortenS32Sx4:
        case Iop_QShortenS64Sx2:
        case Iop_QShortenU16Sx8:
        case Iop_QShortenU32Sx4:
        case Iop_QShortenU64Sx2:
        case Iop_QShortenU16Ux8:
        case Iop_QShortenU32Ux4:
        case Iop_QShortenU64Ux2:
        case Iop_Longen8Ux8:
        case Iop_Longen16Ux4:
        case Iop_Longen32Ux2:
        case Iop_Longen8Sx8:
        case Iop_Longen16Sx4:
        case Iop_Longen32Sx2:*/
        case Iop_InterleaveHI8x16:
        case Iop_InterleaveHI16x8:
        case Iop_InterleaveHI32x4:
        case Iop_InterleaveHI64x2:
        case Iop_InterleaveLO8x16:
        case Iop_InterleaveLO16x8:
        case Iop_InterleaveLO32x4:
        case Iop_InterleaveLO64x2:
        case Iop_InterleaveOddLanes8x16:
        case Iop_InterleaveEvenLanes8x16:
        case Iop_InterleaveOddLanes16x8:
        case Iop_InterleaveEvenLanes16x8:
        case Iop_InterleaveOddLanes32x4:
        case Iop_InterleaveEvenLanes32x4:
        case Iop_CatOddLanes8x16:
        case Iop_CatOddLanes16x8:
        case Iop_CatOddLanes32x4:
        case Iop_CatEvenLanes8x16:
        case Iop_CatEvenLanes16x8:
        case Iop_CatEvenLanes32x4:
        case Iop_GetElem8x16:
        case Iop_GetElem16x8:
        case Iop_GetElem32x4:
        case Iop_GetElem64x2:
        case Iop_Dup8x16:
        case Iop_Dup16x8:
        case Iop_Dup32x4:
        //case Iop_ExtractV128:
        //case Iop_Reverse16_8x16:
        //case Iop_Reverse32_8x16:
        //case Iop_Reverse32_16x8:
        //case Iop_Reverse64_8x16:
        //case Iop_Reverse64_16x8:
        //case Iop_Reverse64_32x4:
        case Iop_Perm8x16:
        //case Iop_Recip32x4:
        //case Iop_Rsqrte32x4:
        case Iop_DivU64E:
        case Iop_DivS64E:
        case Iop_DivU32E: 
        case Iop_DivS32E:
        case Iop_F64toI64U:
        case Iop_QNarrowBin16Sto8Ux8:
        case Iop_QNarrowBin16Sto8Sx8:
        case Iop_QNarrowBin32Sto16Sx4:
        case Iop_NarrowBin16to8x8:
        case Iop_NarrowBin32to16x4:
        case Iop_CmpEQ64x2:
        case Iop_QNarrowBin16Sto8Ux16:
        case Iop_QNarrowBin32Sto16Ux8:
        case Iop_QNarrowBin16Sto8Sx16:
        case Iop_QNarrowBin32Sto16Sx8:
        case Iop_QNarrowBin16Uto8Ux16:
        case Iop_QNarrowBin32Uto16Ux8:
        case Iop_NarrowBin16to8x16:
        case Iop_NarrowBin32to16x8:
        case Iop_NarrowUn16to8x8:
        case Iop_NarrowUn32to16x4:
        case Iop_NarrowUn64to32x2:
        case Iop_QNarrowUn16Sto8Sx8:
        case Iop_QNarrowUn32Sto16Sx4:
        case Iop_QNarrowUn64Sto32Sx2:
        case Iop_QNarrowUn16Sto8Ux8:
        case Iop_QNarrowUn32Sto16Ux4:
        case Iop_QNarrowUn64Sto32Ux2:
        case Iop_QNarrowUn16Uto8Ux8:
        case Iop_QNarrowUn32Uto16Ux4:
        case Iop_QNarrowUn64Uto32Ux2:
        case Iop_Widen8Uto16x8:
        case Iop_Widen16Uto32x4:
        case Iop_Widen32Uto64x2:
        case Iop_Widen8Sto16x8:
        case Iop_Widen16Sto32x4:
        case Iop_Widen32Sto64x2:
            o = UNSUP;
            break;

        case Iop_Sad8Ux4:
            o = Sad8Ux4;
            break;

        case Iop_32HLto64:
            o = C32HLto64;
            break;

        case Iop_Add8:
        case Iop_Add16:
        case Iop_Add32:
        case Iop_Add64:
            o = Add;
            break;

        case Iop_Sub8:
        case Iop_Sub16:
        case Iop_Sub32:
        case Iop_Sub64:
            o = Sub;
            break;

        case Iop_MullS8:
        case Iop_MullS16:
        case Iop_MullS32:
        case Iop_MullS64:
            o = MulS;
            break;

        case Iop_Mul8:
        case Iop_Mul16:
        case Iop_Mul32:
        case Iop_Mul64:
            o = Mul;
            break;
        case Iop_MullU8:
        case Iop_MullU16:
        case Iop_MullU32:
        case Iop_MullU64:
            o = MulS;
            break;

        case Iop_Or8:
        case Iop_Or16:
        case Iop_Or32:
        case Iop_Or64:
            o = Or;
            break;

        case Iop_And8:
        case Iop_And16:
        case Iop_And32:
        case Iop_And64:
            o = And;
            break;

        case Iop_Xor8:
        case Iop_Xor16:
        case Iop_Xor32:
        case Iop_Xor64:
            o = Xor;
            break;

        case Iop_Shl8:
        case Iop_Shl16:
        case Iop_Shl32:
        case Iop_Shl64:
            o = Shl;
            break;
            
        case Iop_Shr8:
        case Iop_Shr16:
        case Iop_Shr32:
        case Iop_Shr64:
            o = Shr;
            break;

        case Iop_Sar8:
        case Iop_Sar16:
        case Iop_Sar32:
        case Iop_Sar64:
            o = Sar;
            break;
        
        case Iop_CasCmpEQ8:
        case Iop_CasCmpEQ16:
        case Iop_CasCmpEQ32:
        case Iop_CasCmpEQ64:
        case Iop_CmpEQ8:
        case Iop_CmpEQ16:
        case Iop_CmpEQ32:
        case Iop_CmpEQ64:
            o = CmpEQ;
            break;
        
        case Iop_CasCmpNE8:
        case Iop_CasCmpNE16:
        case Iop_CasCmpNE32:
        case Iop_CasCmpNE64:
        case Iop_CmpNE8:
        case Iop_CmpNE16:
        case Iop_CmpNE32:
        case Iop_CmpNE64:
            o = CmpNE;
            break;

        case Iop_Not1:
        case Iop_Not8:
        case Iop_Not16:
        case Iop_Not32:
        case Iop_Not64:
            o = Not;
            break;

        case Iop_CmpLE32S:
        case Iop_CmpLE64S:
            o = CmpLES;
            break;

        case Iop_CmpLE32U:
        case Iop_CmpLE64U:
            o = CmpLEU;
            break;

        case Iop_CmpLT32S:
        case Iop_CmpLT64S:
            o = CmpLTS;
            break;

        case Iop_CmpLT32U:
        case Iop_CmpLT64U:
            o = CmpLTU;
            break;

        case Iop_64to8:
            o = C64to8;
            break;

        case Iop_32to8:
            o = C32to8;
            break;
        
        case Iop_64to16:
            o = C64to16;
            break;

        case Iop_64to32:
            o = C64LOto32;
            break;

        case Iop_32to16:
            o = C32LOto16;
            break;

        case Iop_16Uto32:
            o = C16Uto32;
            break;

        case Iop_16HLto32:
            o = C16HLto32;
            break;

        case Iop_32HIto16:
            o = C32HIto16;
            break;

        case Iop_16to8:
            o = C16LOto8;
            break;

        case Iop_16HIto8:
            o = C16HIto8;
            break;
        
        case Iop_64HIto32:
            o = C64HIto32;
            break;
  
        case Iop_QSub8Ux8:
            o = QSub8Ux8;
            break;

        case Iop_QSub16Ux4:
            o = QSub16Ux4;
            break;

        case Iop_QSub32Ux2:
            o = QSub32Ux2;
            break;

        case Iop_QSub64Ux1:
            o = QSub64Ux1;
            break;

        case Iop_QSub8Sx8:
            o = QSub8Sx8;
            break;

        case Iop_QSub16Sx4:
            o = QSub16Sx4;
            break;

        case Iop_QSub32Sx2:
            o = QSub32Sx2;
            break;

        case Iop_QSub64Sx1:
            o = QSub64Sx1;
            break;

        case Iop_QAdd8Ux8:
            o = QAdd8Ux8;
            break;

        case Iop_QAdd16Ux4:
            o = QAdd16Ux4;
            break;

        case Iop_QAdd32Ux2:
            o = QAdd32Ux2;
            break;

        case Iop_QAdd64Ux1:
            o = QAdd64Ux1;
            break;

        case Iop_QAdd8Sx8:
            o = QAdd8Sx8;
            break;

        case Iop_QAdd16Sx4:
            o = QAdd16Sx4;
            break;

        case Iop_QAdd32Sx2:
            o = QAdd32Sx2;
            break;

        case Iop_QAdd64Sx1:
            o = QAdd64Sx1;
            break;

        case Iop_Add8x8:
            o = Add8x8;

        case Iop_Add16x4:
            o = Add16x4;

        case Iop_Add32x2:
            o = Add32x2;
        
        case Iop_Sub8x8:
            o = Sub8x8;

        case Iop_Sub16x4:
            o = Sub16x4;

        case Iop_Sub32x2:
            o = Sub32x2;
        
        case Iop_8Uto16:
            o = C8Uto16;
            break;

        case Iop_8Sto16:
            o = C8Sto16;
            break;
        
        case Iop_F32toF64:
            o = CF32toF64;
            break;

        case Iop_8Sto32:
            o = C8Sto32;
            break;

        case Iop_8Uto32:
            o = C8Uto32;
            break;

        case Iop_8Uto64:
            o = C8Uto64;
            break;

        case Iop_16Uto64:
            o = C16Uto64;
            break;

        case Iop_16Sto32:
            o = C16Sto32;
            break;

        case Iop_DivModS64to32:
            o = DivModS64to32;
            break;
 
        case Iop_DivModU64to32:
            o = DivModU64to32;
            break;
    
        case Iop_32Sto64:
            o = C32Sto64;
            break;

        case Iop_32Uto64:
            o = C32Uto64;
            break;

        case Iop_32to1:
            o = C32to1;
            break;

        case Iop_64to1:
            o = C64to1;
            break;

        case Iop_1Uto32:
            o = C1Uto32;
            break;

        case Iop_1Uto8:
            o = C1Uto8;
            break;

        case Iop_CmpEQ8x8:
            o = CmpEQ8x8;
            break;

        case Iop_CmpEQ16x4:
            o = CmpEQ16x4;
            break;

        case Iop_CmpEQ32x2:
            o = CmpEQ32x2;
            break;
        
        case Iop_CmpGT8Sx8:
            o = CmpGT8Sx8;
            break;

        case Iop_CmpGT16Sx4:
            o = CmpGT16Sx4;
            break;

        case Iop_CmpGT32Sx2:
            o = CmpGT32Sx2;
            break;
        
        case Iop_CmpGT8Ux8:
            o = CmpGT8Ux8;
            break;

        case Iop_CmpGT16Ux4:
            o = CmpGT16Ux4;
            break;

        case Iop_CmpGT32Ux2:
            o = CmpGT32Ux2;
            break;

        case Iop_ShlN8x8:
            o = ShlN8x8;
            break;

        case Iop_ShlN16x4:
            o = ShlN16x4;
            break;

        case Iop_ShlN32x2:
            o = ShlN32x2;
            break;

        case Iop_ShrN8x8:
            o = ShrN8x8;
            break;

        case Iop_ShrN16x4:
            o = ShrN16x4;
            break;
            
        case Iop_ShrN32x2:
            o = ShrN32x2;
            break;

        case Iop_SarN8x8:
            o = SarN8x8;
            break;

        case Iop_SarN16x4:
            o = SarN16x4;
            break;

        case Iop_SarN32x2:
            o = SarN32x2;
            break;

        case Iop_Mul8x8:
            o = Mul8x8;
            break;

        case Iop_Mul16x4:
            o = Mul16x4;
            break;

        case Iop_Mul32x2:
            o = Mul32x2;
            break;

        case Iop_Mul32Fx2:
            o = Mul32Fx2;
            break;

        case Iop_MulHi16Ux4:
            o = MulHi16Ux4;
            break;

        case Iop_MulHi16Sx4:
            o = MulHi16Sx4;
            break;

        case Iop_PolynomialMul8x8:
            o = PolyMul8x8;
            break;

        case Iop_InterleaveHI8x8:
            o = InterleaveHI8x8;
            break;

        case Iop_InterleaveHI16x4:
            o = InterleaveHI16x4;
            break;

        case Iop_InterleaveHI32x2:
            o = InterleaveHI32x2;
            break;

        case Iop_InterleaveLO8x8:
            o = InterleaveLO8x8;
            break;

        case Iop_InterleaveLO16x4:
            o = InterleaveLO16x4;
            break;

        case Iop_InterleaveLO32x2:
            o = InterleaveLO32x2;
            break;

        case Iop_InterleaveOddLanes8x8:
            o = InterleaveOddLanes8x8;
            break;

        case Iop_InterleaveEvenLanes8x8:
            o = InterleaveEvenLanes8x8;
            break;

        case Iop_InterleaveOddLanes16x4:
            o = InterleaveOddLanes16x4;
            break;

        case Iop_InterleaveEvenLanes16x4:
            o = InterleaveEvenLanes16x4;
            break;

        case Iop_Abs8x16:
            o = Abs8x16;
            break;

        case Iop_Abs16x8:
            o = Abs16x8;
            break;

        case Iop_Abs32x4:
            o = Abs32x4;
            break;

        case Iop_Avg8Ux16:
            o = Avg8Ux16;
            break;

        case Iop_Avg16Ux8:
            o = Avg16Ux8;
            break;

        case Iop_Avg32Ux4:
            o = Avg32Ux4;
            break;

        case Iop_Avg8Sx16:
            o = Avg8Sx16;
            break;

        case Iop_Avg16Sx8:
            o = Avg16Sx8;
            break;

        case Iop_Avg32Sx4:
            o = Avg32Sx4;
            break;

        case Iop_Max8Sx16:
            o = Max8Sx16;
            break;

        case Iop_Max16Sx8:
            o = Max16Sx8;
            break;

        case Iop_Max32Sx4:
            o = Max32Sx4;
            break;

        case Iop_Max8Ux16:
            o = Max8Ux16;
            break;

        case Iop_Max16Ux8:
            o = Max16Ux8;
            break;

        case Iop_Max32Ux4:
            o = Max32Ux4;
            break;

        case Iop_Min8Sx16:
            o = Min8Sx16;
            break;

        case Iop_Min16Sx8:
            o = Min16Sx8;
            break;

        case Iop_Min32Sx4:
            o = Min32Sx4;
            break;

        case Iop_Min8Ux16:
            o = Min8Ux16;
            break;

        case Iop_Min16Ux8:
            o = Min16Ux8;
            break;

        case Iop_Min32Ux4:
            o = Min32Ux4;
            break;

        case Iop_Min8Ux8:
            o = Min8Ux8;
            break;

        case Iop_Min16Ux4:
            o = Min16Ux4;
            break;

        case Iop_Min32Ux2:
            o = Min32Ux2;
            break;

        case Iop_Max8Sx8:
            o = Max8Sx8;
            break;

        case Iop_Max16Sx4:
            o = Max16Sx4;
            break;

        case Iop_Max32Sx2:
            o = Max32Sx2;
            break;

        case Iop_Max8Ux8:
            o = Max8Ux8;
            break;

        case Iop_Max16Ux4:
            o = Max16Ux4;
            break;

        case Iop_Max32Ux2:
            o = Max32Ux2;
            break;

        case Iop_Min8Sx8:
            o = Min8Sx8;
            break;

        case Iop_Min16Sx4:
            o = Min16Sx4;
            break;

        case Iop_Min32Sx2:
            o = Min32Sx2;
            break;

        /*case Iop_QNarrow16Ux4:
            o = QNarrow16Ux4;
            break;

        case Iop_QNarrow16Sx4:
            o = QNarrow16Sx4;
            break;

        case Iop_QNarrow32Sx2:
            o = QNarrow32Sx2;*/
            break;
    }

    return o;
}

VExOp::VExOp(BlockPtr parent, OpPtr o, ExpressionPtr e) {
	this->op = o;
	this->exps.push_back(e);
	return;
}

VExOp::VExOp(BlockPtr parent, OpPtr o, ExpressionPtr e1, ExpressionPtr e2) {
	this->op = o;
	this->exps.push_back(e1);
	this->exps.push_back(e2);
	return;
}

VExOp::VExOp(BlockPtr parent, OpPtr o, ExpressionPtr e1, ExpressionPtr e2, ExpressionPtr e3) {
	this->op = o;
	this->exps.push_back(e1);
	this->exps.push_back(e2);
	this->exps.push_back(e3);
	return;
}

VExOp::VExOp(BlockPtr parent, OpPtr o, ExpressionPtr e1, ExpressionPtr e2, ExpressionPtr e3, ExpressionPtr e4) {
	this->op = o;
	this->exps.push_back(e1);
	this->exps.push_back(e2);
	this->exps.push_back(e3);
	this->exps.push_back(e4);
	return;
}

VExConst::VExConst(BlockPtr parent, IRConst *c) {
	memset(&this->cval, 0, sizeof(this->cval));
	this->constType = c->tag;
	switch(c->tag) {
		case Ico_U1:
			this->cval.U1 = c->Ico.U1;
            this->cval.valueType = ConstantValue::T_I1;
            this->cval.width = 1;
            this->cval.valueIsKnown = true;
			break;
		case Ico_U8: 
			this->cval.U8 = c->Ico.U8;
            this->cval.valueType = ConstantValue::T_I8;
            this->cval.width = 8;
            this->cval.valueIsKnown = true;
			break;
		case Ico_U16: 
			this->cval.U16 = c->Ico.U16;
            this->cval.valueType = ConstantValue::T_I16;
            this->cval.width = 16;
            this->cval.valueIsKnown = true;
			break;
		case Ico_U32: 
			this->cval.U32 = c->Ico.U32;
            this->cval.valueType = ConstantValue::T_I32;
            this->cval.width = 32;
            this->cval.valueIsKnown = true;
			break;
		case Ico_U64:
			this->cval.U64 = c->Ico.U64;
            this->cval.valueType = ConstantValue::T_I64;
            this->cval.width = 64;
            this->cval.valueIsKnown = true;
			break;
		case Ico_F32: 
			this->cval.F32 = c->Ico.F32;
            this->cval.valueType = ConstantValue::T_F32;
            this->cval.width = 32;
            this->cval.valueIsKnown = true;
			break;
		case Ico_F32i: 
			this->cval.F32i = c->Ico.F32i;
            this->cval.valueType = ConstantValue::T_F32;
            this->cval.width = 32;
            this->cval.valueIsKnown = true;
			break;
		case Ico_F64: 
			this->cval.F64 = c->Ico.F64;
            this->cval.valueType = ConstantValue::T_F64;
            this->cval.width = 64;
            this->cval.valueIsKnown = true;
			break;
		case Ico_F64i:
			this->cval.F64i = c->Ico.F64i;
            this->cval.valueType = ConstantValue::T_F64;
            this->cval.width = 64;
            this->cval.valueIsKnown = true;
			break;
		case Ico_V128:   
			this->cval.V128 = c->Ico.V128;
            this->cval.valueType = ConstantValue::T_V128;
            this->cval.width = 128;
            this->cval.valueIsKnown = true;
			break;
	}
	return;
}

unsigned long VExConst::getWidthFromConstTag(IRConstTag t ) {
	unsigned long r = 0;
	switch(t){
		case Ico_U1:
			r = 1;
			break;
		case Ico_U8: 
			r = 8;
			break;
		case Ico_U16: 
			r = 16;
			break;
		case Ico_U32: 
			r = 32;
			break;
		case Ico_U64:
			r = 64;
			break;
		case Ico_F32: 
			r = 32;
			break;
		case Ico_F32i: 
			r = 32;
			break;
		case Ico_F64: 
			r = 64;
			break;
		case Ico_F64i:
			r = 64;
			break;
		case Ico_V128:  
			r = 128;
			break;
	}

    assert(r != 0);
	return r;
}

VExRdTmp::VExRdTmp(BlockPtr parent, IRTemp t) { 
	this->tmpIdx = t; 
	this->TVal = parent->getTempAtIndex(this->tmpIdx);
}

ExpressionPtr expressionBuilder(IRExpr *exp, TargetArch arch, BlockPtr b) {
	Expression	*e=NULL;

    assert(exp != NULL);
	switch(exp->tag) {
        case Iex_Binder:
            //should never be seen
            break;

		case Iex_Get:
			e = new VExGet(b, exp->Iex.Get.offset, exp->Iex.Get.ty, arch);
			break;

		case Iex_GetI:
            e = new VExGet(b, exp->Iex.GetI.descr, expressionBuilder(exp->Iex.GetI.ix, arch, b), exp->Iex.GetI.bias, arch);
			break;

		case Iex_RdTmp:
			e = new VExRdTmp(b, exp->Iex.RdTmp.tmp);
			break;

		case Iex_Qop:
			e = new VExOp(b, 
					VOpPtr(new VOp(exp->Iex.Qop.details->op)),
					expressionBuilder(exp->Iex.Qop.details->arg1, arch, b),
					expressionBuilder(exp->Iex.Qop.details->arg2, arch, b),
					expressionBuilder(exp->Iex.Qop.details->arg3, arch, b),
					expressionBuilder(exp->Iex.Qop.details->arg4, arch, b)
				);
			break;

		case Iex_Triop:
			e = new VExOp(b,
					VOpPtr(new VOp(exp->Iex.Triop.details->op)),
					expressionBuilder(exp->Iex.Triop.details->arg1, arch, b),
					expressionBuilder(exp->Iex.Triop.details->arg2, arch, b),
					expressionBuilder(exp->Iex.Triop.details->arg3, arch, b)
				);
			break;

		case Iex_Binop:
			e = new VExOp(b,
					VOpPtr(new VOp(exp->Iex.Binop.op)),
					expressionBuilder(exp->Iex.Binop.arg1, arch, b),
					expressionBuilder(exp->Iex.Binop.arg2, arch, b)
					);
			break;

		case Iex_Unop:
			e = new VExOp(b, VOpPtr(new VOp(exp->Iex.Unop.op)), expressionBuilder(exp->Iex.Unop.arg, arch,b ));
			break;

		case Iex_Load:
			e = new VExLoad(b, exp->Iex.Load.end, exp->Iex.Load.ty, expressionBuilder(exp->Iex.Load.addr, arch, b));
			break;

		case Iex_Const:
			e = new VExConst(b, exp->Iex.Const.con);
			break;

		case Iex_ITE:
			e = new VExMux0X(b, expressionBuilder(exp->Iex.ITE.cond, arch, b), expressionBuilder(exp->Iex.ITE.iftrue, arch, b), expressionBuilder(exp->Iex.ITE.iffalse, arch, b));
			break;

		case Iex_CCall:
			e = new VExCCall(b, exp->Iex.CCall.args, exp->Iex.CCall.cee, exp->Iex.CCall.retty, arch);
			break;

			// TODO: Iex_VECRET, Iex_BBPTR
			//default:
			//FREAK OUT
	}
	
    return ExpressionPtr(e);
}

ExitType VBlock::jmpKindToExitType(IRJumpKind ij) {
    ExitType et = UnknownExit;

    switch(ij) {
        case Ijk_ClientReq:
        case Ijk_Yield:
        case Ijk_EmWarn:
        case Ijk_EmFail:
        case Ijk_NoDecode:
        case Ijk_MapFail:
        //case Ijk_TInval:
        case Ijk_NoRedir:
        case Ijk_SigTRAP:
        case Ijk_SigSEGV:
        case Ijk_SigBUS:
        case Ijk_Sys_syscall:
        case Ijk_Sys_int32:
        case Ijk_Sys_int128:
        case Ijk_Sys_int129:
        case Ijk_Sys_int130:
        case Ijk_Sys_sysenter:
			// TODO: verify other types in case they added new ones
            et = UnknownExit;
            break;

        case Ijk_Call:
            et = Call;
            break;
        case Ijk_Ret:
            et = Return;
            break;
        case Ijk_Boring:
            et = Fallthrough;
            break;
    }

    return et;
}

StatementPtr VBlock::statementBuilder(IRStmt *stmt, TargetArch arch) {
	Statement	*st=NULL;
	switch(stmt->tag) {
	   case Ist_NoOp:
		   st = new StNop();
		   break;

	   case Ist_IMark:
		   st = new VStIMark(shared_from_this(), stmt->Ist.IMark.len, stmt->Ist.IMark.addr);
		   break;

	   case Ist_AbiHint:
		   st = new VStAbiHint(shared_from_this(), stmt->Ist.AbiHint.len, expressionBuilder(stmt->Ist.AbiHint.base, arch, shared_from_this()), expressionBuilder(stmt->Ist.AbiHint.nia, arch, shared_from_this()));
		   break;

	   case Ist_WrTmp:
           st = new VStWrTmp(shared_from_this(), stmt->Ist.WrTmp.tmp, expressionBuilder(stmt->Ist.WrTmp.data, arch, shared_from_this()));
           break;

	   case Ist_Put:
		   st = new VStPut(shared_from_this(), stmt->Ist.Put.offset, expressionBuilder(stmt->Ist.Put.data, arch, shared_from_this()), arch);
		   break;

	   case Ist_PutI:
		   st = new VStPut(shared_from_this(), stmt->Ist.PutI.details->descr, expressionBuilder(stmt->Ist.PutI.details->data, arch, shared_from_this()), arch, expressionBuilder(stmt->Ist.PutI.details->ix, arch, shared_from_this()), stmt->Ist.PutI.details->bias);
		   break;

	   case Ist_Store:
		   st = new VStStore(shared_from_this(), stmt->Ist.Store.end, expressionBuilder(stmt->Ist.Store.data, arch, shared_from_this()), expressionBuilder(stmt->Ist.Store.addr, arch, shared_from_this()));
		   break;

	   case Ist_CAS:
		   st = new VStCAS(shared_from_this(), stmt->Ist.CAS.details, arch);
		   break;

	   case Ist_LLSC:
           //is LL or SC?
           if( stmt->Ist.LLSC.storedata != NULL ) {
                //is Store-Conditional
                IRExpr          *addrExpr = stmt->Ist.LLSC.addr; 
                IRExpr          *storedataExpr = stmt->Ist.LLSC.storedata; 
                ExpressionPtr   addr = 
                    expressionBuilder(addrExpr, arch, shared_from_this());
                ExpressionPtr storedata =
                    expressionBuilder(storedataExpr, arch, shared_from_this());
                
                st = new VStLLSC(   shared_from_this(), 
                                    stmt->Ist.LLSC.end,
                                    stmt->Ist.LLSC.result,
                                    addr,
                                    storedata);
           } else {
                //is Load-Linked
                IRExpr          *addrExpr = stmt->Ist.LLSC.addr;  
                ExpressionPtr   addr = 
                    expressionBuilder(addrExpr, arch, shared_from_this());
                st = new VStLLSC(   shared_from_this(),
                                    stmt->Ist.LLSC.end,
                                    stmt->Ist.LLSC.result,
                                    addr);
           }
		   break;

	   case Ist_Dirty:
		   st = new VStDirty(shared_from_this(), stmt->Ist.Dirty.details, arch);
		   break;

	   case Ist_MBE:
			st = new VStMBE(shared_from_this(), stmt->Ist.MBE.event);
			break;

	   case Ist_Exit:
			st = new VStExit(shared_from_this(), stmt->Ist.Exit.jk, expressionBuilder(stmt->Ist.Exit.guard, arch, shared_from_this()), VExConstPtr(new VExConst(shared_from_this(), stmt->Ist.Exit.dst)));
			break;
       
       // TODO: These are new and need to be implemented
       case Ist_LoadG:
            //std::cout << "Hit Ist_LoadG!\n";
            break;
       
       case Ist_StoreG:
            //std::cout << "Hit Ist_StoreG!\n";
            break;
		   
           //default:
		   //FREAK OUT
	}
    
	return StatementPtr(st);
}

bool VStCAS::hasLoad(void) {
    bool    r = false;
    //{ return (this->DataHi->containsLoad() || this->DataLo->containsLoad() || this->ExpectedHi->containsLoad() || this->ExpectedLo->containsLoad() || this->StoreAddress->containsLoad());}
    do {
        if( this->DataHi ) {
            r = this->DataHi->containsLoad();
            if( r ) {
                break;
            }
        }

        if( this->DataLo ) {
            r = this->DataLo->containsLoad();
            if( r ) {
                break;
            }
        }

        if( this->ExpectedHi ) {
            r = this->ExpectedHi->containsLoad();
            if( r ) {
                break;
            }
        }

        if( this->ExpectedLo ) {
            r = this->ExpectedLo->containsLoad();
            if( r ) {
                break;
            }
        }

        if( this->StoreAddress ) {
            r = this->StoreAddress->containsLoad();
            if( r ) {
                break;
            }
        }

    } while( false );

    return r;
}

VBlock::VBlock(TargetArch target, unsigned long long base, unsigned long len) {
    this->CodeTarget = target;
    this->BlockID = 0;
    this->processedTargets = false;
    this->BlockAddrBegin = base;
    this->BlockAddrEnd = base+len;

    return;
}

VBlock::VBlock(unsigned long id, TargetArch target) {
    this->CodeTarget = target;
    this->BlockID = id;
    this->processedTargets = false;

    return;
}

void VBlock::buildFromIRSB(IRSB *bb) {
	this->exitType = bb->jumpkind;
    this->blockExitType = this->jmpKindToExitType(this->exitType);
	this->processedTargets = false;
	this->Temps = std::vector<TempValPtr>(bb->tyenv->types_used);

	for( int i = 0; i < bb->tyenv->types_used; i++ ) {
		IRType	irTy = bb->tyenv->types[i];
		ConstantValue::ValTy	ty = ConstantValue::T_INVALID;
		int			width =0;

		switch(irTy) {
				case Ity_INVALID:
					break;
				case Ity_I1:
					width = 1;
					ty = ConstantValue::T_I1;
					break;
				case Ity_I8:
					width = 8;
					ty = ConstantValue::T_I8;
					break;
				case Ity_I16:
					width = 16;
					ty = ConstantValue::T_I16;
					break;
				case Ity_I32:
					width = 32;
					ty = ConstantValue::T_I32;
					break;
				case Ity_I64:
					width = 64;
					ty = ConstantValue::T_I64;
					break;
				case Ity_I128:
					width = 128;
					ty = ConstantValue::T_I128;
					break;
				case Ity_F32:
					width = 32;
					ty = ConstantValue::T_F32;
					break;
				case Ity_F64:
					width = 64;
					ty = ConstantValue::T_F64;
					break;
				case Ity_F128:
					width = 128;
					ty = ConstantValue::T_F128;
					break;
				case Ity_V128:
					width = 128;
					ty = ConstantValue::T_V128;
					break;
		}

        assert(width != 0);
		TempValPtr tval = TempValPtr(new TempVal(i, width, ty));

		this->Temps[i] = tval;
	}

	for( int i = 0; i < bb->stmts_used; i++ ) {
		IRStmt		    *vst = bb->stmts[i];
		StatementPtr    st = this->statementBuilder(vst, this->CodeTarget);

        //FIXME: statementBuilder failed if this is NULL, this basically will
        //       is silently ignoring it
		//assert(st != NULL);
        if(st == NULL)
            continue;
        
        VStWrTmpPtr  wrt = dynamic_pointer_cast<VStWrTmp>(st);
		if( wrt != NULL ) {
			//if this writes to a temp, it defines a new temp
			//update the tmp mapping vector so that this statement
			//is the defining value
			ExpressionPtr   definingExpr = wrt->getRHS();
			int             index = wrt->getTmpIndex();
			TempValPtr      t = this->Temps[index];
			t->setCreator(definingExpr);
		}

        StPutPtr put = dynamic_pointer_cast<StPut>(st);
		if( put != NULL ) {
			//this defines a write to guest state, see what's up with that 
		}

        StStorePtr  store = dynamic_pointer_cast<StStore>(st);
		if( store != NULL ) {
			//this defines a write into guest memory, examine/note it
		}

		this->statements.push_back(st);
	}


	this->Next = expressionBuilder( bb->next, 
                                    this->CodeTarget, 
                                    shared_from_this());

	//find the base address for the block
	this->BlockAddrBegin = 0;
	for( int i = 0; i < bb->stmts_used; i++ )
	{
		if( bb->stmts[i]->tag == Ist_IMark )
		{
			this->BlockAddrBegin = bb->stmts[i]->Ist.IMark.addr;
			break;
		}
	}

	assert(this->BlockAddrBegin != 0 );
	this->BlockAddrEnd = 0;
	//find the terminating address for the block, add the len of that ins
	for( int i = bb->stmts_used-1; i >= 0; i-- ) {
		if( bb->stmts[i]->tag == Ist_IMark )
		{
			this->BlockAddrEnd = 
                bb->stmts[i]->Ist.IMark.addr+bb->stmts[i]->Ist.IMark.len;
			break;
		}
	}

	//assert(this->BlockAddrEnd != 0 );
	//assert(this->BlockAddrBegin != this->BlockAddrEnd);

    this->preConditions = InputPtr(new Input(this));
    this->readMem = false;
    this->writeMem = false;
    this->anyCalls = false;

    std::vector<StatementPtr>::iterator   it = this->statements.begin();
    while( it != this->statements.end() ) {
        StatementPtr    s = *it;
        if( StExitPtr e = dynamic_pointer_cast<StExit>(s) ) {
            
            if( e->getJmpKind() == Call ) {
                this->anyCalls = true;
            }
        }

        if( StStorePtr st = dynamic_pointer_cast<StStore>(s) ) {
            this->writeMem = true;
        }

        if( s->hasLoad() ) {
            this->readMem = true;
        }

        ++it;
    }

    if( this->getExitKind() == Return ) {
        this->ret = true;
    } else {
        this->ret = false;
    }

    if( this->Next->isEConst() ) {

        this->condBranch = true;
    } else {
        this->condBranch = false;
    }

    //build up the exit transfer statements
    it = this->statements.begin();
    while( it != this->statements.end() ) {
        StatementPtr    s = *it;
        //does this statement define a write to a register  
        if( StPutPtr p = dynamic_pointer_cast<StPut>(s) ) {
            //does something writing to this register exist already?
            std::list<Transfer>::iterator k = this->transfer.begin();
            while( k != this->transfer.end() ) {
                SinkType sink = (*k).first;
                if( sink.isReg() && sink.getReg() == p->getDstRegister() ) {
                    //erase the old one 
                    k = this->transfer.erase(k);
                } else {
                    ++k;
                }
            }

            SinkType        sink(p->getDstRegister());
           
            ExpressionPtr   s = simplifyExpr(shared_from_this(), p->getData());
            Transfer        t(sink,s);
            //is the sink the program counter? if no, don't update here, 
            //we'll update later
            if( p->getDstRegister() != this->getPC() ) {
                this->transfer.push_back(t);
            }
        }

        if( StStorePtr st = dynamic_pointer_cast<StStore>(s) ) {
            ExpressionPtr   addr = st->getAddr();
            ExpressionPtr   data = st->getData();

            ExpressionPtr   simplifiedAddr = 
                    simplifyExpr(shared_from_this(), addr);
            ExpressionPtr   simplifiedData = 
                simplifyExpr(shared_from_this(), data);
           
            SinkType    sink(simplifiedAddr);
            Transfer    t(sink,simplifiedData);
            this->transfer.push_back(t);
        }
        ++it;
    }

    //create a transfer statement that writes to the PC that is the result of
    //the 'exit' criteria
    SinkType    exitSink(this->getPC());
    Transfer    t(exitSink,simplifyExpr(shared_from_this(), this->Next));
    this->transfer.push_back(t);
 
	return;
}

VBlock::~VBlock(void) {
	return;
}
