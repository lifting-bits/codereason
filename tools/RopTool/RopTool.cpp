#include <RopLib.h>
#include <VEE.h>

#include <boost/program_options/config.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/regex.hpp>

#include <capstone/capstone.h>

using namespace boost;

/*
//for disassembling
class BufferMemoryObject : public llvm::MemoryObject {
private:
    const uint8_t *Bytes;
    uint64_t Length;
public:
    BufferMemoryObject(const uint8_t *bytes, uint64_t length) :
        Bytes(bytes), Length(length) { }

    uint64_t getBase() const { return 0; }
    uint64_t getExtent() const { return Length; }

    int readByte(uint64_t addr, uint8_t *byte) const {
        if (addr > getExtent())
            return -1;
        *byte = Bytes[addr];
        return 0;
    }
};
*/

class BlankBlockProvider : public BlockProvider {
public:
    BlankBlockProvider() { return; }
       virtual BlockPtr getNextBlock(boost::uint64_t VA);
};

typedef shared_ptr<BlankBlockProvider> BlankBlockProviderPtr;
BlockPtr BlankBlockProvider::getNextBlock(boost::uint64_t VA) {
    BlockPtr    b;

    return b;
}

class MatchesVEE : public RopLibVisitor {
    Condition       *cond;
    unsigned int    followCount;
    unsigned int    maxFollow;
public:
    virtual VisitorResult keepBlock(BlockPtr b);
    virtual CodeExplorationResult exploreBlock(BlockPtr b, CodeExplorationStatePtr p);
    virtual CodeExplorationStatePtr initialState(void);

    MatchesVEE(Condition *c, unsigned int max) :    cond(c), 
                                                    followCount(0), 
                                                    maxFollow(max) 
    { 
        applyPreConditions(this->cond->getState(), this->cond);
        return;
    }
};

CodeExplorationStatePtr MatchesVEE::initialState(void)
{
    return CodeExplorationStatePtr();
}

CodeExplorationResult MatchesVEE::exploreBlock( BlockPtr b,
                                                CodeExplorationStatePtr p)
{
    CodeExplorationResult   r;

    r.doAll = false;

    return r;
}

VisitorResult MatchesVEE::keepBlock(BlockPtr b) {
    VisitorResult           r = Discard;

    //early check to see if we have gone too deep
    if( this->followCount > this->maxFollow ) {
        this->followCount = 0;
        return r;
    } else {
        this->followCount++;
    }

    //get us some state
    VexExecutionStatePtr    vssFromCond = this->cond->getState();
    BlankBlockProviderPtr   bpp(new BlankBlockProvider());

    //make a copy of the 'blank' VSS environment
    VexExecutionStatePtr    vss = 
        VexExecutionStatePtr(new VexExecutionState(*vssFromCond.get()));
    VexExecutionEngine      vee(bpp, b, vss);

    VexExecutionEngine::StepState s = VexExecutionEngine::StepOK;
    while( s == VexExecutionEngine::StepOK ) {
        s = vee.step();
    }

    if( s == VexExecutionEngine::StepEnd ) {
        //check and see if we met conditions
        if( checkPostConditions(vss, this->cond) ) {
            r = Accept;
            this->followCount = 0;
        } else {
            r = Keep;
        }
    }

    return r;
}

/*
static const llvm::Target *findDisTarget(std::string arch) {
    const llvm::Target  *tgt = NULL;

    for(llvm::TargetRegistry::iterator  it = llvm::TargetRegistry::begin(),
        ie = llvm::TargetRegistry::end();
        it != ie;
        ++it)
    {
        if( arch == it->getName() ) {
            tgt = &*it;
            break;
        }
    }

    return tgt;
}
*/

//print progress
static
void print_progress(unsigned int Total, unsigned int Done) {
    const int tdot = 40;

    double pctDone = ((double)Done)/((double)Total);

    unsigned int print = floor(pctDone * tdot);

    unsigned int i = 0;
    std::cout << std::setprecision(5) << std::fixed << pctDone*100 << "[";
    for( ; i < print; i++ ) {
        std::cout << "=";
    }
    for( ; i < tdot; i++ ) {
        std::cout << " ";
    }

    std::cout << "]\r";
    std::cout.flush();

    return;
}

