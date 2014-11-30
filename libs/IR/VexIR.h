#ifndef _VEX_IR_H
#define _VEX_IR_H

#include "BasicIR.h"

extern "C" {
#include <libvex_guest_x86.h>
#include <libvex_guest_amd64.h>
#include <libvex_basictypes.h>
#include <libvex_guest_offsets.h>
#include <libvex_emwarn.h>
#include <libvex.h>
}

Register guestOffsetToRegister(int guestOffset, unsigned long width, TargetArch arch);

class VBlock : public Block {
private:
	IRJumpKind						exitType;
    ExitType                        jmpKindToExitType(IRJumpKind ij);
	StatementPtr statementBuilder(IRStmt *stmt, TargetArch arch);
public:
    //constructor from IR
	VBlock(unsigned long id, TargetArch target);
    VBlock(TargetArch, unsigned long long baseAddr, unsigned long len);
    //Destructor
	~VBlock();

    virtual unsigned long getPCOff(void);
    void buildFromIRSB(IRSB *bb);
	virtual std::string	printBlock(void);

    virtual Register getPC(void) {
        Register    r;
        int         off = this->getPCOff();
        int         bits;

        switch(this->CodeTarget.ta) {
            case X86:
                r = guestOffsetToRegister(off, 32, this->CodeTarget);
                break;
            case AMD64:
                r = guestOffsetToRegister(off, 64, this->CodeTarget);
                break;
            case ARM:
                r = guestOffsetToRegister(off, 32, this->CodeTarget);
                break;
            case INVALID:
            case PPC32:
            case PPC64:
            case S390X:
                break;
        }

        return r;
    }
};

typedef boost::shared_ptr<VBlock> VBlockPtr;

/* Operation class */
class VOp : public Op {
private:
	IROp    OpCode;
    Ops     convertOp(IROp o); 
public:
    virtual std::string printOp(void);
	VOp(IROp op){ this->OpCode = op; this->op = this->convertOp(this->OpCode);}
	~VOp(void) { return;	}
};

typedef boost::shared_ptr<VOp> VOpPtr;

//////////////////////////////////////////////////////////////////////////
// Expression class and sub-classes
//////////////////////////////////////////////////////////////////////////

ExpressionPtr expressionBuilder(IRExpr *exp, TargetArch arch, BlockPtr b);
unsigned long getWidthFromTy(IRType ty);

class VExConst : public ExConst {
private:
	IRConstTag	constType;
	unsigned long getWidthFromConstTag(IRConstTag t);
public:
    VExConst(void) { return; }
	VExConst(BlockPtr parent, IRConst *c);
	virtual std::string printExpr(void);
	virtual bool isEConst() { return true; }
	virtual bool isECond() { return false; }

	virtual unsigned long getWidth(void) { return getWidthFromConstTag(this->constType); }
};

typedef boost::shared_ptr<VExConst>  VExConstPtr;

class VExGet : public ExGet {
private:
	IRType		    valType;
public:
	
	VExGet( BlockPtr parent, int off, IRType ty, TargetArch arch ) {
		this->guestOffset = off;
		this->valType = ty;
		this->arch = arch;
		this->sourceRegister = guestOffsetToRegister(this->guestOffset, this->getWidth(), this->arch);
		return;
	}
	    
    VExGet(BlockPtr parent, IRRegArray *descr, ExpressionPtr var, boost::int32_t b, TargetArch arch) {
        this->bias = b;
        this->varOffset = var;
        this->rArr.base = descr->base;
        this->rArr.numElems = descr->nElems;
        this->arch = arch;
        switch(descr->elemTy) {
            case Ity_INVALID:
                assert(true && "Ity_INVALID from source IR!");
                break;
            case Ity_I1:
                this->rArr.ty = ConstantValue::T_I1;
                break;
            case Ity_I8:
                this->rArr.ty = ConstantValue::T_I8;
                break;
            case Ity_I16:
                this->rArr.ty = ConstantValue::T_I16;
                break;
            case Ity_I32:
                this->rArr.ty = ConstantValue::T_I32;
                break;
            case Ity_I64:
                this->rArr.ty = ConstantValue::T_I64;
                break;
            case Ity_I128:  
                this->rArr.ty = ConstantValue::T_I128;
                break;
            case Ity_F32:   
                this->rArr.ty = ConstantValue::T_F32;
                break;
            case Ity_F64:   
                this->rArr.ty = ConstantValue::T_F64;
                break;
            case Ity_F128: 
                this->rArr.ty = ConstantValue::T_F128;
                break;
            case Ity_V128:  
                this->rArr.ty = ConstantValue::T_V128;
                break;
        }

        return;
    }

