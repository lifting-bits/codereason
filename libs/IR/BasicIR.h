#ifndef _BASIC_IR_H
#define _BASIC_IR_H

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/cstdint.hpp>

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <list>
#include <assert.h>
#include <string.h>

class Op;
typedef boost::shared_ptr<Op> OpPtr;
class Flow;
typedef boost::shared_ptr<Flow> FlowPtr;
class TempVal;
typedef boost::shared_ptr<TempVal>  TempValPtr;
class Expression;
typedef boost::shared_ptr<Expression> ExpressionPtr;
class Statement;
typedef boost::shared_ptr<Statement> StatementPtr;
class Block;
typedef boost::shared_ptr<Block> BlockPtr;

enum TargetMajorArch { 
	INVALID,
	X86, 
	AMD64, 
	ARM,
	PPC32,
	PPC64,
	S390X
};

enum TargetSubArch {
    WIDEARM,
    THUMB,
    S_INVALID
};

struct TargetArch {
    TargetMajorArch ta;
    TargetSubArch   tm;
};

enum X86RegisterW8 {
	AL,
	AH,
	BL,
	BH,
	CL,
	CH,
	DL,
	DH,
    ANY8
};

enum X86RegisterW16 {
	AX,
	BX,
	CX,
	DX,
	BP,
	SI,
	DI,
	SP,
    ANY16
};

enum X86RegisterW32 {
	EAX,
	EBX,
	ECX,
	EDX,
	EDI,
	ESI,
	EBP,
	ESP,
	EIP,
	CS,
	DS,
	ES,
	FS,
	GS,
	SS,
	LDT32,
	GDT32,
	EMWARN32,
  OP,
  DEP1,
  DEP2,
  NDEP,
  DFLAG,
  IDFLAG,
  ACFLAG,
  FTOP,
  ANY32
};

enum X86RegisterW64 {
	RAX,
	RBX,
	RCX,
	RDX,
	RDI,
	RSI,
	RSP,
	RBP,
	RIP,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	R14,
	R15,
    GDT,
    TI,
    EMWARN,
    LDT
};

enum X86FlagState {
    };

enum ARMRegister {
    AR1,
    AR2,
    AR3,
    AR4,
    AR5,
    AR6,
    AR7,
    AR8,
    AR9,
    AR10,
    AR11,
    AR12,
    AR13,
    AR14,
    AR15,
    ARUnknown
    };

enum RegisterClass {
    GenericRegister,
    StackPointer,
    ProgramCounter,
    Flags,
    InternalState
};

enum MemoryEnd {
    BigEndian,
    LittleEndian
};

struct Register {
	TargetArch	            arch;
    RegisterClass           regclass; 
	unsigned long           width;
	union {
		X86RegisterW8		Reg8;
		X86RegisterW16		Reg16;
		X86RegisterW32		Reg32;
		X86RegisterW64		Reg64;
        ARMRegister         RegArm;
	};

    bool operator==(const Register &other) const {
        if( this->arch.ta == X86 ) {
            if( (this->width == 8 && this->Reg8 == ANY8) ||
                (this->width == 16 && this->Reg16 == ANY16) ||
                (this->width == 32 && this->Reg32 == ANY32) )
            {
                return true;
            }
        }

        if( this->arch.ta == other.arch.ta ) {
            if( this->arch.ta != X86 ) {
                if( this->arch.tm != other.arch.tm ) {
                    return false;
                }
            }
        } else {
            return false;
        }

        switch( this->width) {
            case 8:
                return (this->Reg8 == other.Reg8);
                break;
            case 16:
                return (this->Reg16 == other.Reg16);
                break;
            case 32:
                return (this->Reg32 == other.Reg32);
                break;
            case 64:
                return ( this->Reg64 == other.Reg64 );
                break;
            default:
                return false;
        }
    }

    bool operator!=(const Register &other) const {
        return !(*this == other);
    }

