#include <stdio.h>

#include "IniFile.h"

#define DEBUG_PRINT(args...) printf(args)
//#define DEBUG_PRINT(args...)

// コンストラクタ
IniFile::IniFile()
{
	m_keyFile = g_key_file_new();
}

// デストラクタ
IniFile::~IniFile()
{
	g_key_file_free(m_keyFile);
}

// ファイルから読み込む
int IniFile::load(const char* filepath)
{
	if(!g_key_file_load_from_file(
		m_keyFile,
		filepath,
		G_KEY_FILE_KEEP_COMMENTS,
		NULL))
	{
		g_key_file_free(m_keyFile);
		m_keyFile = g_key_file_new();
		
		DEBUG_PRINT("ERROR! IniFile::load() g_key_file_load_from_file()\n");
		return -1;
	}
	
	return 0;
}

// ファイルに書き出す
int IniFile::save(const char* filepath)
{
	gchar *pdata;
	gsize len;
	FILE *pconfile;
	
	pdata = g_key_file_to_data(m_keyFile,&len,NULL);
	if(!pdata)
	{
		DEBUG_PRINT("ERROR! IniFile::save() g_key_file_to_data()\n");
		return -1;
	}
	pconfile=fopen(filepath,"w");
	if(!pconfile)
	{
		DEBUG_PRINT("ERROR! IniFile::save() fopen()\n");
		return -2;
	}
	if(fwrite(pdata,len,1,pconfile)<1)
	{
		fclose(pconfile);
		
		DEBUG_PRINT("ERROR! IniFile::save() fwrite()\n");
		return -3;
	}
	fclose(pconfile);
	
	return 0;
}

// ブール型データ書き込み
void IniFile::writeBool   (const char* section, const char* key, bool value)
{
	g_key_file_set_boolean(m_keyFile, section, key, value);
}

// 整数型データ書き込み
void IniFile::writeInteger(const char* section, const char* key, int value)
{
	g_key_file_set_integer(m_keyFile, section, key, value);
}

// 文字列データ書き込み
void IniFile::writeString (const char* section, const char* key, std::string value)
{
	g_key_file_set_string(m_keyFile, section, key, value.c_str());
}

// ブール型データ読み出し
int IniFile::readBool   (const char* section, const char* key, bool *value)
{
	GError *error =NULL;
	*value = g_key_file_get_boolean(m_keyFile, section, key, &error);
	if(error != NULL){
		g_error_free (error);
		DEBUG_PRINT("ERROR! IniFile::writeBool() g_key_file_get_boolean()\n");
		return -1;
	}
	return 0;
}

// 整数型データ読み出し
int IniFile::readInteger(const char* section, const char* key, int *value)
{
	GError *error =NULL;
	*value = g_key_file_get_integer(m_keyFile, section, key, &error);
	if(error != NULL){
		g_error_free (error);
		DEBUG_PRINT("ERROR! IniFile::writeInteger() g_key_file_get_boolean()\n");
		return -1;
	}
	return 0;
}

// 文字列データ読み出し
int IniFile::readString (const char* section, const char* key, std::string* value)
{
	GError *error = NULL;
	gchar* pValue = NULL;
	pValue = g_key_file_get_string(m_keyFile, section, key, &error);
	if(error != NULL){
		g_error_free (error);
		DEBUG_PRINT("ERROR! IniFile::writeString() g_key_file_get_string()\n");
		return -1;
	}
	if(pValue != NULL){
		*value = (std::string)pValue;
		free(pValue);
	}else{
		return -2;
	}
	return 0;
}
