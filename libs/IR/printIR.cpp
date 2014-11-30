#include "VexIR.h"

std::string regToStr(Register reg, TargetArch arch ) {
    switch(arch.ta) {
        case INVALID:
        case AMD64:
        case PPC32:
        case PPC64:
        case S390X:
            return "UNSUP";
            break;
        case ARM:
            switch(reg.RegArm) {
                case AR1:
                    return "R1";
                    break;
                case AR2:
                    return "R2";
                    break;
                case AR3:
                    return "R3";
                    break;
                case AR4:
                    return "R4";
                    break;
                case AR5:
                    return "R5";
                    break;
                case AR6:
                    return "R6";
                    break;
                case AR7:
                    return "R7";
                    break;
                case AR8:
                    return "R8";
                    break;
                case AR9:
                    return "R9";
                    break;
                case AR10:
                    return "R10";
                    break;
                case AR11:
                    return "R11";
                    break;
                case AR12:
                    return "R12";
                    break;
                case AR13:
                    return "R1";
                    break;
                case AR14:
                    return "R14";
                    break;
                case AR15:
                    return "R15";
                    break;
                case ARUnknown:
                    return "UNK";
                    break;
            }
            break;
		case X86:
			switch( reg.width ) {
				case 8:
					switch(reg.Reg8) {
						case AH:
							return "AH";
							break;
						case AL:
							return "AL";
							break;
						case BH:
							return "BH";
							break;
						case BL:
							return "BL";
							break;
						case CL:
							return "CL";
							break;
						case CH:
							return "CH";
							break;
						case DL:
							return "DL";
							break;
						case DH:
							return "DH";
							break;
						case ANY8:
							return "ANY8";
							break;
					}
					break;
				case 16:
					switch(reg.Reg16) {
						case AX:
							return "AX";
							break;
						case BX:
							return "BX";
							break;
						case CX:
							return "CX";
							break;
						case DX:
							return "DX";
							break;
						case BP:
							return "BP";
							break;
						case SP:
							return "SP";
							break;
						case DI:
							return "DI";
							break;
						case SI:
							return "SI";
							break;
						case ANY16:
							return "ANY16";
							break;
					}
					break;
				case 32:
					switch( reg.Reg32 ) {
						case EAX:
							return "EAX";
							break;
						case EBX:
							return "EBX";
							break;
						case ECX:
							return "ECX";
							break;
						case EDX:
							return "EDX";
							break;
						case ESI:
							return "ESI";
							break;
						case EDI:
							return "EDI";
							break;
						case EBP:
							return "EBP";
							break;
						case ESP:
							return "ESP";
							break;
						case EIP:
							return "EIP";
							break;
						case CS:
							return "CS";
							break;
						case DS:
							return "DS";
							break;
						case SS:
							return "SS";
							break;
                        case ES:
                            return "ES";
                            break;
						case FS:
							return "FS";
							break;
						case GS:
							return "GS";
							break;
                        case OP:
                            return "OP";
                            break;
                        case DEP1:
                            return "DEP1";
                            break;
                        case DEP2:
                            return "DEP2";
                            break;
                        case NDEP:
                            return "NDEP";
                            break;
                        case DFLAG:
                            return "DFLAG";
                            break;
                        case IDFLAG:
                            return "IDFLAG";
                            break;
                        case ACFLAG:
                            return "ACFLAG";
                            break;
                        case FTOP:
                            return "FTOP";
                            break;
                       	case ANY32:
                       		return "ANY32";
					}
					break;
                case 64:
                    switch(reg.Reg64) {
                        case GDT:
                            return "GDT";
                            break;
                        case LDT:
                            return "LDT";
                            break;
                    }
                    break;
			}
			break;
	}
	return "UNK";
}

std::string printEnd(MemoryEnd end) {
    std::string r = "";
    switch(end) { 
        case LittleEndian:
            r = r + "le";
            break;
        case BigEndian:
            r = r + "be";
            break;
    }

    return r;
}

