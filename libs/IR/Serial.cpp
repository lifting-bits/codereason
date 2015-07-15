#include <stdio.h>
#include <iostream>
#include <fstream>

#include "VexIR.h"

#include "IR.pb.h"

#include <google/protobuf/text_format.h>

using namespace boost;
using namespace std;

class ContextHandle {
public:
  DModule *M;
};

TargetArch TArchToTargetArch(const DModule::TArch &t) {
    TargetArch  ta = { INVALID, S_INVALID };

    switch(t.major()) {
        case DModule::X86:
            ta.ta = X86;
            break;

        case DModule::AMD64:
            ta.ta = AMD64;
            break;

        case DModule::ARM:
            ta.ta = ARM;
            break;
        
        case DModule::PPC32:
            ta.ta = PPC32;
            break;
        
        case DModule::PPC64:
            ta.ta = PPC64;
            break;

        case DModule::S390X:
            ta.ta = S390X;
            break;
    }

    switch(t.minor()) {
        case DModule::WIDEARM:
            ta.tm = WIDEARM;
            break;
        
        case DModule::THUMB:
            ta.tm = THUMB;
            break;
    }

    return ta;
}

void TargetArchToTArch(TargetArch *ta, DModule::TArch *t) {
    switch(ta->ta) {
        case X86:
            t->set_major(DModule::X86);
            break;

        case AMD64:
            t->set_major(DModule::AMD64);
            break;

        case ARM:
            t->set_major(DModule::ARM);
            break;
        
        case PPC32:
            t->set_major(DModule::PPC32);
            break;
        
        case PPC64:
            t->set_major(DModule::PPC64);
            break;

        case S390X:
            t->set_major(DModule::S390X);
            break;
        
        default:
            assert(!"not possible");
    }

    switch(ta->tm) {
        case WIDEARM:
            t->set_minor(DModule::WIDEARM);
            break;
        
        case THUMB:
            t->set_minor(DModule::THUMB);
            break;
        
        default:
            t->set_minor(DModule::TA_MI_INVALID);
    }


    return;
}

MemoryEnd deserializeEnd(DModule::MemoryEnd e) {
    switch(e) {
        case DModule::BigEndian:
            return BigEndian;
            break;
        
        case DModule::LittleEndian:
            return LittleEndian;
            break;
    }
}

DModule::MemoryEnd serializeEnd(MemoryEnd e) {
    switch(e) {
        case BigEndian:
            return DModule::BigEndian;
            break;

        case LittleEndian:
            return DModule::LittleEndian;
            break;
    }
}



ConstantValue::ValTy deserializeValTy(const DModule::ValTy t) {
    ConstantValue::ValTy    k = ConstantValue::T_INVALID;

    switch(t) {
        case DModule::T_I1:
            k = ConstantValue::T_I1;
            break;

        case DModule::T_I8:
            k = ConstantValue::T_I8;
            break;

        case DModule::T_I16:
            k = ConstantValue::T_I16;
            break;

        case DModule::T_I32:
            k = ConstantValue::T_I32;
            break;

        case DModule::T_I64:
            k = ConstantValue::T_I64;
            break;

        case DModule::T_I128:
            k = ConstantValue::T_I128;
            break;

        case DModule::T_F32:
            k = ConstantValue::T_F32;
            break;

        case DModule::T_F64:
            k = ConstantValue::T_F64;
            break;

        case DModule::T_F128:
            k = ConstantValue::T_F128;
            break;

        case DModule::T_V128:
            k = ConstantValue::T_V128;
            break;
    }

    return k;
}

DModule::ValTy serializeValTy(ConstantValue::ValTy t) {
    DModule::ValTy  k = DModule::T_INVALID;

    switch(t) {
        case ConstantValue::T_I1:
            k = DModule::T_I1;
            break;

        case ConstantValue::T_I8:
            k = DModule::T_I8;
            break;

        case ConstantValue::T_I16:
            k = DModule::T_I16;
            break;

        case ConstantValue::T_I32:
            k = DModule::T_I32;
            break;

        case ConstantValue::T_I64:
            k = DModule::T_I64;
            break;

        case ConstantValue::T_I128:
            k = DModule::T_I128;
            break;

        case ConstantValue::T_F32:
            k = DModule::T_F32;
            break;

        case ConstantValue::T_F64:
            k = DModule::T_F64;
            break;

        case ConstantValue::T_F128:
            k = DModule::T_F128;
            break;

        case ConstantValue::T_V128:
            k = DModule::T_V128;
            break;
    }

    return k;
}

void TempValToTVar(TempValPtr t, DModule::TVar  *var) {
    var->set_width(t->getVal().width);
    var->set_ty(serializeValTy(t->getTy()));
    var->set_varidx(t->getVarIndex());

    return;
}

TempValPtr  TVarToTempVal(const DModule::TVar &var) {
    ConstantValue::ValTy    ty = deserializeValTy(var.ty());
    int                     idx = var.varidx();
    int                     width = var.width();

    return TempValPtr(new TempVal(idx, width, ty));
}

