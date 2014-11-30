#include <getExec.h>

#include <boost/program_options/config.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>

#include <boost/regex.hpp>

using namespace boost;
using namespace std;

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

TargetArch archFromVM(program_options::variables_map &vm) {
    TargetArch  tarch;

    tarch.ta = INVALID;

    if( vm.count("arch") ) {
        string archStr = vm["arch"].as<std::string>();

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

FileFormat fmtFromVM(program_options::variables_map &vm) {
    if( vm.count("pe") ) {
        return PEFmt;
    }

    if( vm.count("mach-o") ) {
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

int main(int argc, char *argv[]) {
    program_options::options_description    d("options");
    program_options::variables_map          vm;

    d.add_options()
        ("help,h", "print help")
        ("pe", "specify PE input file")
        ("mach-o", "specify mach-o input file")
        ("ucache", "usercache")
        ("raw", "raw input file")
        ("list-files", "list the available files")
        ("list-sections", "list all the avilable sections and files")
        ("arch", program_options::value<string>(), "target architecture")
        ("infile,f", program_options::value<string>(), "input file")
        ("filter", program_options::value<string>(), "file filter expression");

    program_options::store(
                program_options::parse_command_line(argc, argv, d), vm);

    if( vm.count("help") ) {
        cout << d << endl;
        return 0;
    }
    
    string inFilePath;

    if( vm.count("infile" ) ) {
        inFilePath = vm["infile"].as<string>();
    }

    if( inFilePath.size() == 0 ) {
        cout << d << endl;
        return -1;
    }

    string              fileFilter = ".*";

    if( vm.count("filter") ) {
        fileFilter = vm["filter"].as<string>();
    }

    regex               filter(fileFilter);
    smatch              sm;

    TargetArch          tarch = archFromVM(vm);
    FileFormat          fmt = fmtFromVM(vm);
    assert(fmt != Invalid);
    assert(tarch.ta != INVALID);
    ExecCodeProvider    ecp = ExecCodeProvider(inFilePath, fmt, tarch); 

    if( vm.count("list-files") ) {
        list<string>    fn = ecp.filenames();
        cout << "files: " << endl;
        for( list<string>::iterator i = fn.begin(); i != fn.end(); ++i ) {
            if( regex_search(*i, sm, filter) ) {
                cout << *i << endl;
            }
        }
    }

    if( vm.count("list-sections") ) {
        list<string>    fn = ecp.filenames();
        for( list<string>::iterator i = fn.begin(); i != fn.end(); ++i ) { 
            secVT   secs = ecp.sections_in_file(*i);
           
            if( !regex_search(*i, sm, filter) ) {
                continue;
            }

            cout << "in file " << *i << endl;
            cout << "got " << secs.size() << " +X sections" << endl;
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
                cout << " of size " << 
                    to_string<uint64_t>(lat.first, dec) << endl;
                cout << "------------------" << endl;
            } 
        }
    }

    return 0;
}