std::string TempVal::printTemp() {
    if( this->varName.size() ) { 
        return this->varName;
    }

	std::string	r = "t";
	r.append(to_string<int>(this->varIndex, std::dec, 0));

	switch(this->cval.valueType){
        case ConstantValue::T_INVALID:
			break;
        case ConstantValue::T_I1:
			r.append(":I1");
			break;
        case ConstantValue::T_I8:
			r.append(":I8");
			break;
        case ConstantValue::T_I16:
			r.append(":I16");
			break;
        case ConstantValue::T_I32:
			r.append(":I32");
			break;
        case ConstantValue::T_I64:
			r.append(":I64");
			break;
        case ConstantValue::T_I128:
			r.append("I128");
			break;
        case ConstantValue::T_F32:
			r.append(":F32");
			break;
        case ConstantValue::T_F64:
			r.append(":F64");
			break;
        case ConstantValue::T_F128:
			r.append(":F128");
			break;
        case ConstantValue::T_V128:
			r.append(":V128");
			break;
	}

    if( this->cval.valueIsKnown ) {
        r.append("(");
        //print the value if we know it 
        switch(this->cval.valueType) {
            case ConstantValue::T_I1:
                r.append(to_string<unsigned short>(this->cval.U1, std::hex,0));
                break;
            case ConstantValue::T_I8:
                r.append(to_string<unsigned short>(this->cval.U8, std::hex,0));
                break;
            case ConstantValue::T_I16:
                r.append(to_string<unsigned short>(this->cval.U16, std::hex,0));
                break;
            case ConstantValue::T_I32:
                r.append(to_string<unsigned long>(this->cval.U32, std::hex,0));
                break;
            case ConstantValue::T_I64:
                r.append(to_string<unsigned long long>(this->cval.U64, std::hex,0));
                break;
            case ConstantValue::T_INVALID:
            case ConstantValue::T_F32:
            case ConstantValue::T_F64:
            case ConstantValue::T_V128:
            case ConstantValue::T_F128:
            case ConstantValue::T_I128:
                r.append("UNSUP");
                break;
        }
        r.append(")");
    }
	
	return r;
}

//////////////////////////////////////////////////////////////////////////
// Printers for Expression subclasses
//////////////////////////////////////////////////////////////////////////

std::string ExGet::printExpr(void) { 
    std::string	r="";
    if( this->varOffset ) {
        r.append("GETI[");
		r.append(to_string<boost::int32_t>(this->rArr.base, std::dec,0));
        r.append("x");
		r.append(to_string<boost::int32_t>(this->rArr.numElems, std::dec,0));
        switch(this->rArr.ty){
            case ConstantValue::T_INVALID:
                break;
            case ConstantValue::T_I1:
                r.append(":I1");
                break;
            case ConstantValue::T_I8:
                r.append(":I8");
                break;
            case ConstantValue::T_I16:
                r.append(":I16");
                break;
            case ConstantValue::T_I32:
                r.append(":I32");
                break;
            case ConstantValue::T_I64:
                r.append(":I64");
                break;
            case ConstantValue::T_I128:
                r.append("I128");
                break;
            case ConstantValue::T_F32:
                r.append(":F32");
                break;
            case ConstantValue::T_F64:
                r.append(":F64");
                break;
            case ConstantValue::T_F128:
                r.append(":F128");
                break;
            case ConstantValue::T_V128:
                r.append(":V128");
                break;
	    }
        r.append("]");
        r.append("(");
        r.append(this->varOffset->printExpr());
        r.append(",");
		r.append(to_string<boost::int32_t>(this->bias, std::dec,0));
        r.append(")");
    } else {
	    r.append("REGREAD(");
        std::string k = regToStr(this->sourceRegister, this->arch); 
        if( k.size() == 0 ) {
            k = to_string<boost::int32_t>(this->guestOffset, std::dec, 0);
        } 
        r.append(k);
	    r.append(")");
    }
	return r;
}

