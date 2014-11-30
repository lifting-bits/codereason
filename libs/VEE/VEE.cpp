/* VEX Execution Engine
 *
 */

#include "VEE.h"

#include <boost/shared_ptr.hpp>
#include <iostream>

using namespace boost;
using namespace std;

VexExecutionEngine::VexExecutionEngine(BlockProviderPtr p, VexExecutionStatePtr vss) :
    blockProvider(p),
    steppedExit(false),
    state(vss)
{
    return;
}

VexExecutionEngine::VexExecutionEngine( BlockProviderPtr        p, 
                                        BlockPtr                b, 
                                        VexExecutionStatePtr    ves ) :
    blockProvider(p),
    state(ves),
    steppedExit(false)
{
    this->block = b;
    this->curStmt = this->block->begin();    
    
    return;
}

VexExecutionEngine::StepState VexExecutionEngine::step() {
    if( !this->block ) {
        return StepERR;
    }

    if( this->curStmt != this->block->end() ) {
        StatementPtr    s = *this->curStmt;
        
        try {
            this->stepStmt(s);
        } catch( StepErr se ) {
            //cout << "ERR in VEE: ";
            //cout << se.get() << endl;
            //cout << "Statement: " << s->printStmt() << endl;
            //return StepERR;
        } catch( StepClientErr sce ) {
            //cout << "ERR in VEE emu: ";
            //cout << sce.get() << endl;

            //return StepClientERR;
        }

        if( this->steppedExit ) {
            this->curStmt = this->block->end();

            return StepEnd;
        }

        this->curStmt++;

        return StepOK;
    } else {
        if( !this->steppedExit ) {
            ExpressionPtr next = this->block->getNext();
            ExitType exitType = this->block->getExitKind();
            this->nextBlock = next;
            this->stepBlockExit(next);
            this->steppedExit = true;
            this->state->setVEEExit(exitType);

            return StepOK;
        } else {
            return StepEnd;
        }
    }
}

