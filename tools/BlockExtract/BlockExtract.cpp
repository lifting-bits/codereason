#include <VEE.h>
#include <getExec.h>
#include <decodeToIR.h>
#include <math.h>

#include <boost/program_options/config.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>

#include <boost/regex.hpp>

using namespace std;
using namespace boost;

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
        } else if( archStr == "AMD64" ) {
            tarch.ta = AMD64;
        }
    }

    return tarch;
}

int main(int argc, char *argv[]) {
    program_options::options_description    d("options");
    program_options::variables_map          vm;
    TargetArch                              ta;
    FileFormat                              fmt;
    string                                  inputFile;
    string                                  outputFile;
    string                                  filter(".*");

    d.add_options()
        ("pe", "specify PE input file")
        ("mach-o", "specify mach-o input file")
        ("ucache", "usercache")
        ("raw", "raw input file")
        ("architecture,a", program_options::value<string>(), "architecture to search")
        ("input,i", program_options::value<string>(), "input file")
        ("filter", program_options::value<string>(), "file filter")
        ("db-out", program_options::value<string>(), "DB output file")
        ("version,v", "show version");

    program_options::store(
        program_options::parse_command_line(argc, argv, d), vm);

    if( vm.count("version") ) {
        cout << d << endl;
        return 0;
    }

    if( vm.count("input") ) {
        inputFile = vm["input"].as<string>();
    }

    if( vm.count("db-out") ) {
        outputFile = vm["db-out"].as<string>();
    }

    if( vm.count("filter") ) {
        filter = vm["filter"].as<string>();
    }

    if( inputFile.size() == 0 || outputFile.size() == 0 ) {
        cout << d << endl;
        return -1;
    }

    ta = archFromVM(vm);
    fmt = fmtFromVM(vm);

    if( fmt == Invalid ) {
        cout << d << endl;
        return -1;
    }

    if( ta.ta == INVALID ) {
        cout << d << endl;

        return -1;
    }

    /* read executable sections that match the filter */
    ExecCodeProvider    ecp(inputFile, fmt, ta);
    regex               matcher(filter);
    smatch              sm;
    secVT               sections;
    list<string>        fn = ecp.filenames();
    list<BlockPtr>      blocksFound;
   
    for( list<string>::iterator i = fn.begin(); i != fn.end(); ++i ) {
        if( regex_search(*i, sm, matcher) ) {
            secVT   tmp = ecp.sections_in_file(*i);
            
            sections.insert(sections.begin(), tmp.begin(), tmp.end());
        }
    }
    
    void    *ctx = initDecodeLib2(ta, true, false);
    ContextHandle *K = openFile(inputFile);

    if( ctx == NULL ) {
        cout << "Failed to initialize decoder library" << endl;
        return -1;
    }

    int k = 0;
    /* produce a block for every offset from the executable section */
    for( secVT::iterator i = sections.begin(); i != sections.end(); ++i ) {
        TargetArch  a = (*i).first;
        secPT       s = (*i).second;
        uint32_t    len = s.second.first;
        uint32_t    baseAddr = s.second.second;
        uint8_t     *secBasePtr = s.first;

        cout << "section " << to_string<int>(k, dec);
        cout << " of " << to_string<int>(sections.size(), dec) << endl;

        for( uint32_t i = 0; i < len; i++ ) {
            uint8_t     *curP = secBasePtr+i;
            bool        r;
            BlockPtr    bl;

            print_progress(len, i);
            r = convertToOneBlock(  ctx,
                                    curP,
                                    (len-i),
                                    baseAddr+i,
                                    a,
                                    bl);
            if( r ) {
                //blocksFound.push_back(bl);
            } 
        }

        cout << endl;
    }

    cout << "all sections read" << endl;
    finiDecodeLib(ctx);
    cout << to_string<int>(blocksFound.size(), dec) << endl;

    /* serialize the block list into a file */
    if( writeToFile(outputFile, inputFile, blocksFound) ) {
        cout << "Wrote out to file " << outputFile << endl;
    } else {
        cout << "Failed to write out" << endl;
    }

    /* aaand done */

    return 0;
}
