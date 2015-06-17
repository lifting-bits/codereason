bool transfersRegister(BlockPtr, Register);

bool exprReadsRegister(ExpressionPtr, Register, BlockPtr);

std::vector<Register> getGPRsForPlatform(TargetArch );

ConstantValue getValue(ExpressionPtr );

std::list<Transfer> simplifyTransfers(BlockPtr b);
ExpressionPtr simplifyExpr(BlockPtr b, ExpressionPtr e);