    bool operator<(const Register &other) const {
        if( this->arch.ta == other.arch.ta &&
            this->arch.tm == other.arch.tm &&
            this->width == other.width
            )
        {
            switch( this->width) {
                case 8:
                    return (this->Reg8 < other.Reg8);
                    break;
                case 16:
                    return (this->Reg16 < other.Reg16);
                    break;
                case 32:
                    return (this->Reg32 < other.Reg32);
                    break;
                case 64:
                    return ( this->Reg64 < other.Reg64 );
                    break;
                default:
                    return false;
            }
        }
        else 
        {
            return false;
        }
    }

    bool operator<=(const Register &other) const {
        return (*this < other) || (*this == other);
    }

};

class SinkType {
private:
    enum SinkTypeTy {
        Reg,
        Mem
    };
    SinkTypeTy      ty;
    Register        regSink;
    ExpressionPtr   addrSink;
public:
    bool isReg(void) const { return this->ty == Reg; }
    bool isMem(void) const { return this->ty == Mem; }
    SinkType(Register r) : regSink(r), ty(Reg) { }
    SinkType(ExpressionPtr a) : addrSink(a), ty(Mem) { }

    bool operator==(const SinkType &other) const {
        if( this->ty == other.ty ) {
            if( this->ty == Reg ) {
                return (this->regSink == other.regSink );
            } else if ( this->ty == Mem ) {
                return false;
            }
        } else {
            return false;
        }
        return false;
    }

    Register getReg(void) const { return this->regSink; }
    ExpressionPtr getMem(void) const { return this->addrSink; }
};

typedef std::pair<SinkType,ExpressionPtr>   Transfer;

//#include "Output.h"
#include "Input.h"

enum ExitType {
    Fallthrough,
    Call,
    Return,
    UnknownExit
};

class Block : public boost::enable_shared_from_this<Block> {
protected:
	std::vector<StatementPtr>		statements;
    ExitType                        blockExitType;
	ExpressionPtr                   Next;
	std::vector<ExpressionPtr>		Targets;
	std::set<boost::uint64_t>		HardTargets;
	bool							processedTargets;
	std::vector<TempValPtr>			Temps;
	unsigned long long				BlockAddrBegin;
	unsigned long long				BlockAddrEnd;
	unsigned long					BlockID;
	std::vector<unsigned long>		TargetIDs;
	TargetArch						CodeTarget;
    InputPtr                        preConditions;

    //information on exits
    bool                    readMem;
    bool                    writeMem;
    bool                    ret;
    bool                    condBranch;
    bool                    anyCalls;
    std::list<Transfer>     transfer;


public:
    Block(void) { }
    Block(TargetArch ta) : CodeTarget(ta) { } 

    std::string printTransfer(Transfer);

    bool readsMem(void) { return this->readMem; }
    bool writesMem(void) { return this->writeMem; }
    bool returns(void) { return this->ret; }
    bool conditionalBranch(void) { return this->condBranch; }
    bool calls(void) { return this->anyCalls; }
    std::list<Transfer> getTransfers(void);
    void insertTempVal(TempValPtr p) { this->Temps.push_back(p); }
	TempValPtr getTempAtIndex(int i) { return this->Temps[i]; }
    
//iterator stuff
    std::vector<TempValPtr>::iterator begin_temps() {
        return this->Temps.begin();
    }
    std::vector<TempValPtr>::iterator end_temps() { return this->Temps.end(); }
    std::vector<StatementPtr>::iterator begin() { 
        return this->statements.begin(); 
    }
    std::vector<StatementPtr>::iterator end() { return this->statements.end(); }
    void insertStmt(StatementPtr s) { this->statements.push_back(s); }
    TargetArch getArch(void) { return this->CodeTarget; }
    virtual unsigned long getPCOff(void)=0;
    virtual Register getPC(void)=0;

	virtual std::string	printBlock(void)=0;

	std::set<boost::uint64_t> getHardTargets(void);

	unsigned long long getBlockBase(void) { return this->BlockAddrBegin; }
	unsigned long long getBlockEnd(void) { return this->BlockAddrEnd; }
    unsigned long long getBlockLen(void);
	unsigned long getBlockId(void) { return this->BlockID; }
	void resolveTargets(FlowPtr);

