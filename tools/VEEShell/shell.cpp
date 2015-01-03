#include <VEE.h>
#include <decodeLib.h>
#include <PeLib.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <boost/regex.hpp>
#include <boost/cstdint.hpp>
#include <boost/program_options/config.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>

extern "C" {
#include <libvex_basictypes.h>
#include <libvex_guest_offsets.h>
#include <libvex_emwarn.h>
#include <libvex.h>
}

using namespace boost;
using namespace std;

BlockPtr getBlockFromOff(   void *ctx,
                            uint8_t     *buf,
                            uint32_t    len,
                            uint32_t    off,
                            uint32_t    va,
                            TargetArch  ta,
                            unsigned int  m);

class PeBlockProvider : public BlockProvider {
private:
    const PeLib::PeHeaderT<32> &peHeader;
    unsigned long peBufLen;
    unsigned char *peBuf;
    void *dcctx;
    unsigned int  m;
public:
    PeBlockProvider(const PeLib::PeHeaderT<32> &p, 
                    unsigned long len, 
                    unsigned char *buf,
                    void *c,
                    unsigned int m);

    virtual BlockPtr getNextBlock(uint64_t VA);
};

class BlankBlockProvider : public BlockProvider {
private:
public:
    BlankBlockProvider() { return; }
    virtual BlockPtr getNextBlock(uint64_t VA);
};

typedef boost::shared_ptr<PeBlockProvider> PeBlockProviderPtr;
typedef boost::shared_ptr<BlankBlockProvider> BlankBlockProviderPtr;

BlockPtr BlankBlockProvider::getNextBlock(uint64_t VA) {
    BlockPtr    b;

    return b;
}

PeBlockProvider::PeBlockProvider(   const PeLib::PeHeaderT<32>  &p,
                                    unsigned long               len,
                                    unsigned char               *buf,
                                    void                        *c,
                                    unsigned int                mi) : 
 peHeader(p),
 peBufLen(len),
 peBuf(buf),
 dcctx(c),
 m(mi)
{

    return;
}

BlockPtr PeBlockProvider::getNextBlock(uint64_t VA) {
    BlockPtr nextBlock;
    //okay pretty straightforward, get the offset
    unsigned long off = this->peHeader.vaToOffset(VA);

    if( off < this->peBufLen ) {
        //get the next block
        TargetArch  ta = {INVALID};

        ta.ta = X86;
        nextBlock = getBlockFromOff(    this->dcctx,
                                        this->peBuf,
                                        this->peBufLen,
                                        off,
                                        VA,
                                        ta,
                                        this->m);
    }

    return nextBlock;
}

string console(void) {
    return "> ";
}

std::string help(void) {
    std::string s = "";

    s = s + "q: quit\n";
    s = s + "h: help\n";
    s = s + "g: start running\n";
    s = s + "p: [off] [width] [value]: set register state\n";
    s = s + "u: print register state\n";
    s = s + "b: print block status\n";
    s = s + "m: dump mem state\n";
    s = s + "t [addr] [width] [value]: set mem state\n";
    s = s + "s: step\n";
    s = s + "r: print transfer functions for this block\n";
    s = s + "y: print out permutations of registers\n";
    return s;
}

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

void walkPE32(const PeLib::PeHeaderT<32> &p) {
    PeLib::word     numSections = p.calcNumberOfSections();
    PeLib::dword    off = p.getBaseOfCode();

    while( numSections > 0 ) {

        numSections--;
    }

    return;
}

BlockPtr getBlockFromOff(   void          *ctx, 
                            uint8_t       *buf, 
                            uint32_t      len,
                            uint32_t      off,
                            uint32_t      va,
                            TargetArch    ta,
                            unsigned int  m) 
{
    BlockPtr    b;
    BlockPtr    br;

    bool r = false; 
    r = convertToOneBlock( ctx, 
                            buf+off, 
                            len, 
                            va, 
                            ta,
                            m,
                            br);
    if( r ) {
        b = br;
    }

    return b;
}

string pehelp(void) {
    string s = "";
    s = s + "q: quit\n";
    s = s + "h: help\n";
    s = s + "p: print current off/VA\n";
    s = s + "a: wind to specified VA\n";
    s = s + "b: dump current block\n";
    s = s + "y: run block to end\n";
    s = s + "g: get block from next offset\n";
    s = s + "s: step current insn\n";
    s = s + "w: wind to specified offset\n";
    s = s + "r: run to end of section\n";
    s = s + "z: begin search from current offset/VA\n";
    return s;
}

