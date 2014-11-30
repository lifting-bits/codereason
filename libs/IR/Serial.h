class ContextHandle;

ContextHandle *openFile(std::string);
bool appendToFile(ContextHandle *, BlockPtr);
bool writeOutFile(ContextHandle *, std::string);

bool writeToFile(std::string, std::string, std::list<BlockPtr>);
bool readFromFile(std::string, std::string &, std::list<BlockPtr> &);