	bool overlaps(BlockPtr other);
	bool overlaps(unsigned long long addr);
	std::string dumpBlockToProto(void);

    friend class Input;

	//OutputPtr getPostConditions(void) { return this->postConditions; }

    ExitType getExitKind(void) { return this->blockExitType; }
    void setExitKind(ExitType t) { this->blockExitType = t; }
    ExpressionPtr getNext(void) { return this->Next; }
    void setNext(ExpressionPtr e) { this->Next = e; }
};

//void guestOffsetToRegister(int guestOffset, unsigned long width, TargetArch arch, Register &reg);

template <class T>
std::string to_string(T t, std::ios_base & (*f)(std::ios_base&), int width=3)
{
	std::ostringstream oss;
	oss << std::setw(width) << f << t;
	return oss.str();
}

/* Operation class */
class Op : public boost::enable_shared_from_this<Op> {
public:
    enum Ops {
        /* Basic arithmetic */
        Add,
        Sub,
        /* this multiply does not widen */
        Mul,
        /* these multiplies widen*/
        MulU,
        MulS,
        Or,
        And,
        Xor,
        Shl,
        Shr,
        Sar,
        CmpEQ,
        CmpNE,
        CmpLTS,
        CmpLTU,
        CmpLES,
        CmpLEU,
        Not,
        /* floating point conversions */
        CF32toF64,
        /* narrowing onversions */
        C64to8,
        C32to8,
        C64to16,
        C64LOto32, //the low part
        C64HIto32, //the high part
        C32LOto16,
        C32HIto16,
        C16LOto8,
        C16HIto8,
        C16HLto32,
        /* widening conversions */
        C1Uto32,
        C1Uto8,
        C8Uto32,
        C8Sto32,
        C8Uto16,
        C8Sto16,
        C8Uto64,
        C16Uto64,
        C16Uto32,
        C16Sto32,
        C32Uto64,
        C32Sto64,
        C32HLto64,
        /* some 1 bit ops */
        C32to1,
        C64to1,
        /* CRAZY */
        DivModS64to32,
        DivModU64to32,
        /* CRAZIER */
        Sad8Ux4,
        Add8x8,
        Add16x4,
        Add32x2,
        Add64x1,
        QAdd8Sx8,
        QAdd16Sx4,
        QAdd32Sx2,
        QAdd64Sx1,
        QAdd8Ux8,
        QAdd16Ux4,
        QAdd32Ux2,
        QAdd64Ux1,
        Sub8x8,
        Sub16x4,
        Sub32x2,
        QSub8Sx8,
        QSub16Sx4,
        QSub32Sx2,
        QSub64Sx1,
        QSub8Ux8,
        QSub16Ux4,
        QSub32Ux2,
        QSub64Ux1,
        CmpEQ8x8,
        CmpEQ16x4,
        CmpEQ32x2,
        CmpGT8Ux8,
        CmpGT16Ux4,
        CmpGT32Ux2,
        CmpGT8Sx8,
        CmpGT16Sx4,
        CmpGT32Sx2,
        ShlN8x8,
        ShlN16x4,
        ShlN32x2,
        ShrN8x8,
        ShrN16x4,
        ShrN32x2,
        SarN8x8,
        SarN16x4,
        SarN32x2,
        Mul8x8,
        Mul16x4,
        Mul32x2,
        Mul32Fx2,
        MulHi16Ux4,
        MulHi16Sx4,
        PolyMul8x8,
        InterleaveHI8x8, 
        InterleaveHI16x4, 
        InterleaveHI32x2,
        InterleaveLO8x8, 
        InterleaveLO16x4, 
        InterleaveLO32x2,
        InterleaveOddLanes8x8, 
        InterleaveEvenLanes8x8,
        InterleaveOddLanes16x4, 
        InterleaveEvenLanes16x4,
        Abs8x16, 
        Abs16x8, 
        Abs32x4,
        Avg8Ux16, 
        Avg16Ux8, 
        Avg32Ux4,
        Avg8Sx16, 
        Avg16Sx8, 
        Avg32Sx4,
        Max8Sx16, 
        Max16Sx8, 
        Max32Sx4,
        Max8Ux16, 
        Max16Ux8, 
        Max32Ux4,
        Min8Sx16, 
        Min16Sx8, 
        Min32Sx4,
        Min8Ux16, 
        Min16Ux8, 
        Min32Ux4,
        Min8Ux8, 
        Min16Ux4, 
        Min32Ux2,
        Max8Sx8, 
        Max16Sx4, 
        Max32Sx2,
        Max8Ux8, 
        Max16Ux4, 
        Max32Ux2,
        Min8Sx8, 
        Min16Sx4, 
        Min32Sx2,
        QNarrow16Ux4,
        QNarrow16Sx4,
        QNarrow32Sx2,
        UNSUP
    };

protected:
    Ops     op;
public:
    Op(void) : op(UNSUP) { } 
    Op(Ops o) : op(o) { }