void serializeRegister(Register r, DModule::Register *rs, int off) {
    
    TargetArchToTArch(&r.arch, rs->mutable_arch());
    rs->set_width(r.width);
    switch(r.regclass) {
        case GenericRegister:
            rs->set_regclass(DModule::GenericRegister);
            break;
        case StackPointer:
            rs->set_regclass(DModule::StackPointer);
            break;
        case ProgramCounter:
            rs->set_regclass(DModule::ProgramCounter);
            break;
        case Flags:
            rs->set_regclass(DModule::Flags);
            break;
        case InternalState:
            rs->set_regclass(DModule::InternalState);
            break;
    }

    switch(r.arch.ta) {
        case AMD64:
        case X86:
            switch(r.width) {
                case 8:
                    switch(r.Reg8) {
                        case AL:
                            rs->set_reg8(DModule::AL);
                            break;
                        case AH:
                            rs->set_reg8(DModule::AH);
                            break;
                        case BL:
                            rs->set_reg8(DModule::BL);
                            break;
                        case BH:
                            rs->set_reg8(DModule::BH);
                            break;
                        case CL:
                            rs->set_reg8(DModule::CL);
                            break;
                        case CH:
                            rs->set_reg8(DModule::CH);
                            break;
                        case DL:
                            rs->set_reg8(DModule::DL);
                            break;
                        case DH:
                            rs->set_reg8(DModule::DH);
                            break;
                        default:
                            assert(!"no reg to serialize");
                    }
                    break;
                case 16:
                    switch(r.Reg16) {
                        case AX:
                            rs->set_reg16(DModule::AX);
                            break;
                        case BX:
                            rs->set_reg16(DModule::BX);
                            break;
                        case CX:
                            rs->set_reg16(DModule::CX);
                            break;
                        case DX:
                            rs->set_reg16(DModule::DX);
                            break;
                        case BP:
                            rs->set_reg16(DModule::BP);
                            break;
                        case SP:
                            rs->set_reg16(DModule::SP);
                            break;
                        case SI:
                            rs->set_reg16(DModule::SI);
                            break;
                        case DI:
                            rs->set_reg16(DModule::DI);
                            break;
                        default:
                            assert(!"no reg to serialize");
                    }
                    break;
                case 32:
                    switch(r.Reg32) {
                        case EAX:
                            rs->set_reg32(DModule::EAX);
                            break;
                        case EBX:
                            rs->set_reg32(DModule::EBX);
                            break;
                        case ECX:
                            rs->set_reg32(DModule::ECX);
                            break;
                        case EDX:
                            rs->set_reg32(DModule::EDX);
                            break;
                        case EDI:
                            rs->set_reg32(DModule::EDI);
                            break;
                        case ESI:
                            rs->set_reg32(DModule::ESI);
                            break;
                        case EBP:
                            rs->set_reg32(DModule::EBP);
                            break;
                        case ESP:
                            rs->set_reg32(DModule::ESP);
                            break;
                        case EIP:
                            rs->set_reg32(DModule::EIP);
                            break;
                        case CS:
                            rs->set_reg32(DModule::CS);
                            break;
                        case DS:
                            rs->set_reg32(DModule::DS);
                            break;
                        case ES:
                            rs->set_reg32(DModule::ES);
                            break;
                        case FS:
                            rs->set_reg32(DModule::FS);
                            break;
                        case GS:
                            rs->set_reg32(DModule::GS);
                            break;
                        case SS:
                            rs->set_reg32(DModule::SS);
                            break;
                        case OP:
                            rs->set_reg32(DModule::OP);
                            break;
                        case DEP1:
                            rs->set_reg32(DModule::DEP1);
                            break;
                        case DEP2:
                            rs->set_reg32(DModule::DEP2);
                            break;
                        case NDEP:
                            rs->set_reg32(DModule::NDEP);
                            break;
                        case DFLAG:
                            rs->set_reg32(DModule::DFLAG);
                            break;
                        case IDFLAG:
                            rs->set_reg32(DModule::IDFLAG);
                            break;
                        case ACFLAG:
                            rs->set_reg32(DModule::ACFLAG);
                            break;
                        case FTOP:
                            rs->set_reg32(DModule::FTOP);
                            break;
                        case LDT32:
                            rs->set_reg32(DModule::LDT32);
                            break;
                        case GDT32:
                            rs->set_reg32(DModule::GDT32);
                            break;
                        case EMWARN32:
                            rs->set_reg32(DModule::EMWARN32);
                            break;
                        default:
                            printf("Register 0x%08x\n", r.Reg32); 
                            assert(!"X86: Serialization for this register is not implemented yet");
                    }
                    break;
                case 64:
                    switch(r.Reg64) {
                        case GDT:
                            rs->set_reg64(DModule::GDT);
                            break;
                        case LDT:
                            rs->set_reg64(DModule::LDT);
                            break;
                        case RAX:
                            rs->set_reg64(DModule::RAX);
                            break;
                        case RBX:
                            rs->set_reg64(DModule::RBX);
                            break;
                        case RCX:
                            rs->set_reg64(DModule::RCX);
                            break;
                        case RDX:
                            rs->set_reg64(DModule::RDX);
                            break;
                        case RDI:
                            rs->set_reg64(DModule::RDI);
                            break;
                        case RSI:
                            rs->set_reg64(DModule::RSI);
                            break;
                        case RSP:
                            rs->set_reg64(DModule::RSP);
                            break;
                        case RBP:
                            rs->set_reg64(DModule::RBP);
                            break;
                        case RIP:
                            rs->set_reg64(DModule::RIP);
                            break;
                        case R8:
                            rs->set_reg64(DModule::R8);
                            break;
                        case R9:
                            rs->set_reg64(DModule::R9);
                            break;
                        case R10:
                            rs->set_reg64(DModule::R10);
                            break;
                        case R11:
                            rs->set_reg64(DModule::R11);
                            break;
                        case R12:
                            rs->set_reg64(DModule::R12);
                            break;
                        case R13:
                            rs->set_reg64(DModule::R13);
                            break;
                        case R14:
                            rs->set_reg64(DModule::R14);
                            break;
                        case R15:
                            rs->set_reg64(DModule::R15);
                            break;
                        default:
                            printf("Register 0x%08x\n", r.Reg64); 
                            assert(!"AMD64: Serialization for this register is not implemented yet");
                    }
                    break;
            }
            break;

        case ARM:
            switch(r.RegArm) {
                case AR1:
                    rs->set_regarm(DModule::AR1);
                    break;
                case AR2:
                    rs->set_regarm(DModule::AR2);
                    break;
                case AR3:
                    rs->set_regarm(DModule::AR3);
                    break;
                case AR4:
                    rs->set_regarm(DModule::AR4);
                    break;
                case AR5:
                    rs->set_regarm(DModule::AR5);
                    break;
                case AR6:
                    rs->set_regarm(DModule::AR6);
                    break;
                case AR7:
                    rs->set_regarm(DModule::AR7);
                    break;
                case AR8:
                    rs->set_regarm(DModule::AR8);
                    break;
                case AR9:
                    rs->set_regarm(DModule::AR9);
                    break;
                case AR10:
                    rs->set_regarm(DModule::AR10);
                    break;
                case AR11:
                    rs->set_regarm(DModule::AR11);
                    break;
                case AR12:
                    rs->set_regarm(DModule::AR12);
                    break;
                case AR13:
                    rs->set_regarm(DModule::AR13);
                    break;
                case AR14:
                    rs->set_regarm(DModule::AR14);
                    break;
                case AR15:
                    rs->set_regarm(DModule::AR15);
                    break;
                default:
                    assert(!"no reg to serialize");
            }
            break;

        default:
            assert(!"NIY");
    }

    return;
}

ExitType deserializeExitType(DModule::ExitType t) {
    ExitType    k = UnknownExit;

    switch(t) { 
        case DModule::Fallthrough:
            k = Fallthrough;
            break;
        
        case DModule::Call:
            k = Call;
            break;

        case DModule::Return:
            k = Return;
            break;
        
        case DModule::UnknownExit:
            k = UnknownExit;
            break;
    }

    return k;
}

DModule::ExitType serializeExitType(ExitType t) {
    DModule::ExitType   k = DModule::UnknownExit;

    switch(t) {
        case Fallthrough:
            k = DModule::Fallthrough;
            break;

        case Call:
            k = DModule::Call;
            break;

        case Return:
            k = DModule::Return;
            break;

        case UnknownExit:
            k = DModule::UnknownExit;
            break;
    }

    return k;
}