std::string VExGet::printExpr(void) {
	std::string	r="";
	//TODO, deal with GETI
    if( this->varOffset ) {
        r.append("GETI[");
		r.append(to_string<boost::int32_t>(this->rArr.base, std::dec,0));
        r.append("x");
		r.append(to_string<boost::int32_t>(this->rArr.numElems, std::dec,0));
        switch(this->rArr.ty){
            case ConstantValue::T_INVALID:
                break;
            case ConstantValue::T_I1:
                r.append(":I1");
                break;
            case ConstantValue::T_I8:
                r.append(":I8");
                break;
            case ConstantValue::T_I16:
                r.append(":I16");
                break;
            case ConstantValue::T_I32:
                r.append(":I32");
                break;
            case ConstantValue::T_I64:
                r.append(":I64");
                break;
            case ConstantValue::T_I128:
                r.append("I128");
                break;
            case ConstantValue::T_F32:
                r.append(":F32");
                break;
            case ConstantValue::T_F64:
                r.append(":F64");
                break;
            case ConstantValue::T_F128:
                r.append(":F128");
                break;
            case ConstantValue::T_V128:
                r.append(":V128");
                break;
	    }
        r.append("]");
        r.append("(");
        r.append(this->varOffset->printExpr());
        r.append(",");
		r.append(to_string<boost::int32_t>(this->bias, std::dec,0));
        r.append(")");
    } else {
	    r.append("REGREAD(");
        //r.append(to_string<int>(this->guestOffset, std::dec));
        r.append(regToStr(this->sourceRegister, this->arch));
	    r.append(")");
    }
	return r;
}

std::string ExCCall::printExpr(void) {
    std::string r ="";
    r.append("CCALL[");
    r.append(this->TargetFunc);
    r.append("]");
    r.append("(");
    for(std::vector<ExpressionPtr>::iterator it = this->Args.begin();
        it != this->Args.end();
        ++it )
    {
        ExpressionPtr   ep = *it;
        r.append(ep->printExpr());
        r.append(",");
    }
    r.append(")");
    return r;
}

std::string VExCCall::printExpr(void) {
    std::string r ="";
    r.append("CCALL[");
    r.append(this->TargetFunc);
    r.append("]");
    r.append("(");
    for(std::vector<ExpressionPtr>::iterator it = this->Args.begin();
        it != this->Args.end();
        ++it )
    {
        ExpressionPtr   ep = *it;
        r.append(ep->printExpr());
        r.append(",");
    }
    r.append(")");
    return r;
}

std::string ExConst::printExpr(void) {
	std::string r="";
	if( this->cval.valueIsKnown ) {
		switch(this->cval.valueType) {
			case ConstantValue::T_I1:
				r.append("I:U1(0x"+to_string<unsigned short>(this->cval.U1, std::hex,0)+")");
				break;
			case ConstantValue::T_I8: 
				r.append("I:U8(0x"+to_string<unsigned short>(this->cval.U8, std::hex,0)+")");
				break;
			case ConstantValue::T_I16: 
				r.append("I:U16(0x"+to_string<unsigned short>(this->cval.U16, std::hex,0)+")");
				break;
			case ConstantValue::T_I32: 
				r.append("I:U32(0x"+to_string<unsigned int>(this->cval.U32, std::hex,0)+")");
				break;
			case ConstantValue::T_I64:
				r.append("I:U64(0x"+to_string<unsigned long long>(this->cval.U64, std::hex,0)+")");
				break;
			case ConstantValue::T_F32: 
				r.append("I:F32(0x"+to_string<float>(this->cval.F32, std::hex,0)+")");
				break;
			case ConstantValue::T_F64: 
				r.append("I:F64(0x"+to_string<double>(this->cval.F64, std::hex,0)+")");
				break;
			case ConstantValue::T_V128:  
				r.append("I:V128(0x"+to_string<unsigned short>(this->cval.V128, std::hex,0)+")");
				break;
		}
	} else {
		switch(this->cval.valueType) {
			case ConstantValue::T_I1:
				r.append("I:U1(UNKNOWN)");
				break;
			case ConstantValue::T_I8: 
				r.append("I:U8(UNKNOWN)");
				break;
			case ConstantValue::T_I16: 
				r.append("I:U16(UNKNOWN)");
				break;
			case ConstantValue::T_I32: 
				r.append("I:U32(UNKNOWN)");
				break;
			case ConstantValue::T_I64:
				r.append("I:U64(UNKNOWN)");
				break;
			case ConstantValue::T_F32: 
				r.append("I:F32(UNKNOWN)");
				break;
			case ConstantValue::T_F64: 
				r.append("I:F64(UNKNOWN)");
				break;
			case ConstantValue::T_V128:  
				r.append("I:V128(UNKNOWN)");
				break;
		}
	}
	return r;
}