	virtual std::string printOp(void);
    Ops getOp(void) { return this->op; }
};

//////////////////////////////////////////////////////////////////////////
// Expression class and sub-classes
//////////////////////////////////////////////////////////////////////////

/* base class for Expression */
class Expression : public boost::enable_shared_from_this<Expression> {
public:
	Expression(void) { return; }
	virtual ~Expression(){ return; }
	virtual std::string printExpr(void) { return ""; }
	virtual bool isECond() { return false; }
	virtual bool isEConst() { return false; }
	virtual bool containsLoad(void) { return false; }
	virtual unsigned long getWidth(void) { assert(false && "Does not support width!"); return 0; }
};

struct ConstantValue {
    enum ValTy {
		T_INVALID,
		T_I1, 
		T_I8, 
		T_I16, 
		T_I32, 
		T_I64,
		T_I128,  /* 128-bit scalar */
		T_F32,   /* IEEE 754 float */
		T_F64,   /* IEEE 754 double */
		T_F128,  /* 128-bit floating point; implementation defined */
		T_V128,   /* 128-bit SIMD */
	};

    bool    valueIsKnown;

    union {
        boost::uint8_t      U1;
        boost::uint8_t      U8;
        boost::uint16_t     U16;
        boost::uint32_t     U32;
        boost::uint64_t     U64;
        float				F32;
		boost::uint32_t     F32i;
		double				F64;
		boost::uint64_t     F64i;
		boost::uint16_t		V128;
    };

    ValTy   valueType;
    int     width;

    bool operator==(const ConstantValue &other) const {
        if( !this->valueIsKnown && !other.valueIsKnown ) {
            return true;
        } else if( this->valueIsKnown != other.valueIsKnown) {
            return false;
        } else if( this->width != other.width) {
            return false;
        } else {
            switch(this->width) {
                case 1:
                    return this->U1 == other.U1;
                    break;

                case 8:
                    return this->U8 == other.U8;
                    break;

                case 16:
                    return this->U16 == other.U16;
                    break;

                case 32:
                    return this->U32 == other.U32;
                    break;

                case 64:
                    return this->U64 == other.U64;
                    break;
                default:
                    return false;
            }
        }
    }

    template <class T>
    T getValue(void) { 
        switch(this->valueType) {
            case T_I128:
            case T_F32:
            case T_F64:
            case T_F128:
            case T_V128:
            case T_INVALID:
                return 0;
                break;
            case T_I1:
                return this->U1;
                break;
            case T_I8:
                return this->U8;
                break;
            case T_I16:
                return this->U16;
                break;
            case T_I32:
                return this->U32;
                break;
            case T_I64:
                return this->U64;
                break; 
        }
    }

    template <class T>
    void setValue(T val) {
        this->valueIsKnown = true;
        switch(this->valueType) {
            case T_I128:
            case T_F32:
            case T_F64:
            case T_F128:
            case T_V128:
            case T_INVALID:
                this->valueIsKnown = false;
                break;
            case T_I1:
                this->U1 = val;
                break;
            case T_I8:
                this->U8 = val;
                break;
            case T_I16:
                this->U16 = val;
                break;
            case T_I32:
                this->U32 = val;
                break;
            case T_I64:
                this->U64 = val;
                break; 
        } 
    }
};