Register deserializeRegister(const DModule::Register &rs) {
    Register    r;

    r.arch = TArchToTargetArch(rs.arch());
    r.width = rs.width();
    switch(rs.regclass()) {
        case DModule::GenericRegister:
            r.regclass = GenericRegister;
            break;
        case DModule::StackPointer:
            r.regclass = StackPointer;
            break;
        case DModule::ProgramCounter:
            r.regclass = ProgramCounter;
            break;
        case DModule::Flags:
            r.regclass = Flags;
            break;
        case DModule::InternalState:
            r.regclass = InternalState;
            break;
    }

    switch(r.arch.ta) {
        case AMD64:
        case X86:
            switch(r.width) {
                case 8:
                    switch(rs.reg8()) {
                        case DModule::AL:
                            r.Reg8 = AL;
                            break;
                        case DModule::AH:
                            r.Reg8 = AH;
                            break;
                        case DModule::BL:
                            r.Reg8 = BL;
                            break;
                        case DModule::BH:
                            r.Reg8 = BH;
                            break;
                        case DModule::CL:
                            r.Reg8 = CL;
                            break;
                        case DModule::CH:
                            r.Reg8 = CH;
                            break;
                        case DModule::DL:
                            r.Reg8 = DL;
                            break;
                        case DModule::DH:
                            r.Reg8 = DH;
                            break;
                    }
                    break;
                case 16:
                    switch(rs.reg16()) {
                        case DModule::AX:
                            r.Reg16 = AX;
                            break;
                        case DModule::BX:
                            r.Reg16 = BX;
                            break;
                        case DModule::CX:
                            r.Reg16 = CX;
                            break;
                        case DModule::DX:
                            r.Reg16 = DX;
                            break;
                        case DModule::BP:
                            r.Reg16 = BP;
                            break;
                        case DModule::SP:
                            r.Reg16 = SP;
                            break;
                        case DModule::SI:
                            r.Reg16 = SI;
                            break;
                        case DModule::DI:
                            r.Reg16 = DI;
                            break;
                    }
                    break;
                case 32:
                    switch(rs.reg32()) {
                        case DModule::EAX:
                            r.Reg32 = EAX;
                            break;
                        case DModule::EBX:
                            r.Reg32 = EBX;
                            break;
                        case DModule::ECX:
                            r.Reg32 = ECX;
                            break;
                        case DModule::EDX:
                            r.Reg32 = EDX;
                            break;
                        case DModule::EDI:
                            r.Reg32 = EDI;
                            break;
                        case DModule::ESI:
                            r.Reg32 = ESI;
                            break;
                        case DModule::EBP:
                            r.Reg32 = EBP;
                            break;
                        case DModule::ESP:
                            r.Reg32 = ESP;
                            break;
                        case DModule::EIP:
                            r.Reg32 = EIP;
                            break;
                        case DModule::CS:
                            r.Reg32 = CS;
                            break;
                        case DModule::DS:
                            r.Reg32 = DS;
                            break;
                        case DModule::ES:
                            r.Reg32 = ES;
                            break;
                        case DModule::FS:
                            r.Reg32 = FS;
                            break;
                        case DModule::GS:
                            r.Reg32 = GS;
                            break;
                        case DModule::SS:
                            r.Reg32 = SS;
                            break;
                        case DModule::OP:
                            r.Reg32 = OP;
                            break;
                        case DModule::DEP1:
                            r.Reg32 = DEP1;
                            break;
                        case DModule::DEP2:
                            r.Reg32 = DEP2;
                            break;
                        case DModule::NDEP:
                            r.Reg32 = NDEP;
                            break;
                        case DModule::DFLAG:
                            r.Reg32 = DFLAG;
                            break;
                        case DModule::IDFLAG:
                            r.Reg32 = IDFLAG;
                            break;
                        case DModule::ACFLAG:
                            r.Reg32 = ACFLAG;
                            break;
                        case DModule::FTOP:
                            r.Reg32 = FTOP;
                            break;
                        case DModule::LDT32:
                            r.Reg32 = LDT32;
                            break;
                        case DModule::GDT32:
                            r.Reg32 = GDT32;
                            break;
                        case DModule::EMWARN32:
                            r.Reg32 = EMWARN32;
                            break;
                    }
                    break;
                case 64:
                    if(rs.has_reg64() ) {
                        switch(rs.reg64()) {
                            case DModule::GDT:
                                r.Reg64 = GDT;
                                break;
                            case DModule::LDT:
                                r.Reg64 = LDT;
                                break;
                            case DModule::RAX:
                                r.Reg64 = RAX;
                                break;
                            case DModule::RBX:
                                r.Reg64 = RBX;
                                break;
                            case DModule::RCX:
                                r.Reg64 = RCX;
                                break;
                            case DModule::RDX:
                                r.Reg64 = RDX;
                                break;
                            case DModule::RDI:
                                r.Reg64 = RDI;
                                break;
                            case DModule::RSI:
                                r.Reg64 = RSI;
                                break;
                            case DModule::RSP:
                                r.Reg64 = RSP;
                                break;
                            case DModule::RBP:
                                r.Reg64 = RBP;
                                break;
                            case DModule::RIP:
                                r.Reg64 = RIP;
                                break;
                            case DModule::R8:
                                r.Reg64 = R8;
                                break;
                            case DModule::R9:
                                r.Reg64 = R9;
                                break;
                            case DModule::R10:
                                r.Reg64 = R10;
                                break;
                            case DModule::R11:
                                r.Reg64 = R11;
                                break;
                            case DModule::R12:
                                r.Reg64 = R12;
                                break;
                            case DModule::R13:
                                r.Reg64 = R13;
                                break;
                            case DModule::R14:
                                r.Reg64 = R14;
                                break;
                            case DModule::R15:
                                r.Reg64 = R15;
                                break;
                        }
                    } else {
                        r.width = 0;
                    }
                    break;
            }
            break;

        case ARM:
            switch(rs.regarm()) {
                case DModule::AR1:
                    r.RegArm = AR1;
                    break;
                case DModule::AR2:
                    r.RegArm = AR2;
                    break;
                case DModule::AR3:
                    r.RegArm = AR3;
                    break;
                case DModule::AR4:
                    r.RegArm = AR4;
                    break;
                case DModule::AR5:
                    r.RegArm = AR5;
                    break;
                case DModule::AR6:
                    r.RegArm = AR6;
                    break;
                case DModule::AR7:
                    r.RegArm = AR7;
                    break;
                case DModule::AR8:
                    r.RegArm = AR8;
                    break;
                case DModule::AR9:
                    r.RegArm = AR9;
                    break;
                case DModule::AR10:
                    r.RegArm = AR10;
                    break;
                case DModule::AR11:
                    r.RegArm = AR11;
                    break;
                case DModule::AR12:
                    r.RegArm = AR12;
                    break;
                case DModule::AR13:
                    r.RegArm = AR13;
                    break;
                case DModule::AR14:
                    r.RegArm = AR14;
                    break;
                case DModule::AR15:
                    r.RegArm = AR15;
                    break;
            }
            break;

        default:
            assert(!"NIY");
    }

    return r;
}

void serializeRegArray(RegArray *r, DModule::RegArray *rs) {
    
    rs->set_base(r->base);
    rs->set_ty(serializeValTy(r->ty));
    rs->set_numelems(r->numElems);

    return;
}

RegArray deserializeRegArray(const DModule::RegArray &rs) {
    RegArray    r;

    r.base = rs.base();
    r.ty = deserializeValTy(rs.ty());
    r.numElems = rs.numelems();

    return r;
}

ConstantValue deserializeConstant(const DModule::ConstantValue &cs) {
    ConstantValue   v;
    
    v.valueType = deserializeValTy(cs.ty());
    v.valueIsKnown = cs.isknown();
    if( v.valueIsKnown ) {
        v.setValue<uint64_t>(cs.val());
    }

    return v;
}

void serializeConstant(ConstantValue *c, DModule::ConstantValue *cs) {

    DModule::ValTy  v = serializeValTy(c->valueType);
    cs->set_ty(v);
    cs->set_isknown(c->valueIsKnown);
    cs->set_width(c->width);

    if( c->valueIsKnown ) {
        uint64_t    v = c->getValue<uint64_t>();
        cs->set_val(v);
    }

    return;
}