ConstantValue VexExecutionEngine::stepOp(ExOpPtr op) {
    ConstantValue cv;

    cv.valueIsKnown = false;

    OpPtr                   o = op->getOp();
    vector<ExpressionPtr>   opArgs = op->getArgs();

    switch( o->getOp() ) {
        case Op::Add:
            cv = doAdd(opArgs);
            break;

        case Op::Sub:
            cv = doSub(opArgs);
            break;
    
        case Op::Mul:
            cv = doMul(opArgs);
            break;

        case Op::MulU:
            cv = doMulU(opArgs);
            break;

        case Op::MulS:
            cv = doMulS(opArgs);
            break;

        case Op::Or:
            cv = doOr(opArgs);
            break;
            
        case Op::And:
            cv = doAnd(opArgs);
            break;

        case Op::Xor:
            cv = doXor(opArgs);
            break;

        case Op::Shl:
            cv = doShl(opArgs);
            break;

        case Op::Shr:
            cv = doShr(opArgs);
            break;

        case Op::Sar:
            cv = doSar(opArgs);
            break;

        case Op::CmpEQ:
            cv = doCmpEQ(opArgs);
            break;

        case Op::CmpNE:
            cv = doCmpNE(opArgs);
            break;

        case Op::Not:
            cv = doNot(opArgs);
            break;

        case Op::C64to8:
            cv = doC64To8(opArgs);
            break;

        case Op::C32HLto64:
            cv = doC32HLTo64(opArgs);
            break;

        case Op::C32to8:
            cv = doC32To8(opArgs);
            break;

        case Op::C32Uto64:
            cv = doC32Uto64(opArgs);
            break;

        case Op::CF32toF64:
            //TODO: FLOATINGPOINT
            throw StepClientErr("FPTODO");
            break;

        case Op::C64to16:
            cv = doC64To16(opArgs);
            break;

        case Op::C32Sto64:
            cv = doC32STo64(opArgs);
            break;

        case Op::C8Sto16:
            cv = doC8STo16(opArgs);
            break;

        case Op::DivModS64to32: 
            cv = doDivModS64to32(opArgs);
            break;

        case Op::DivModU64to32: 
            cv = doDivModU64to32(opArgs);
            break;

        case Op::C16Sto32:
            cv = doC16STo32(opArgs);
            break;
        
        case Op::C16HLto32:
            cv = doC16HLTo32(opArgs);
            break;

        case Op::QNarrow16Ux4:
        case Op::QNarrow16Sx4:
        case Op::QNarrow32Sx2:
        case Op::Max8Sx8:
        case Op::Max16Sx4:
        case Op::Max32Sx2:
        case Op::Max8Ux8:
        case Op::Max16Ux4:
        case Op::Max32Ux2:
        case Op::Min8Sx8:
        case Op::Min16Sx4:
        case Op::Min32Sx2:
        case Op::Min8Ux8:
        case Op::Min16Ux4:
        case Op::Min32Ux2:
        case Op::Abs8x16:
        case Op::Abs16x8:
        case Op::Abs32x4:
        case Op::Avg8Ux16:
        case Op::Avg16Ux8:
        case Op::Avg32Ux4:
        case Op::Avg8Sx16:
        case Op::Avg16Sx8:
        case Op::Avg32Sx4:
        case Op::Max8Sx16:
        case Op::Max16Sx8:
        case Op::Max32Sx4:
        case Op::Max8Ux16:
        case Op::Max16Ux8:
        case Op::Max32Ux4:
        case Op::Min8Sx16:
        case Op::Min16Sx8:
        case Op::Min32Sx4:
        case Op::Min8Ux16:
        case Op::Min16Ux8:
        case Op::Min32Ux4:
        case Op::InterleaveHI8x8:
        case Op::InterleaveHI16x4:
        case Op::InterleaveHI32x2:
        case Op::InterleaveLO8x8:
        case Op::InterleaveLO16x4:
        case Op::InterleaveLO32x2:
        case Op::InterleaveOddLanes8x8:
        case Op::InterleaveEvenLanes8x8:
        case Op::InterleaveOddLanes16x4:
        case Op::InterleaveEvenLanes16x4:
        case Op::Mul8x8:
        case Op::Mul16x4:
        case Op::Mul32x2:
        case Op::Mul32Fx2:
        case Op::MulHi16Ux4:
        case Op::MulHi16Sx4:
        case Op::PolyMul8x8:
        case Op::ShlN8x8:
        case Op::ShlN16x4:
        case Op::ShlN32x2:
        case Op::ShrN8x8:
        case Op::ShrN16x4:
        case Op::ShrN32x2:
        case Op::SarN8x8:
        case Op::SarN16x4:
        case Op::SarN32x2:
        case Op::CmpEQ8x8:
        case Op::CmpEQ16x4:
        case Op::CmpEQ32x2:
        case Op::CmpGT8Ux8:
        case Op::CmpGT16Ux4:
        case Op::CmpGT32Ux2:
        case Op::CmpGT8Sx8:
        case Op::CmpGT16Sx4:
        case Op::CmpGT32Sx2:
        case Op::Add8x8:
        case Op::Add16x4:
        case Op::Add32x2:
        case Op::Sub8x8:
        case Op::Sub16x4:
        case Op::Sub32x2:
        case Op::QSub8Sx8:
        case Op::QSub16Sx4:
        case Op::QSub32Sx2:
        case Op::QSub64Sx1:
        case Op::QSub8Ux8:
        case Op::QSub16Ux4:
        case Op::QSub32Ux2:
        case Op::QSub64Ux1:
        case Op::QAdd8Sx8:
        case Op::QAdd16Sx4:
        case Op::QAdd32Sx2:
        case Op::QAdd64Sx1:
        case Op::QAdd8Ux8:
        case Op::QAdd16Ux4:
        case Op::QAdd32Ux2:
        case Op::QAdd64Ux1:
        case Op::Add64x1:
            throw StepClientErr("NIY: crazymath");
            break;

        case Op::C64LOto32:
            cv = doC64LOto32(opArgs);
            break;

        case Op::C64HIto32:
            cv = doC64HIto32(opArgs);
            break;
       
        case Op::C8Uto16:
            cv = doC8UTo16(opArgs);
            break;

        case Op::C8Uto32:
            cv = doC8UTo32(opArgs);
            break;

        case Op::C8Sto32:
            cv = doC8STo32(opArgs);
            break;

        case Op::C8Uto64:
            cv = doC8UTo64(opArgs);
            break;

        case Op::C16Uto64:
            cv = doC16UTo64(opArgs);
            break;

        case Op::C32to1:
            cv = doC32To1(opArgs);
            break;

        case Op::C64to1:
            cv = doC64To1(opArgs);
            break;

        case Op::C1Uto32:
            cv = doC1UTo32(opArgs);
            break;

        case Op::C1Uto8:
            cv = doC1UTo8(opArgs);
            break;

        case Op::C16Uto32:
            cv = doC16UTo32(opArgs);
            break;

        case Op::C32LOto16:
            cv = doC32LOTo16(opArgs);
            break;

        case Op::C32HIto16:
            cv = doC32HITo16(opArgs);
            break;

        case Op::C16LOto8:
            cv = doC16LOTo8(opArgs);
            break;

        case Op::C16HIto8:
            cv = doC16HITo8(opArgs);
            break;

        case Op::CmpLTS:
            cv = doCmpLTS(opArgs);
            break;

        case Op::CmpLTU:
            cv = doCmpLTU(opArgs);
            break;

        case Op::CmpLES:
            cv = doCmpLES(opArgs);
            break;

        case Op::CmpLEU:
            cv = doCmpLEU(opArgs);
            break;

        case Op::Sad8Ux4:
            cv = doUSad8(opArgs);
            break;

        case Op::UNSUP:
            //we don't translate this, so.. throw
            //throw StepErr("UNMAPPED OP");
            break;
    }

    return cv;
}