class ExConst : public Expression {
protected:
    ConstantValue   cval;
public:
    ExConst(void) { this->cval.valueIsKnown = false; }
    ExConst(ConstantValue c) : cval(c) { }

	ConstantValue getVal(void) { return this->cval; }
	virtual ConstantValue::ValTy getTy(void) { return this->cval.valueType; }
    virtual std::string printExpr(void);
};

typedef boost::shared_ptr<ExConst>  ExConstPtr;

class TempVal : public ExConst {
private:
	int             varIndex;
	ExpressionPtr   creator;
    std::string     varName;

public:
    TempVal(void);

	TempVal(int index, int width, ConstantValue::ValTy ty, ExpressionPtr creator);
	TempVal(int index, int width, ConstantValue::ValTy ty);

    TempVal(std::string v) : varName(v),varIndex(-1) { }

	ExpressionPtr getCreator() { return this->creator; }
	void setCreator(ExpressionPtr e) { this->creator = e; }
	int getVarIndex() { return this->varIndex; }
	virtual unsigned long getWidth() { return this->cval.width; }
    virtual bool isEConst() { return this->cval.valueIsKnown; }
    void setValue(ConstantValue cv) { this->cval = cv; }

	std::string printTemp(void);
};

struct RegArray {
    boost::int32_t              base;
    ConstantValue::ValTy        ty;
    boost::int32_t              numElems;
};

class ExGet : public Expression {
protected:
	ExpressionPtr   varOffset;
	TargetArch	    arch;
	Register	    sourceRegister;
    RegArray        rArr;
    boost::int32_t  bias;
    int             guestOffset;
public:
    ExGet(void) { 
        this->sourceRegister.width = 0;
    }

    ExGet(Register reg) : sourceRegister(reg),arch(reg.arch) { }
    ExGet(RegArray ra, ExpressionPtr v, boost::int32_t b, TargetArch t) :
        rArr(ra), 
        varOffset(v),
        bias(b),
        arch(t) { }

    virtual std::string printExpr(void);

    ExpressionPtr getVarPart(void) { return this->varOffset; }
    RegArray getRegArray(void) { return this->rArr; }
    boost::int32_t getBias(void) { return this->bias; }
    Register getSrcReg(void) { return this->sourceRegister; }
    TargetArch getArch(void) { return this->arch; }

    int getOff(void) { return this->guestOffset; }
    void setOff(int k) { this->guestOffset = k; return; }
};

typedef boost::shared_ptr<ExGet>    ExGetPtr;

class ExRdTmp : public Expression {
protected:
	TempValPtr  TVal;
public:
    ExRdTmp(void) : TVal(TempValPtr()) { }
    ExRdTmp(TempValPtr p) : TVal(p) { }

    TempValPtr getTmp(void) { return this->TVal; }

    virtual std::string printExpr(void);
};

typedef boost::shared_ptr<ExRdTmp>  ExRdTmpPtr;

class ExOp : public Expression {
protected:
	std::vector<ExpressionPtr>	exps;
	OpPtr                       op;
public:
    ExOp(void) { }
    ExOp(OpPtr o, std::vector<ExpressionPtr> args) : exps(args), op(o) { }

	unsigned long getExpArgCount() { return this->exps.size(); }
	
    OpPtr getOp(void) { return this->op; }

    virtual std::string printExpr(void);

    std::vector<ExpressionPtr>  getArgs(void) { return this->exps; }
};

typedef boost::shared_ptr<ExOp> ExOpPtr;

class ExLoad : public Expression {
protected:
	ConstantValue::ValTy    loadTy;
	ExpressionPtr           loadAddr;
public:
    ExLoad(void) { }
    ExLoad(ExpressionPtr addr, ConstantValue::ValTy t) : loadTy(t), loadAddr(addr) { }

    ExpressionPtr   getAddr(void) { return this->loadAddr; }
    ConstantValue::ValTy getTy(void) { return this->loadTy; }

    virtual std::string printExpr(void);
};

typedef boost::shared_ptr<ExLoad>   ExLoadPtr;