/*
std::string disAtVAInBuff(   uint8_t     *buff,
                        uint64_t    disVA,
                        uint32_t    disLen,
                        TargetArch  tarch)
{
    std::string r = "";

    assert(tarch.ta == X86 || tarch.ta == AMD64 || tarch.ta == ARM);

    const llvm::Target          *t = NULL;
    const llvm::MCSubtargetInfo *STI = NULL;
    const llvm::MCAsmInfo       *AsmInfo = NULL;
#if(LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >=2)
    llvm::MCRegisterInfo        *MRI = NULL;
    const llvm::MCInstrInfo     *MCII = NULL;
#endif

    switch(tarch.ta) {
        case X86:
            t = findDisTarget("x86");
            assert( t != NULL );
            STI = t->createMCSubtargetInfo("i386-unknown-linux-gnu", "", "");
            AsmInfo = t->createMCAsmInfo("i386-unknown-linux-gnu");
#if(LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 2)
            MRI = t->createMCRegInfo("i386-unknown-linux-gnu");
#endif
            break;
        
        case AMD64:
            t = findDisTarget("x86-64");
            assert( t != NULL );
            STI = t->createMCSubtargetInfo("amd64-unknown-linux-gnu", "", "");
            AsmInfo = t->createMCAsmInfo("amd64-unknown-linux-gnu");
#if(LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 2)
            MRI = t->createMCRegInfo("amd64-unknown-linux-gnu");
#endif
            break;

        case ARM:
            switch(tarch.tm) {
                case THUMB:
                    t = findDisTarget("thumb");
                    assert( t != NULL );
                    STI = 
                       t->createMCSubtargetInfo("i386-unknown-linux-gnu","","");
                    AsmInfo = t->createMCAsmInfo("i386-unknown-linux-gnu");
#if(LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 2)
                    MRI = t->createMCRegInfo("i386-unknown-linux-gnu");
#endif
                    break;

                case WIDEARM:
                    t = findDisTarget("arm");
                    assert( t != NULL );
                    STI = 
                       t->createMCSubtargetInfo("i386-unknown-linux-gnu","","");
                    AsmInfo = t->createMCAsmInfo("i386-unknown-linux-gnu");
#if(LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 2)
                    MRI = t->createMCRegInfo("i386-unknown-linux-gnu");
#endif
                    break;
                case S_INVALID:
                    assert(!"Invalid subarch!");
                    break;
            }
            break;

        default:
            assert(!"NIY");
    }

    assert(t);
    assert(STI);
    assert(AsmInfo);

    //make a printer object
    int APV = AsmInfo->getAssemblerDialect();
#ifndef LLVM_VERSION_MAJOR
    llvm::MCInstPrinter
        *IP = t->createMCInstPrinter(APV, *AsmInfo, *STI);
#endif
#if(LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 2)
    MCII = t->createMCInstrInfo();
    llvm::MCInstPrinter
        *IP = t->createMCInstPrinter(1, *AsmInfo, *MCII, *MRI, *STI);
#endif

    assert( IP );
    //make a disassembler
    const llvm::MCDisassembler *DisAsm = t->createMCDisassembler(*STI);
    assert( DisAsm );

    //describe the buffer
    BufferMemoryObject  bmo(buff, disLen);

    //do the decode
    uint64_t    Size;
    uint64_t    Index;

    for( Index = 0; Index < disLen; Index += Size ) { 
        llvm::MCInst  inst;
        llvm::MCDisassembler::DecodeStatus  s;
        bool                                br=false;
        std::string                              outS;
        llvm::raw_string_ostream            osOut(outS);

        s = DisAsm->getInstruction( inst,
                                    Size,
                                    bmo,
                                    Index,
                                    llvm::nulls(),
                                    llvm::nulls());
        switch(s) {
            case llvm::MCDisassembler::Success:
                //print the bytes 
                for(uint64_t k = 0; k < Size; k++)
                {
                    uint32_t    bfr;
                    uint8_t     bt;
                    bmo.readByte(Index+k, &bt);
                    bfr = bt;
                    r = r + to_string<uint32_t>(bfr, std::hex) + " ";
                }
                //print the instruction
                IP->printInst(&inst, osOut, "");
                r = r + osOut.str() + "\n";
                break;
            default:
                std::cout << "decode failed" << std::endl;
                br = true;
        }

        if( br ) {
            break;
        }
    }

    return r;
}
*/

