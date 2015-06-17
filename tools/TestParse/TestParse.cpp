#include <iostream>
#include <ExprParser.h>

#include <boost/program_options/config.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>

int main(int argc, char *argv[]) {
	boost::program_options::options_description    d("options");    
    boost::program_options::variables_map          vm;
	//parse command line 
   	d.add_options()
        ("input,i", boost::program_options::value<std::string>(),"input file")
        ("help,h", "print help");

   	boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, d), vm);

   	if( vm.count("help") || !vm.count("input") ) {
   		std::cout << d << std::endl;
   		return 0;
   	}

   	std::string 	inFile = vm["input"].as<std::string>();

   	if( inFile.size() > 0 ) {
	   	std::vector<StatementPtr>	stmts;

	   	try {
	   		stmts = getExprsForFile(inFile);	
	   	} 
	   	catch ( SemanticFail s ) {
	   		std::cout << "semantic fail: " << s.fc << std::endl;
	   	}
	   	catch (ParseFail p ) {
	   		std::cout << "parser fail: " << p.fc << std::endl;
	   	}

	   	for(std::vector<StatementPtr>::iterator it = stmts.begin();
	   		it != stmts.end();
	   		++it)
	   	{
	   		StatementPtr 	s = *it;

	   		std::cout << s->printStmt();
	   	}
   	}

    return 0;
}