void VexExecutionEngine::stepStmt(StatementPtr s) {
    //cout << "Stepping statement " << s->printStmt() << endl;
    do {
        //we need to look in the statement and see if it has a 
        //GET. if it has a GET, need to resolve the GET to a 
        //temp or constant

        if( StPutPtr put = dynamic_pointer_cast<StPut>(s) ) {
            ExpressionPtr   rhs = put->getData();
            //we should get the put offset and mark 
            //but first, is this a put to a variable location?
            ExpressionPtr   var = put->getVarPart();
            int32_t         guestOffset=-1;
            if( var ) {
                //get the base 
                RegArray rArr = put->getRegArray();
                int32_t base = rArr.base;
                int32_t width = rArr.numElems;

                do {
                    if(ExRdTmpPtr rdtmp = dynamic_pointer_cast<ExRdTmp>(var)) {
                        ConstantValue   v = rdtmp->getTmp()->getVal();
                        if( v.valueIsKnown ) {
                            switch(v.valueType) {
                                case ConstantValue::T_I8:
                                    guestOffset = 
                                        base + ((v.U8 + put->getBias())%width);
                                    break;
                                case ConstantValue::T_I16:
                                    guestOffset = 
                                        base + ((v.U16 + put->getBias())%width);
                                    break;
                                case ConstantValue::T_I32:
                                    guestOffset = 
                                        base + ((v.U32 + put->getBias())%width);
                                    break;
                                case ConstantValue::T_I64:
                                    guestOffset = 
                                        base + ((v.U64 + put->getBias())%width);
                                    break;
                                default:
                                    guestOffset = -1;
                            }
                        }
                        break;
                    }

                    if(ExConstPtr cst = dynamic_pointer_cast<ExConst>(var) ) {
                        ConstantValue   v = cst->getVal();
                        assert(v.valueIsKnown);
                        switch(v.valueType) {
                            case ConstantValue::T_I8:
                                guestOffset = 
                                    base + ((v.U8 + put->getBias())%width);
                                break;
                            case ConstantValue::T_I16:
                                guestOffset = 
                                    base + ((v.U16 + put->getBias())%width);
                                break;
                            case ConstantValue::T_I32:
                                guestOffset = 
                                    base + ((v.U32 + put->getBias())%width);
                                break;
                            case ConstantValue::T_I64:
                                guestOffset = 
                                    base + ((v.U64 + put->getBias())%width);
                                break;
                            default:
                                guestOffset = -1;
                        }
                        break;
                    }

                    throw StepErr("Unsupported expression in variable PUT");
                } while(false);
            } else {
                guestOffset = put->getGuestOff();
            }
            do {
                //what kind of data is being put?
                if( ExRdTmpPtr rdtmp = dynamic_pointer_cast<ExRdTmp>(rhs) ) {
                    //for now, just support tmps...
                    TempValPtr      tv = rdtmp->getTmp(); 
                    ConstantValue   cv = tv->getVal();
                    RegConstant     rc = { guestOffset, cv.width, cv }; 

                    //TODO:128-bit
                    if( cv.valueType == ConstantValue::T_V128 ) {
                        throw StepClientErr("V128 NIY");
                    }

                    if( guestOffset >= 0 ) {
                        this->state->setStateFromConst(guestOffset, rc);
                    }
                    break;
                }

                if( ExConstPtr cst = dynamic_pointer_cast<ExConst>(rhs) ) {
                    ConstantValue   cv = cst->getVal();
                    RegConstant     rc = {guestOffset, cv.width, cv};
                     
                    //TODO:128-bit
                    if( cv.valueType == ConstantValue::T_V128 ) {
                        throw StepClientErr("V128 NIY");
                    }
       
                    assert(cv.valueIsKnown);

                    if( guestOffset >= 0 ) {
                        this->state->setStateFromConst(guestOffset, rc);
                    }
                    break;
                }
            } while(false);
            break;
        }

        if( StWrTmpPtr wrtmp = dynamic_pointer_cast<StWrTmp>(s) ) {            
            ExpressionPtr   rhs = wrtmp->getRHS();
            TempValPtr      valWritten = wrtmp->getTarget();

			//TODO: FLOATINGPOINT
			ConstantValue::ValTy	writtenType = valWritten->getTy();
			if( writtenType == ConstantValue::T_F32 || 
				writtenType == ConstantValue::T_F64 || 
				writtenType == ConstantValue::T_F128)
			{
                //what if we just continue?
                return;
				//throw StepClientErr("Floating point not supported yet");
			}

            //TODO: 128-bit vectors
            if( writtenType == ConstantValue::T_V128 ) {
                throw StepClientErr("V128 NIY");
            }

            do {
                if( ExGetPtr get = dynamic_pointer_cast<ExGet>(rhs) ) {
                    //alright, this is a write from a get
                    //we should see if the get is backed by "static state"
                    ExpressionPtr var = get->getVarPart();
                    if( var ) {
                        //lets just punt on this for now
                        throw StepClientErr("floating GET NIY");
                    } else {
                        int off = get->getOff();
                        unsigned long wid = get->getWidth();
                        ConstantValue v = 
                            this->state->getConstFromState(off, wid);
                        if( v.valueIsKnown ) {
                            //the resulting value is known in our state map
                            //write it into this temp val
                            valWritten->setValue(v);
                        } 
                    }
                    break;
                }

                if( ExOpPtr op = dynamic_pointer_cast<ExOp>(rhs) ) {
                    //this is a write from an op... handle the emulation of
                    //the op!!
                    ConstantValue cv = this->stepOp(op);

                    if( cv.valueIsKnown ) {
                        //the step produced a constant
                        //promote the tmp by assigning a value to it
                        valWritten->setValue(cv);
                    }
                    break;
                }

                if( ExLoadPtr load = dynamic_pointer_cast<ExLoad>(rhs) ) {
                    //this is a load, so check the memory map and see if
                    //there is anything there
                    ConstantValue cv = this->doLoad(load);

                    if( cv.valueIsKnown ) {
                        valWritten->setValue(cv);
                    }
                    break;
                }

                if( ExCCallPtr ccall = dynamic_pointer_cast<ExCCall>(rhs) ) {
                    ConstantValue cv = this->stepCCall(ccall);
                    break;
                }

                if( ExRdTmpPtr rdtmp = dynamic_pointer_cast<ExRdTmp>(rhs) ) {
                    ConstantValue cv = rdtmp->getTmp()->getVal();
                    if( cv.valueIsKnown ) {
                        valWritten->setValue(cv);
                    }
                    break;
                }

                if( ExConstPtr cst = dynamic_pointer_cast<ExConst>(rhs) ) {
                    ConstantValue cv = cst->getVal();
                    valWritten->setValue(cv);

                    break;
                }

                if( ExMux0XPtr mux = dynamic_pointer_cast<ExMux0X>(rhs) ) {
                    ConstantValue   cv = doExMux0X(mux);
                    if( cv.valueIsKnown ) {
                        valWritten->setValue(cv);
                    }
                    break;
                }
                
                //unsupported
                throw StepErr("WRTMP-UNSUP");
            } while( false );

            break;
        }

        if( StNopPtr nop = dynamic_pointer_cast<StNop>(s) ) {
            //nothing to do
            break;
        }

        if( StIMarkPtr im = dynamic_pointer_cast<StIMark>(s) ) {
            //there is nothing to do for these 
            break;
        }

        if( StStorePtr store = dynamic_pointer_cast<StStore>(s) ) {
            ExpressionPtr   rhs = store->getData();
            ExpressionPtr   addr = store->getAddr();
            ConstantValue   addrVal;
            addrVal.valueIsKnown = false;
            ConstantValue   storeVal;
            storeVal.valueIsKnown = false;

            do {
                //is this a const
                if( ExConstPtr  cst = dynamic_pointer_cast<ExConst>(addr) ) {
                    //this is easy
                    addrVal = cst->getVal();
                    break;
                }

                if( ExRdTmpPtr rpt = dynamic_pointer_cast<ExRdTmp>(addr) ) {
                    //is the temp read const
                    TempValPtr t = rpt->getTmp();

                    if( t->isEConst() ) {
                        addrVal = t->getVal();
                    }
                    break;
                }

            } while( false );
            
            do {
                if( ExConstPtr  cst = dynamic_pointer_cast<ExConst>(rhs) ) {
                    storeVal = cst->getVal();
                    break;
                }

                if( ExRdTmpPtr rpt = dynamic_pointer_cast<ExRdTmp>(rhs) ) {
                    TempValPtr t = rpt->getTmp();

                    storeVal = t->getVal();
                    break;
                }

            } while( false );

            if( addrVal.valueIsKnown ) {
                uint64_t    addrV;
                assert(addrVal.width == 32 || addrVal.width == 64);
                switch(addrVal.width) {
                    case 32:
                        addrV = addrVal.U32; 
                        break;
                    case 64:
                        addrV = addrVal.U64;
                        break;
                }
                this->state->setMemFromConst(addrV, storeVal);
            } //if addr is not known, cannot execute store
            break;
        }

        if( StExitPtr exit = dynamic_pointer_cast<StExit>(s) ) {
            //update our state with how we can exit
            //cout << "exit" << endl;
            if( exit->getJmpKind() == Call ) {
                this->state->setDidCall(true);
            }

            //for now, do not support call or return
            /*if( exit->getJmpKind() == Call || exit->getJmpKind() == Return ) {
                break;
            }*/

            //test the guard and see if we can evaluate it
            ExpressionPtr   ex = exit->getCondition();
            ConstantValue   takeBranch;

            takeBranch.valueIsKnown = false;

            do {
                if( ExConstPtr  cst = dynamic_pointer_cast<ExConst>(ex) ) {
                    takeBranch = cst->getVal();
                    break;
                }

                if( ExRdTmpPtr rpt = dynamic_pointer_cast<ExRdTmp>(ex) ) {
                    TempValPtr t = rpt->getTmp();

                    takeBranch = t->getVal();
                    break;
                }

            } while( false );

            //if we can evaluate the guard and it is true, stop executing
            //and update the current PC with where we are going to go
            if( takeBranch.valueIsKnown == true ) {
                assert( takeBranch.width == 1 );
                if( takeBranch.U1 == 1 ) {
                    ExConstPtr  tgt = exit->getTarget();

                    this->nextBlock = tgt;
                    this->state->setVEEExit(exit->getJmpKind());
                    this->stepBlockExit(tgt);
                }
            } else {
                //if we can't evaluate the guard, throw
                //this isn't really accurate. well, it is, but it is 
                //uselessly accurate. for the moment, let's just say that 
                //we will continue on to the next statement
                //throw StepClientErr("non-evaluable guard condition");
            }
            break;
        }

        if( StDirtyPtr dirty = dynamic_pointer_cast<StDirty>(s) ) {
            this->doDirtyStep(dirty);
            break;
        }

        if( StCASPtr cas = dynamic_pointer_cast<StCAS>(s) ) {
            this->doCAS(cas);
            break;
        }

        if( StLLSCPtr llsc = dynamic_pointer_cast<StLLSC>(s) ) {
            this->doLLSC(llsc);
            break;
        }

        //if we make it this far, 
        //either something went wrong, or StatementPtr
        //is not a subclass we support stepping yet
        //so terminate our stepping here
        throw StepErr("UNSUPPORTED");
    } while( false );

    return;
}