class ExMux0X : public Expression {
protected:
	ExpressionPtr   condition;
	ExpressionPtr   expTrue;
	ExpressionPtr   expFalse;
public:
    ExMux0X(void) { }

    ExMux0X(ExpressionPtr c, ExpressionPtr t, ExpressionPtr f) :
        condition(c), expTrue(t), expFalse(f) { }

    ExpressionPtr getCondition(void) { return this->condition; }
    ExpressionPtr getTrue(void) { return this->expTrue; }
    ExpressionPtr getFalse(void) { return this->expFalse; }

    virtual std::string printExpr(void);
};

typedef boost::shared_ptr<ExMux0X>  ExMux0XPtr;

/* we're probably going to need to find a way to remove this 
 * entire expression class from the IR, or at least, 
 * special-case enum all the helpers that VEX has
 */
class ExCCall : public Expression {
protected:
	std::vector<ExpressionPtr>	Args;
	std::string					TargetFunc;
public:
    ExCCall(void) { }
    ExCCall(std::vector<ExpressionPtr> a, std::string f) : Args(a), TargetFunc(f) { }

    std::string getTarget(void) { return this->TargetFunc; }
    std::vector<ExpressionPtr> getArgs(void) { return this->Args; }
    
    virtual std::string printExpr(void);
};

typedef boost::shared_ptr<ExCCall> ExCCallPtr;

class ExLogic : public Expression { 
public:
    enum LogicType {
        And,
        Or,
        Eq,
        Range,
        LessThan,
        GreaterThan
    };
private:

    LogicType       ty;
    ExpressionPtr   lhs;
    ExpressionPtr   rhs;
    boost::uint32_t lower;
    boost::uint32_t upper;

public:

    ExLogic(LogicType t, ExpressionPtr l, ExpressionPtr r) : lhs(l), rhs(r), ty(t) { }

    ExLogic(boost::uint32_t l, boost::uint32_t g) : lower(l), upper(g), ty(Range) { } 

    LogicType getType(void) { return this->ty; }

    virtual std::string printExpr(void);

    ExpressionPtr getLhs(void) { return this->lhs; }
    ExpressionPtr getRhs(void) { return this->rhs; }
    boost::uint32_t getLower(void) { return this->lower; } 
    boost::uint32_t getUpper(void) { return this->upper; }
};

typedef boost::shared_ptr<ExLogic>  ExLogicPtr;

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//Statement class and sub-classes
//////////////////////////////////////////////////////////////////////////

class Statement {
private:
	std::vector<ExpressionPtr>	expressions;
public:
	Statement() { return; }
	virtual ~Statement();
	virtual std::string printStmt(void) { return ""; }
	virtual void updateState(void) { return; }
	virtual bool hasLoad(void) { return false; }
    virtual bool contains(ExpressionPtr e) { return false; };
};

class StIMark : public Statement { };

typedef boost::shared_ptr<StIMark>  StIMarkPtr;

class StWrTmp : public Statement {
protected:
	ExpressionPtr   RHS;
	TempValPtr      tmpWritten;
public:
    StWrTmp(void) { }

    StWrTmp(TempValPtr t, ExpressionPtr r) : RHS(r), tmpWritten(t) { }

	ExpressionPtr getRHS() { return this->RHS; }
    TempValPtr getTarget() { return this->tmpWritten; }
    virtual bool contains(ExpressionPtr e) { return this->RHS == e; }

    virtual std::string printStmt(void);
};

typedef boost::shared_ptr<StWrTmp>  StWrTmpPtr;

class StHold : public Statement {
private:

    ExpressionPtr   condition;

public:
    StHold(void) { } 
    StHold(ExpressionPtr cond) : condition(cond) { } 

    ExpressionPtr getCond(void) { return this->condition; }

    virtual std::string printStmt(void) {
        std::string r = "";

        r.append("HOLD = ");
        r.append(this->condition->printExpr());

        return r;
    }

};

class StPut : public Statement {
protected:
    int             guestOffset;
	boost::int32_t  bias;
	ExpressionPtr   data;
	ExpressionPtr   varpart;
	TargetArch		arch;
	Register		targetRegister;
    RegArray        rArr;
public:
    StPut(void) { }
    StPut(Register r, ExpressionPtr d) : arch(r.arch), targetRegister(r), data(d) { }

