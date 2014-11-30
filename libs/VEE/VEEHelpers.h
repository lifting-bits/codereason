void getConstArgs(std::vector<ExpressionPtr> args, std::vector<ConstantValue> &consts );

void applyPreConditions(VexExecutionStatePtr vss, Condition *c);
bool checkPostConditions(VexExecutionStatePtr vss, Condition *c);
boost::uint16_t getArgLen(ExpressionPtr opArg);
boost::uint16_t getStrideFromWidth(boost::uint16_t width);