void VexExecutionEngine::doDirtyStep(StDirtyPtr dirty) {
     
    if( dirty->getTarget() == "x86g_dirtyhelper_IN" ) {
        throw StepClientErr("x86g_dirtyhelper_IN wrecks state");
    } else if( dirty->getTarget() == "x86g_dirtyhelper_OUT" ) {
        throw StepClientErr("x86g_dirtyhelper_OUT wrecks state");
    } else if( dirty->getTarget() == "x86g_dirtyhelper_FSTENV" ) {
        throw StepClientErr("FSTENV NIY");
    } else if( dirty->getTarget() == "x86g_dirtyhelper_CPUID_sse2" ) {
        throw StepClientErr("CPUID NIY");
    } else if( dirty->getTarget() == "x86g_dirtyhelper_loadF80le" ) {
        throw StepClientErr("loadF89le NIY");
    } else if( dirty->getTarget() == "x86g_dirtyhelper_FLDENV" ) {
        throw StepClientErr("FLDENV NIY");
    } else if( dirty->getTarget() == "x86g_dirtyhelper_FSAVE" ) { 
        throw StepClientErr("FSAVE NIY");
    } else if( dirty->getTarget() == "x86g_dirtyhelper_FRSTOR" ) {
        throw StepClientErr("FRSTOR");
    } else if( dirty->getTarget() == "x86g_dirtyhelper_SxDT" ) {
        throw StepClientErr("SxDT");
    } else if( dirty->getTarget() == "x86g_dirtyhelper_RDTSC" ) {
        throw StepClientErr("RDTSC");
    }
    
    throw StepErr("DIRTY unsupported");

    return;
}