DModule::Ops serializeOp(Op::Ops    o) {
    DModule::Ops    k = DModule::UNSUP;
        switch(o) {
            case Op::Add:
                k = DModule::Add;
                break;
            case Op::Sub:
                k = DModule::Sub;
                break;
            case Op::Mul:
                k = DModule::Mul;
                break;
            case Op::MulU:
                k = DModule::MulU;
                break;
            case Op::MulS:
                k = DModule::MulS;
                break;
            case Op::Or:
                k = DModule::Or;
                break;
            case Op::And:
                k = DModule::And;
                break;
            case Op::Xor:
                k = DModule::Xor;
                break;
            case Op::Shl:
                k = DModule::Shl;
                break;
            case Op::Shr:
                k = DModule::Shr;
                break;
            case Op::Sar:
                k = DModule::Sar;
                break;
            case Op::CmpEQ:
                k = DModule::CmpEQ;
                break;
            case Op::CmpNE:
                k = DModule::CmpNE;
                break;
            case Op::CmpLTS:
                k = DModule::CmpLTS;
                break;
            case Op::CmpLTU:
                k = DModule::CmpLTU;
                break;
            case Op::CmpLES:
                k = DModule::CmpLES;
                break;
            case Op::CmpLEU:
                k = DModule::CmpLEU;
                break;
            case Op::Not:
                k = DModule::Not;
                break;
            case Op::CF32toF64:
                k = DModule::CF32toF64;
                break;
            case Op::C64to8:
                k = DModule::C64to8;
                break;
            case Op::C32to8:
                k = DModule::C32to8;
                break;
            case Op::C64to16:
                k = DModule::C64to16;
                break;
            case Op::C64LOto32:
                k = DModule::C64LOto32;
                break;
            case Op::C64HIto32:
                k = DModule::C64HIto32;
                break;
            case Op::C32LOto16:
                k = DModule::C32LOto16;
                break;
            case Op::C32HIto16:
                k = DModule::C32HIto16;
                break;
            case Op::C16LOto8:
                k = DModule::C16LOto8;
                break;
            case Op::C16HIto8:
                k = DModule::C16HIto8;
                break;
            case Op::C16HLto32:
                k = DModule::C16HLto32;
                break;
            case Op::C1Uto32:
                k = DModule::C1Uto32;
                break;
            case Op::C1Uto8:
                k = DModule::C1Uto8;
                break;
            case Op::C8Uto32:
                k = DModule::C8Uto32;
                break;
            case Op::C8Sto32:
                k = DModule::C8Sto32;
                break;
            case Op::C8Uto16:
                k = DModule::C8Uto16;
                break;
            case Op::C8Sto16:
                k = DModule::C8Sto16;
                break;
            case Op::C8Uto64:
                k = DModule::C8Uto64;
                break;
            case Op::C16Uto64:
                k = DModule::C16Uto64;
                break;
            case Op::C16Uto32:
                k = DModule::C16Uto32;
                break;
            case Op::C16Sto32:
                k = DModule::C16Sto32;
                break;
            case Op::C32Uto64:
                k = DModule::C32Uto64;
                break;
            case Op::C32Sto64:
                k = DModule::C32Sto64;
                break;
            case Op::C32HLto64:
                k = DModule::C32HLto64;
                break;
            case Op::C32to1:
                k = DModule::C32to1;
                break;
            case Op::C64to1:
                k = DModule::C64to1;
                break;
            case Op::DivModS64to32:
                k = DModule::DivModS64to32;
                break;
            case Op::DivModU64to32:
                k = DModule::DivModU64to32;
                break;
            case Op::Sad8Ux4:
                k = DModule::Sad8Ux4;
                break;
            case Op::Add8x8:
                k = DModule::Add8x8;
                break;
            case Op::Add16x4:
                k = DModule::Add16x4;
                break;
            case Op::Add32x2:
                k = DModule::Add32x2;
                break;
            case Op::Add64x1:
                k = DModule::Add64x1;
                break;
            case Op::QAdd8Sx8:
                k = DModule::QAdd8Sx8;
                break;
            case Op::QAdd16Sx4:
                k = DModule::QAdd16Sx4;
                break;
            case Op::QAdd32Sx2:
                k = DModule::QAdd32Sx2;
                break;
            case Op::QAdd64Sx1:
                k = DModule::QAdd64Sx1;
                break;
            case Op::QAdd8Ux8:
                k = DModule::QAdd8Ux8;
                break;
            case Op::QAdd16Ux4:
                k = DModule::QAdd16Ux4;
                break;
            case Op::QAdd32Ux2:
                k = DModule::QAdd32Ux2;
                break;
            case Op::QAdd64Ux1:
                k = DModule::QAdd64Ux1;
                break;
            case Op::Sub8x8:
                k = DModule::Sub8x8;
                break;
            case Op::Sub16x4:
                k = DModule::Sub16x4;
                break;
            case Op::Sub32x2:
                k = DModule::Sub32x2;
                break;
            case Op::QSub8Sx8:
                k = DModule::QSub8Sx8;
                break;
            case Op::QSub16Sx4:
                k = DModule::QSub16Sx4;
                break;
            case Op::QSub32Sx2:
                k = DModule::QSub32Sx2;
                break;
            case Op::QSub64Sx1:
                k = DModule::QSub64Sx1;
                break;
            case Op::QSub8Ux8:
                k = DModule::QSub8Ux8;
                break;
            case Op::QSub16Ux4:
                k = DModule::QSub16Ux4;
                break;
            case Op::QSub32Ux2:
                k = DModule::QSub32Ux2;
                break;
            case Op::QSub64Ux1:
                k = DModule::QSub64Ux1;
                break;
            case Op::CmpEQ8x8:
                k = DModule::CmpEQ8x8;
                break;
            case Op::CmpEQ16x4:
                k = DModule::CmpEQ16x4;
                break;
            case Op::CmpEQ32x2:
                k = DModule::CmpEQ32x2;
                break;
            case Op::CmpGT8Ux8:
                k = DModule::CmpGT8Ux8;
                break;
            case Op::CmpGT16Ux4:
                k = DModule::CmpGT16Ux4;
                break;
            case Op::CmpGT32Ux2:
                k = DModule::CmpGT32Ux2;
                break;
            case Op::CmpGT8Sx8:
                k = DModule::CmpGT8Sx8;
                break;
            case Op::CmpGT16Sx4:
                k = DModule::CmpGT16Sx4;
                break;
            case Op::CmpGT32Sx2:
                k = DModule::CmpGT32Sx2;
                break;
            case Op::ShlN8x8:
                k = DModule::ShlN8x8;
                break;
            case Op::ShlN16x4:
                k = DModule::ShlN16x4;
                break;
            case Op::ShlN32x2:
                k = DModule::ShlN32x2;
                break;
            case Op::ShrN8x8:
                k = DModule::ShrN8x8;
                break;
            case Op::ShrN16x4:
                k = DModule::ShrN16x4;
                break;
            case Op::ShrN32x2:
                k = DModule::ShrN32x2;
                break;
            case Op::SarN8x8:
                k = DModule::SarN8x8;
                break;
            case Op::SarN16x4:
                k = DModule::SarN16x4;
                break;
            case Op::SarN32x2:
                k = DModule::SarN32x2;
                break;
            case Op::Mul8x8:
                k = DModule::Mul8x8;
                break;
            case Op::Mul16x4:
                k = DModule::Mul16x4;
                break;
            case Op::Mul32x2:
                k = DModule::Mul32x2;
                break;
            case Op::Mul32Fx2:
                k = DModule::Mul32Fx2;
                break;
            case Op::MulHi16Ux4:
                k = DModule::MulHi16Ux4;
                break;
            case Op::MulHi16Sx4:
                k = DModule::MulHi16Sx4;
                break;
            case Op::PolyMul8x8:
                k = DModule::PolyMul8x8;
                break;
            case Op::InterleaveHI8x8:
                k = DModule::InterleaveHI8x8;
                break;
            case Op::InterleaveHI16x4:
                k = DModule::InterleaveHI16x4;
                break;
            case Op::InterleaveHI32x2:
                k = DModule::InterleaveHI32x2;
                break;
            case Op::InterleaveLO8x8:
                k = DModule::InterleaveLO8x8;
                break;
            case Op::InterleaveLO16x4:
                k = DModule::InterleaveLO16x4;
                break;
            case Op::InterleaveLO32x2:
                k = DModule::InterleaveLO32x2;
                break;
            case Op::InterleaveOddLanes8x8:
                k = DModule::InterleaveOddLanes8x8;
                break;
            case Op::InterleaveEvenLanes8x8:
                k = DModule::InterleaveEvenLanes8x8;
                break;
            case Op::InterleaveOddLanes16x4:
                k = DModule::InterleaveOddLanes16x4;
                break;
            case Op::InterleaveEvenLanes16x4:
                k = DModule::InterleaveEvenLanes16x4;
                break;
            case Op::Abs8x16:
                k = DModule::Abs8x16;
                break;
            case Op::Abs16x8:
                k = DModule::Abs16x8;
                break;
            case Op::Abs32x4:
                k = DModule::Abs32x4;
                break;
            case Op::Avg8Ux16:
                k = DModule::Avg8Ux16;
                break;
            case Op::Avg16Ux8:
                k = DModule::Avg16Ux8;
                break;
            case Op::Avg32Ux4:
                k = DModule::Avg32Ux4;
                break;
            case Op::Avg8Sx16:
                k = DModule::Avg8Sx16;
                break;
            case Op::Avg16Sx8:
                k = DModule::Avg16Sx8;
                break;
            case Op::Avg32Sx4:
                k = DModule::Avg32Sx4;
                break;
            case Op::Max8Sx16:
                k = DModule::Max8Sx16;
                break;
            case Op::Max16Sx8:
                k = DModule::Max16Sx8;
                break;
            case Op::Max32Sx4:
                k = DModule::Max32Sx4;
                break;
            case Op::Max8Ux16:
                k = DModule::Max8Ux16;
                break;
            case Op::Max16Ux8:
                k = DModule::Max16Ux8;
                break;
            case Op::Max32Ux4:
                k = DModule::Max32Ux4;
                break;
            case Op::Min8Sx16:
                k = DModule::Min8Sx16;
                break;
            case Op::Min16Sx8:
                k = DModule::Min16Sx8;
                break;
            case Op::Min32Sx4:
                k = DModule::Min32Sx4;
                break;
            case Op::Min8Ux16:
                k = DModule::Min8Ux16;
                break;
            case Op::Min16Ux8:
                k = DModule::Min16Ux8;
                break;
            case Op::Min32Ux4:
                k = DModule::Min32Ux4;
                break;
            case Op::Min8Ux8:
                k = DModule::Min8Ux8;
                break;
            case Op::Min16Ux4:
                k = DModule::Min16Ux4;
                break;
            case Op::Min32Ux2:
                k = DModule::Min32Ux2;
                break;
            case Op::Max8Sx8:
                k = DModule::Max8Sx8;
                break;
            case Op::Max16Sx4:
                k = DModule::Max16Sx4;
                break;
            case Op::Max32Sx2:
                k = DModule::Max32Sx2;
                break;
            case Op::Max8Ux8:
                k = DModule::Max8Ux8;
                break;
            case Op::Max16Ux4:
                k = DModule::Max16Ux4;
                break;
            case Op::Max32Ux2:
                k = DModule::Max32Ux2;
                break;
            case Op::Min8Sx8:
                k = DModule::Min8Sx8;
                break;
            case Op::Min16Sx4:
                k = DModule::Min16Sx4;
                break;
            case Op::Min32Sx2:
                k = DModule::Min32Sx2;
                break;
            case Op::QNarrow16Ux4:
                k = DModule::QNarrow16Ux4;
                break;
            case Op::QNarrow16Sx4:
                k = DModule::QNarrow16Sx4;
                break;
            case Op::QNarrow32Sx2:
                k = DModule::QNarrow32Sx2;
                break;
            case Op::UNSUP:
                k = DModule::UNSUP;
                break;
            default:
                assert(!"not possible");
        }

    return k;
}

