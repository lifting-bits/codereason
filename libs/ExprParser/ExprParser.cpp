#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include <string>
#include <vector>
#include <fstream>

#include "ExprAST.h"
#include "ExprParser.h"

#include <BasicIR.h>

namespace fusion = boost::fusion;
namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename Iterator>
struct statement_grammar 
  : qi::grammar<Iterator, 
    statement(), 
    qi::locals<std::string>, 
    ascii::space_type>
{
    statement_grammar()
      : statement_grammar::base_type(stmt)
    {
        using qi::int_;
        using ascii::char_;
        using namespace qi::labels;
        
        text %= char_("a-zA-Z_?$") >> *char_("a-zA-Z_0-9");
        
        expr %= ( expr_call | int_ | text );

        expr_call %= text
                    >> "[" 
                    >> expr % ','
                    >> "]";

        stmt %= expr >> "=" >> expr;
    }

    qi::rule<   Iterator, 
                statement(), 
                qi::locals<std::string>, 
                ascii::space_type> 
        stmt;
    qi::rule<   Iterator, 
                expression(), 
                ascii::space_type>
        expr;
    qi::rule<   Iterator,
                expression_call(),
                ascii::space_type>
        expr_call;
    qi::rule<Iterator, std::string(), ascii::space_type> text;
};

struct expression_call_printer {

	void operator()(expression_call const &) const;
};

struct expression_printer : boost::static_visitor<> {

	void operator()(expression_call const &call) const {
		expression_call_printer()(call);
		return;
	}

	void operator()(std::string const &text) const {
		std::cout << " " << text << " ";
		return;
	}

	void operator()(int const &i) const {
		std::cout << " " << i << " ";
		return;
	}
};

struct statement_printer {
	void operator()(statement const &s) const {
		boost::apply_visitor(expression_printer(), s.assign);
		std::cout << " = ";
		boost::apply_visitor(expression_printer(), s.source);
		std::cout << std::endl;
		return;
	}
};

void expression_call_printer::operator()(expression_call const &c) const {
	std::cout << c.symName << "[";
	BOOST_FOREACH(expression const &e, c.args) {
		boost::apply_visitor(expression_printer(), e);
	}

	std::cout << "]";
	return;
}

conditionFile parse_file(std::string filePath) {
	conditionFile 				fileContents;
	std::vector<std::string>	fileStrings;

	//std::cout << "reading from file " << filePath << std::endl;
	//read in the file into a vector of strings 
	std::ifstream 	in(filePath.c_str());

	while( in.good() ) {
		std::string 	line;

		std::getline(in, line, '\n');

		if( line.size() > 0 ) {
			//std::cout << "read line " << line << std::endl;

			fileStrings.push_back(line);
		}
	}

	//for each line, parse out a statement
	for(std::vector<std::string>::iterator it = fileStrings.begin();
		it != fileStrings.end();
		++it)
	{
		std::string 	s = *it;

		std::string::const_iterator 	iter = s.begin();
		std::string::const_iterator 	end = s.end();

		using boost::spirit::ascii::space;
		typedef statement_grammar<std::string::const_iterator>
			statement_grammar;
		statement_grammar 	g;
		statement 			st;

		bool r = phrase_parse(iter, end, g, space, st);

		if( r ) {
			//parsing successful
			//std::cout << "Parsing line success" << std::endl;
			fileContents.push_back(st);
		} else {
			//throw some kind of exception
			throw ParseFail("Error parsing");
		}
	}

	return fileContents;
}

OpPtr op_from_str(std::string symName) {
	Op 	o = Op::UNSUP;

	if( symName == "Add" ) {
		o = Op::Add;
	} else if( symName == "Sub" ) {
		o = Op::Sub;
	}

	return OpPtr( new Op(o) );
}

