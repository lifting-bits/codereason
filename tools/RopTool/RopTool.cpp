#include <inttypes.h>
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

//print progress
static
void print_progress(uint64_t Total, uint64_t Done) {
    const int tdot = 40;

    double pctDone = ((double)Done)/((double)Total);

    unsigned int print = floor(pctDone * tdot);

    unsigned int i = 0;
    std::cout.width(9); std::cout << std::setprecision(5) << std::fixed << std::right << pctDone*100 << "[";
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

/* computes mode flags for Capstone */
cs_mode archToCapstoneMode(TargetArch tarch)
{
    cs_mode mode = (cs_mode)-1;

    if( tarch.ta == X86 )
        mode = CS_MODE_32;
    else if( tarch.ta == AMD64 )
        mode = CS_MODE_64;
    else if( tarch.ta == ARM )
    {
        if( tarch.tm == WIDEARM )
            mode = CS_MODE_ARM;
        else if( tarch.tm == THUMB )
            mode = CS_MODE_THUMB;
    }

    // possible to add support for capstone endianess/extra modes here

    return mode;
}

/* translates TargetArch for Capstone */
cs_arch archToCapstone(TargetArch tarch)
{
    cs_arch carch = (cs_arch)-1;

    if( tarch.ta == X86 || tarch.ta == AMD64 )
        carch = CS_ARCH_X86;
    else if( tarch.ta == ARM )
        carch = CS_ARCH_ARM;
    
    return carch;     
}

// parses the arch from the variable map, autodetect if not set
TargetArch archFromVM(program_options::variables_map &vm) {
    TargetArch  tarch;

    if( vm.count("arch") ) {
        std::string archStr = vm["arch"].as<std::string>();
        std::transform(archStr.begin(), archStr.end(), archStr.begin(), ::tolower);
        
        if( archStr  == "x86" ) {
            tarch.ta = X86;
        } else if( archStr == "x64" ) {
            tarch.ta = AMD64;
        } else if( archStr == "arm" ) {
            tarch.ta = ARM;
            tarch.tm = WIDEARM;
        } else if( archStr == "thumb") {
            tarch.ta = ARM;
            tarch.tm = THUMB;
        } else {
            tarch.ta = INVALID;
        }

    } else {
        tarch.ta = AUTODETECT;
    }

    return tarch;
}

int main(int argc, char *argv[]) {
    program_options::options_description    d("options");    
    program_options::variables_map          vm;
    TargetArch                              ta;
    ExecCodeProvider*                       codeProvider;
    std::string                             inputFile;
    std::string                             blocksOutput;
    Condition                               *cs;
    int                                     jumps;
    unsigned int                            maxSize;
    uint32_t                                bucketSize;
    unsigned int                            good;
    unsigned int                            error;
    bool                                    raw;

    d.add_options()
        ("help,h", "print help")
        ("file,f", program_options::value<std::string>(), "input file")
        ("conditions,c", program_options::value<std::string>(), "lua conditions script file")
        ("arch,a", program_options::value<std::string>(),"architecture - x86, x64, arm, thumb")
        ("jumps,j", program_options::value<int>(), "jumps to follow")
        ("raw", "interpret input file as raw blob")
        ("block-size", program_options::value<unsigned int>(), "max size in statements of blocks to search")
        ("blocks-out", program_options::value<std::string>(), "seralize input to a DB")
        ("bucket-size", program_options::value<uint32_t>(), "bucket size");

    program_options::store(
        program_options::parse_command_line(argc, argv, d), vm);
    
    if( vm.count("bucket-size") ) {
        bucketSize = vm["bucket-size"].as<uint32_t>();
    } else {
        bucketSize = 0x1000;
    }

    if( vm.count("block-size") ) {
      maxSize = vm["block-size"].as<unsigned int>();
    } else {
      maxSize = 25;
    }

    if( vm.count("blocks-out") ) {
        blocksOutput = vm["blocks-out"].as<std::string>();
    }

    if( vm.count("help") ) {
        std::cout << d << std::endl;
        return 0;
    }
    
    ta = archFromVM(vm);

    if( vm.count("jumps") ) { 
        jumps = vm["jumps"].as<int>();
    } else {
        jumps = 1;
    }

    if( vm.count("raw") )
    {
        raw = true;
    } else {
        raw = false;
    }

    if ( ! vm.count("arch") ) {
      std::cout << "Must specify an input file architecture" << std::endl;
      std::cout << d << std::endl;
      return 0;
    }

    // get the input binary filename
    if( vm.count("file") ) {
        inputFile = vm["file"].as<std::string>();
    
        // create our executable wrapper
        codeProvider = new ExecCodeProvider(inputFile, ta, raw);    
        if(codeProvider->getError())
        {
            return 0;
        }

        // retrieve the detected architecture
        ta = codeProvider->getArch();
    
        //TODO: sanity checks on codeProvider
    
    } else {
        std::cout << "Must specify an input file" << std::endl;
        std::cout << d << std::endl; 
        return 0;
    }

    // load the lua conditions script
    if( vm.count("conditions") ) {
        cs = getConditionsFromFile(vm["conditions"].as<std::string>(), ta);
        assert(cs != NULL);
        if( !cs->loaded() ) {
            std::cout << "Could not load conditions from file" << std::endl;
            return 0;
        }
    } else {
        std::cout << "Must specify input condition file" << std::endl;
        std::cout << d << std::endl;
        return 0;
    }

    std::cout << std::string(50, '-') << std::endl;
    std::cout << "--[  Searching" << std::endl;
    std::cout << std::string(40, '-') << std::endl;

    //construct a visitor class
    RopLibVisitorPtr    mvee(new MatchesVEE(cs, jumps));

    //construct a searcher class 
    RopLibSearcher  rls(mvee, codeProvider, Invalid, ta, maxSize);
    std::cout << " Enumerated " << rls.getNumBlocks() << " blocks!" << std::endl;
    std::cout << " Searching for gadgets that match constraints..." << std::endl;

    //initialize with some blocks
    rls.getBlocks(bucketSize);
    
    //now, while we have searching to do, apply our search class
    while( rls.canSearch() ) 
    {
        if(rls.needsMoreBlocks())
        {
            if(rls.getBlocks(bucketSize) == false)
            {
			    //std::cout << "No more blocks!" << std::endl;
			    break;
		    }
            continue;
        }
      
      print_progress(rls.getNumBlocks(), rls.getBlocksDone());
      rls.evalOneBlock();
    }

    std::cout << std::endl;
    std::cout << " Done searching!" << std::endl;
   
    csh handle;
    cs_insn * insn;
    size_t count;

    // init the capstone engine
    if(cs_open( archToCapstone(ta), archToCapstoneMode(ta), &handle) != CS_ERR_OK)
    {
       std::cout << "[Error] Failed to initialize the Capstone Engine" << std::endl;
       std::cout << "[Error] Capstone Error: " << cs_strerror(cs_errno(handle));
       return 0;
    }

    
    std::cout << std::string(50, '-') << std::endl;
    std::cout << "--[  Gadgets " << std::endl;
    std::cout << std::string(40, '-') << std::endl;
    
    std::list<std::list<BlockPtr> >  found = rls.getBlocksFound();
    good = error = 0;
    
    // pretty print our found gadgets
    for(std::list<std::list<BlockPtr> >::iterator tit = found.begin(); 
        tit != found.end(); 
        ++tit) 
    {
        std::list<BlockPtr>  thisTrace = *tit;
        
        //std::cout << "trace len: " << thisTrace.size() << std::endl;
        for(std::list<BlockPtr>::iterator it = thisTrace.begin();
            it != thisTrace.end();
            ++it)
        {
            BlockPtr    b = *it;
            uint64_t    addr = b->getBlockBase();
            uint64_t    blockBase = b->getBlockBase();
            uint64_t    blockLen = b->getBlockLen();
            bool        found = false;

            //disassemble the buffer
            secVT sectionsToSearch = rls.getSections();
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
                
                //printf("blockBase %llx >= baseAddr %llx\n", blockBase, baseAddr);
                //printf("blockBase %llx < baseAddr+bufLen %llx bufLen %x\n", blockBase, baseAddr+bufLen, bufLen);
                //printf("blockBase+blockLen %llx <= baseAddr+bufLen %llx blockLen %llx\n\n", blockBase+blockLen, baseAddr+bufLen, blockLen);
                
                // does this section have that VA?
                if( ((blockBase >= baseAddr) &&
                    (blockBase < (baseAddr+bufLen))) &&
                    ((blockBase+blockLen <= (baseAddr+bufLen))) )
                {
                    // it does, print the disassembly and leave
                    uint64_t    delta = blockBase-baseAddr;
                    uint8_t     *disBuf = buf+delta; 
                    found = true;
                    
                    // if a raw blob, readjust the base back down 0x1000
                    if(raw)
                        blockBase -= 0x1000;
                
                    // send buffer to capstone for disassembly
                    count = cs_disasm(handle, disBuf, blockLen, blockBase, 0, &insn); 

                    if(count > 0)
                    {
                        // disassembly succeeded, print instructions
                        for(size_t i = 0; i < count; i++)
                            printf("0x%llx: %s\t%s\n", insn[i].address, insn[i].mnemonic, insn[i].op_str);
                    
                        cs_free(insn, count);
                        good++;
                    }
                    else
                    {
                        // TODO: maybe just drop these? anytime capstone fails, IDA seems to agree
                        printf("[Error] Could not disassemble gadget at 0x%08llx!\n", blockBase);
                        error++;
                    }

                    break;
                
                }
            }
            
            // if the gadget was not found in any of our executable sections
            // something is going horribly wrong
            if(!found)
            {
                // print an error if our virtual address is way out of bounds for some reason
                std::cout << "[Error] VA " << std::hex << blockBase;
                std::cout << " - " << blockBase+blockLen << std::dec;
                std::cout << " not in maps" << std::endl;
                error++;
            }
        }
        std::cout << " " << std::string(30, '-') << std::endl;
    }
   
    // print some stats
    // TODO: timing would be fun
    std::cout << std::string(50, '-') << std::endl;
    std::cout << "--[  Stats " << std::endl;
    std::cout << std::string(40, '-') << std::endl;
    std::cout << "Gadgets found: " << good << std::endl;
    printf("[Debug] good: %u - errors: %u\n", good, error);
    
    return 0;
}
