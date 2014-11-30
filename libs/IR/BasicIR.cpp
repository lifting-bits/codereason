#include "BasicIR.h"

using namespace boost;

Statement::~Statement() {

}

TempVal::TempVal(void) {
    this->varIndex = -1;
    this->cval.valueIsKnown = false;
    return;
}

TempVal::TempVal(int index, int width, ConstantValue::ValTy ty) {
    assert(width != 0);
	this->varIndex = index;
    this->cval.valueType = ty;
    this->cval.width = width;
    this->cval.valueIsKnown = false;
	return;
}

TempVal::TempVal(int index, int width, ConstantValue::ValTy ty, ExpressionPtr creator) {
    assert(width != 0);
	this->varIndex = index;
    this->cval.valueType = ty;
    this->cval.width = width;
	this->creator = creator;
    this->cval.valueIsKnown = false;
	return;
}

bool Block::overlaps(BlockPtr other) {
	return (this->overlaps(other->BlockAddrBegin) || this->overlaps(other->BlockAddrEnd));
}

bool Block::overlaps(unsigned long long addr) {
	bool doesOverlap = false;

	if( this->BlockAddrBegin <= addr && this->BlockAddrEnd > addr )
		doesOverlap = true;

	return doesOverlap;
}

void Block::resolveTargets(FlowPtr f) {
	//iterate over the hard targets of this block
	std::set<uint64_t>::iterator	it = this->HardTargets.begin();
	std::set<uint64_t>::iterator	e = this->HardTargets.end();
	
	while( it != e ) {
		unsigned long long target = *it;
		//resolve each hard target to a block in the flow
		BlockPtr b = f->getBlockWithAddr(target);

		//insert this blocks ID into the list of targeted IDs
		this->TargetIDs.push_back(b->getBlockId());

		++it;
	}
	return;
}

unsigned long long Block::getBlockLen(void) {
    return (this->getBlockEnd() - this->getBlockBase());
}

Flow::Flow(void) {
	return;
}

bool Flow::isAddrInFlowCode(unsigned long long addr) {
	bool found = false;
	std::set<BlockPtr>::iterator	it = this->blocks.begin();
	std::set<BlockPtr>::iterator e = this->blocks.end();

	while( it != e ) {
		BlockPtr    b = *it;

		//if( b->overlaps(addr) ) {
        if( b->getBlockBase() == addr ) {
			found = true;
			break;
		}

		++it;
	}

	return found;
}

BlockPtr Flow::getBlockWithAddr(unsigned long long addr) {
	BlockPtr b;
	std::set<BlockPtr>::iterator	it = this->blocks.begin();
	std::set<BlockPtr>::iterator	e = this->blocks.end();

	while( it != e ) {
		BlockPtr n = *it;

		if( n->getBlockBase() <= addr && n->getBlockEnd() > addr ) {
			b = n;
			break;
		}
		++it;
	}

	return b;
}

bool Flow::addBlockToFlow(BlockPtr b) {
	this->blocks.insert(b);
	return true;
}

//so after we have added all the flow targets we can find, we can 
//go through the blocks and resolve their targets to block IDs
//we can also do some other things like dealing with block split / merge
void Flow::complete(void) {/*
	std::set<Block *>::iterator	it = this->blocks.begin();
	std::set<Block *>::iterator	e = this->blocks.end();

	while( it != e ) {
		Block	*b = *it;

		b->resolveTargets(this);

		++it;
	}*/

	return;
}

ConstantValue::ValTy getITyFromWidth(uint16_t width) {
    ConstantValue::ValTy    ty;

    switch(width) {
        case 1:
            ty = ConstantValue::T_I1;
            break;
        case 8:
            ty = ConstantValue::T_I8;
            break;
        case 16:
            ty = ConstantValue::T_I16;
            break;
        case 32:
            ty = ConstantValue::T_I32;
            break;
        case 64:
            ty = ConstantValue::T_I64;
            break;
        default:
            assert(!"impossible");
    }

    return ty;
}

const std::set<BlockPtr> Flow::getBlocks(void) {
	return this->blocks;
}

std::string Block::printTransfer(Transfer t) {
    std::string rem = "";
    SinkType        sink = t.first;
    ExpressionPtr   e = t.second;

    if( sink.isReg() ) {
        Register    r = sink.getReg();
        rem = rem + regToStr(r, this->getArch());
        rem = rem + " = ";
        rem = rem + e->printExpr();
    } else if( sink.isMem() ) {
        ExpressionPtr   a = sink.getMem(); 
        
        rem = rem + a->printExpr();
        rem = rem + " = ";
        rem = rem + e->printExpr();
    }

    return rem;
}