void doWalkFromOff( void                        *ctx,
                    unsigned char               *buf, 
                    unsigned long               bufLen,
                    unsigned long               startOff,
                    const PeLib::PeHeaderT<32>  &peh,
                    vector<BlockPtr>            &blocksOut,
                    TargetArch                  ta,
                    unsigned int                m)
{
    unsigned long curOff = startOff;

    while( curOff < bufLen ) {
        cout << "block at " << hex << peh.offsetToVa(curOff) << dec << endl;
        BlockPtr b = getBlockFromOff(   ctx, 
                                        buf, 
                                        bufLen-curOff, 
                                        curOff, 
                                        peh.offsetToVa(curOff),
                                        ta,
                                        m);

        if( b ) {
            //okay, we have a block, add it
            blocksOut.push_back(b);
        }
        
        curOff++;
    }

    return;
}

void doPeMode(string filePath, Condition *cond, unsigned int m) {
    PeLib::PeFile   *pEF = PeLib::openPeFile(filePath);

    pEF->readMzHeader();
    pEF->readPeHeader();

    if( pEF->getBits() == 32 ) {
        //currently support 32 bit images
        const PeLib::PeHeaderT<32>& peh =
            static_cast<PeLib::PeFileT<32>&>(*pEF).peHeader();
        TargetArch  peArch = {INVALID};
        peArch.ta = X86;
        TargetInfo  ti;
        if( peArch.ta == X86 ) {
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
        ti.tarch = peArch;

        void *dcctx = initDecodeLib(ti, true, true);

        assert(dcctx != NULL);
        
        FILE *f = fopen(pEF->getFileName().c_str(), "rb");
        assert(f != NULL);

        //okay find the code section
        PeLib::word numSections = peh.calcNumberOfSections();
        PeLib::dword codeSec = peh.getBaseOfCode();
        PeLib::word of = peh.getSectionWithOffset(codeSec);
        string name = peh.getSectionName(of);
        PeLib::dword secLen = peh.getSizeOfRawData(of);
        PeLib::dword    secChars = peh.getCharacteristics(of);

        unsigned char *sec = (unsigned char *)malloc(secLen);
        assert(sec != NULL);

        fseek(f, of, SEEK_SET);

        size_t read = fread(sec, sizeof(unsigned char), secLen, f);
        unsigned long currentOff = 200;
        assert(read == secLen);
        unsigned long bytesLeft = secLen;

        cout << "have section " << name << endl;
        if( secChars & PeLib::PELIB_IMAGE_SCN_MEM_EXECUTE ) {
            cout << "section is +X" << endl;
        }

        //okay drive the shell for PE mode
        BlockPtr curBlock;

        PeBlockProviderPtr pep(new PeBlockProvider(peh, secLen, sec, dcctx, m));
        VexExecutionStatePtr    oldVss = cond->getState();
        VexExecutionStatePtr    vss = 
            VexExecutionStatePtr(new VexExecutionState(*oldVss.get()));
        VexExecutionEngine      vee(pep, vss);
        
        unsigned long dw;

        bool leave=false;
        while(!leave) {
            unsigned long curVA;
            VexExecutionEngine::StepState   st;
            char c;

            cout << console();
            cin >> c;
            switch(c) {
                case 'p':
                    //print current VA/off
                    cout << "current offset " << currentOff << endl;
                    cout << "current VA " << hex << peh.offsetToVa(currentOff);
                    cout << dec << endl;
                    break;
                case 'z':
                    //LETS FUCKING GO
                    {
                        vector<BlockPtr>    fb;
                        doWalkFromOff(  dcctx, 
                                        sec, 
                                        secLen, 
                                        currentOff, 
                                        peh, 
                                        fb,
                                        peArch,
                                        m);
                        cout << "got " << fb.size() << " blocks" << endl;
                    }
                    break;
                case 'h':
                    cout << pehelp();
                    break;
                case 'q':
                    //leave
                    leave = true;
                    break;
                case 'g':
                    //get next block, if possible
                    dw = peh.offsetToVa(currentOff);
                    curBlock = getBlockFromOff( dcctx, 
                                                sec, 
                                                bytesLeft, 
                                                currentOff, 
                                                dw,
                                                peArch,
                                                m);

                    if( curBlock) {
                        vee = VexExecutionEngine(pep, curBlock, vss);
                    }
                    currentOff++;
                    bytesLeft--;
                    break;
                case 'b':
                    //dump current VEE block state
                    cout << vee.printState() << endl;
                    break;
                case 's':
                    //step vee
                    st = vee.step();
                    if( st != VexExecutionEngine::StepOK ) {
                        //we did failure
                        if( st == VexExecutionEngine::StepERR ) {
                            cout << "Step err" << endl;
                        }
                        if( st == VexExecutionEngine::StepEnd ) {
                            cout << "Step end" << endl;
                        }
                    }
                    break;
                case 'w':
                    cout << "new off?";
                    cin >> currentOff;
                    break;
                case 'a':
                    cout << "new addr?";
                    cin >> hex >> curVA;
                    currentOff = peh.vaToOffset(curVA);
                    break;
                case 'y':
                    //run current block to end
                    do {
                        st = vee.step();
                    } while( st == VexExecutionEngine::StepOK );

                    if( st == VexExecutionEngine::StepERR ) {
                        cout << "block exited with error" << endl;
                    } else {
                        cout << "block exit OK" << endl;
                        //check postconditions
                        if( checkPostConditions( vss, cond ) ) {
                            cout << "block matches postconditions" << endl;
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        finiDecodeLib(dcctx);
    }

    return;
}

void doBlockMode(string filePath, Condition *cond, TargetArch tarch, unsigned int m) {
    unsigned long bufLen;
    unsigned char *mappedFile = NULL;

    //okay, read in this file 
    mappedFile = memoryMapFile(filePath, &bufLen);

    if( mappedFile == NULL ) {
        return;
    }
    
    TargetInfo  ti;
    ti.maxInstructions = 99;
    ti.chaseThreshold = 10;
	ti.opLevel = FullOpt;
	ti.tarch = tarch;
    if( tarch.ta == X86 ) {
        ti.guestHWcaps = 
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
        ti.hostHWcaps =
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
    } else if( tarch.ta == ARM ) {
        ti.guestHWcaps = 7|VEX_HWCAPS_ARM_VFP3;
        ti.hostHWcaps = 
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3; 
    }else {
        ti.guestHWcaps = 0;
        ti.hostHWcaps = 0;
    }

    void *dlctx = initDecodeLib(ti, true, true);

    assert(dlctx != NULL);

    //turn the mapped file into one BlockPtr
    BlockPtr    block;
    bool        res;
    uint32_t    baseAddr=0x1000;

    res = convertToOneBlock(dlctx,
                            mappedFile,
                            bufLen,
                            baseAddr,
                            tarch,
                            m,
                            block);

    if( !res ) {
        cout << "convertToOneBlock failed" << endl;
        return;
    }

    //okay, give the block to the VEE thing
    BlankBlockProviderPtr bpp = 
        BlankBlockProviderPtr(new BlankBlockProvider());
    VexExecutionStatePtr    oldVss;
    if( cond ) {
        oldVss = cond->getState();
    } else {
        oldVss = VexExecutionStatePtr(new VexExecutionState(tarch));
    }
    VexExecutionStatePtr    vss = 
        VexExecutionStatePtr(new VexExecutionState(*oldVss.get()));
    VexExecutionEngine  vee(bpp, block, vss);

    applyPreConditions(vss, cond);
    
    cout << "blockLen: " << block->getBlockLen() << endl;
    //okay, drive an event loop around printing current state
    bool leave=false;
    while(!leave) {
        VexExecutionEngine::StepState   ss;
        string inS;
        char command;

        cout << console();

        if( !getline(cin, inS) ) {
            leave = true;
            continue;
        }
        if( inS.size() == 0 ) {
            continue;
        }

        command = inS[0];
        switch(command) {
            case 'y':
            {
                //just get the permutations and then print them out 
            }
            break;
            case 'r':
            {
                //get all the transfer functions for the block
                //list<Transfer>  trans = simplifyTransfers(block);
                list<Transfer>  trans = block->getTransfers();
                list<Transfer>::iterator it = trans.begin();
                while( it != trans.end() ) {
                    cout << block->printTransfer(*it) << endl;
                    ++it;
                }
            }
            break;
            case 't':
            {
                uint64_t    addr;
                uint32_t    width;
                uint64_t    value;
                int         i = 0;

                regex       e(string("t ([0-9|a-f|A-F]+) ([0-9]+) ([0-9|a-f|A-F]+)"));
                smatch      s;
                if( regex_match( inS, s, e, match_extra ) ) {
#ifdef BOOST_REGEX_MATCH_EXTRA
                    istringstream   addrS(s.captures(1)[0]);
                    istringstream   widthS(s.captures(2)[0]);
                    istringstream   valueS(s.captures(3)[0]);
                    if( (addrS >> addr) &&
                        (widthS >> width) &&
                        (valueS >> value))
                    {
                        vss->setMem(addr, width, value);
                    }
                    else {
                        cerr << "bad command" << endl;
                    }
#endif
                } else {
                    cerr << "bad command" << endl;
                }
            }
                break;
            case 'g':
                //just go until we can't and report why
                do {
                    ss = vee.step();
                    if( ss == VexExecutionEngine::StepEnd ) {
                        //can we get a new block?
                        ExpressionPtr e = vee.getNext();
                        //see if we can get that block
                        ConstantValue   addr;
                        addr.valueIsKnown = false;

                        do {
                            if(ExRdTmpPtr r = dynamic_pointer_cast<ExRdTmp>(e)){
                                addr = r->getTmp()->getVal();
                                break;
                            }

                            if(ExConstPtr c = dynamic_pointer_cast<ExConst>(e)){
                                addr = c->getVal();
                                break;
                            }

                        } while(false);

                        if( addr.valueIsKnown ) {
                            uint32_t    newOff;

                            newOff = addr.U32-baseAddr;
                            if( newOff < bufLen ) {
                                BlockPtr    newBlock;
                                res = convertToOneBlock(dlctx,
                                    mappedFile+newOff,
                                    bufLen-newOff,
                                    baseAddr+newOff,
                                    tarch,
                                    m,
                                    newBlock);
                                if( res ) {
                                    block = newBlock;
                                    cout << "have a new block!" << endl;
                                    vee = 
                                        VexExecutionEngine(bpp, newBlock, vss);
                                    ss = VexExecutionEngine::StepOK;
                                } else {
                                    cout << "could not read new block, no more";
                                    cout << endl;
                                    if( checkPostConditions(vss, cond) ) {
                                        cout << "trace matches" << endl;
                                    }
                                }
                            }
                        }
                    }
                } while( ss == VexExecutionEngine::StepOK );

                if( ss == VexExecutionEngine::StepERR ) {
                    cout << "trace ended in err" << endl;
                    cout << vee.getCurStmt()->printStmt() << endl;
                } else if ( ss == VexExecutionEngine::StepClientERR ) {
                    cout << "trace ended in client err" << endl;
                    cout << vee.getCurStmt()->printStmt() << endl;
                } else {
                    //cout << vee.printState() << endl;
                    cout << "trace ended OK" << endl;
                    if( checkPostConditions(vss, cond) ) {
                        cout << "block matches postconditions" << endl;
                    }
                }

                break;
            case 'b':
                cout << vee.printState() << endl;
                break;
            case 'h':
                cout << help() << endl;
                break;
            case 'm':
                cout << "MEMORY" << endl;
                cout << vss->printMemState() << endl;
                break;
            case 'p':
            {
                uint64_t    off;
                uint32_t    width;
                uint64_t    value;
                int         i = 0;

                regex       e(string("p ([0-9]+) ([0-9]+) ([0-9|a-f|A-F]+)"));
                smatch      s;
                if( regex_match( inS, s, e, match_extra ) ) {
#ifdef BOOST_REGEX_MATCH_EXTRA
                    istringstream   addrS(s.captures(1)[0]);
                    istringstream   widthS(s.captures(2)[0]);
                    istringstream   valueS(s.captures(3)[0]);
                    if( (addrS >> off) &&
                        (widthS >> width) &&
                        (valueS >> value))
                    {
                        vss->setState(off, width, value);
                    }
                    else {
                        cerr << "bad command" << endl;
                    }
#endif
                } else {
                    cerr << "bad command" << endl;
                }

            }
                break;
            case 's':
            {
                VexExecutionEngine::StepState s = vee.step();
                if( s == VexExecutionEngine::StepERR ) {
                    cout << "Error in step" << endl;
                } else if( s == VexExecutionEngine::StepEnd ) {
                    cout << "Block is ended" << endl;
                    cout << "Next block is: ";
                    ExpressionPtr e = vee.getNext();
                    cout << e->printExpr() << endl;
                    //see if we can get that block
                    ConstantValue   addr;
                    addr.valueIsKnown = false;

                    do {
                        if( ExRdTmpPtr r = dynamic_pointer_cast<ExRdTmp>(e) ) {
                            addr = r->getTmp()->getVal();
                            break;
                        }

                        if( ExConstPtr c = dynamic_pointer_cast<ExConst>(e) ) {
                            addr = c->getVal();
                            break;
                        }

                    } while(false);

                    if( addr.valueIsKnown ) {
                        cout << hex << addr.U32 << dec << endl;
                        uint32_t    newOff;

                        newOff = addr.U32-baseAddr;
                        BlockPtr    newBlock;
                        res = convertToOneBlock(dlctx,
                            mappedFile+newOff,
                            bufLen-newOff,
                            baseAddr+newOff,
                            tarch,
                            m,
                            newBlock);
                        if( res ) {
                            block = newBlock;
                            cout << "have a new block!" << endl;
                            vee = VexExecutionEngine(bpp, newBlock, vss);
                        } else {
                            cout << "could not read new block, no more to do";
                            cout << endl;
                            leave=true;
                        }
                    }
                }
            }
                break;
            case 'q':
                leave=true;
                break;
            case 'u':
                cout << "DUMPING REG STATE" << endl;
                cout << "unknown values won't be shown" << endl;
                cout << vss->printRegState();
                cout << endl;
                break;
        }
    }

    unMapFile(mappedFile);
    finiDecodeLib(dlctx);
    return;
}

int main(int argc, char *argv[]) {
    TargetArch    tarch = { INVALID };
    std::string   inFilePath; 
    std::string   inPeFilePath;
    unsigned int  maxBlockSize; 

    std::string stateFilePath;
    boost::program_options::options_description d("options");
    boost::program_options::variables_map vm;

    d.add_options()
        ("version,v", "show version")
        ("help,h", "print help")
        ("block-size,n", program_options::value<unsigned int>(), "max size in statements of blocks to search")
        ("conditions-file,i", boost::program_options::value<std::string>(), "conditions")
        ("raw-file,f", boost::program_options::value<std::string>(), "input raw file")
        ("pe-file,p", boost::program_options::value<std::string>(), "input PE file")
        ("architecture,a", boost::program_options::value<std::string>(), "architecture to decode as")
        ;

    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, d), vm);

    if( vm.count("help") ) {
        std::cout << d << std::endl;
        return 0;
    }

    if( vm.count("version") ) {
        std::cout << d << std::endl;
        return 0;
    }

    if( vm.count("block-size") ) {
      maxBlockSize = vm["block-size"].as<unsigned int>();
    } else {
      maxBlockSize = 25;
    }

    if( vm.count("conditions-file") ) {
        stateFilePath = vm["conditions-file"].as<std::string>();
    }

    if( vm.count("raw-file") ) {
        inFilePath = vm["raw-file"].as<std::string>();
    }

    if( vm.count("pe-file") ) {
        inPeFilePath = vm["pe-file"].as<std::string>();
    }

    if( vm.count("architecture") ) {
        std::string v = vm["architecture"].as<std::string>();
        if( v == "X86" ) {
            tarch.ta = X86;
        } else if( v == "AMD64" ) {
            tarch.ta = AMD64;
        } else if ( v == "ARM" ) {
            tarch.ta = ARM;
            tarch.tm = WIDEARM;
        } else if ( v == "ARM-THUMB" ) {
            tarch.ta = ARM;
            tarch.tm = THUMB;
        }
    } else {
        return 0;
    }

    Condition   *cs=NULL;

    if( inFilePath.size() != 0 && inPeFilePath.size() != 0 ) {
        std::cout << "Must supply either a PE or raw file path" << std::endl;
        return 0;
    }

    if( stateFilePath.size() > 0 ) {
        cs = getConditionsFromFile(stateFilePath, tarch);
        
        if( !cs->loaded() ) {
            cout << "failed to load" << endl;
            return 0;
        }
    }

    initLibState();
    if( inFilePath.size() > 0 ) {
        doBlockMode(inFilePath, cs, tarch, maxBlockSize);
    } else if( inPeFilePath.size() > 0 ) {
        doPeMode(inPeFilePath, cs, maxBlockSize);
    }
    
    return 0;
}
