#include "blobeater.h"

extern "C" {
#include <libvex_basictypes.h>
#include <libvex_guest_offsets.h>
#include <libvex_emwarn.h>
#include <libvex.h>
}

#define GETOPT_FMT "po:lf:hj:"

using namespace std;

unsigned char *memoryMapFile(string file, unsigned long *fileLen) {
    FILE    *f = fopen(file.c_str(), "rb");

    assert(f != NULL);
    //get size of f
    fseek( f, 0, SEEK_END );
    unsigned long size = ftell( f );
    fseek(f, 0, SEEK_SET );
    //alloc buffer for data
    unsigned char *buf = (unsigned char *) malloc(size);

    assert( buf != NULL );

    //read all data out

    size_t read = fread(buf, sizeof(unsigned char), size, f);

    assert( read == size );

    fclose(f);
    *fileLen = size;
    return buf;
}

void unMapFile(unsigned char *p) {
    free(p);
	return;
}

void eatBlob(unsigned char *blob, unsigned long blobLen, void *ctx) {
	vector<BlockPtr>	blocks;

    std::cout << "eating blob" << std::endl;
	if( convertBlobToBlocks(ctx, blob, blobLen, blocks) ) {
		//print out the block
		std::vector<BlockPtr>::iterator	it = blocks.begin();
		std::vector<BlockPtr>::iterator	e = blocks.end();

		std::cout << "Dumping blocks" << std::endl;
		while( it != e ) {
			BlockPtr    b = *it;
			//std::cout << "Block at base " << std::hex << b->getBlockBase() << std::dec << std::endl;
			std::cout << b->printBlock() << std::endl;

			++it;
		}
	}
	return;
}

#if 0
void decodeFlowFromFile(string fileName, void *ctx, unsigned long offt, string *outProtoFile) {
	unsigned long fileLen = 0;

	cout << "decoding flow from " << fileName << endl;
	unsigned char *file = (unsigned char *) memoryMapFile(fileName, &fileLen);
	if( file ) {
		Flow *f;
		bool r = convertToFlow(ctx, file, fileLen, offt, 0x1000, &f);
		if( r ) {
			cout << "Flow decode success" << endl;
			cout << f->printFlow() << endl;
			if( outProtoFile && outProtoFile->length() > 0) {
				//write information about these blocks out to a protocol buffer
				//std::string	s = f->dumpFlowToProto();
				
			}
		} else {
			cout << "Flow decode failure" << endl;
		}
		unMapFile(file);
	}

	return;
}
#endif

void eatFile(string fileName, void *ctx) {
	unsigned long blobLen = 0;
	cout << "eating file " << fileName << endl;
	//open and memory map this file
	unsigned char *mappedBlob = 
        (unsigned char *) memoryMapFile(fileName, &blobLen);

	if( mappedBlob ) {
		eatBlob(mappedBlob, blobLen, ctx);
		unMapFile(mappedBlob);
	} else {
        std::cout << "failure to map file" << std::endl;
    }
	return;
}

void usage(void) {
	cout << "USAGE: [program] -f [blob file to eat]" << endl;
	return;
}

int main(int argc, char *argv[]) {
	//parse command line options
	int c;
	string	*inFile=NULL;
	bool	needHelp=false;
	void    *dlctx;
	unsigned long ePointOffset = 0;
	bool decodeFlow=false;
	bool paranoid=false;
	string *outProtoFile=NULL;

	while( 1 ) {
		c = getopt(argc, argv, GETOPT_FMT );

		if( c == EOF ) {
			break;
		}

		switch(c) {
			case 'f':
				inFile = new string(optarg);
				break;
			case 'h':
				needHelp = true;
				break;
			case 'l':
				decodeFlow = true;
				break;
			case 'o':
				//TODO, get offset from optarg
				break;
			case 'p':
				paranoid=true;
				break;
			case 'j':
				outProtoFile = new string(optarg);
				break;
			default:
				needHelp = true;
		}
	}

	//if we passed in -h, display help
	if( needHelp ) {
		usage();
		return -1;
	}

	TargetInfo	ti;
#ifdef __i386__
	ti.hostHWcaps = VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
#else
	ti.hostHWcaps = VEX_HWCAPS_AMD64_SSE3|VEX_HWCAPS_AMD64_CX16|VEX_HWCAPS_AMD64_LZCNT;
#endif
	if(paranoid) {
		ti.maxInstructions = 1;
		ti.chaseThreshold = 0; //disable chasing, one ins at a time
	} else {
		ti.maxInstructions = 99;
		ti.chaseThreshold = 10;
	}
	ti.opLevel = FullOpt;
	ti.tarch.ta = X86;
	ti.guestHWcaps = VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
	
	dlctx = initDecodeLib(ti, false, true);

	//if we got a filename, eat that file
	if( inFile != NULL && inFile->length() > 0 && !decodeFlow ) {
		eatFile(*inFile, dlctx);
	} /*else if( inFile != NULL && inFile->length() > 0 && decodeFlow ) {
		decodeFlowFromFile(*inFile, dlctx, ePointOffset, outProtoFile);
	}*/

	if( inFile ) {
		delete inFile;
		inFile = NULL;
	}

	finiDecodeLib(dlctx);

	return 0;
}
