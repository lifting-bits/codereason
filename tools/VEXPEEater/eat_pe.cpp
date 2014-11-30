#include "eat_pe.h"

extern "C" {
#include <libvex_basictypes.h>
#include <libvex_guest_offsets.h>
#include <libvex_emwarn.h>
#include <libvex.h>
}

#define GETOPT_FMT "i:h"

void usage(void) {
	std::cout << "-i <input> file to input" << std::endl;
	std::cout << "-h help" << std::endl;
	return;
}

void walkAllExecutableSections( PeLib::PeFile *pEF ) {
	void	*dcctx;
	const PeLib::PeHeaderT<32>& peh = 
		static_cast<PeLib::PeFileT<32>&>(*pEF).peHeader();

	TargetArch	peArch;
    
    peArch.ta = INVALID;

	switch( pEF->getBits() ) {
		case 32:
			peArch.ta = X86;
			break;
		case 64:
			peArch.ta = AMD64;
			break;
	}

	assert( peArch.ta != INVALID );

	TargetInfo	ti;
	if( peArch.ta == X86 ) {
		ti.guestHWcaps = VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
	} else {
		ti.guestHWcaps = VEX_HWCAPS_AMD64_SSE3|VEX_HWCAPS_AMD64_CX16|VEX_HWCAPS_AMD64_LZCNT;
	}
	ti.maxInstructions = 99;
	ti.chaseThreshold = 10;
	ti.opLevel = FullOpt;
	ti.tarch = peArch;

	dcctx = initDecodeLib(ti, true, false);

	assert( dcctx != NULL );

	FILE *f = fopen(pEF->getFileName().c_str(), "rb");

	assert(f != NULL);

	//go through and find all sections that are marked +x
	PeLib::word		numSections = peh.calcNumberOfSections();
	PeLib::dword	off = peh.getBaseOfCode();

	std::list<BlockPtr>	blocksThatReturn;
	while(numSections > 0 ) {
		PeLib::word		curSecIdx = peh.getSectionWithOffset(off);
		std::string		secName = peh.getSectionName(curSecIdx);
		PeLib::dword	secLen = peh.getSizeOfRawData(curSecIdx);

		std::cout << "In section " << secName << std::endl;
		std::cout << "Section is " << secLen << std::endl;

		PeLib::dword	secChars = peh.getCharacteristics(curSecIdx);

		if( secChars & PeLib::PELIB_IMAGE_SCN_MEM_EXECUTE ) {
			std::cout << "This section is executable!" << std::endl;
			//iterate over this section
			unsigned char			*buf = 
				(unsigned char *)malloc(secLen);
			assert(buf != NULL);

			fseek(f, 0, SEEK_SET);
			fseek(f, off, SEEK_SET);

			//get the data at this offset
			size_t read = fread(buf, sizeof(unsigned char), secLen, f);

			assert( read == secLen );

			unsigned long curOff=0;
			do 
			{
				unsigned char *startHere = buf+curOff;
				
				BlockPtr    b;
				/*bool r = convertToOneBlock( dcctx, 
                                            startHere, 
                                            secLen-curOff, 
                                            peh.offsetToVa(off), 
                                            peArch, 
                                            b);
				if( r ) {
					if( b->getPostConditions()->returns() ) {
						blocksThatReturn.push_front(b);
					} 
				}*/
				curOff++;
			} while (curOff < secLen);

			free(buf);
			off += secLen;
		} else {
			std::cout << "Skipping non-executable section " << std::endl;
			off += secLen;;
		}

		numSections--;
	}

	std::cout << "Got " << blocksThatReturn.size() << " blocks that return" << std::endl;

	/*for( std::list<Block *>::iterator it = blocksThatReturn.begin(), e = blocksThatReturn.end();
		it != e;
		++it)
	{
		Block	*b = *it;
		delete b;
	}

	blocksThatReturn.clear();*/

	finiDecodeLib(dcctx);
	fclose(f);

	return;
}

int main(int argc, char *argv[]) {
	bool printHelp=true;
	std::string fileName;

	do 
	{
		int c = getopt(argc, argv, GETOPT_FMT);

		if( c == EOF ) {
			break;
		}

		if( c == 'i' ) {
			printHelp=false;
			fileName = std::string(optarg);
		}
	} while ( 1 );

	if(printHelp) {
		usage();
		return 0;
	}

	if( fileName.size() == 0 ) {
		usage();
		return 0;
	}

	PeLib::PeFile	*pEF = PeLib::openPeFile(fileName);

	int res = pEF->readMzHeader();
	//assert(res == 0);
	res = pEF->readPeHeader();
	//assert(res == 0);

	switch( pEF->getBits() ) {
		case 32:
			walkAllExecutableSections(pEF);
			break;
		case 64:
			{

			}
			break;
	}


	return 0;
}