	virtual ~VExGet() { }
	virtual std::string printExpr(void);
	virtual unsigned long getWidth(void){return getWidthFromTy(this->valType);}
};

typedef boost::shared_ptr<VExGet>    VExGetPtr;

class VExRdTmp : public ExRdTmp {
private:
	IRTemp	    tmpIdx;
public:
	VExRdTmp(BlockPtr parent, IRTemp t);
	virtual std::string printExpr(void);

	virtual unsigned long getWidth(void ) { return this->TVal->getWidth(); }
};

typedef boost::shared_ptr<VExRdTmp>  VExRdTmpPtr;

class VExOp : public ExOp {
public:
	VExOp(BlockPtr parent, OpPtr o, ExpressionPtr e);
	VExOp(BlockPtr parent, OpPtr o, ExpressionPtr e1, ExpressionPtr e2);
	VExOp(BlockPtr parent, OpPtr o, ExpressionPtr e1, ExpressionPtr e2, ExpressionPtr e3);
	VExOp(BlockPtr parent, OpPtr o, ExpressionPtr e1, ExpressionPtr e2, ExpressionPtr e3, ExpressionPtr e4);

	virtual std::string printExpr(void);

	virtual bool containsLoad(void ) {
		bool loadFound = false;

		std::vector<ExpressionPtr >::iterator	eit = this->exps.begin();
		while( eit != this->exps.end()) {
			ExpressionPtr e = *eit;
			loadFound |= e->containsLoad();
			++eit;
		}

		return loadFound;
	}

	virtual ~VExOp() {
        /*
		delete this->op;
		while( this->exps.size() > 0 ) {
			Expression	*e = this->exps[this->exps.size()-1];
			delete e;
			this->exps.pop_back();
		}*/
	}

	//virtual unsigned long getWidth(void) { return 0; }
};

typedef boost::shared_ptr<ExOp> ExOpPtr;

class VExLoad : public ExLoad {
private:
	IREndness	            loadEnd;
    IRType                  vexLoadTy;
public:
	VExLoad(BlockPtr parent, IREndness en, IRType ty, ExpressionPtr addr) { 
		this->loadAddr = addr;
        this->vexLoadTy = ty;
        switch( ty ) {
            case Ity_I8:
                this->loadTy = ConstantValue::T_I8;
                break;
            case Ity_I16:
                this->loadTy = ConstantValue::T_I16;
                break;
            case Ity_I32:
                this->loadTy = ConstantValue::T_I32;
                break;
            case Ity_I64:
                this->loadTy = ConstantValue::T_I64;
                break;
			case Ity_F32:
				this->loadTy = ConstantValue::T_F32;
				break;
			case Ity_F64:
				this->loadTy = ConstantValue::T_F64;
				break;
			case Ity_V128:
				this->loadTy = ConstantValue::T_V128;
				break;
            default:
                assert(!"Bad load type");
        }
		this->loadEnd = en;
		return; 
	}

	virtual ~VExLoad() {
		//delete this->loadAddr;
	}
	virtual std::string printExpr(void);

	virtual bool containsLoad(void) { return true; }
	virtual unsigned long getWidth(void) { return getWidthFromTy(this->vexLoadTy); }
};

typedef boost::shared_ptr<VExLoad>   VExLoadPtr;

class VExMux0X : public ExMux0X {
public:
	VExMux0X(BlockPtr parent, ExpressionPtr condition, ExpressionPtr trueCond, ExpressionPtr falseCond) { 
		this->condition = condition;
		this->expTrue = trueCond;
		this->expFalse = falseCond;
		return; 
	}

	virtual ~VExMux0X() {
		//delete this->condition;
		//delete this->true;
		//delete this->false;
	}
	virtual std::string printExpr(void);

	virtual bool containsLoad(void) { return (this->condition->containsLoad() || this->expTrue->containsLoad() || this->expFalse->containsLoad()); }
};

typedef boost::shared_ptr<ExMux0X>  ExMux0XPtr;

/* we're probably going to need to find a way to remove this 
 * entire expression class from the IR, or at least, 
 * special-case enum all the helpers that VEX has
 */
class VExCCall : public ExCCall {
private:
	IRType						retTy;
public:
	VExCCall(BlockPtr parent, IRExpr **args, IRCallee *cee, IRType ty, TargetArch arch) { 
		this->retTy = ty;
		this->TargetFunc = std::string(cee->name);
		for( int i = 0; args[i] != NULL; i++ ) {
			this->Args.push_back(expressionBuilder(args[i], arch, parent));
		}
		return; 
	}

