#include <RopLib.h>
#include <ExprParser.h>

#include <boost/regex.hpp>
#include <boost/program_options/config.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>

using namespace std;
using namespace boost;

typedef std::pair<std::string, std::vector<StatementPtr> >    nameAndRule;

class RuleMatchingVisitor : public RopLibVisitor {
private:
    std::vector<nameAndRule>    rules;
public:

    RuleMatchingVisitor(std::vector<nameAndRule> r) : rules(r) { }
    
    virtual VisitorResult keepBlock(BlockPtr b);
    virtual CodeExplorationResult exploreBlock(BlockPtr b, CodeExplorationStatePtr p);
    virtual CodeExplorationStatePtr initialState(void);
};

typedef pair<Register,Register> regPair;

CodeExplorationStatePtr RuleMatchingVisitor::initialState(void)
{
    return CodeExplorationStatePtr();
}

CodeExplorationResult RuleMatchingVisitor::exploreBlock(BlockPtr b,
                                                    CodeExplorationStatePtr p)
{
    return CodeExplorationResult();
}

VisitorResult RuleMatchingVisitor::keepBlock(BlockPtr b) {
    VisitorResult       r = Discard;
    //get the transfers for this specific block 
    list<Transfer> block_transfers = b->getTransfers();
    
    //iterate over every rule that we have
    //if any of these rules hold, then we accept
    for(vector<nameAndRule>::iterator it = this->rules.begin(); 
        it != this->rules.end(); 
        ++it)
    {
        nameAndRule             nr = *it;
        vector<StatementPtr>    these_rules = nr.second; 
        bool                    all_accepted = true;
        
        //all of these rules must hold for us to accept
        for(vector<StatementPtr>::iterator sit = these_rules.begin();
            sit != these_rules.end();
            ++sit)
        {
            StatementPtr    s = *sit;
            bool            broken = false;
            bool            tested = false;

            for(list<Transfer>::iterator   tit = block_transfers.begin();
                tit != block_transfers.end();
                ++tit)
            {
                Transfer        t = *tit;
                SinkType        sink = t.first;

                //if our rules statement is a put or a store
                if( StPutPtr p = dynamic_pointer_cast<StPut>(s) ) {
                    //a put rule matches with a reg sink
                    if( sink.isReg() ) {
                        Register        ruleReg = p->getDstRegister();
                        ExpressionPtr   ruleData = p->getData();
                        Register        tstReg = sink.getReg();
                        ExpressionPtr   tstData = t.second;
                    
                        if( ruleReg == tstReg ) {
                            tested = true;
                            if( !exprsMatch(ruleData, tstData) ) {
                                all_accepted = false;
                                broken = false;
                                break;
                            }
                        }
                    }

                } else if(StStorePtr st = dynamic_pointer_cast<StStore>(s)) {
                    //a store rule matches with a mem sink
                    if( sink.isMem() ) { 
                        ExpressionPtr   ruleAddr = st->getAddr();
                        ExpressionPtr   ruleData = st->getData();
                        ExpressionPtr   tstAddr = sink.getMem();
                        ExpressionPtr   tstData = t.second;
                        
                        tested = true;
                        if( !exprsMatch(ruleAddr, tstAddr) ||
                            !exprsMatch(ruleData, tstData)) {
                            all_accepted = false;
                            broken = true;
                            break;
                        }
                    }
                }
            }

            if( tested == false ) {
                all_accepted = false;
                broken = false;
            }

            if( broken ) {
                break;
            }
        }

        if( all_accepted ) {
            r = Accept;
            break;
        }
    }

    return r;
}

class SymbolicState : public CodeExplorationState {
public:
    SymbolicState() { }

    virtual CodeExplorationState *clone();
};

CodeExplorationState * SymbolicState::clone(void)
{
    return new SymbolicState();
}

typedef shared_ptr<SymbolicState> SymbolicStatePtr;

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
        string archStr = vm["architecture"].as<std::string>();

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
        } else if( archStr == "AMD64" ) {
            tarch.ta = AMD64;
        } 
    }

    return tarch;
}

string consoleHelp(void) {
    string  k;

    k = k + "q: quit\n";
    k = k + "h: help\n";
    k = k + "s [block VA]: select a block by VA\n";
    k = k + "b: print current block\n";
    k = k + "e: evaluate current block\n";

    return k;
}

string console(void) {
    return " >>> ";
}