Op::Ops deserializeOp(DModule::Ops  o) {
    Op::Ops k = Op::UNSUP;
    switch(o) {
        case DModule::Add:
            k = Op::Add;
            break;
        case DModule::Sub:
            k = Op::Sub;
            break;
        case DModule::Mul:
            k = Op::Mul;
            break;
        case DModule::MulU:
            k = Op::MulU;
            break;
        case DModule::MulS:
            k = Op::MulS;
            break;
        case DModule::Or:
            k = Op::Or;
            break;
        case DModule::And:
            k = Op::And;
            break;
        case DModule::Xor:
            k = Op::Xor;
            break;
        case DModule::Shl:
            k = Op::Shl;
            break;
        case DModule::Shr:
            k = Op::Shr;
            break;
        case DModule::Sar:
            k = Op::Sar;
            break;
        case DModule::CmpEQ:
            k = Op::CmpEQ;
            break;
        case DModule::CmpNE:
            k = Op::CmpNE;
            break;
        case DModule::CmpLTS:
            k = Op::CmpLTS;
            break;
        case DModule::CmpLTU:
            k = Op::CmpLTU;
            break;
        case DModule::CmpLES:
            k = Op::CmpLES;
            break;
        case DModule::CmpLEU:
            k = Op::CmpLEU;
            break;
        case DModule::Not:
            k = Op::Not;
            break;
        case DModule::CF32toF64:
            k = Op::CF32toF64;
            break;
        case DModule::C64to8:
            k = Op::C64to8;
            break;
        case DModule::C32to8:
            k = Op::C32to8;
            break;
        case DModule::C64to16:
            k = Op::C64to16;
            break;
        case DModule::C64LOto32:
            k = Op::C64LOto32;
            break;
        case DModule::C64HIto32:
            k = Op::C64HIto32;
            break;
        case DModule::C32LOto16:
            k = Op::C32LOto16;
            break;
        case DModule::C32HIto16:
            k = Op::C32HIto16;
            break;
        case DModule::C16LOto8:
            k = Op::C16LOto8;
            break;
        case DModule::C16HIto8:
            k = Op::C16HIto8;
            break;
        case DModule::C16HLto32:
            k = Op::C16HLto32;
            break;
        case DModule::C1Uto32:
            k = Op::C1Uto32;
            break;
        case DModule::C1Uto8:
            k = Op::C1Uto8;
            break;
        case DModule::C8Uto32:
            k = Op::C8Uto32;
            break;
        case DModule::C8Sto32:
            k = Op::C8Sto32;
            break;
        case DModule::C8Uto16:
            k = Op::C8Uto16;
            break;
        case DModule::C8Sto16:
            k = Op::C8Sto16;
            break;
        case DModule::C8Uto64:
            k = Op::C8Uto64;
            break;
        case DModule::C16Uto64:
            k = Op::C16Uto64;
            break;
        case DModule::C16Uto32:
            k = Op::C16Uto32;
            break;
        case DModule::C16Sto32:
            k = Op::C16Sto32;
            break;
        case DModule::C32Uto64:
            k = Op::C32Uto64;
            break;
        case DModule::C32Sto64:
            k = Op::C32Sto64;
            break;
        case DModule::C32HLto64:
            k = Op::C32HLto64;
            break;
        case DModule::C32to1:
            k = Op::C32to1;
            break;
        case DModule::C64to1:
            k = Op::C64to1;
            break;
        case DModule::DivModS64to32:
            k = Op::DivModS64to32;
            break;
        case DModule::DivModU64to32:
            k = Op::DivModU64to32;
            break;
        case DModule::Sad8Ux4:
            k = Op::Sad8Ux4;
            break;
        case DModule::Add8x8:
            k = Op::Add8x8;
            break;
        case DModule::Add16x4:
            k = Op::Add16x4;
            break;
        case DModule::Add32x2:
            k = Op::Add32x2;
            break;
        case DModule::Add64x1:
            k = Op::Add64x1;
            break;
        case DModule::QAdd8Sx8:
            k = Op::QAdd8Sx8;
            break;
        case DModule::QAdd16Sx4:
            k = Op::QAdd16Sx4;
            break;
        case DModule::QAdd32Sx2:
            k = Op::QAdd32Sx2;
            break;
        case DModule::QAdd64Sx1:
            k = Op::QAdd64Sx1;
            break;
        case DModule::QAdd8Ux8:
            k = Op::QAdd8Ux8;
            break;
        case DModule::QAdd16Ux4:
            k = Op::QAdd16Ux4;
            break;
        case DModule::QAdd32Ux2:
            k = Op::QAdd32Ux2;
            break;
        case DModule::QAdd64Ux1:
            k = Op::QAdd64Ux1;
            break;
        case DModule::Sub8x8:
            k = Op::Sub8x8;
            break;
        case DModule::Sub16x4:
            k = Op::Sub16x4;
            break;
        case DModule::Sub32x2:
            k = Op::Sub32x2;
            break;
        case DModule::QSub8Sx8:
            k = Op::QSub8Sx8;
            break;
        case DModule::QSub16Sx4:
            k = Op::QSub16Sx4;
            break;
        case DModule::QSub32Sx2:
            k = Op::QSub32Sx2;
            break;
        case DModule::QSub64Sx1:
            k = Op::QSub64Sx1;
            break;
        case DModule::QSub8Ux8:
            k = Op::QSub8Ux8;
            break;
        case DModule::QSub16Ux4:
            k = Op::QSub16Ux4;
            break;
        case DModule::QSub32Ux2:
            k = Op::QSub32Ux2;
            break;
        case DModule::QSub64Ux1:
            k = Op::QSub64Ux1;
            break;
        case DModule::CmpEQ8x8:
            k = Op::CmpEQ8x8;
            break;
        case DModule::CmpEQ16x4:
            k = Op::CmpEQ16x4;
            break;
        case DModule::CmpEQ32x2:
            k = Op::CmpEQ32x2;
            break;
        case DModule::CmpGT8Ux8:
            k = Op::CmpGT8Ux8;
            break;
        case DModule::CmpGT16Ux4:
            k = Op::CmpGT16Ux4;
            break;
        case DModule::CmpGT32Ux2:
            k = Op::CmpGT32Ux2;
            break;
        case DModule::CmpGT8Sx8:
            k = Op::CmpGT8Sx8;
            break;
        case DModule::CmpGT16Sx4:
            k = Op::CmpGT16Sx4;
            break;
        case DModule::CmpGT32Sx2:
            k = Op::CmpGT32Sx2;
            break;
        case DModule::ShlN8x8:
            k = Op::ShlN8x8;
            break;
        case DModule::ShlN16x4:
            k = Op::ShlN16x4;
            break;
        case DModule::ShlN32x2:
            k = Op::ShlN32x2;
            break;
        case DModule::ShrN8x8:
            k = Op::ShrN8x8;
            break;
        case DModule::ShrN16x4:
            k = Op::ShrN16x4;
            break;
        case DModule::ShrN32x2:
            k = Op::ShrN32x2;
            break;
        case DModule::SarN8x8:
            k = Op::SarN8x8;
            break;
        case DModule::SarN16x4:
            k = Op::SarN16x4;
            break;
        case DModule::SarN32x2:
            k = Op::SarN32x2;
            break;
        case DModule::Mul8x8:
            k = Op::Mul8x8;
            break;
        case DModule::Mul16x4:
            k = Op::Mul16x4;
            break;
        case DModule::Mul32x2:
            k = Op::Mul32x2;
            break;
        case DModule::Mul32Fx2:
            k = Op::Mul32Fx2;
            break;
        case DModule::MulHi16Ux4:
            k = Op::MulHi16Ux4;
            break;
        case DModule::MulHi16Sx4:
            k = Op::MulHi16Sx4;
            break;
        case DModule::PolyMul8x8:
            k = Op::PolyMul8x8;
            break;
        case DModule::InterleaveHI8x8:
            k = Op::InterleaveHI8x8;
            break;
        case DModule::InterleaveHI16x4:
            k = Op::InterleaveHI16x4;
            break;
        case DModule::InterleaveHI32x2:
            k = Op::InterleaveHI32x2;
            break;
        case DModule::InterleaveLO8x8:
            k = Op::InterleaveLO8x8;
            break;
        case DModule::InterleaveLO16x4:
            k = Op::InterleaveLO16x4;
            break;
        case DModule::InterleaveLO32x2:
            k = Op::InterleaveLO32x2;
            break;
        case DModule::InterleaveOddLanes8x8:
            k = Op::InterleaveOddLanes8x8;
            break;
        case DModule::InterleaveEvenLanes8x8:
            k = Op::InterleaveEvenLanes8x8;
            break;
        case DModule::InterleaveOddLanes16x4:
            k = Op::InterleaveOddLanes16x4;
            break;
        case DModule::InterleaveEvenLanes16x4:
            k = Op::InterleaveEvenLanes16x4;
            break;
        case DModule::Abs8x16:
            k = Op::Abs8x16;
            break;
        case DModule::Abs16x8:
            k = Op::Abs16x8;
            break;
        case DModule::Abs32x4:
            k = Op::Abs32x4;
            break;
        case DModule::Avg8Ux16:
            k = Op::Avg8Ux16;
            break;
        case DModule::Avg16Ux8:
            k = Op::Avg16Ux8;
            break;
        case DModule::Avg32Ux4:
            k = Op::Avg32Ux4;
            break;
        case DModule::Avg8Sx16:
            k = Op::Avg8Sx16;
            break;
        case DModule::Avg16Sx8:
            k = Op::Avg16Sx8;
            break;
        case DModule::Avg32Sx4:
            k = Op::Avg32Sx4;
            break;
        case DModule::Max8Sx16:
            k = Op::Max8Sx16;
            break;
        case DModule::Max16Sx8:
            k = Op::Max16Sx8;
            break;
        case DModule::Max32Sx4:
            k = Op::Max32Sx4;
            break;
        case DModule::Max8Ux16:
            k = Op::Max8Ux16;
            break;
        case DModule::Max16Ux8:
            k = Op::Max16Ux8;
            break;
        case DModule::Max32Ux4:
            k = Op::Max32Ux4;
            break;
        case DModule::Min8Sx16:
            k = Op::Min8Sx16;
            break;
        case DModule::Min16Sx8:
            k = Op::Min16Sx8;
            break;
        case DModule::Min32Sx4:
            k = Op::Min32Sx4;
            break;
        case DModule::Min8Ux16:
            k = Op::Min8Ux16;
            break;
        case DModule::Min16Ux8:
            k = Op::Min16Ux8;
            break;
        case DModule::Min32Ux4:
            k = Op::Min32Ux4;
            break;
        case DModule::Min8Ux8:
            k = Op::Min8Ux8;
            break;
        case DModule::Min16Ux4:
            k = Op::Min16Ux4;
            break;
        case DModule::Min32Ux2:
            k = Op::Min32Ux2;
            break;
        case DModule::Max8Sx8:
            k = Op::Max8Sx8;
            break;
        case DModule::Max16Sx4:
            k = Op::Max16Sx4;
            break;
        case DModule::Max32Sx2:
            k = Op::Max32Sx2;
            break;
        case DModule::Max8Ux8:
            k = Op::Max8Ux8;
            break;
        case DModule::Max16Ux4:
            k = Op::Max16Ux4;
            break;
        case DModule::Max32Ux2:
            k = Op::Max32Ux2;
            break;
        case DModule::Min8Sx8:
            k = Op::Min8Sx8;
            break;
        case DModule::Min16Sx4:
            k = Op::Min16Sx4;
            break;
        case DModule::Min32Sx2:
            k = Op::Min32Sx2;
            break;
        case DModule::QNarrow16Ux4:
            k = Op::QNarrow16Ux4;
            break;
        case DModule::QNarrow16Sx4:
            k = Op::QNarrow16Sx4;
            break;
        case DModule::QNarrow32Sx2:
            k = Op::QNarrow32Sx2;
            break;
        case DModule::UNSUP:
            k = Op::UNSUP;
            break;
        default:
            assert(!"not possible");
    }
    return k;
}