    StPut(RegArray r, ExpressionPtr d, ExpressionPtr v, boost::int32_t b) :
        rArr(r),
        data(d),
        varpart(v),
        bias(b) { }

    ExpressionPtr getData(void) { return this->data; }
    ExpressionPtr getVarPart(void) { return this->varpart; }
    RegArray getRegArray(void) { return this->rArr; }
    boost::int32_t getBias(void) { return this->bias; }

    int getGuestOff(void) { return this->guestOffset; }

    Register getDstRegister(void) { return this->targetRegister; }

    virtual bool contains(ExpressionPtr e) { return this->data == e; }

    virtual std::string printStmt(void);
};

typedef boost::shared_ptr<StPut>    StPutPtr;

class StStore : public Statement {
protected:
	ExpressionPtr   data;
	ExpressionPtr   addr;
public:
    StStore() { }

    StStore(ExpressionPtr lhs, ExpressionPtr rhs) : addr(lhs), data(rhs) { }

    ExpressionPtr getData(void) { return this->data; }
    ExpressionPtr getAddr(void) { return this->addr; }

    virtual bool contains(ExpressionPtr e) { return ( (this->data == e ) || ( this->addr == e) ); }

	virtual std::string printStmt(void);
};

typedef boost::shared_ptr<StStore>  StStorePtr;

class StNop : public Statement {
public:
	StNop(void) { return; }

	virtual std::string printStmt(void);

    virtual bool contains(ExpressionPtr e) { return false; }
};

typedef boost::shared_ptr<StNop>    StNopPtr;

class StAbiHint : public Statement {
protected:
	unsigned long	len;
	ExpressionPtr   base;
	ExpressionPtr   nia;
public:
    virtual bool contains(ExpressionPtr e) { return ((false) || (false) ); }
};

typedef boost::shared_ptr<StAbiHint>    StAbiHintPtr;

class StCAS : public Statement {
protected:
	ExpressionPtr   StoreAddress;
	ExpressionPtr   DataHi;
	ExpressionPtr   DataLo;
	ExpressionPtr   ExpectedHi;
	ExpressionPtr   ExpectedLo;
    TempValPtr      OldHi;
    TempValPtr      OldLo;
    MemoryEnd       endian;
public:
    StCAS(void) { }
    StCAS(  ExpressionPtr   sa,
            ExpressionPtr   dh,
            ExpressionPtr   dl,
            ExpressionPtr   eh,
            ExpressionPtr   el,
            TempValPtr      oh,
            TempValPtr      ol,
            MemoryEnd       e) :
        StoreAddress(sa),
        DataHi(dh),
        DataLo(dl),
        ExpectedHi(eh),
        ExpectedLo(el),
        OldHi(oh),
        OldLo(ol),
        endian(e) { }
    ExpressionPtr   getStoreAddress(void) { return this->StoreAddress; }
    ExpressionPtr   getDataHi(void) { return this->DataHi; }
    ExpressionPtr   getDataLo(void) { return this->DataLo; }
    ExpressionPtr   getExpectedHi(void) { return this->ExpectedHi; }
    ExpressionPtr   getExpectedLo(void) { return this->ExpectedLo; }
    TempValPtr      getOldHi(void) { return this->OldHi; }
    TempValPtr      getOldLo(void) { return this->OldLo; }
    MemoryEnd       getEndian(void) { return this->endian; }

    virtual bool contains(ExpressionPtr e) {
        return ((this->StoreAddress == e ) ||
                (this->DataHi == e) ||
                (this->DataLo == e) ||
                (this->ExpectedHi == e) ||
                (this->ExpectedLo == e) ||
                (this->OldHi == e) ||
                (this->OldLo == e)
                );
    }

};

typedef boost::shared_ptr<StCAS>    StCASPtr;

