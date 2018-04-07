#pragma once


class SetItem
{
public:
	SetItem(string k, string v)
	{
		key = k;
		value = v;
	}
	friend class SetSection;
	friend class Setting;
protected:
	string key;
	string value;
};


class SetSection
{
public:
	SetSection(string name)
	{
		section = name;
	}
	friend class Setting;

protected:
	//һ�����ж����ͬKey�����ﲻ�жϣ�ʹ��Setting��Ĺ�������������һ����ָ���ؼ��ֵ�ֵ
	void PushBack(SetItem * pValue)
	{
		if (pValue != NULL)
			data.push_back(*pValue);
	}

	string GetValue(string key)
	{
		string value;
		list<SetItem>::iterator iter = data.begin();
		while (iter != data.end())
		{
			if (iter->key == key)
				return iter->value;
			iter++;
		}
		return value;
	}

	
	void Assign(string key, string value)
	{
		list<SetItem>::iterator iter = data.begin();
		while (iter != data.end())
		{
			if (iter->key == key)
			{
				iter->value = value;
				return;
			}
			iter++;
		}
		if (iter == data.end())
		{
			SetItem pItem(key, value);
			data.push_back(pItem);
		}
	}

	string section;//[LoginGate] => LoginGate
	list<SetItem> data;
	
};

class Setting
{
public:
	Setting(char * file)
	{
		LoadCfg(file);
	}
	~Setting()
	{
		Save();
	}
	char filepath[MAX_PATH];
	void LoadCfg(char * file);
	bool Save(char * file = NULL);
	bool SetValueString(string section, string key, string value);
	bool SetValueInt(string section, string key, int value);
	string GetValueString(string section, string key);
	int GetValueInt(string section, string key);
	SetSection * FindSection(string section);
protected:
	list<SetSection*> mData;
};