std::string VexExecutionEngine::printState(void) {
    if( this->block ) {
        return this->block->printBlock();
    }

    return "NOBLOCK";
}

std::string VexExecutionState::printRegState(void) {
    string  s = "";

    int i =0;
    //print the current reg state values
    for(regMapT::iterator it = this->regState.begin();
        it != this->regState.end();
        ++it, i++)
    {
        ConstantValue   v = (*it).v;
        if( v.valueIsKnown ) {
            s = s + "State " + to_string<int>(i, dec,0) + " ";
            s = s + to_string<unsigned short>(v.U8, dec,0);
            s = s + "\n";
        }         
    } 

    return s;
}

ConstantValue VexExecutionEngine::doLoad(ExLoadPtr load) {
    ConstantValue           cv;
    ConstantValue           address;
    ExpressionPtr           loadAddr = load->getAddr();
    ConstantValue::ValTy    loadTy = load->getTy();

    cv.valueIsKnown = false;
    address = getValue(loadAddr);

    //first, determine if the address of the load is known
    if( address.valueIsKnown ) {
        uint64_t    addrKey;
        uint16_t    stride;
        //alright, we have an address, actually perform a dereference
        //TODO make sure that the address is native pointer width
        assert( address.width == 32 || address.width == 64 );
        switch( address.width ) {
            case 32:
                addrKey = address.U32;
                break;
            case 64:
                addrKey = address.U64;
                break;
        }
        
        assert( loadTy == ConstantValue::T_I8 ||
         loadTy == ConstantValue::T_I16 ||
         loadTy == ConstantValue::T_I32 ||
         loadTy == ConstantValue::T_I64 );

        switch( loadTy ) {
            case ConstantValue::T_I8:
                stride = 1;
                break;
            case ConstantValue::T_I16:
                stride = 2;
                break;
            case ConstantValue::T_I32:
                stride = 4;
                break;
            case ConstantValue::T_I64:
                stride = 8;
                break;
            default:
                //can't be here due to asserts above
                stride = 0;
                break;
        }

        cv = this->state->getConstFromMem(addrKey, stride);
    }

    //client should only get updated if valueIsKnown is true
    return cv;
}

