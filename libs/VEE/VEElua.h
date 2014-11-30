struct ScriptState {
    lua_State   *S;
    int         preCallbackIdx;
    int         postCallbackIdx;
};

void runPre(ScriptState *ss, VexExecutionStatePtr vss);
void runPost(ScriptState *ss, VexExecutionStatePtr vss, bool &res);
void runChase(ScriptState *ss, VexExecutionStatePtr vss, bool &res);
void finiScript(ScriptState *ss);
ScriptState *initScript(std::string scriptPath);
ScriptState *initScriptFromString(std::string script);