	virtual ~VExCCall() {
		/*while( this->Args.size() > 0 ) {
			Expression	*e = this->Args[this->Args.size()-1];
			delete e;
			this->Args.pop_back();
		}*/
	}
	virtual std::string printExpr(void);

    std::string getTarget(void) { return this->TargetFunc; }
    std::vector<ExpressionPtr> getArgs(void) { return this->Args; }
};

typedef boost::shared_ptr<VExCCall> VExCCallPtr;

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//Statement class and sub-classes
//////////////////////////////////////////////////////////////////////////

class VStIMark : public StIMark {
private:
	unsigned long long	Addr;
	unsigned long		InstLen;
public:
	VStIMark(BlockPtr parent, unsigned int instLen, unsigned long long addr) { 
		this->Addr = addr;
		this->InstLen = instLen;
		return; 
	}

	virtual ~VStIMark() {
		return;
	}

	virtual std::string printStmt(void);
};

typedef boost::shared_ptr<VStIMark>  VStIMarkPtr;

class VStWrTmp : public StWrTmp {
private:
	IRTemp		    tgt;
public:
	VStWrTmp(BlockPtr parent, IRTemp t, ExpressionPtr rhs) {
		this->tgt = t;
		this->RHS = rhs;
		this->tmpWritten = parent->getTempAtIndex(this->tgt);
		//assert(this->tmpWritten != NULL);
		return; 
	}

	virtual ~VStWrTmp() {
		//delete this->RHS;
		return;
	}

	int getTmpIndex() { return this->tgt; }
	virtual bool hasLoad(void) { return this->RHS->containsLoad(); }
};

typedef boost::shared_ptr<VStWrTmp>  VStWrTmpPtr;

class VStPut : public StPut {
public:
	VStPut(BlockPtr parent, int guestOffset, ExpressionPtr data, TargetArch arch) { 
		this->guestOffset = guestOffset;
		this->data = data;
		//this->varpart = NULL;
		this->bias = 0;
		this->arch = arch;
	
		this->targetRegister = guestOffsetToRegister(  this->guestOffset, 
                                this->data->getWidth(), 
                                this->arch);
		return; 
	}

	virtual ~VStPut() {
	}

	VStPut(BlockPtr parent, IRRegArray *ra, ExpressionPtr data, TargetArch arch, ExpressionPtr varPart, boost::int32_t bias) { 
		this->data = data;
		this->bias = bias;
		this->varpart = varPart;
		this->arch = arch;
        this->rArr.base = ra->base;
        this->rArr.numElems = ra->nElems;
        switch(ra->elemTy) {
            case Ity_INVALID:
                assert(true && "Ity_INVALID from VEX IR!");
                break;
            case Ity_I1:
                this->rArr.ty = ConstantValue::T_I1;
                break;
            case Ity_I8:
                this->rArr.ty = ConstantValue::T_I8;
                break;
            case Ity_I16:
                this->rArr.ty = ConstantValue::T_I16;
                break;
            case Ity_I32:
                this->rArr.ty = ConstantValue::T_I32;
                break;
            case Ity_I64:
                this->rArr.ty = ConstantValue::T_I64;
                break;
            case Ity_I128:  
                this->rArr.ty = ConstantValue::T_I128;
                break;
            case Ity_F32:   
                this->rArr.ty = ConstantValue::T_F32;
                break;
            case Ity_F64:   
                this->rArr.ty = ConstantValue::T_F64;
                break;
            case Ity_F128: 
                this->rArr.ty = ConstantValue::T_F128;
                break;
            case Ity_V128:  
                this->rArr.ty = ConstantValue::T_V128;
                break;
        }

		return; 
	}
	virtual std::string printStmt(void);

	virtual bool hasLoad(void) { return this->data->containsLoad(); }
};

typedef boost::shared_ptr<VStPut>    VStPutPtr;

class VStStore : public StStore {
private:
	IREndness	    end;
public:
	VStStore(BlockPtr parent, IREndness end, ExpressionPtr data, ExpressionPtr addr) { 
		this->end = end;
		this->data = data;
		this->addr = addr;
		return; 
	}

	virtual ~VStStore() {
	}

    ExpressionPtr getData(void) { return this->data; }
    ExpressionPtr getAddr(void) { return this->addr; }

	virtual std::string printStmt(void);

	virtual bool hasLoad(void) { return (this->data->containsLoad() || this->addr->containsLoad()); }
};

typedef boost::shared_ptr<VStStore>  VStStorePtr;

class VStAbiHint : public StAbiHint {
public:
	VStAbiHint(BlockPtr parent, unsigned long len, ExpressionPtr base, ExpressionPtr nia) { 
		this->len = len;
		this->base = base;
		this->nia = nia;
		return; 
	}