std::string VExConst::printExpr(void) {
	std::string r="";
	switch(this->constType) {
		case Ico_U1:
			r.append("I:U1(0x"+to_string<unsigned short>(this->cval.U1, std::hex,0)+")");
			break;
		case Ico_U8: 
			r.append("I:U8(0x"+to_string<unsigned short>(this->cval.U8, std::hex,0)+")");
			break;
		case Ico_U16: 
			r.append("I:U16(0x"+to_string<unsigned short>(this->cval.U16, std::hex,0)+")");
			break;
		case Ico_U32: 
			r.append("I:U32(0x"+to_string<unsigned int>(this->cval.U32, std::hex,0)+")");
			break;
		case Ico_U64:
			r.append("I:U64(0x"+to_string<unsigned long long>(this->cval.U64, std::hex,0)+")");
			break;
		case Ico_F32: 
			r.append("I:F32(0x"+to_string<float>(this->cval.F32, std::hex,0)+")");
			break;
		case Ico_F32i: 
			r.append("I:F32i(0x"+to_string<unsigned int>(this->cval.F32i, std::hex,0)+")");
			break;
		case Ico_F64: 
			r.append("I:F64(0x"+to_string<double>(this->cval.F64, std::hex,0)+")");
			break;
		case Ico_F64i:
			r.append("I:F64i(0x"+to_string<unsigned long long>(this->cval.F64i, std::hex,0)+")");
			break;
		case Ico_V128:  
			r.append("I:V128(0x"+to_string<unsigned short>(this->cval.V128, std::hex,0)+")");
			break;
	}
	return r;
}

std::string VExLoad::printExpr(void) {
	std::string r="";
	r.append("MEMREAD("+this->loadAddr->printExpr()+")");
	return r;
}

std::string ExLoad::printExpr(void) {
	std::string r="";
	r.append("MEMREAD("+this->loadAddr->printExpr()+")");
	return r;
}

std::string VExMux0X::printExpr(void) {
	std::string r="";
    r.append("ExMux0X(");
    r.append(this->condition->printExpr());
    r.append(",");
    r.append(this->expTrue->printExpr());
    r.append(",");
    r.append(this->expFalse->printExpr());
    r.append(")");
	return r;
}

std::string ExMux0X::printExpr(void) {
	std::string r="";
    r.append("ExMux0X(");
    r.append(this->condition->printExpr());
    r.append(",");
    r.append(this->expTrue->printExpr());
    r.append(",");
    r.append(this->expFalse->printExpr());
    r.append(")");
	return r;
}

std::string ExOp::printExpr(void) {
	std::string r= "";
	r.append(this->op->printOp()+"[ ");
	std::vector<ExpressionPtr>::iterator	it = this->exps.begin();
	std::vector<ExpressionPtr>::iterator e = this->exps.end();
	while( it != e ) {
		ExpressionPtr   ex = *it;
		r.append(ex->printExpr()+" ");
		++it;
	}
	r.append("]");
	return r;
}

