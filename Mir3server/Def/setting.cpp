#include <iostream>
#include <Windows.h>
#include <string>
#include <Shlwapi.h>
#include <map>
#include <list>
using namespace std;
#include "Setting.h"
#pragma comment(lib, "shlwapi.lib")

void Setting::LoadCfg(char * file)
{
	mData.clear();
	if (file == NULL)
	{
		string strError = "参数为空";
		MessageBoxA(0, strError.c_str(), "错误", MB_OK);
		return;
	}
	sprintf(filepath, "%s", file);
	FILE * fp = fopen(filepath, "rb+");
	if (fp == NULL)
	{
		char path[MAX_PATH];
		char driver[64];
		char dir[MAX_PATH];
		char filename[MAX_PATH];
		char ext[MAX_PATH];
		GetModuleFileNameA(NULL, path, MAX_PATH);
		_splitpath(path, driver, dir, filename, ext);
		sprintf(filepath, "%s%s%s", driver, dir, file);
		if (NULL == (fp = fopen(filepath, "rb+")))
		{
			string strError = "can not find file:";
			strError += file;
			MessageBoxA(0, strError.c_str(), "错误", MB_OK);
			return;
		}
	}
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char * readbuffer = new char[len];
	fread(readbuffer, len, 1, fp);
	const char * pSectionStart = NULL;
	const char * pSectionEnd = NULL;
	const char * pSectionKeyStart = NULL;
	const char * pSectionKeyEnd = NULL;
	const char * pSectionValueStart = NULL;
	const char * pSectionValueEnd = NULL;

	const char * linebegin = NULL;
	const char * lineend = NULL;
	string strValid;

	for (int i = 0; i < len; i++)
	{
		if (readbuffer[i] == '\t')
			continue;
		strValid += readbuffer[i];
	}

	const char * text = strValid.c_str();
	linebegin = text;
	pSectionKeyStart = text;
	
	string sSection;
	int line = 0;
	int sectionIdx = 0;
	map<int, bool> lineequal;
	for (int i = 0; i < strValid.length(); i++)
	{
		if (text[i] == '[')
			pSectionStart = text + i + 1;
		if (text[i] == ']')
		{
			sectionIdx++;
			pSectionEnd = text + i;
			int skiped = 0;
			char * section = new char[pSectionEnd - pSectionStart + 1];
			for (int j = 0; j < pSectionEnd - pSectionStart; j++)
				section[j] = (*(pSectionStart + j));
			section[pSectionEnd - pSectionStart] = 0x00;
			map<string, string> innerValue;
			sSection = section;
			SetSection * pInsert = new SetSection(section);
			if (pInsert != NULL)
				mData.push_back(pInsert);
			delete []section;
		}

		if ((text[i] == '\r' && text[i + 1] == '\n') || (i == strValid.length() - 1) || text[i] == '\n')
		{
			bool bWriteKeyValuePair = false;
			lineend = text + i;
			pSectionValueEnd = text + i;
			if (pSectionEnd != NULL && 
				pSectionStart != NULL && 
				pSectionKeyStart != NULL && 
				pSectionKeyEnd != NULL && 
				pSectionValueStart != NULL && 
				pSectionValueEnd != NULL && sSection != "")
			{
				if (pSectionValueEnd >= pSectionValueStart && pSectionKeyEnd > pSectionKeyStart && pSectionValueStart > pSectionKeyEnd)
				{
					char * key = new char[pSectionKeyEnd - pSectionKeyStart + 1];
					for (int j = 0; j < pSectionKeyEnd - pSectionKeyStart; j++)
						key[j] = (*(pSectionKeyStart + j));
					key[pSectionKeyEnd - pSectionKeyStart] = 0x00;

					char * value = new char[pSectionValueEnd - pSectionValueStart + 1];
					for (int j = 0; j < pSectionValueEnd - pSectionValueStart; j++)
						value[j] = (*(pSectionValueStart + j));
					value[pSectionValueEnd - pSectionValueStart] = 0x00;

					string keyTmp = key;
					string valueTmp = value;

					SetItem * pItem = new SetItem(keyTmp, value);
					SetSection * pSection = FindSection(sSection);
					if (pSection != NULL)
						pSection->PushBack(pItem);

					delete[]key;
					delete[]value;
					bWriteKeyValuePair = true;
				}
			}

			if (!bWriteKeyValuePair)
			{
				string sline;
				for (int i = 0; i < lineend - linebegin; i++)
				{
					char alpha = (*(linebegin + i));
					if (alpha == ' ')
						continue;
					sline += (*(linebegin + i));
				}

				if (sline.find(';') == 0)
				{
					SetItem * pItem = new SetItem("", sline);
					SetSection * pSection = FindSection(sSection);
					if (pSection != NULL)
						pSection->PushBack(pItem);
				}
			}

			int offset = 0;
			if (text[i + 1] == '\n')
				offset = 2;
			else
				offset = 1;
			pSectionKeyStart = text + i + offset;
			line++;
			linebegin = text + i + offset;

			if (*linebegin == ';')
			{
				int xxx = 0;
			}
		}

		if (text[i] == '=')
		{
			if (lineequal.find(line) == lineequal.end())
			{
				pSectionKeyEnd = text + i;
				pSectionValueStart = text + i + 1;
				lineequal.insert(make_pair(line, true));
			}
		}
	}
	fclose(fp);
}