Register regFromStr(std::string s, TargetArch ta) {
	Register	r;
	TargetArch	x86Arch = { X86 };

	r.width = 0;
	if( s == "EAX" ) {
		r.arch = x86Arch;
		r.width = 32;
		r.Reg32 = EAX;
	} else if( s == "EBX" ) {
		r.arch = x86Arch;
		r.width = 32;
		r.Reg32 = EBX;
	} else if( s == "ECX" ) {
		r.arch = x86Arch;
		r.width = 32;
		r.Reg32 = ECX;
	} else if( s == "EDX" ) {
		r.arch = x86Arch;
		r.width = 32;
		r.Reg32 = EDX;
	} else if( s == "EBP" ) {
		r.arch = x86Arch;
		r.width = 32;
		r.Reg32 = EBP;
	} else if( s == "ESP" ) {
		r.arch = x86Arch;
		r.width = 32;
		r.Reg32 = ESP;
	} else if( s == "EDI" ) {
		r.arch = x86Arch;
		r.width = 32;
		r.Reg32 = EDI;
	} else if( s == "ESI" ) {
		r.arch = x86Arch;
		r.width = 32;
		r.Reg32 = ESI;
	} else if( s == "EIP" ) {
		r.arch = x86Arch;
		r.width = 32;
		r.Reg32 = EIP;
	} else if( s == "?" ) {
		r.arch = x86Arch;
		r.width = 32;
		r.Reg32 = ANY32;
	}

	return r;
}

struct expression_builder : boost::static_visitor<> {

	expression_builder(ExpressionPtr &e) : building_expr(e) { }

	//strings are turned into ExGet 
	//UNLESS, they are "_", in which case they become an ANY regread
    //OR, they are MEMREAD, in which case they become ExLoad
	void operator()(std::string const &text) const {
		
		if( text == "_" ) {
      /* FIXME for ARM and AMD64 support later */
      Register    r;
      TargetArch  tarch = { X86 };
      r.arch = tarch;
      r.width = 32;
      r.Reg32 = ANY32;

      Expression 	    *e = new ExGet(r);
      ExpressionPtr	t(e);

			building_expr = t;
		} else if( text[0] == '$') {
      ConstantValue v;

      v.valueIsKnown = false;
      v.width = boost::lexical_cast<int>(text.substr(1));
      switch(v.width) {
        case 8:
          v.valueType = ConstantValue::T_I8;
          break;
        case 16:
          v.valueType = ConstantValue::T_I16;
          break;
        case 32:
          v.valueType = ConstantValue::T_I32;
          break;
        case 64:
          v.valueType = ConstantValue::T_I64;
          break;
      }

      Expression  *e = new ExConst(v);
      ExpressionPtr t(e);
      building_expr = t;
    } else {
      TargetArch  tarch = { X86 };
			Register	r = regFromStr(text, tarch);

			if( r.width == 0 ) {
				throw SemanticFail("Unknown register "+text);
			}

			Expression 	*e = new ExGet(r);
			ExpressionPtr 	t(e);
			this->building_expr = t;
		}

		return;
	}

	//ints are turned into ExConst 
	void operator()(int const &i) const {
		ConstantValue 	v;

		v.valueIsKnown = true;
		v.width = 32;
		v.U32 = i;
		v.valueType = ConstantValue::T_I32;

		Expression *e = new ExConst(v);
		ExpressionPtr 	t(e);
		building_expr = t;

		return;
	}

