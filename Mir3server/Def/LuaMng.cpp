//#include "../../Mir3server/GameSvr/StdAfx.h"
//#include "Shlwapi.h"
//#pragma comment(lib,"Shlwapi.lib") 
//
//std::map<lua_State*, std::list<std::string>> LuaMng::luaMap;
//LuaMng::LuaMng()
//{
//}
//
//
//LuaMng::~LuaMng()
//{
//}
//
//
//extern "C" int LuaInclude(lua_State * state)
//{
//	const char * file;
//	file = lua_tostring(state, 1);
//	return (int)g_xLuaMng.DoFile((char*)file);
//}
//
//extern "C" int MsgBox(lua_State* state)
//{
//	const char * text;
//	text = lua_tostring(state, 1);
//	const char * title;
//	title = lua_tostring(state, 2);
//	int code = lua_tonumber(state, 3);
//	return MessageBoxA(NULL, text, title, code);
//}
//
//
//static luaL_Reg AllFunction[] = 
//{
//	{"include", LuaInclude},
//	{"MsgBox", MsgBox},
//};
//
//void LuaMng::Init()
//{
//	if (pLuaState != NULL)
//		return;
//	pLuaState = luaL_newstate();
//	luaL_openlibs(pLuaState);
//	std::list<std::string> values;
//	luaMap.insert(pair<lua_State*, list<std::string>>(pLuaState, values));
//	//注册接口给lua脚本调用.
//	for (int i = 0; i < sizeof(AllFunction)/sizeof(luaL_Reg); i++)
//		lua_register(pLuaState, AllFunction[i].name, AllFunction[i].func);
//	luaMap[pLuaState].push_back("luascript/startup.lua");
//	luaL_dofile(pLuaState, "luascript/startup.lua");
//}
//
//bool LuaMng::DoFile(char * file)
//{
//	char filename[MAX_PATH + 1];
//	int len = strlen(file);
//	if (len > MAX_PATH)
//		return false;
//	filename[len] = 0x00;
//	for (int i = 0; i < len; i++)
//	{
//		if (file[i] == '\\')
//			filename[i] = '/';
//		else
//			filename[i] = tolower(file[i]);
//	}
//
//
//	if (pLuaState != NULL && PathFileExistsA(filename))
//	{
//		list<string>::iterator iter = luaMap[pLuaState].begin();
//		while (iter != luaMap[pLuaState].end())
//		{
//			//exist
//			if (strcmp(iter->c_str(), file) == 0)
//				return true;
//			iter++;
//		}
//		luaL_dofile(pLuaState, file);
//		string str(file);
//		luaMap[pLuaState].push_back(str);
//		return true;
//	}
//	return false;
//}
//
//void LuaMng::DoString(char * string)
//{
//	luaL_dostring(pLuaState, string);
//}
//
//void LuaMng::DoLuaVoidFun(char * function)
//{
//	if (pLuaState != NULL)
//	{
//		lua_getglobal(pLuaState, function);
//		lua_call(pLuaState, 0, 0);
//	}
//}
//
//void LuaMng::AddFunction(char * funname, lua_CFunction func)
//{
//	if (pLuaState != NULL)
//		lua_register(pLuaState, funname, func);
//}