FileFormat fmtFromVM(program_options::variables_map &vm) {
    if( vm.count("pe") ) {
        return PEFmt;
    }

    if( vm.count("mach") ) {
        return MachOFmt;
    }

    if( vm.count("raw") ) {
        return RawFmt;
    }

    if( vm.count("ucache") ) {
        return DyldCacheFmt;
    }

    return Invalid;
}

TargetArch archFromVM(program_options::variables_map &vm) {
    TargetArch  tarch;

    tarch.ta = INVALID;
    
    if( vm.count("architecture") ) {
        std::string archStr = vm["architecture"].as<std::string>();

        if( archStr == "X86" ) {
            tarch.ta = X86;
        } else if( archStr == "AMD64" ) {
            tarch.ta = AMD64;
        } else if( archStr == "ARM" ) {
            tarch.ta = ARM;
            tarch.tm = WIDEARM;
        } else if( archStr == "ARM-THUMB") {
            tarch.ta = ARM;
            tarch.tm = THUMB;
        } 
    }

    return tarch;
}

int main(int argc, char *argv[]) {
    program_options::options_description    d("options");    
    program_options::variables_map          vm;
    TargetArch                              ta;
    FileFormat                              fmt;
    std::string                             inputFile;
    std::string                             filesToSearch;
    std::string                             dbOutFile;
    Condition                               *cs;
    int                                     jumps;
    unsigned int                            maxSize;
    uint32_t                                bucketSize;
/*
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllDisassemblers();
*/
    d.add_options()
        ("version,v", "show version")
        ("help,h", "print help")
        ("pe", "is a PE file")
        ("mach", "is a mach file")
        ("ucache", "is a dyld cache")
        ("raw", "is a raw file")
        ("db", "is a DB file")
        ("block-size,n", program_options::value<unsigned int>(), "max size in statements of blocks to search")
        ("search-files,s", program_options::value<std::string>(), "files in input to search")
        ("blocks-out", program_options::value<std::string>(), "seralize input to a DB")
        ("conditions,i", program_options::value<std::string>(), "input script file")
        ("in-file,f", program_options::value<std::string>(), "input file")
        ("architecture,a", program_options::value<std::string>(),"architecture")
        ("jumps,j", program_options::value<int>(), "jumps to follow")
        ("bucketsize,b", program_options::value<uint32_t>(), "bucket size");


    program_options::store(
        program_options::parse_command_line(argc, argv, d), vm);
    
    if( vm.count("bucketsize") ) {
        bucketSize = vm["bucketsize"].as<uint32_t>();
    } else {
        bucketSize = 0x1000;
    }

    if( vm.count("block-size") ) {
      maxSize = vm["block-size"].as<unsigned int>();
    } else {
      maxSize = 25;
    }

    if( vm.count("blocks-out") ) {
        dbOutFile = vm["blocks-out"].as<std::string>();
    }

    if( vm.count("search-files") ) {
        filesToSearch = vm["search-files"].as<std::string>();
    }

    if( vm.count("version") ) {
        std::cout << d << std::endl;
        return 0;
    } 

    if( vm.count("help") ) {
        std::cout << d << std::endl;
        return 0;
    }
    
    ta = archFromVM(vm);
    fmt = fmtFromVM(vm);

    if( fmt == Invalid ) {
        std::cout << "Must specify a format" << std::endl;
        std::cout << d << std::endl;
        return 0;
    }

    if( fmt == MachOFmt && ta.ta == INVALID ) {
        std::cout << "Must specify an architecture for mach-o objects" << std::endl;
        std::cout << d << std::endl;
        return 0;
    }

    if( vm.count("jumps") ) { 
        jumps = vm["jumps"].as<int>();
    } else {
        jumps = 1;
    }

    if( vm.count("in-file") ) {
        inputFile = vm["in-file"].as<std::string>();
    } else {
        std::cout << "Must specify an input file" << std::endl;
        std::cout << d << std::endl; 
        return 0;
    }

    if( vm.count("conditions") ) {
        cs = getConditionsFromFile(vm["conditions"].as<std::string>(), ta);
        assert(cs != NULL);
        if( !cs->loaded() ) {
            std::cout << "Could not load conditions from file" << std::endl;
            return 0;
        }
    } else if( dbOutFile.size() == 0) {
        std::cout << "Must specify input condition file" << std::endl;
        std::cout << d << std::endl;
        return 0;
    }
    
    //construct a visitor class
    RopLibVisitorPtr    mvee(new MatchesVEE(cs, jumps));

    //construct a searcher class 
    RopLibSearcher  rls(mvee, inputFile, filesToSearch, fmt, ta, maxSize);

    //initialize with some blocks
    //std::cerr << "building blocks..." << std::endl;
    
    rls.getBlocks(bucketSize);
    
    //now, while we have searching to do, apply our search class
    std::cerr << "searching ... " << std::endl;
    uint64_t    cur = 1;
    uint64_t    end = rls.getNumBlocks();

    while( rls.canSearch() ) 
    {
      if(rls.needsMoreBlocks())
      {
        if(rls.getBlocks(bucketSize) == false) {
			std::cout << "failed to get blocks, bailing" << std::endl;
			break;
		}
        continue;
      }

      print_progress(end, cur);
      cur++;
      //std::cout << cur << " " << end << std::endl;
      rls.evalOneBlock();
    }

    std::cout << std::endl;

    std::cerr << "done searching " << std::endl;

    std::list<std::list<BlockPtr> >  found = rls.getBlocksFound();

    std::cerr << "found " << found.size() << std::endl;

    for(std::list<std::list<BlockPtr> >::iterator tit = found.begin(); 
        tit != found.end(); 
        ++tit) 
    {
        std::list<BlockPtr>  thisTrace = *tit;
        
        std::cout << "trace len: " << thisTrace.size() << std::endl;
        for(std::list<BlockPtr>::iterator   it = thisTrace.begin();
            it != thisTrace.end();
            ++it)
        {
            BlockPtr    b = *it;
            uint64_t    addr = b->getBlockBase();
            secVT       sectionsToSearch = rls.getSections();
            uint64_t    blockBase = b->getBlockBase();
            uint64_t    blockLen = b->getBlockLen();

            std::cout << std::hex << addr << std::dec << std::endl;
            //disassemble the buffer
            for(secVT::iterator sit = sectionsToSearch.begin();
                sit != sectionsToSearch.end();
                ++sit)
            {
                secAndArchT saa = *sit;
                TargetArch arch = saa.first;
                secPT secAndLen = saa.second;
                lenAddrT len = secAndLen.second;
                uint8_t *buf = secAndLen.first;
                uint64_t baseAddr = len.second;
                uint32_t bufLen = len.first;

                //does this section have that VA?
                if( ((blockBase >= baseAddr) &&
                    (blockBase < (baseAddr+bufLen)) ) &&
                                ((blockBase+blockLen <= (baseAddr+bufLen)) ) )
                {
                    //it does, print the disassembly and leave
                    uint64_t    delta = blockBase-baseAddr;
                    uint8_t     *disBuf = buf+delta; 
                   
                    std::string s = "";/*disAtVAInBuff(   disBuf, 
                                                blockBase, 
                                                blockLen, 
                                                arch);
                                    */
                    std::cout << s << std::endl;
                    break;
                } else {
                    std::cout << "ERR: VA " << std::hex << blockBase;
                    std::cout << " - " << blockBase+blockLen << std::dec;
                    std::cout << " not in maps" << std::endl;
                }
            }
        }
        std::cout << " --------- " << std::endl;
    }

    return 0;
}