void serializeExpression(ExpressionPtr e, DModule::Expression *se) {
    /* what kind of expression is it? */
    if( ExConstPtr cst = dynamic_pointer_cast<ExConst>(e) ) {
        se->set_ty(DModule::Const);
        
        ConstantValue   t = cst->getVal();
        serializeConstant(&t, se->mutable_const_cval()); 

    } else if( ExGetPtr get = dynamic_pointer_cast<ExGet>(e) ) {
        se->set_ty(DModule::Get); 
       
        if( ExpressionPtr v = get->getVarPart() ) {
            RegArray    rarr = get->getRegArray();
            TargetArch  a = get->getArch();
            serializeExpression(v, se->mutable_get_varpart());
            serializeRegArray(&rarr, se->mutable_get_regarray());
            se->set_get_bias(get->getBias());
            TargetArchToTArch(&a, se->mutable_get_arch());
        } else {
            serializeRegister(  get->getSrcReg(), 
                                se->mutable_get_register(),
                                get->getOff());
            se->set_get_offset(get->getOff());
        }
    
    } else if( ExRdTmpPtr rdtmp = dynamic_pointer_cast<ExRdTmp>(e) ) {
        se->set_ty(DModule::RdTmp);
        TempValToTVar(rdtmp->getTmp(), se->mutable_rdtmp_tval());
        
    } else if( ExOpPtr op = dynamic_pointer_cast<ExOp>(e) ) {
        vector<ExpressionPtr>   args;
        se->set_ty(DModule::Op);

        args = op->getArgs();
        for(vector<ExpressionPtr>::iterator it = args.begin();
            it != args.end();
            ++it)
        {
            serializeExpression(*it, se->add_op_arguments());
        }
        
        se->set_op_opcode(serializeOp(op->getOp()->getOp()));

    } else if( ExLoadPtr load = dynamic_pointer_cast<ExLoad>(e) ) { 
        se->set_ty(DModule::Load);

        serializeExpression(load->getAddr(), se->mutable_load_addr());
        se->set_load_loadty(serializeValTy(load->getTy()));

    } else if( ExMux0XPtr mux = dynamic_pointer_cast<ExMux0X>(e) ) {
        se->set_ty(DModule::Mux0X);

        serializeExpression(mux->getCondition(), se->mutable_mux0x_condition());
        serializeExpression(mux->getTrue(), se->mutable_mux0x_iftrue());
        serializeExpression(mux->getFalse(), se->mutable_mux0x_iffalse());

    } else if( ExCCallPtr ccall = dynamic_pointer_cast<ExCCall>(e) ) { 
        vector<ExpressionPtr>   args;
        se->set_ty(DModule::CCall);

        se->set_ccall_targetfunc(ccall->getTarget());
        args = ccall->getArgs();
        for(vector<ExpressionPtr>::iterator it = args.begin();
            it != args.end();
            ++it)
        {
            serializeExpression(*it, se->add_ccall_args());
        }
    } else {
        //cout << dynamic_pointer_cast<Expression>(e)->printExpr() << endl;
        assert(!"Unsupported serialization type");
    }

    return;
}

