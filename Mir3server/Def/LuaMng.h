//#pragma once
//using namespace std;
//#include <list>
//#include <iostream>
//class LuaMng
//{
//public:
//	LuaMng();
//	~LuaMng();
//	void Init();
//	bool DoFile(char * file);
//	void DoString(char * string);
//	void AddFunction(char * funname, lua_CFunction func);
//	//����lua�ű���ķ���
//	void DoLuaVoidFun(char * function);
//protected:
//	lua_State * pLuaState;
//	static std::map<lua_State*, std::list<std::string>> luaMap;
//};
//extern LuaMng				g_xLuaMng;