void VexExecutionEngine::doLLSC(StLLSCPtr llsc) {
    //this is different based on whether or not it is a LL or an SC
    ExpressionPtr           addr = llsc->getAddr();
    ExpressionPtr           storeData = llsc->getStoreData();
    ConstantValue           addr_v = getValue(addr);
    TempValPtr              dest = llsc->getResult();
    ConstantValue::ValTy    loadTy = dest->getTy();

    if( storeData ) { 
        //is SC
        //we'll treat this as just a store
        ConstantValue   storeData_v = getValue(storeData);        
        if( storeData_v.valueIsKnown && addr_v.valueIsKnown ) {
            uint64_t    addrV;
            assert(addr_v.width == 32 || addr_v.width == 64);
            switch(addr_v.width) {
                case 32:
                    addrV = addr_v.U32; 
                    break;
                case 64:
                    addrV = addr_v.U64;
                    break;
            }

            this->state->setMemFromConst(addrV, storeData_v);
            ConstantValue   one;

            one.valueIsKnown = true;
            one.valueType = ConstantValue::T_I1;
            one.U1 = 1;
            one.width = 1;
            //just set it to 1
            dest->setValue(one);
        } else {
            //cannot do a wild write, but will swallow and ignore for now
        }
    } else {
        //is LL
        //we'll treat this as just a load
        if( addr_v.valueIsKnown ) {
            uint64_t    addrKey;
            uint16_t    stride;
            //alright, we have an address, actually perform a dereference
            //TODO make sure that the address is native pointer width
            assert( addr_v.width == 32 || addr_v.width == 64 );
            switch( addr_v.width ) {
                case 32:
                    addrKey = addr_v.U32;
                    break;
                case 64:
                    addrKey = addr_v.U64;
                    break;
            }
            
            assert( loadTy == ConstantValue::T_I8 ||
             loadTy == ConstantValue::T_I16 ||
             loadTy == ConstantValue::T_I32 ||
             loadTy == ConstantValue::T_I64 );

            switch( loadTy ) {
                case ConstantValue::T_I8:
                    stride = 1;
                    break;
                case ConstantValue::T_I16:
                    stride = 2;
                    break;
                case ConstantValue::T_I32:
                    stride = 4;
                    break;
                case ConstantValue::T_I64:
                    stride = 8;
                    break;
                default:
                    //can't be here due to asserts above
                    stride = 0;
                    break;
            }

            ConstantValue   cv = this->state->getConstFromMem(addrKey, stride);

            //if cv is known, put into dest
            if( cv.valueIsKnown ) {
                dest->setValue(cv);
            }
        } else {
            //cannot do a wild read, but will swallow and ignore for now
        }
    }

    return;
}