ExpressionPtr deserializeExpression(const DModule::Expression &es) {
    ExpressionPtr           e;
    vector<ExpressionPtr>   args;

    switch(es.ty()) {
        case DModule::Const:
            assert(es.has_const_cval());
            e = ExpressionPtr(new ExConst(
                    deserializeConstant(es.const_cval())));
            break;

        case DModule::RdTmp:
            assert(es.has_rdtmp_tval());
            e = ExpressionPtr(new ExRdTmp(TVarToTempVal(es.rdtmp_tval())));
            break;
        
        case DModule::Op:
            assert(es.op_arguments_size() > 0);
            assert(es.has_op_opcode());
            for(int i = 0; i < es.op_arguments_size(); i++) {
                args.push_back(deserializeExpression(es.op_arguments(i)));
            }
            e = ExpressionPtr(new ExOp(
                OpPtr(new Op(deserializeOp(es.op_opcode()))), args));
            break;

        case DModule::Load:
            assert(es.has_load_loadty());
            assert(es.has_load_addr());
            e = ExpressionPtr(new ExLoad(
                    deserializeExpression(es.load_addr()),
                    deserializeValTy(es.load_loadty())));
            break;

        case DModule::Mux0X:
            assert(es.has_mux0x_condition());
            assert(es.has_mux0x_iftrue());
            assert(es.has_mux0x_iffalse());
            e = ExpressionPtr(new ExMux0X(
                    deserializeExpression(es.mux0x_condition()),
                    deserializeExpression(es.mux0x_iftrue()),
                    deserializeExpression(es.mux0x_iffalse())));
            break;

        case DModule::CCall:
            assert(es.has_ccall_targetfunc());
            assert(es.ccall_args_size() > 0);

            for( int i = 0; i < es.ccall_args_size(); i++ ) {
                args.push_back(deserializeExpression(es.ccall_args(i)));
            }

            e = ExpressionPtr(new ExCCall(args, es.ccall_targetfunc()));
            break;

        case DModule::Get:
            if( es.has_get_regarray()) {
                assert(es.has_get_varpart());
                assert(es.has_get_bias());
                assert(es.has_get_arch());

                e = ExpressionPtr(new ExGet(
                    deserializeRegArray(es.get_regarray()),
                    deserializeExpression(es.get_varpart()),
                    es.get_bias(),
                    TArchToTargetArch(es.get_arch())));
            } else {
                assert(es.has_get_register());
                Register    r = deserializeRegister(es.get_register());
                ExGet *tmp  = new ExGet(r);
                if( r.width == 0 ) {
                    tmp->setOff(es.get_offset());
                }

                e = ExpressionPtr(tmp);
            }
            break;
        
        default:
            assert(!"not possible deserialization");
    }

    return e;
}

void serializeStatement(StatementPtr s, DModule::Statement *ss) {
    
    if( StIMarkPtr imark = dynamic_pointer_cast<StIMark>(s) ) {
        ss->set_ty(DModule::IMark);

    } else if( StWrTmpPtr wrtmp = dynamic_pointer_cast<StWrTmp>(s) ) { 
        ss->set_ty(DModule::WrTmp);
        TempValToTVar(wrtmp->getTarget(), ss->mutable_wrtmp_tmpwritten());    
        serializeExpression(wrtmp->getRHS(), ss->mutable_wrtmp_rhs());

    } else if( StPutPtr put = dynamic_pointer_cast<StPut>(s) ) { 
        ss->set_ty(DModule::Put);

        serializeExpression(put->getData(), ss->mutable_put_data());

        if( ExpressionPtr v = put->getVarPart() ) {
            RegArray    rarr = put->getRegArray();
            serializeRegArray(&rarr, ss->mutable_put_regarray());
            ss->set_put_bias(put->getBias());
            serializeExpression(put->getVarPart(), ss->mutable_put_varpart());

        } else {
            serializeRegister(  put->getDstRegister(),
                                ss->mutable_put_register(),
                                put->getGuestOff());
        }

    } else if( StStorePtr store = dynamic_pointer_cast<StStore>(s) ) { 
        ss->set_ty(DModule::Store);
        serializeExpression(store->getData(), ss->mutable_store_data());
        serializeExpression(store->getAddr(), ss->mutable_store_addr());

    } else if( StNopPtr nop = dynamic_pointer_cast<StNop>(s) ) { 
        ss->set_ty(DModule::NOP);

    } else if( StAbiHintPtr abihint = dynamic_pointer_cast<StAbiHint>(s) ) { 
        ss->set_ty(DModule::AbiHint);

    } else if( StCASPtr cas = dynamic_pointer_cast<StCAS>(s) ) { 
        ss->set_ty(DModule::CAS);

        serializeExpression(cas->getStoreAddress(),ss->mutable_cas_storeaddr());
        if( ExpressionPtr e = cas->getDataHi() ) {
            serializeExpression(e, ss->mutable_cas_datahi());
        }

        serializeExpression(cas->getDataLo(), ss->mutable_cas_datalo());

        if( ExpressionPtr e = cas->getExpectedHi() ) {
            serializeExpression(e, ss->mutable_cas_expectedhi());
        }

        serializeExpression(cas->getExpectedLo(), ss->mutable_cas_expectedlo());

        if( TempValPtr t = cas->getOldHi() ) {
            TempValToTVar(t, ss->mutable_cas_oldhi());
        }

        TempValToTVar(cas->getOldLo(), ss->mutable_cas_oldlo());
        
        ss->set_cas_end(serializeEnd(cas->getEndian()));
    } else if( StLLSCPtr llsc = dynamic_pointer_cast<StLLSC>(s) ) { 
        ss->set_ty(DModule::LLSC);
    } else if( StDirtyPtr dirty = dynamic_pointer_cast<StDirty>(s) ) { 
        ss->set_ty(DModule::Dirty);
        
        if( ExpressionPtr addr = dirty->getAddr() ) {
            serializeExpression(addr, ss->mutable_dirty_addr());
        }

        if( ExpressionPtr guard = dirty->getGuard() ) {
            serializeExpression(guard, ss->mutable_dirty_guard());
        }

        if( TempValPtr  t = dirty->getTmp() ) {
            TempValToTVar(t, ss->mutable_dirty_tmp());
        }

        vector<ExpressionPtr>   args = dirty->getArgs();
        //cout << dirty->printStmt() << endl;
        for(vector<ExpressionPtr>::iterator it = args.begin(); 
            it != args.end();
            ++it)
        {
            //cout << "dirty expr " << (*it)->printExpr() << endl;
            if(*it != NULL) {
                serializeExpression(*it, ss->add_dirty_args());
            }
        }

        ss->set_dirty_calleename(dirty->getTarget());
    } else if( StMBEPtr mbe = dynamic_pointer_cast<StMBE>(s) ) {
        ss->set_ty(DModule::MBE);
    } else if( StExitPtr exit = dynamic_pointer_cast<StExit>(s) ) {
        ss->set_ty(DModule::Exit);
        ss->set_exit_blockexit(serializeExitType(exit->getJmpKind()));
        serializeExpression(exit->getCondition(), ss->mutable_exit_guardexp());
        serializeExpression(exit->getTarget(), ss->mutable_exit_jmptarget());
    } else {
        assert(!"Unsupported serialization type");
    }

    return;
}