class StLLSC : public Statement {
protected:
	ExpressionPtr   addr;
	ExpressionPtr   storeData;
    TempValPtr      result;
    MemoryEnd       endian;
public:
    StLLSC(void) { } 
    StLLSC(ExpressionPtr a, ExpressionPtr d, TempValPtr r, MemoryEnd e) :
        addr(a),
        storeData(d),
        result(r),
        endian(e) { }

    virtual bool contains(ExpressionPtr e) { 
        return  ((this->addr == e) ||
                 (this->storeData == e)
                );
    }

    ExpressionPtr   getAddr(void) { return this->addr; }
    ExpressionPtr   getStoreData(void) { return this->storeData; }
    TempValPtr      getResult(void) { return this->result; }
};

typedef boost::shared_ptr<StLLSC>   StLLSCPtr;

class StDirty : public Statement {
protected:
	ExpressionPtr               Addr;
	ExpressionPtr               Guard;
	std::vector<ExpressionPtr>	Args;
	std::string					CalleeName;
    TempValPtr                  tmp;
public:    
    StDirty(void) { } 
    StDirty(ExpressionPtr a,
            ExpressionPtr g,
            std::vector<ExpressionPtr>  args,
            std::string                 tgt,
            TempValPtr                  t) :
                Addr(a),
                Guard(g),
                Args(args),
                CalleeName(tgt),
                tmp(t) { }
    
    virtual bool contains(ExpressionPtr e) {
        if( this->Addr == e ||
            this->Guard == e )
        {
            return true;
        } 
       
        std::vector<ExpressionPtr>::iterator it;
        it = this->Args.begin();
        while( it != this->Args.end() ) {
            ExpressionPtr t = *it;
            if( t == e ) {
                return true;
            }
            ++it;
        }
        return false;
    }

    std::string getTarget(void) { return this->CalleeName; }

    ExpressionPtr   getAddr(void) { return this->Addr; }
    ExpressionPtr   getGuard(void) { return this->Guard; }

    std::vector<ExpressionPtr>  getArgs(void) { return this->Args; }

    TempValPtr  getTmp(void) { return this->tmp; }

    virtual std::string printStmt(void);
};

typedef boost::shared_ptr<StDirty>  StDirtyPtr;

class StMBE : public Statement {
public:

	virtual ~StMBE() { }

    virtual bool contains(ExpressionPtr e) { return false; }

    virtual std::string printStmt(void);
};

typedef boost::shared_ptr<StMBE>    StMBEPtr;

class StExit : public Statement {
protected:
    ExitType        blockExit;
	ExpressionPtr   GuardExp;
	ExConstPtr      jmpTarget;
public:
    StExit(void) { } 
    StExit(ExpressionPtr g, ExConstPtr e, ExitType t) : 
        blockExit(t), 
        GuardExp(g),
        jmpTarget(e) { } 
	virtual ~StExit() { }

	ExitType getJmpKind(void) { return this->blockExit; }
	ExpressionPtr getCondition(void) { return this->GuardExp; }
	ExConstPtr getTarget(void) { return this->jmpTarget; }

    virtual bool contains(ExpressionPtr e) { return ((this->GuardExp==e) ||
                                                    (this->jmpTarget == e));}
};

typedef boost::shared_ptr<StExit>   StExitPtr;

//////////////////////////////////////////////////////////////////////////

class Flow {
private:
	std::set<BlockPtr>				blocks;
	std::set<unsigned long long>	ConstantCalls;
	std::set<ExpressionPtr>			Calls;
public:
	Flow(void);

	std::set<unsigned long long>	getConstantCalls(void);
	std::set<ExpressionPtr>			getCalls(void);
	bool addBlockToFlow(BlockPtr b);
	bool isAddrInFlowCode(unsigned long long addr);
	std::string dumpFlowToProto(void);
	void complete(void);
	const std::set<BlockPtr>	getBlocks(void);
	BlockPtr getBlockWithAddr(unsigned long long addr);
	std::string printFlow(void);
};

ConstantValue::ValTy getITyFromWidth(boost::uint16_t width);

std::string regToStr(Register, TargetArch);

#include "Helpers.h"
#include "IRMatcher.h"
#include "Serial.h"

#endif