	//expression_calls are turned into ExOp with some number of parameters
	//they could also be turned into ExLoad 
	//they could also be turned into ExLogic
	void operator()(expression_call const &c) const {
		std::string 	symName = c.symName;
		if( symName == "MEMREAD") {
			//there should only be one argument to memread
			//TODO make throw
			assert(c.args.size() == 1);
			ExpressionPtr 		srcAddr;
			expression_builder 	exBldr(srcAddr);

			expression 	e = c.args[0];

			boost::apply_visitor(exBldr, e);

			Expression 	*n = new ExLoad( srcAddr, ConstantValue::T_I32);
			ExpressionPtr 	t(n);

			this->building_expr = t;
		//if it is not a memread, then maybe it is an ExLogic
		} else if( symName == "EQ" || symName == "NOT" || symName == "GT" ||
			symName == "AND" || symName == "OR" || symName == "LT" ||
			symName == "RANGE") 
		{
			if( symName == "EQ" ) {
				if( c.args.size() != 2 ) {
					throw SemanticFail("Too many arguments to logic function");
				}

				//get the expression arguments to this logic operation
				ExpressionPtr	lhs, rhs;
				expression		lhs_e = c.args[0];
				expression		rhs_e = c.args[1];

				expression_builder 	exLhsBuilder(lhs);
				expression_builder	exRhsBuilder(rhs);
				boost::apply_visitor(exLhsBuilder, lhs_e);
				boost::apply_visitor(exRhsBuilder, rhs_e);

				Expression		*l = new ExLogic(ExLogic::Eq, lhs, rhs);
				ExpressionPtr 	p(l);
				this->building_expr = p;
			} else if( symName == "NOT" ) {

			} else if( symName == "GT" ) {
				if( c.args.size() != 2 ) {
					throw SemanticFail("Too many arguments to logic function");
				}

				//get the expression arguments to this logic operation
				ExpressionPtr	lhs, rhs;
				expression		lhs_e = c.args[0];
				expression		rhs_e = c.args[1];

				expression_builder 	exLhsBuilder(lhs);
				expression_builder	exRhsBuilder(rhs);
				boost::apply_visitor(exLhsBuilder, lhs_e);
				boost::apply_visitor(exRhsBuilder, rhs_e);

				Expression		*l = new ExLogic(ExLogic::GreaterThan, lhs, rhs);
				ExpressionPtr 	p(l);
				this->building_expr = p;
			} else if( symName == "AND" ) {
				if( c.args.size() != 2 ) {
					throw SemanticFail("Too many arguments to logic function");
				}

				//get the expression arguments to this logic operation
				ExpressionPtr	lhs, rhs;
				expression		lhs_e = c.args[0];
				expression		rhs_e = c.args[1];

				expression_builder 	exLhsBuilder(lhs);
				expression_builder	exRhsBuilder(rhs);
				boost::apply_visitor(exLhsBuilder, lhs_e);
				boost::apply_visitor(exRhsBuilder, rhs_e);

				Expression		*l = new ExLogic(ExLogic::And, lhs, rhs);
				ExpressionPtr 	p(l);
				this->building_expr = p;
			} else if( symName == "OR" ) {
				if( c.args.size() != 2 ) {
					throw SemanticFail("Too many arguments to logic function");
				}

				//get the expression arguments to this logic operation
				ExpressionPtr	lhs, rhs;
				expression		lhs_e = c.args[0];
				expression		rhs_e = c.args[1];

				expression_builder 	exLhsBuilder(lhs);
				expression_builder	exRhsBuilder(rhs);
				boost::apply_visitor(exLhsBuilder, lhs_e);
				boost::apply_visitor(exRhsBuilder, rhs_e);

				Expression		*l = new ExLogic(ExLogic::Or, lhs, rhs);
				ExpressionPtr 	p(l);
				this->building_expr = p;
			} else if( symName == "LT" ) {
				if( c.args.size() != 2 ) {
					throw SemanticFail("Too many arguments to logic function");
				}

				//get the expression arguments to this logic operation
				ExpressionPtr	lhs, rhs;
				expression		lhs_e = c.args[0];
				expression		rhs_e = c.args[1];

				expression_builder 	exLhsBuilder(lhs);
				expression_builder	exRhsBuilder(rhs);
				boost::apply_visitor(exLhsBuilder, lhs_e);
				boost::apply_visitor(exRhsBuilder, rhs_e);

				Expression		*l = new ExLogic(ExLogic::LessThan, lhs, rhs);
				ExpressionPtr 	p(l);
				this->building_expr = p;
			} else if( symName == "RANGE" ) {
				if( c.args.size() != 2 ) {
					throw SemanticFail("Too many arguments to logic function");
				}

				//get the expression arguments to this logic operation
				ExpressionPtr	lhs, rhs;
				expression		lhs_e = c.args[0];
				expression		rhs_e = c.args[1];

				expression_builder 	exLhsBuilder(lhs);
				expression_builder	exRhsBuilder(rhs);
				boost::apply_visitor(exLhsBuilder, lhs_e);
				boost::apply_visitor(exRhsBuilder, rhs_e);

				Expression		*l = new ExLogic(ExLogic::Range, lhs, rhs);
				ExpressionPtr 	p(l);
				this->building_expr = p;
			}
		} else {
			std::vector<ExpressionPtr> 	OpArgs;
			OpPtr 						o = op_from_str(c.symName);

			//if we can't classify it as an ExOp, then we throw a SemanticFail
			if( o->getOp() != Op::UNSUP ) {
				BOOST_FOREACH(expression const &e, c.args) {
					ExpressionPtr 				arg;
					expression_builder 			exBldr(arg);	

					boost::apply_visitor(exBldr, e);

					assert( arg );
					OpArgs.push_back(arg);
				}

				Expression 	*e = new ExOp(o, OpArgs);
				ExpressionPtr 	t(e);

				this->building_expr = t;
			} else {
				throw SemanticFail("Unknown function "+c.symName);	
			}
		}
		return;
	}