StatementPtr deserializeStatement(const DModule::Statement &ss) {
    StatementPtr            s;
    ExpressionPtr           expectedHi;
    ExpressionPtr           dataHi;
    ExpressionPtr           addr;
    ExpressionPtr           n;
    ExConstPtr              tgt;
    TempValPtr              tmp;
    ExpressionPtr           guard;
    TempValPtr              oldHi;
    vector<ExpressionPtr>   args;

    switch(ss.ty()) {
        case DModule::WrTmp:
            assert(ss.has_wrtmp_rhs());
            assert(ss.has_wrtmp_tmpwritten());
            s = StatementPtr(new StWrTmp(
                                    TVarToTempVal(ss.wrtmp_tmpwritten()),
                                    deserializeExpression(ss.wrtmp_rhs())));
            break;

        case DModule::Store:
            assert(ss.has_store_data());
            assert(ss.has_store_addr());
            s = StatementPtr(new StStore(
                                    deserializeExpression(ss.store_addr()),
                                    deserializeExpression(ss.store_data())));
            break;

        case DModule::CAS:
            assert(ss.has_cas_storeaddr());
            assert(ss.has_cas_datalo());
            assert(ss.has_cas_expectedlo());
            assert(ss.has_cas_oldlo());
            assert(ss.has_cas_end());

            if(ss.has_cas_expectedhi()) {
                expectedHi = deserializeExpression(ss.cas_expectedhi());
            }

            if(ss.has_cas_oldhi()) {
                oldHi = TVarToTempVal(ss.cas_oldhi());
            }

            if(ss.has_cas_datahi()) {
                dataHi = deserializeExpression(ss.cas_datahi());
            }

            s = StatementPtr(new StCAS(
                                    deserializeExpression(ss.cas_storeaddr()),
                                    dataHi,
                                    deserializeExpression(ss.cas_datalo()),
                                    expectedHi,
                                    deserializeExpression(ss.cas_expectedlo()),
                                    oldHi,
                                    TVarToTempVal(ss.cas_oldlo()),
                                    deserializeEnd(ss.cas_end())));
            break;

        case DModule::Put:
            assert(ss.has_put_data());
            if(ss.has_put_register()) {
                s = StatementPtr(new StPut( 
                                    deserializeRegister(ss.put_register()),
                                    deserializeExpression(ss.put_data()))); 
            } else {
                /* this statement is a puti */
                assert(ss.has_put_regarray());
                assert(ss.has_put_bias());
                assert(ss.has_put_varpart());
                
                s = StatementPtr(new StPut(
                                        deserializeRegArray(ss.put_regarray()),
                                        deserializeExpression(ss.put_data()),
                                        deserializeExpression(ss.put_varpart()),
                                        ss.put_bias()));
            }
            break;

        case DModule::LLSC:
            assert(ss.has_llsc_addr());
            assert(ss.has_llsc_storedata());
            assert(ss.has_llsc_result());
            assert(ss.has_llsc_end());
            s = StatementPtr(new StLLSC(deserializeExpression(ss.llsc_addr()),
                                    deserializeExpression(ss.llsc_storedata()),
                                    TVarToTempVal(ss.llsc_result()),
                                    deserializeEnd(ss.llsc_end())));
            break;
        
        case DModule::Dirty:
            assert(ss.has_dirty_calleename());
            
            if(ss.has_dirty_guard()) {
                guard = deserializeExpression(ss.dirty_guard());
            }

            if(ss.has_dirty_tmp()) {
                tmp = TVarToTempVal(ss.dirty_tmp());
            }

            if( ss.has_dirty_addr() ) {
                addr = deserializeExpression(ss.dirty_addr());
            }

            for( int i = 0; i < ss.dirty_args_size(); i++ ) {
                args.push_back(deserializeExpression(ss.dirty_args(i)));
            }

            s = StatementPtr(new StDirty(   addr, 
                                            guard, 
                                            args, 
                                            ss.dirty_calleename(),
                                            tmp));

            break;

        case DModule::Exit:
            assert(ss.has_exit_blockexit());
            assert(ss.has_exit_guardexp());
            assert(ss.has_exit_jmptarget());

            n = deserializeExpression(ss.exit_jmptarget());
            tgt = dynamic_pointer_cast<ExConst>(n);
            assert(tgt);

            s = StatementPtr(new StExit(
                                deserializeExpression(ss.exit_guardexp()),
                                tgt,
                                deserializeExitType(ss.exit_blockexit())));
            break;

        case DModule::IMark:
            s = StatementPtr(new StIMark());
            break;

        case DModule::NOP:
            s = StatementPtr(new StNop());
            break;

        case DModule::AbiHint:
            s = StatementPtr(new StAbiHint());
            break;

        case DModule::MBE:
            s = StatementPtr(new StMBE());
            break;

        default:
            assert(!"unsupported deserialization");
    }

    return s;
}

void blockToDBlock(BlockPtr b, DModule::DBlock *d) {

    /* set the blocks entry address */
    d->set_baseaddr(b->getBlockBase());
    d->set_len(b->getBlockLen());

    /* set the block architecture */
    TargetArch  ta = b->getArch();
    TargetArchToTArch(&ta, d->mutable_arch());

    /* assign the blocks temporary values to the DBlock */
    for(vector<TempValPtr>::iterator it = b->begin_temps();
        it != b->end_temps();
        ++it)
    {
        TempValToTVar(*it, d->add_temps()); 
    }

    /* assign the blocks statements to the DBlock */
    for(vector<StatementPtr>::iterator  it = b->begin(); it != b->end(); ++it) {
        serializeStatement(*it, d->add_stmts());
    }

    d->set_blockexit(serializeExitType(b->getExitKind()));

    /* assign the exit expression to the DBlock */
    serializeExpression(b->getNext(), d->mutable_next());

    /* done */

    return;
}

void blockFromDBlock(BlockPtr &b, const DModule::DBlock &d) {
    /* get the information we need to construct the block initially */
    TargetArch  tarch = TArchToTargetArch(d.arch());

    b = BlockPtr(new VBlock(tarch, d.baseaddr(), d.len()));

    /* iterate over the temporary values and add them to the block */
    for( int i = 0; i < d.temps_size(); i++ ) {
        b->insertTempVal(TVarToTempVal(d.temps(i)));
    }

    /* and now iterate over the statements,
     * re-build them, and add them to the block
     */
    for( int i = 0; i < d.stmts_size(); i++ ) {
        StatementPtr s = deserializeStatement(d.stmts(i));
        assert(s);

        b->insertStmt(s);
    }

    b->setExitKind(deserializeExitType(d.blockexit()));

    /* add the exit expression to the block */
    b->setNext(deserializeExpression(d.next()));

    /* done, block is initialized */

    return;
}

ContextHandle *openFile(string modName) {
  DModule       *M = new DModule();
  ContextHandle *C = new ContextHandle();

  M->set_filename(modName);
  C->M = M;

  return C;
}

bool appendToFile(ContextHandle *F, BlockPtr B) {
  DModule *M = F->M;

  DModule::DBlock *b = M->add_blocks();
  blockToDBlock(B, b);

  return true;
}

bool writeOutFile(ContextHandle *F, string outPath) {
  filebuf fb;

  if( fb.open(outPath.c_str(), ios::out|ios::binary|ios::trunc) ) {
      ostream os(&fb);
      return F->M->SerializeToOstream(&os);
  } else {
    return false;
  }
}

bool writeToFile(string outPath, string modName, list<BlockPtr> blocks) {
    DModule m;
    bool    done=false;
    m.set_filename(modName);

    /* iterate over each block in blocks */
    for(list<BlockPtr>::iterator it = blocks.begin();
        it != blocks.end();
        ++it)
    {
        DModule::DBlock *b = m.add_blocks();
        blockToDBlock(*it, b);
    }

    /* write the module, as a string, to the file specified */
    filebuf fb;

    if( fb.open(outPath.c_str(), ios::out|ios::binary|ios::trunc) ) {
        ostream os(&fb);
        done = m.SerializeToOstream(&os);
    }

    return done;
}

bool readFromFile(string filePath, string &modName, list<BlockPtr> &blocks) {
    bool    done=false;
    DModule m;
    filebuf fb;

    if( fb.open(filePath.c_str(), ios::in|ios::binary) ) {
        istream is(&fb); 

        /* serialize the entire thing into a message */
        if( m.ParseFromIstream(&is) ) {
            /* set the module name */
            modName = m.filename();

            /* and now, iterate over every DBlock */
            for( int i = 0; i < m.blocks_size(); i++ ) {
                const DModule::DBlock   &c = m.blocks(i);
                BlockPtr                newBlock; 

                /* temporarily print out what we get back*/
                /*string                  outS;
                google::protobuf::TextFormat::PrintToString(c, &outS);
                cout << outS << endl;*/

                blockFromDBlock(newBlock, c);

                blocks.push_back(newBlock);
            }
            
            done = true;
        }
    }

    return done;
}
