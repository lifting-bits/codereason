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

int main(int argc, char *argv[]) {
    program_options::options_description    d("options");
    program_options::variables_map          vm;
    string                                  inputFile;

    d.add_options()
        ("database,db", program_options::value<string>(), "input block database");

    program_options::store(
            program_options::parse_command_line(argc, argv, d), vm);

    /* try and read the input */  
    if( vm.count("database") ) {
        inputFile = vm["database"].as<string>();
    }

    if( inputFile.size() == 0 ) {
        cout << d << endl;
        return 1;
    }
    
    list<BlockPtr>  blocks;
    string          modName;
    if( !readFromFile(inputFile, modName, blocks) ) {
        cout << "Failure to read input file " << inputFile << endl;
        return 1;
    }

    cout << "read module " << modName << endl;
    cout << "read in blocks " << to_string<size_t>(blocks.size(), dec);
    for(list<BlockPtr>::iterator it = blocks.begin(); it != blocks.end(); ++it){
        cout << (*it)->printBlock();
    }
    cout << endl;

    return 0;
}