int Setting::GetValueInt(string section, string key)
{
	string strVal;
	SetSection * pSection = FindSection(section);
	if (pSection != NULL)
	{
		strVal = pSection->GetValue(key);
	}

	if (strVal == "")
		return 0;
	return atoi(strVal.c_str());
}


string Setting::GetValueString(string section, string key)
{
	string strVal;
	SetSection * pSection = FindSection(section);
	if (pSection != NULL)
	{
		return pSection->GetValue(key);
	}
	return strVal;
}

bool Setting::SetValueInt(string section, string key, int value)
{
	bool ret = false;
	char szValue[32];
	sprintf(szValue, "%d", value);
	SetSection * pSection = FindSection(section);
	if (pSection != NULL)
	{
		pSection->Assign(key, szValue);
	}
	else
	{
		pSection = new SetSection(section);
		SetItem * pItem = new SetItem(key, szValue);
		pSection->PushBack(pItem);
		mData.push_back(pSection);
	}
	return ret;
}

bool Setting::SetValueString(string section, string key, string value)
{
	bool ret = false;
	SetSection * pSection = FindSection(section);
	if (pSection != NULL)
	{
		pSection->Assign(key, value);
	}
	else
	{
		pSection = new SetSection(section);
		SetItem * pItem = new SetItem(key, value);
		pSection->PushBack(pItem);
		mData.push_back(pSection);
	}
	return ret;
}

//找到已经存在的文件，然后存储设置.
bool Setting::Save(char * file)
{
	char * path = file;
	if (file == NULL)
		path = filepath;
	
	FILE * fp = NULL;
	if (!::PathFileExistsA(path))
	{
		char path[MAX_PATH];
		char driver[64];
		char dir[MAX_PATH];
		char filename[MAX_PATH];
		char ext[MAX_PATH];
		GetModuleFileNameA(NULL, path, MAX_PATH);
		_splitpath(path, driver, dir, filename, ext);
		if (file != NULL)
		{
			sprintf(path, "%s%s%s", driver, dir, file);
			fp = fopen(path, "wb+");
		}
	}

	if (fp == NULL)
		fp = fopen(path, "wb+");
	//文件被独占了
	if (fp == NULL)
		return false;

	string textWriter;

	list<SetSection*>::iterator iter = mData.begin();
	while (iter != mData.end())
	{
		textWriter += "[" + (*iter)->section + "]\r\n";
		list<SetItem>::iterator inner = (*iter)->data.begin();
		while (inner != (*iter)->data.end())
		{
			if (inner->key == "")
				textWriter += inner->value + "\r\n";
			else
				textWriter += inner->key + "=" + inner->value + "\r\n";
			inner++;
		}
		iter++;
	}

	fwrite(textWriter.c_str(), textWriter.length(), 1, fp);
	fclose(fp);
	return true;
}

SetSection * Setting::FindSection(string section)
{
	SetSection * pRet = NULL;
	list<SetSection*>::iterator iter = mData.begin();
	while (iter != mData.end())
	{
		if ((*iter)->section == section)
			return *iter;
		iter++;
	}
	return NULL;
}