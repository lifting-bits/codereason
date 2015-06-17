#include "BasicIR.h" 
#include "VexIR.h"

using namespace boost;
using namespace std;

ConstantValue getValue(ExpressionPtr addr) {
    ConstantValue   address;

    address.valueIsKnown = false;

    if( ExRdTmpPtr rdtmp = boost::dynamic_pointer_cast<ExRdTmp>(addr) ) {
        //is the tmp backed by a known value?
        TempValPtr      t = rdtmp->getTmp();
        ConstantValue   tempV = t->getVal();
        if( tempV.valueIsKnown ) {
            address = tempV;
        }

    } else if( ExConstPtr cst = boost::dynamic_pointer_cast<ExConst>(addr) ) {
        //okay, the const has to be known ... 
        address = cst->getVal();
    }

    return address;
}

bool exprTransferRegister(Register r, ExpressionPtr expr, BlockPtr block) {
    
    if( ExConstPtr c = dynamic_pointer_cast<ExConst>(expr) ) {
        return false;
    }

    if( ExRdTmpPtr rd = dynamic_pointer_cast<ExRdTmp>(expr)) {
        ExpressionPtr n = rd->getTmp()->getCreator();
        return exprTransferRegister(r, n, block);
    }

    if( ExGetPtr g = dynamic_pointer_cast<ExGet>(expr) ){
        /* it might be worthwhile to check if this register 
         * is written somewhere previously
         * to do that, we will walk all the statements to the 
         * statement that contains this one 
         */
        Register reg = g->getSrcReg();
        vector<StatementPtr>::iterator it = block->begin();
        ExpressionPtr   write;
        while( it != block->end() ) {
            StatementPtr s = *it;
            if( s->contains(expr) ) {
                break;
            } else {
                //does it write to our register?
                if( StPutPtr put = dynamic_pointer_cast<StPut>(s) ) {
                    if( reg == put->getDstRegister() ) {
                        write = put->getData();
                    }
                }
            }

            ++it;
        }
        if( write ) {
            return exprTransferRegister(r, write, block);
        } else {
            return (reg==r);
        }
    }

    if( ExLoadPtr l = dynamic_pointer_cast<ExLoad>(expr) ) {
        ExpressionPtr k = l->getAddr();
        return exprTransferRegister(r, k, block);
    }

    if( ExOpPtr o = dynamic_pointer_cast<ExOp>(expr) ) {
        //go over each op arg
        bool res = false;
        std::vector<ExpressionPtr>  args = o->getArgs();
        std::vector<ExpressionPtr>::iterator it = args.begin();
        while( it != args.end() ) {
            ExpressionPtr  e = *it;

            res |= exprTransferRegister(r, e, block);

            ++it;
        } 

        return res;
    }
    if( ExCCallPtr c = dynamic_pointer_cast<ExCCall>(expr) ) {
        std::vector<ExpressionPtr>  args = c->getArgs();
        std::vector<ExpressionPtr>::iterator it = args.begin();
        bool res = false;
        while( it != args.end() ) {
            ExpressionPtr  e = *it;

            res |= exprTransferRegister(r, e, block);

            ++it;
        }
        return res;
    }

    if( ExMux0XPtr m = dynamic_pointer_cast<ExMux0X>(expr) ) {
        ExpressionPtr   guard = m->getCondition();
        ExpressionPtr   tr = m->getTrue();
        ExpressionPtr   fa = m->getFalse();

        return (exprTransferRegister(r, guard, block) ||
                exprTransferRegister(r, tr, block) ||
                exprTransferRegister(r, fa, block)
                );
    }

    return false;
}

bool transfersRegister(BlockPtr b, Register r) {
    list<Transfer>  transfers = b->getTransfers();
    list<Transfer>::iterator it = transfers.begin();
    while( it != transfers.end() ) {
        Transfer t = *it;
        ExpressionPtr e = t.second;

        if( exprTransferRegister(r, e, b) ) {
            return true;
        } 

        ++it;
    }

    return false;
}

vector<Register> getGPRsForPlatform(TargetArch arch) {
    vector<Register>    GPRs;

    switch(arch.ta) {
        case X86:
        {
            uint32_t    regOffs[] = {
                            OFFSET_x86_EAX, OFFSET_x86_EBX, OFFSET_x86_ECX,
                            OFFSET_x86_EDX, OFFSET_x86_ESI, OFFSET_x86_EDI,
                            OFFSET_x86_EBP };
            for( uint32_t i = 0; i < sizeof(regOffs)/sizeof(uint32_t); i++ ) {
                uint32_t    regIdx = regOffs[i];
                Register    reg; 

                reg = guestOffsetToRegister(regIdx, 32, arch);

                GPRs.push_back(reg);
            }
        }
            break;
    }

    return GPRs;
}