void VexExecutionEngine::doCAS(StCASPtr cas) {
    //oh blarg.
    //okay this works 2 ways, SCAS and DCAS
    if( cas->getDataHi() || cas->getExpectedHi() || cas->getOldHi() ) {
        throw StepErr("DCAS NIY");
        //DCAS
    } else {
        //SCAS
        ExpressionPtr   storeExpr = cas->getStoreAddress();
        ConstantValue   addrR;

        addrR.valueIsKnown = false;
        
        if( ExRdTmpPtr rd = dynamic_pointer_cast<ExRdTmp>(storeExpr) ) {
            addrR = rd->getTmp()->getVal();
        } else if( ExConstPtr cst = dynamic_pointer_cast<ExConst>(storeExpr)) {
            addrR = cst->getVal();
        } else {
            throw StepErr("SCAS invalid state");
        }

        if( !addrR.valueIsKnown ) {
            return;
        }

        uint64_t    addr;
        switch(addrR.width) {
            case 8:
                addr = addrR.U8;
                break;
            case 16:
                addr = addrR.U16;
                break;
            case 32:
                addr = addrR.U32;
                break;
            case 64:
                addr = addrR.U64;
                break;
            default:
                throw StepErr("Invalid state");
        }

        ExpressionPtr   expected = cas->getExpectedLo();
        ExpressionPtr   data = cas->getDataLo();
        TempValPtr      result = cas->getOldLo();
        ConstantValue   vRes = result->getVal();
        ConstantValue   vExpected;
        ConstantValue   vData;

        vExpected.valueIsKnown = false;
        vRes.valueIsKnown = false; 
        vData.valueIsKnown = false;

        //see if we can get a hard value for vExpected
        if( ExRdTmpPtr erdt = dynamic_pointer_cast<ExRdTmp>(expected) ) {
            ConstantValue tmp = erdt->getTmp()->getVal();

            if( tmp.valueIsKnown ) {
                vExpected = tmp;
            }
        } 
        
        if( ExConstPtr ecst = dynamic_pointer_cast<ExConst>(expected) ) {
            ConstantValue tmp = ecst->getVal();

            assert(tmp.valueIsKnown);
            vExpected = tmp;
        }

        //see if we can get a hard value for vData
         if( ExRdTmpPtr drdt = dynamic_pointer_cast<ExRdTmp>(data) ) {
            ConstantValue tmp = drdt->getTmp()->getVal();

            if( tmp.valueIsKnown ) {
                vData = tmp;
            }
        } 
        
        if( ExConstPtr dcst = dynamic_pointer_cast<ExConst>(data) ) {
            ConstantValue tmp = dcst->getVal();

            assert(tmp.valueIsKnown);
            vData = tmp;
        }


        //if we can, see if we can do a memory read for the address 
        uint16_t    stride = getStrideFromWidth(vRes.width);
        assert(stride != 0 );
        ConstantValue   fromMem = this->state->getConstFromMem(addr, stride);

        if( fromMem.valueIsKnown && 
            vData.valueIsKnown && 
            vExpected.valueIsKnown ) 
        {
            //throw StepErr("NIY");
            bool    valsEq = false;

            assert( fromMem.width == vExpected.width );
            switch(fromMem.width) {
                case 8:
                    if( fromMem.U8 == vExpected.U8 ) {
                        valsEq = true;
                    }
                    break;
                case 16:
                    if( fromMem.U16 == vExpected.U16 ) {
                        valsEq = true;
                    }
                    break;
                case 32:
                    if( fromMem.U32 == vExpected.U32 ) {
                        valsEq = true;
                    }
                    break;
                case 64:
                    if( fromMem.U64 == vExpected.U64 ) {
                        valsEq = true;
                    }
                    break;
            }

            if( valsEq ) {
                //write vData into memory at addr
                this->state->setMemFromConst(addr, vData);
            } 

            //copy the fromMem value into the result
            result->setValue(fromMem);
        }
    }

    return;
}