	ExpressionPtr 	&building_expr;
};

struct statement_builder : boost::static_visitor<> {

	statement_builder(StatementPtr &out, ExpressionPtr in) : 
		building_stmt(out), rhs(in) { }	

	void operator()(int const &i) const {
		//most likely, this is always a failure. I can't think
		//of a time when this would be valid, so just throw a 
		//semantic fail

		throw SemanticFail("Statement assign to integer");
		return;
	}

	void operator()(std::string const &text) const {
		//these could be regwrites
		//they could also be temp defines
		//if neither of those, then throw a semantic fail
        TargetArch  ta = { X86 };
		Register r = regFromStr(text, ta); 

		if( r.width != 0 ) {
			Statement 	*s = new StPut(r, this->rhs);
			StatementPtr t(s);
			this->building_stmt = t;
		} else if( text[0] == 'x' )	{
            /* need a statement for variable writes */ 
            TempValPtr      c = 
                TempValPtr(new TempVal(text));
            StatementPtr    s =
                StatementPtr(new StWrTmp(c, this->rhs));
            this->building_stmt = s;
		} else if( text == "_" ) {
            /* FIXME for ARM support later */
            TargetArch  tarch = { X86 };
            r.arch = tarch;
            r.width = 32;
            r.Reg32 = ANY32;
            Statement       *s = new StPut(r, this->rhs);
            StatementPtr    t(s);
            this->building_stmt = t;
        } else {
			throw SemanticFail("Unsupported assignment "+text);	
		}
		return;
	}

	void operator()(expression_call const &c) const {
		//could be MEMWRITE, in which case evals to StStore
		//if something else, throw a semantic fail
		std::string 	tgt = c.symName;	

		if( tgt == "MEMWRITE" ) {
			if( c.args.size() != 1 ) {
				throw SemanticFail("Incorrect number of args to MEMWRITE");
			}
			expression 			arg = c.args[0];
			ExpressionPtr 		exAddr;
			expression_builder 	exBuild(exAddr);
			boost::apply_visitor(exBuild, arg);

			//create a StStore
			Statement 		*s = new StStore(exAddr, this->rhs);
			StatementPtr 	t(s);
			this->building_stmt = t; 
		} else {
			throw SemanticFail("Unknown assign type"+tgt);
		}
		return;
	}

	StatementPtr 	&building_stmt;
	ExpressionPtr 	rhs;
};

StatementPtr statement_to_statement(statement &s) {
	//statement_printer()(s);

	//first get the rhs expression 
	ExpressionPtr 		rhs;
	expression_builder 	exBldr(rhs);
	boost::apply_visitor(exBldr, s.source);
	//std::cout << rhs->printExpr() << std::endl;

	//then build the appropriate statement by examining the lhs
	StatementPtr 	lhs;
	statement_builder	stBldr(lhs, rhs);
	boost::apply_visitor(stBldr, s.assign);
	//std::cout << lhs->printStmt() << std::endl;

	return lhs;
}

std::vector<StatementPtr> conditions_to_statements(conditionFile c) {
	std::vector<StatementPtr>	exprs;

	for( conditionFile::iterator it = c.begin(); it != c.end(); ++it) {
		statement 		astStmt = *it;	
		StatementPtr	stmt = statement_to_statement(astStmt);

		if( stmt ) {
			exprs.push_back(stmt);
		}
	}

	return exprs;
}

std::vector<StatementPtr> getExprsForFile(std::string filePath) {
	std::vector<StatementPtr>	exprs;
	conditionFile 				cf;

	cf = parse_file(filePath);
	exprs = conditions_to_statements(cf);

	return exprs;	
}