	virtual ~VStAbiHint() {/*
		delete this->base;
		delete this->nia;*/
	}

	virtual std::string printStmt(void);
};

typedef boost::shared_ptr<VStAbiHint>    VStAbiHintPtr;

class VStCAS : public StCAS {
public:
	VStCAS(BlockPtr parent, IRCAS	*cas, TargetArch arch) {
		this->StoreAddress = expressionBuilder(cas->addr, arch, parent);
		if( cas->dataHi ) {
			this->DataHi = expressionBuilder(cas->dataHi, arch, parent);
		}

		this->DataLo = expressionBuilder(cas->dataLo, arch, parent);
        switch(cas->end) {
            case Iend_LE:
                this->endian = LittleEndian;
                break;
            
            case Iend_BE:
                this->endian = BigEndian;
                break;
        }

		if( cas->expdHi ) {
			this->ExpectedHi = expressionBuilder(cas->expdHi, arch, parent);
		} 

		this->ExpectedLo = expressionBuilder(cas->expdLo, arch, parent);

        if( cas->oldHi != IRTemp_INVALID ) {
            this->OldHi = parent->getTempAtIndex(cas->oldHi);
        }

        if( cas->oldLo != IRTemp_INVALID ) {
            this->OldLo = parent->getTempAtIndex(cas->oldLo);
        }

		return;
	}
	
    virtual ~VStCAS() { }
	virtual std::string printStmt(void);
    virtual bool hasLoad(void);
};

typedef boost::shared_ptr<VStCAS>    VStCASPtr;

class VStLLSC : public StLLSC {
public:

    //Store-Conditional
	VStLLSC(BlockPtr parent, IREndness end, IRTemp res, ExpressionPtr addr, ExpressionPtr storeData) { 
		this->result = parent->getTempAtIndex(res);
        switch(end) {
            case Iend_LE:
                this->endian = LittleEndian;
                break;
            
            case Iend_BE:
                this->endian = BigEndian;
                break;
        }

		this->storeData = storeData;
		this->addr = addr;

		return; 
	}

    //Load-Linked 
    VStLLSC(BlockPtr parent, IREndness end, IRTemp res, ExpressionPtr addr) {
        this->result = parent->getTempAtIndex(res);
        switch(end) {
            case Iend_LE:
                this->endian = LittleEndian;
                break;
            
            case Iend_BE:
                this->endian = BigEndian;
                break;
        }

        this->addr = addr;
        return;
    }

	virtual ~VStLLSC() {/*
		delete this->addr;
		delete this->storeData;*/
	}

	virtual std::string printStmt(void);
};

typedef boost::shared_ptr<VStLLSC>   VStLLSCPtr;

class VStDirty : public StDirty {
private:
	IRTemp						TmpTarget;
public:
	VStDirty(BlockPtr parent, IRDirty *dt, TargetArch arch) { 
		if( dt->mAddr ) {
			this->Addr = expressionBuilder(dt->mAddr, arch, parent);
		}

		for( int i = 0; dt->args[i] != NULL; i++ ) {
			this->Args.push_back(expressionBuilder(dt->args[i], arch, parent));
		}

		this->Guard = expressionBuilder(dt->guard, arch, parent);

		this->CalleeName = std::string(dt->cee->name);
		this->TmpTarget = dt->tmp;
		
		return; 
	}

	virtual ~VStDirty() {
	}

    //std::string getTarget(void) { return this->CalleeName; }

	virtual std::string printStmt(void);
};

typedef boost::shared_ptr<VStDirty>  VStDirtyPtr;

class VStMBE : public StMBE {
	IRMBusEvent	mBusEvt;
public:
	VStMBE(BlockPtr parent, IRMBusEvent e) { this->mBusEvt = e; }

	virtual std::string printStmt(void);

	virtual ~VStMBE() {
		return;
	}
};

typedef boost::shared_ptr<VStMBE>    VStMBEPtr;

class VStExit : public StExit {
private:
	IRJumpKind	    jmpKind;
public:
	VStExit(BlockPtr parent, IRJumpKind jk, ExpressionPtr e, ExConstPtr jt) {
		this->jmpKind = jk;
		this->GuardExp = e;
		this->jmpTarget = jt;
		return; 
	}

	virtual ~VStExit() {/*
		delete this->jmpTarget;
		delete this->GuardExp;*/
	}

	virtual std::string printStmt(void);
	virtual bool hasLoad(void ) { return (this->GuardExp->containsLoad() || this->jmpTarget->containsLoad()); }
};

typedef boost::shared_ptr<VStExit>   VStExitPtr;

//////////////////////////////////////////////////////////////////////////
#endif
