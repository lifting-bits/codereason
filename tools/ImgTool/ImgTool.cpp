#include <getExec.h>

#include <boost/program_options/config.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>

#include <boost/regex.hpp>

using namespace boost;
using namespace std;

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

// TODO: just make this a public function of ExecCodeProvider? ecp->getArchStr()?
string tarchToString(TargetArch a ) {
    switch(a.ta) {
        case INVALID:
            return "INVALID";
            break;
        case X86:
            return "X86";
            break;
        case AMD64:
            return "AMD64";
            break;
        case ARM:
            return "ARM";
            break;
        case PPC32:
            return "PPC32";
            break;
        case PPC64:
            return "PPC64";
            break;
        case S390X:
            return "S390X";
            break;
    }
}

// ImgTool should demo the functionality and usage of ExecCodeProvider
int main(int argc, char *argv[]) {
    program_options::options_description    d("options");
    program_options::variables_map          vm;
    string                                  inputFile;
    TargetArch                              ta;
    ExecCodeProvider *                      codeProvider;

    d.add_options()
        ("help,h", "print help")
        ("arch,a", program_options::value<string>(), "target architecture")
        ("file,f", program_options::value<string>(), "input file");

    program_options::store(
                program_options::parse_command_line(argc, argv, d), vm);

    if( vm.count("help") ) {
        cout << d << endl;
        return 1;
    }

    ta = archFromVM(vm);
    
    // get the input binary filename
    if( vm.count("file") ) {
        inputFile = vm["file"].as<std::string>();

        // create our executable wrapper
        codeProvider = new ExecCodeProvider(inputFile, ta, false);
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
    
    // print out info regarding the executable we just parsed
    secVT   secs = codeProvider->getExecSections();
    cout << "In file " << inputFile << endl;
    cout << "found " << secs.size() << " +X sections" << endl;
    cout << "------------------" << endl;
    for(secVT::iterator it = secs.begin(); it != secs.end(); ++it) {
        secAndArchT saa = *it;
        TargetArch  ta = saa.first;
        secPT       sp = saa.second;
        lenAddrT    lat = sp.second; 

        cout << "Section of arch ";
        cout << tarchToString(ta) << endl;
        cout << "beginning at 0x" << 
            to_string<uint64_t>(lat.second, hex);
        cout << " of size 0x" << 
            to_string<uint64_t>(lat.first, hex) << endl;
        cout << "------------------" << endl;
    } 

    return 0;
}
