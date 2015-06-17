//this is the AST for the expression stuff

#include <boost/variant/recursive_variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <string>
#include <vector>

struct expression_call;

typedef
    boost::variant<
        boost::recursive_wrapper<expression_call>,
        int,
        std::string 
    >
	expression;

struct expression_call
{
    std::string                     symName;
    std::vector<expression>         args;
};

struct statement 
{
    expression  assign;
    expression  source;
};

BOOST_FUSION_ADAPT_STRUCT(
    expression_call,
    (std::string, symName)
    (std::vector<expression>, args)
)

BOOST_FUSION_ADAPT_STRUCT(
   	statement,
    (expression, assign)
    (expression, source)
)

typedef std::vector<statement> conditionFile;