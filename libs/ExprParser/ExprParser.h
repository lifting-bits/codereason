#include <vector>
#include <string>
#include <BasicIR.h>

class ParseFail {
public:
	ParseFail(std::string failCause) : fc(failCause) { }

	std::string fc;
};

class SemanticFail {
public:
	SemanticFail(std::string failCause) : fc(failCause) { }

	std::string fc;
};

std::vector<StatementPtr> getExprsForFile(std::string filePath);