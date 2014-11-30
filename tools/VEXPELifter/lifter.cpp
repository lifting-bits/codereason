#include <BasicIR.h>
#include <decodeLib.h>
#include <iostream>
#include <list>
#include <stdio.h>

#include "getopt.h"

#include <PeLib.h>

#define GETOPT_FMT "p:"

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


template <int bits>
bool getFlows(const PeLib::PeHeaderT<bits> &p, vector<FlowPtr> fps, string fn) {
    /*unsigned long len;
    unsigned char *mbuf = memoryMapFile(fn, &len); 

    if( mbuf == NULL ) {
        return false;
    }

    TargetArch  arch;

    switch(bits) {
        case 32:
            arch = X86;
            break;
        case 64:
            arch = AMD64;
            break;
    }
    
    TargetInfo  ti;
    if( arch == X86 ) {
        ti.guestHWcaps =
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
    } else {
        ti.guestHWcaps =
            VEX_HWCAPS_AMD64_SSE3|VEX_HWCAPS_AMD64_CX16|VEX_HWCAPS_AMD64_LZCNT;
        }
    ti.hostHWcaps =
        VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;

    ti.maxInstructions = 99;
    ti.chaseThreshold = 10;
    ti.opLevel = FullOpt;
    ti.tarch = arch;

    void *dcctx = initDecodeLib(ti, true, true);

    //okay, we now have most of the information we need
    list<uint32_t>  offsetsToRender;

    //push the entry points offsets into the set
    PeLib::dword entryRVA = p.getAddressOfEntryPoint();
    PeLib::dword entryOff = p.rvaToOffset(entryRVA);
    offsetsToRender.push_front(entryOff);

    //go until this set is empty
    while( !offsetsToRender.empty() ) {
        uint32_t off = offsetsToRender.front();
        offsetsToRender.pop_front();

        //make a flow for this offset
        FlowPtr newFlow = FlowPtr(new Flow());
        list<uint32_t> offsForFlow;

        offsForFlow.push_front(off);

        cout << "Decoding flow starting at offset ";
        cout << off << " with VA " << hex << p.offsetToVa(off) << dec << endl;

        while( !offsForFlow.empty() ) {
            uint32_t o = offsForFlow.front();
            offsForFlow.pop_front();

            PeLib::dword va = p.offsetToVa(o);
            cout << "Decoding block at VA " << hex << va << dec << endl;
           
            assert(o < len);
            BlockPtr b;
            //make a block
            bool r;
            r = convertToOneBlock(  dcctx,
                                    mbuf+o,
                                    len-o,
                                    va,
                                    arch,
                                    b);

            if( !r ) {
                break;
            }
            
            //okay, we have a block, add it to our current flow
            newFlow->addBlockToFlow(b);

            //ask this block how it leaves, and add those targets to offsets
            //to decode
            set<uint64_t> targets = b->getHardTargets();
            set<uint64_t>::const_iterator it = targets.begin();

            while( it != targets.end() ) {
                //this is a VA
                unsigned long long k = *it;
                offsForFlow.push_front(p.vaToOffset(k));

                ++it;
            }
        }

        //okay, we got a flow, now, ask the flow for its call targets

    }

    finiDecodeLib(dcctx);
    unMapFile(mbuf);*/
    return true;
}

int main(int argc, char *argv[]) {
    string  fileName;

    do {
        int c = getopt(argc, argv, GETOPT_FMT);

        if( c == EOF ) {
            break;
        }

        switch(c) {
            case 'p':
                fileName = string(optarg);
                break;
        }

    } while(true);

    if( fileName.size() > 0 ) {
        PeLib::PeFile   *pf = PeLib::openPeFile(fileName);

        pf->readMzHeader();
        pf->readPeHeader();
        
        vector<FlowPtr> flows;

        switch( pf->getBits() ) {
            case 32:
                {
                    const PeLib::PeHeaderT<32>& peh =
                        static_cast<PeLib::PeFileT<32>&>(*pf).peHeader();
                    getFlows<32>(peh, flows, fileName);
                }
                break;
            
            case 64:
                {
                    const PeLib::PeHeaderT<64>& peh = 
                        static_cast<PeLib::PeFileT<64>&>(*pf).peHeader();
                    getFlows<64>(peh, flows, fileName);
                }
                break;
        }
    }

    return 0;
}
