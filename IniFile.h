#ifndef	_INI_FILE_H_
#define	_INI_FILE_H_

#include <glib.h>
#include <string>

// iniファイル処理クラス
class IniFile
{
public:
	// コンストラクタ
	IniFile();
	// デストラクタ
	~IniFile();
	
	// ファイルから読み込む
	int load(const char* filepath);
	// ファイルに書き出す
	int save(const char* filepath);
	
	// ブール型データ書き込み
	void writeBool   (const char* section, const char* key, bool value);
	// 整数型データ書き込み
	void writeInteger(const char* section, const char* key, int value);
	// 文字列データ書き込み
	void writeString (const char* section, const char* key, std::string value);

	// ブール型データ読み出し
	int readBool   (const char* section, const char* key, bool *value);
	// 整数型データ読み出し
	int readInteger(const char* section, const char* key, int *value);
	// 文字列データ読み出し
	int readString (const char* section, const char* key, std::string* value);

private:
	GKeyFile *m_keyFile;
};

#endif

