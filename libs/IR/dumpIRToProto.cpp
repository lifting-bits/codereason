/*#include "BasicIR.h"

#include "DisFmt.pb.h"

std::string Block::dumpBlockToProto(void) {
	return "";
}

DisFmt::Block
convertBlockToProto(Block *b) {
	DisFmt::Block	protoB = DisFmt::Block();

	protoB.set_blockid(b->getBlockId());
	protoB.set_blockstart(b->getBlockBase());
	protoB.set_blockend(b->getBlockEnd());

	return protoB;
}

std::string Flow::dumpFlowToProto(void) {
	DisFmt::Flow	protoF = DisFmt::Flow();
	//convert all the blocks to proto format
	std::set<Block *>::iterator	it = this->m_blocks.begin();
	std::set<Block *>::iterator	e = this->m_blocks.end();
	while( it != e ) { 
		Block	*b = *it;
		
		++it;
	}

	return protoF.SerializeAsString();
}*/
#include "BasicIR.h"

std::string Flow::dumpFlowToProto(void) {
	return "";
}