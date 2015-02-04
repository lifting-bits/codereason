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
    ExecCodeProvider*                       codeProvider;
    TargetArch                              ta;
    FileFormat                              fmt;
    string                                  inputFile;
    string                                  outputFile;
    string                                  filter(".*");
    unsigned int                            maxBlockSize;
    bool                                    raw;

    d.add_options()
        ("help,h", "print help")
        ("file,f", program_options::value<std::string>(), "input file")
        ("arch,a", program_options::value<std::string>(),"architecture - x86, x64, arm, thumb")
        ("raw", "interpret input file as raw blob")
        ("block-size", program_options::value<unsigned int>(), "max size in statements of blocks to search")
        ("blocks-out", program_options::value<std::string>(), "seralize input to a DB");
    
    program_options::store(
        program_options::parse_command_line(argc, argv, d), vm);

    if( vm.count("block-size") ) {
      maxBlockSize = vm["block-size"].as<unsigned int>();
    } else {
      maxBlockSize = 25;
    }

    if( vm.count("raw") )
    {
        raw = true;
    } else {
        raw = false;
    }

    if( vm.count("blocks-out") ) {
        outputFile = vm["blocks-out"].as<string>();
    }

    if( outputFile.size() == 0 ) {
        cout << "[ Error ] You must have an output file specified" << std::endl;
        cout << d << endl;
        return 1;
    }

    ta = archFromVM(vm);

    // get the input binary filename
    if( vm.count("file") ) {
        inputFile = vm["file"].as<std::string>();

        // create our executable wrapper
        codeProvider = new ExecCodeProvider(inputFile, ta, raw);
        if(codeProvider->getError())
        {
            return 1;
        }

        // retrieve the detected architecture
        ta = codeProvider->getArch();

        //TODO: sanity checks on codeProvider

    } else {
        std::cout << "Must specify an input file" << std::endl;
        std::cout << d << std::endl;
        return 1;
    }

    if( ta.ta == INVALID ) {
        cout << "[ Error ] Invalid architecture" << std::endl;
        cout << d << endl;
        return 1;
    }

    list<BlockPtr> blocksFound;
    secVT sections = codeProvider->getExecSections();
    void *ctx = initDecodeLib2(ta, true, false);

    if( ctx == NULL ) {
        cout << "Failed to initialize decoder library" << endl;
        return 1;
    }

    int k = 1;
    cout << "Extracting blocks..." << std::endl;

    /* produce a block for every offset from the executable section */
    for( secVT::iterator i = sections.begin(); i != sections.end(); ++i ) {
        TargetArch  a = (*i).first;
        secPT       s = (*i).second;
        uint32_t    len = s.second.first;
        uint64_t    baseAddr = s.second.second;
        uint8_t     *secBasePtr = s.first;

        cout << "Executable section " << to_string<int>(k, dec);
        cout << " of " << to_string<int>(sections.size(), dec) << endl;
        for( uint32_t i = 0; i < len; i++ ) {
            uint8_t     *curP = secBasePtr+i;
            uint64_t    baseVA = baseAddr + i;
            bool        r;
            BlockPtr    bl;

            r = convertToOneBlock(  ctx,
                                    curP,
                                    (len-i),
                                    baseVA,
                                    a,
                                    maxBlockSize,
                                    bl);
            if( r ) {
                blocksFound.push_back(bl);
            } 
        }

        k++;
        // print progress
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