std::string VexExecutionState::printMemState(void) {
    string  s = "";
    //iterate over mem state, print everything that is not defined
    addrMapT::iterator it = this->memMap.begin();
    while( it != this->memMap.end() ) {
        icl::discrete_interval<uint64_t>    l = (*it).first;
        MemLocation                         ml = (*it).second;
        ConstantValue                       j = ml.get();
        s.append("ADDR: "+to_string<uint64_t>(l.lower(), hex,0));
        s.append("-"+to_string<uint64_t>(l.upper(), hex,0));
        s.append("= ");
        if( j.valueIsKnown ) {
            s.append(to_string<uint32_t>(j.U8, hex,0)); 
        } else {
            s.append("UNK");
        }
        s.append("\n");
        ++it;
    }
    return s;
}

void VexExecutionEngine::stepBlockExit(ExpressionPtr exit) {
    //okay well the block is exiting, we have two questions
    //a. how is the block exiting? is it via a call, return, what?
    //b. what is the target of our next block? is it constant?
    ConstantValue   next;

    next.valueIsKnown = false;

    if( ExConstPtr cst = dynamic_pointer_cast<ExConst>(exit)) {
        next = cst->getVal(); 
    }

    if( ExRdTmpPtr tmp = dynamic_pointer_cast<ExRdTmp>(exit) ) {
        next = tmp->getTmp()->getVal();
    }

    if( next.valueIsKnown ) {
        //okay well we know where we're going
        //TODO LATER - see if we should step through 
        //if we can, then do it
        //if we can't, just leave
        Register    pc = this->block->getPC();

        RegConstant nextr = { this->block->getPCOff(), pc.width, next };
        
        //but also, note in our current context where we are going to
        this->state->setStateFromConst(this->block->getPCOff(), nextr);
    } else {
    }

    return;
}

ConstantValue VexExecutionEngine::doExMux0X(ExMux0XPtr mux) {
    ConstantValue   v;
    vector<ExpressionPtr>   exps;
    vector<ConstantValue>   constants;

    v.valueIsKnown = false;

    exps.push_back(mux->getCondition());
    exps.push_back(mux->getTrue());
    exps.push_back(mux->getFalse());

    getConstArgs(exps, constants);
    if( constants.size() == 3 ) {
        ConstantValue   cond = constants[0];
        ConstantValue   tr = constants[1];
        ConstantValue   fa = constants[2];

        bool isCondSet = false;

        switch(cond.width) {
            case 1:
                if( cond.U1 == 0 ) {
                    isCondSet = true;
                }
                break;
            case 8:
                if( cond.U8 == 0 ) {
                    isCondSet = true;
                }
                break;
            case 16:
                if( cond.U16 == 0 ) {
                    isCondSet = true;
                }
                break;
            case 32:
                if( cond.U32 == 0 ) {
                    isCondSet = true;
                }
                break;
            case 64:
                if( cond.U64 == 0 ) {
                    isCondSet = true;
                }
                break;
        }

        if( isCondSet ) {
            v = tr;
        } else {
            v = fa;
        }
    }

    return v;
}