int runShell(RopLibSearcher &rls, RopLibVisitorPtr  visitor) {

    bool    leave = false;
    BlockPtr    curBlock;
    regex       matchSelect("s ([0-9|a-f|A-F]+)");
    smatch      s;

    while( !leave ) {
        string  inS;

        cout << console();

        if( !getline(cin, inS) ) {
            leave = true;
            continue;
        }

        if( inS.size() == 0 ) {
            continue;
        }

        switch(inS[0]) {
            case 's':
#ifdef BOOST_REGEX_MATCH_EXTRA
                if( regex_match( inS, s, matchSelect, match_extra ) ) {
                    //hooray, correct command
                    uint64_t        addr;
                    istringstream   addrS(s.captures(1)[0]);
                    if( addrS >> addr ) {
                        BlockPtr    tmp = rls.getBlockWithBaseVA(addr);
                        if( tmp ) {
                            curBlock = tmp;
                        } else {
                            cout << "could not find that block" << endl;
                        }
                    } else {
                        cout << "bad input" << endl;
                    }
                } else {
                    cout << "bad input" << endl;
                }
#endif
                break;
            case 'b':
                if( curBlock ) {
                    cout << curBlock->printBlock() << endl;
                } else {
                    cout << "no block is currently selected" << endl;
                }
                break;
            case 'e':
                if( curBlock ) {
                    if( visitor->keepBlock(curBlock) == Accept ) {
                        cout << "visitor says keep block" << endl;
                    } else {
                        cout << "visitor says do not accept block" << endl;
                    }
                } else {
                    cout << "no block is currently selected" << endl;
                }
                break;
            case 'q':
                leave = true;
                break;
            case 'h':
                cout << consoleHelp() << endl;
                break;
            default:
                cout << "Unknown command!" << endl;
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    program_options::options_description    d("options");    
    program_options::variables_map          vm;
    TargetArch                              ta;
    FileFormat                              fmt;
    string                                  inputFile;
    string                                  fileToSearch;
    std::vector<std::string>                inputRules;

    d.add_options()
        ("version,v", "show version")
        ("help,h", "print help")
        ("pe", "is a PE file")
        ("mach", "is a mach file")
        ("raw", "is a raw file")
        ("ucache", "is a user cache file")
        ("in-file,f", program_options::value<std::string>(), "input file")
        ("conditions,c", 
            program_options::value<std::vector<std::string> >(), "conditions")
        ("architecture,a", 
            program_options::value<std::string>(),"architecture")
        ("shell,s", "start a shell");

    program_options::store(
        program_options::parse_command_line(argc, argv, d), vm);

    if( vm.count("version") ) {
        cout << d << endl;
        return 0;
    } 

    if( vm.count("help") ) {
        cout << d << endl;
        return 0;
    }
    
    ta = archFromVM(vm);
    fmt = fmtFromVM(vm);

    if( fmt == Invalid ) {
        cout << "Must specify a format" << endl;
        cout << d << endl;
        return 0;
    }

    if( fmt == MachOFmt && ta.ta == INVALID ) {
        cout << "Must specify an architecture for mach-o objects" << endl;
        cout << d << endl;
        return 0;
    }

    if( vm.count("in-file") ) {
        inputFile = vm["in-file"].as<string>();
    } else {
        cout << "Must specify input file" << endl;
        cout << d << endl; 
        return 0;
    }

    if( vm.count("conditions") ) {
        inputRules = vm["conditions"].as<std::vector<string> >();
    } else {
        std::cout << "Must specify input rules" << std::endl;
        std::cout << d << std::endl;
        return 0;
    }

    //construct a vector of statement/name pairs
    std::vector<nameAndRule>    rules;

    for(std::vector<std::string>::iterator it = inputRules.begin();
        it != inputRules.end();
        ++it)
    {
        std::string                 inputFile = *it;
        std::vector<StatementPtr>   ruleStatements;

        //get the rule name by parsing out the file name from the input 
        std::string ruleName = inputFile;

        try {
            ruleStatements = getExprsForFile(inputFile);
        } 
        catch (SemanticFail s ) {
            std::cout << "semantic fail in file " << inputFile;
            std::cout << std::endl;
            return 0;
        }
        catch ( ParseFail p ) {
            std::cout << "parse fail in file " << inputFile;
            std::cout << std::endl;
            return 0;
        }

        //if we made it without throwing, and there were any statements
        //to be parsed, add it to the rules
        if( ruleStatements.size() > 0 ) {
            rules.push_back(nameAndRule(ruleName, ruleStatements));
            cout << ruleName << endl;
            for(vector<StatementPtr>::iterator it = ruleStatements.begin();
                it != ruleStatements.end();
                ++it)
            {  
                cout << (*it)->printStmt();
            }
        }
    }

    //construct a visitor class
    RopLibVisitorPtr    rv(new RuleMatchingVisitor(rules));

    //construct a searcher class 
    RopLibSearcher  rls(rv, inputFile, fileToSearch, fmt, ta);

    //create blocks for everything
    cerr << "building blocks..." << endl;
    rls.getBlocks();

    //check and see if we are going to do an interactive vs non interactive search
    if( vm.count("shell") ) {
        cout << "starting shell" << endl;

        return runShell(rls, rv);
    }

    //now, while we have searching to do, apply our search class
    cerr << "searching ... " << endl;
    while( rls.canSearch() ) {
        rls.evalOneBlock();
    }

    cerr << "done searching " << endl;

    list<list<BlockPtr> > found = rls.getBlocksFound();

    cerr << "found " << found.size() << endl;

    for(list<list<BlockPtr> >::iterator tit = found.begin(); 
        tit != found.end(); 
        ++tit)
    {
        list<BlockPtr>  trace = *tit;

        for(list<BlockPtr>::iterator   it = trace.begin();
            it != trace.end();
            ++it)
        {
            BlockPtr    b = *it;

            cout << hex << b->getBlockBase() << dec << endl;
            cout << b->printBlock();
        }
    }

    return 0;
}