std::string VExOp::printExpr(void) {
	std::string r="";
	r.append(this->op->printOp()+"[ ");
	std::vector<ExpressionPtr>::iterator	it = this->exps.begin();
	std::vector<ExpressionPtr>::iterator e = this->exps.end();
	while( it != e ) {
		ExpressionPtr   ex = *it;
		r.append(ex->printExpr()+" ");
		++it;
	}
	r.append("]");
	return r;
}

std::string ExRdTmp::printExpr(void) {
    return this->TVal->printTemp();
}

std::string VExRdTmp::printExpr(void) {
	return this->TVal->printTemp();
}

std::string ExLogic::printExpr(void) {
	std::string 	r = "";

	switch( this->ty ) {
		case And:
			r = r + "AND";
			r = r + "[ "+this->lhs->printExpr();
			r = r + ", "+this->rhs->printExpr();
			r = r + "]";
			break;
        case Or:
        	r = r + "OR";
        	r = r + "[ "+this->lhs->printExpr();
			r = r + ", "+this->rhs->printExpr();
			r = r + "]";
        	break;
        case Eq:
        	r = r + "EQ";
        	r = r + "[ "+this->lhs->printExpr();
			r = r + ", "+this->rhs->printExpr();
			r = r + "]";
        	break;
        case Range:
        	r = r + "RANGE";
        	r = r + "[ "+this->lhs->printExpr();
			r = r + ", "+this->rhs->printExpr();
			r = r + "]";
        	break;
        case LessThan:
        	r = r + "LT";
        	r = r + "[ "+this->lhs->printExpr();
			r = r + ", "+this->rhs->printExpr();
			r = r + "]";
        	break;
        case GreaterThan:
        	r = r + "GT";
        	r = r + "[ "+this->lhs->printExpr();
			r = r + ", "+this->rhs->printExpr();
			r = r + "]";
        	break;
	}

	return r;
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Printers for Statement subclasses
//////////////////////////////////////////////////////////////////////////
std::string VStStore::printStmt(void) {
	std::string	r="";
	r.append("MEMWRITE("+this->addr->printExpr()+")"+" = "+this->data->printExpr());
	r.append("\n");
	return r;
}

std::string StStore::printStmt(void) {
	std::string	r="";
	r.append("MEMWRITE("+this->addr->printExpr()+")"+" = "+this->data->printExpr());
	r.append("\n");
	return r;
}

std::string StNop::printStmt(void) {
	std::string r="";
	r.append("NOP");
	r.append("\n");
	return r;
}

std::string VStMBE::printStmt(void) {
	std::string r="";
	r.append("MBE: ");
	r.append("TODO");
	r.append("\n");
	return r;
}

std::string StMBE::printStmt(void) {
	std::string r="";
	r.append("MBE: ");
	r.append("TODO");
	r.append("\n");
	return r;
}

std::string VStDirty::printStmt(void) {
	std::string r="";
	r.append("DIRTY: ");
    r.append(this->CalleeName);
	r.append("\n");
	return r;
}

std::string StDirty::printStmt(void) {
	std::string r="";
	r.append("DIRTY: ");
    r.append(this->CalleeName);
	r.append("\n");
	return r;
}

std::string VStExit::printStmt(void) {
	std::string r="";
	r.append("EXIT: ");
	switch( this->blockExit ) {
		case Call:
			r.append("CALL ");
			break;

		case Return:
			r.append("RET ");
			break;

		case Fallthrough:
			r.append("FALLTHRU ");
			break;
       
        case UnknownExit:
        default:
            r.append("UNK END ");
            break;
	}
	r.append("IF "+this->GuardExp->printExpr());
	r.append(" TGT "+this->jmpTarget->printExpr());
	r.append("\n");
	return r;
}

std::string VStIMark::printStmt(void) {
	std::string r="";
	r.append("INSTRUCTION BEGIN: ");
	r.append("ADDR: 0x"+to_string<unsigned long long>(this->Addr, std::hex,0)+" ");
	r.append("LEN: 0x"+to_string<unsigned long>(this->InstLen, std::hex,0));
	r.append("\n");
	return r;
}

std::string VStAbiHint::printStmt(void) {
	std::string r="";
	r.append("ABIHINT: ");
	r.append("TODO");
	r.append("\n");
	return r;
}

std::string StWrTmp::printStmt(void) {
	std::string r="";
	r.append(this->tmpWritten->printTemp()+" = ");
	r.append(this->RHS->printExpr());
	r.append("\n");
	return r;
}

std::string StPut::printStmt(void) {
	std::string r="";
	if( this->varpart ) {
		r.append("PUTI[");
		//TODO
		r.append(to_string<boost::int32_t>(this->rArr.base, std::dec,0));
        r.append("x");
		r.append(to_string<boost::int32_t>(this->rArr.numElems, std::dec,0));
        switch(this->rArr.ty){
            case ConstantValue::T_INVALID:
                break;
            case ConstantValue::T_I1:
                r.append(":I1");
                break;
            case ConstantValue::T_I8:
                r.append(":I8");
                break;
            case ConstantValue::T_I16:
                r.append(":I16");
                break;
            case ConstantValue::T_I32:
                r.append(":I32");
                break;
            case ConstantValue::T_I64:
                r.append(":I64");
                break;
            case ConstantValue::T_I128:
                r.append("I128");
                break;
            case ConstantValue::T_F32:
                r.append(":F32");
                break;
            case ConstantValue::T_F64:
                r.append(":F64");
                break;
            case ConstantValue::T_F128:
                r.append(":F128");
                break;
            case ConstantValue::T_V128:
                r.append(":V128");
                break;
	    }
        r.append("]");
        r.append("(");
        r.append(this->varpart->printExpr());
        r.append(",");
		r.append(to_string<boost::int32_t>(this->bias, std::dec,0));
        r.append(") = ");
        r.append(this->data->printExpr());
	} else {
		r.append("RWRITE(");
		std::string reg = regToStr(this->targetRegister, this->arch);
		if( reg.size() > 0 ) {
			r.append(reg);
		} else {
        	r.append(to_string<int>(this->guestOffset, std::dec,0));
		}
		r.append(") = ");
		r.append(this->data->printExpr());
	}

	r.append("\n");
	return r;
}

std::string VStPut::printStmt(void) {
	std::string r="";
	if( this->varpart ) {
		r.append("PUTI[");
		//TODO
		r.append(to_string<boost::int32_t>(this->rArr.base, std::dec,0));
        r.append("x");
		r.append(to_string<boost::int32_t>(this->rArr.numElems, std::dec,0));
        switch(this->rArr.ty){
            case ConstantValue::T_INVALID:
                break;
            case ConstantValue::T_I1:
                r.append(":I1");
                break;
            case ConstantValue::T_I8:
                r.append(":I8");
                break;
            case ConstantValue::T_I16:
                r.append(":I16");
                break;
            case ConstantValue::T_I32:
                r.append(":I32");
                break;
            case ConstantValue::T_I64:
                r.append(":I64");
                break;
            case ConstantValue::T_I128:
                r.append("I128");
                break;
            case ConstantValue::T_F32:
                r.append(":F32");
                break;
            case ConstantValue::T_F64:
                r.append(":F64");
                break;
            case ConstantValue::T_F128:
                r.append(":F128");
                break;
            case ConstantValue::T_V128:
                r.append(":V128");
                break;
	    }
        r.append("]");
        r.append("(");
        r.append(this->varpart->printExpr());
        r.append(",");
		r.append(to_string<boost::int32_t>(this->bias, std::dec,0));
        r.append(") = ");
        r.append(this->data->printExpr());
	} else {
		r.append("RWRITE(");
		std::string reg = regToStr(this->targetRegister, this->arch);
		if( reg.size() > 0 ) {
			r.append(reg);
		} else {
        r.append(to_string<int>(this->guestOffset, std::dec,0));
		}
		r.append(") = ");
		r.append(this->data->printExpr());
	}
	r.append("\n");
	return r;
}

std::string VStCAS::printStmt(void) {
	std::string r="";
	r.append("[");
    
    if( this->OldHi ) {
        r.append(this->OldHi->printTemp());
        r.append("-");
    }

    r.append(this->OldLo->printTemp());
    r.append("] = ");

    r.append("CAS(");
    r.append(this->StoreAddress->printExpr());
    r.append(")D:[");
    if( this->DataHi ) {
        r.append(this->DataHi->printExpr());
        r.append("-");
    }
    r.append(this->DataLo->printExpr());
    r.append("]E:[");
    if( this->ExpectedHi ) {
        r.append(this->ExpectedHi->printExpr());
        r.append("-");
    }
    r.append(this->ExpectedLo->printExpr());
    r.append("]");

    	r.append("\n");
	return r;
}

std::string VStLLSC::printStmt(void) {
	std::string r="";
    //print out the temp that is written
    r = r + this->result->printTemp() + " = ";
    if( this->storeData ) {
        //is Store-Conditional
        r = r + "ST" + printEnd(this->endian);
        r = r + "-Conditional[";
        r = r + this->addr->printExpr() + ",";
        r = r + this->storeData->printExpr() + "]";
    } else {
        //is Load-Linked
        r = r + "LD" + printEnd(this->endian);
        r = r + "-Linked[" + this->addr->printExpr() + "]";
    }
	return r;
}

//////////////////////////////////////////////////////////////////////////


std::string VBlock::printBlock(void) {
	std::string	r = "BLOCK 0x";

	r.append(to_string<unsigned long long>(this->BlockAddrBegin, std::hex,0));
	r.append(" BLOCK ID: "+to_string<unsigned long>(this->BlockID, std::dec,0)+"\n");

	r.append("TVALS:\n");
	std::vector<TempValPtr>::iterator	tit = this->Temps.begin();
	std::vector<TempValPtr>::iterator	te = this->Temps.end();
	while( tit != te ) {
		TempValPtr  ti = *tit;
		if( ti != NULL ) {
			r.append(ti->printTemp()+"\n");
		}
		++tit;
	}
	r.append("INSTRUCTION DATA:\n");

	std::vector<StatementPtr>::iterator	it = this->statements.begin();
	std::vector<StatementPtr>::iterator	e = this->statements.end();

	while( it != e ) {
		StatementPtr    s = *it;
		r.append(s->printStmt());
		++it;
	}

  r.append("BLOCK EXIT: ");
	switch( this->blockExitType ) {
		case Call:
			r.append("CALL ");
			break;

		case Return:
			r.append("RET ");
			break;

		case Fallthrough:
			r.append("FALLTHRU ");
			break;

    case UnknownExit:
        r.append("UNK EXIT ");
        break;
	}

	r.append(this->Next->printExpr());
	r.append("\n");
	if( this->TargetIDs.size() > 0 ) {
		r.append("BLOCK TARGET IDS: ");
		std::vector<unsigned long>::iterator	it = this->TargetIDs.begin();
		std::vector<unsigned long>::iterator	e = this->TargetIDs.end();

		while( it != e ) {
			r.append(to_string<unsigned long>(*it, std::dec,0)+" ");
			++it;
		}

		r.append("\n");
	}
	r.append("BLOCK END 0x"+to_string<unsigned long long>(this->BlockAddrEnd, std::hex,0));

	r.append("\n\n");
	return r;
}

std::string Flow::printFlow(void) {
	std::string	r = "FLOW: \n";

	std::set<BlockPtr>::iterator	it = this->blocks.begin();
	std::set<BlockPtr>::iterator	e = this->blocks.end();

	while( it != e ) {
		BlockPtr    b = *it;
		r.append(b->printBlock());
		++it;
	}

	return r;
}