ExpressionPtr simplifyExpr(BlockPtr b, ExpressionPtr e) {

    if( ExRdTmpPtr rd = dynamic_pointer_cast<ExRdTmp>(e)) {
        ExpressionPtr n = rd->getTmp()->getCreator();
        
        if( !n ) {
            return e;
        } else {
            return simplifyExpr(b, n);
        }
    }

    if( ExGetPtr g = dynamic_pointer_cast<ExGet>(e) ){
        /* it might be worthwhile to check if this register 
         * is written somewhere previously
         * to do that, we will walk all the statements to the 
         * statement that contains this one 
         */
        Register reg = g->getSrcReg();
        vector<StatementPtr>::iterator it = b->begin();
        ExpressionPtr   write;
        while( it != b->end() ) {
            StatementPtr s = *it;
            if( s->contains(e) ) {
                break;
            } else {
                //does it write to our register?
                if( StPutPtr put = dynamic_pointer_cast<StPut>(s) ) {
                    if( reg == put->getDstRegister() ) {
                        write = put->getData();
                    }
                }
            }

            ++it;
        }
        if( write ) {
            return simplifyExpr(b, write);
        } else {
            return e;
        }
    }

    if( ExLoadPtr l = dynamic_pointer_cast<ExLoad>(e) ) {
        ExpressionPtr   k = l->getAddr();
        ExpressionPtr   simpleAddr = simplifyExpr(b, k);

        return ExpressionPtr(new ExLoad(simpleAddr, l->getTy()));
    }

    if( ExOpPtr o = dynamic_pointer_cast<ExOp>(e) ) {
        vector<ExpressionPtr>           newArgs;
        vector<ExpressionPtr>           myArgs = o->getArgs();
        vector<ExpressionPtr>::iterator it = myArgs.begin();

        while( it != myArgs.end() ) {
            ExpressionPtr   s = simplifyExpr(b, *it);
            
            newArgs.push_back(s);

            ++it;
        }

        //check and see if newArgs[0] is an Op of our type, and 
        //if newArgs[0] is a const, and if childArgs[1] is a 
        //const
        if( newArgs.size() == 2 ) {
            ExpressionPtr   lhs = newArgs[0];
            ExpressionPtr   rhs = newArgs[1];
            if(ExOpPtr childOp = dynamic_pointer_cast<ExOp>(lhs)) {
                vector<ExpressionPtr>   car = childOp->getArgs();
                if( car.size() == 2 ) {
                    if(ExConstPtr cn = dynamic_pointer_cast<ExConst>(rhs)) {
                        ExpressionPtr   car1 = car[1];
                        if( ExConstPtr cac = 
                            dynamic_pointer_cast<ExConst>(car1) ) 
                        {
                            Op::Ops childsOp = childOp->getOp()->getOp();
                            Op::Ops         ourOp = o->getOp()->getOp();
                            ConstantValue   newConst;
                            ConstantValue   ourConst = cn->getVal();
                            ConstantValue   childConst = cac->getVal();
                            uint64_t        val;
                            uint64_t        ourVal = 
                                ourConst.getValue<uint64_t>();
                            uint64_t        childVal = 
                                childConst.getValue<uint64_t>();
                            newConst.valueIsKnown = false;

                            if( childsOp == ourOp ) {
                                if( ourConst.width == childConst.width ) {
                                    newConst.width = ourConst.width;
                                    newConst.valueType = ourConst.valueType;

                                    switch(o->getOp()->getOp()) {
                                        case Op::Sub:
                                        case Op::Add:
                                            val = ourVal + childVal;
                                            newConst.setValue<uint64_t>(val);
                                            break;
                                    }
                                }
                           } else {
                                //certain pairings are okay
                                switch(childsOp) {
                                    case Op::Add:
                                        switch(ourOp) {
                                            case Op::Sub:
                                                val = ourVal - childVal; 
                                                break;
                                        }
                                        break;
                                    case Op::Sub:
                                        switch(ourOp) {
                                            case Op::Add:
                                                val = ourVal - childVal;
                                                break;
                                        }
                                        break;
                                }
                            }

                            if( newConst.valueIsKnown ) {
                                newArgs[0] = car[0];
                                newArgs[1] = ExConstPtr(new ExConst(newConst));
                            }
                        }
                    }
                }
            }
        }

        return ExpressionPtr(new ExOp(o->getOp(), newArgs) );
    }

    /*if( ExCCallPtr c = dynamic_pointer_cast<ExCCall>(e) ) {
        return e;
    }

    if( ExMux0XPtr m = dynamic_pointer_cast<ExMux0X>(e) ) {
       return e;
    }*/

    return e;
}

list<Transfer> simplifyTransfers(BlockPtr b) {
    list<Transfer>    transfers = b->getTransfers();
    list<Transfer>    st;

    list<Transfer>::iterator  it = transfers.begin();

    while( it != transfers.end() ) {
        Transfer        t = *it;
        SinkType        s = t.first;
        ExpressionPtr   e = t.second;

        //simplify each expression
        ExpressionPtr simplified = simplifyExpr(b, e);

        //add the expression to the simplified list
        st.push_back(pair<SinkType,ExpressionPtr>(s,simplified));

        ++it;
    }

    return st;
}

//check and see if the expr reads from reg, but ONLY from reg
bool exprReadsRegister(ExpressionPtr expr, Register reg, BlockPtr block) {
    if( ExRdTmpPtr rd = dynamic_pointer_cast<ExRdTmp>(expr)) {
        ExpressionPtr n = rd->getTmp()->getCreator();
        return exprReadsRegister(n, reg, block);
    }
    
    if( ExGetPtr g = dynamic_pointer_cast<ExGet>(expr) ){
        Register                        srcReg = g->getSrcReg();
        vector<StatementPtr>::iterator  it = block->begin();
        ExpressionPtr                   write;
        while( it != block->end() ) {
            StatementPtr s = *it;
            if( s->contains(expr) ) {
                break;
            } else {
                //does it write to our register?
                if( StPutPtr put = dynamic_pointer_cast<StPut>(s) ) {
                    if( srcReg == put->getDstRegister() ) {
                        write = put->getData();
                    }
                }
            }

            ++it;
        }
        if( write ) {
            return exprReadsRegister(write, reg, block);
        } else {
            return (srcReg == reg);
        }
    }
   
    return false;
}
