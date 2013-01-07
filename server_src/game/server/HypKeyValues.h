#pragma once
//#include <vector>
//using namespace std;

// I opted to have the ability to load several main blocks
// without needing to have them encased in one main block.
// This means that when you load keyvalues from a file or
// a buffer, you must do FindKey on the block you want to 
// access, even if the file/buffer has only one block.
// For example:

// FILE START - testfile.txt

// TestData
// {
//		"derp"	"derp1"
// }

// FILE END - testfile.txt

// HypKeyValues *pKV = new KeyValues();
// pKV->FileToKeyValues("testfile.txt");
// HypKeyValues *pTestDataKV = pKV->FindKey("TestData");
// if (pTestDataKV)
// {
//		const char *data = pTestDataKV->GetString("derp");
// }
// delete pKV;

// Note: Keyvalue names ("derp" in this example) are not case
// sensitive. You would still get the return value "derp1" from
// GetString() if you called GetString("DeRp");
#undef MAX_KEY
#undef MAX_VALUE
#define MAX_KEY 256				// The maximum length of a key.
#define MAX_VALUE (MAX_KEY * 2)	// The maximum length of a value.

// Max value of a color. If this is set to 3, the 
// maximum value of a color would be 999 999 999 999.
#define MAX_COLOR_DIGITS 3

enum HKVExistsType
{
	DOES_NOT_EXIST = 0,			// Key doens't exist.
	EXISTS_KVSET = (1 << 0),	// "keyname" { etc... }
	EXISTS_KEY = (1 << 1),		// "keyname" "value"
};

class HypKeyValue
{
public:
	HypKeyValue();
	~HypKeyValue();

	const char *GetName()		{ return m_name; }
	const char *GetString()		{ return m_content; }
	int GetInt();
	bool GetBool();
	float GetFloat();
	Color GetColor();
	
	void SetName(const char *name);
	void SetValue(const char *value);
	void SetValue(int value);
	void SetValue(float value);
	void SetValue(bool value);

private:
	//CUtlBuffer m_name;
	//CUtlBuffer m_content;
	char m_name[MAX_KEY];
	char m_content[MAX_VALUE];
};

class HypKeyValues
{
public:
	HypKeyValues(void);
	~HypKeyValues(void);

	bool FileToKeyValues(const char *szPath);			// Loads a file into the current scope.
	void KeyValuesToFile(const char *szPath);			// Saves the current scope into a file.
	void StringToKeyValues(CUtlBuffer &buffer, const char *szFileName = NULL);
	void KeyValuesToString(CUtlBuffer &buffer, int level = 0);			// Converts the current scope into a string.

	void SetName(const char *name);
	
	// Returns the name of the current keyValues structure.
	const char *GetName()				{ return m_name; }
	
	void SetParent(HypKeyValues *parent)	{ m_parent = parent; }
	HypKeyValues *GetParent()				{ return m_parent; }

	// Returns true if a new key was created, false otherwise.
	// forceNew forces a new key to be created even if another
	// already exists with the same name.
	bool SetValue(const char *keyName, const char *value, bool forceNew = false);
	bool SetValue(const char *keyName, int value, bool forceNew = false);
	bool SetValue(const char *keyName, float value, bool forceNew = false);
	bool SetValue(const char *keyName, bool value, bool forceNew = false);

	void GetString(const char *keyName, char *szOut, size_t maxSize, const char *defaultValue = "");
	CUtlVector<const char*> *GetStrings(const char *keyName, const char *defaultValues = "");

	void GetInt(const char *keyName, int &iOut, int defaultValue = 0);
	void GetFloat(const char *keyName, float &flOut, float defaultValue = 0.0f);
	void GetBool(const char *keyName, bool &bOut, bool defaultValue = false);
	void GetColor(const char *keyName, Color &out, Color defaultValue = Color());
	void GetColor(const char *keyName, ColorRGBExp32 &out, ColorRGBExp32 defaultValue = ColorRGBExp32());
	
	// Returns the first key in the current scope with the specified
	// name, or creates a new key if one with the specified name 
	// cannot be found.
	HypKeyValues *FindKey(const char *keyName, bool bCreate = false);

	// Returns the specified key in the current scope.
	HypKeyValues *GetKey(int keyIndex);
	int KeyCount() { return m_children.Count(); }

	// Returns the specified value in the current scope.
	HypKeyValue *GetValue(int valueIndex);
	int ValueCount() { return m_subKeyValues.Count(); }

	HKVExistsType ExistsInCurrentScope(const char *keyName);
	
	// Returns all keys in the current scope with the specified name, or 
	// creates a new key if one with the specified name cannot be found.
	CUtlVector<HypKeyValues*> *FindKeys(const char *keyName, bool bCreate = false);

	void AddSubKey( HypKeyValues *pKV )	{ m_children.AddToTail(pKV); }

private:
	// Checks the current subkeys for duplicate keys
	// with the same AssertHasDuplicatesname.
	void AssertHasDuplicates(const char *keyName);

	// Escapes the '"', '{', and '}' characters with '\' within a string.
	void EscapeString(const char *szString, char szDest[]);

	// Tests whether the current char (Tell()) has been escaped.
	bool IsEscaped(CUtlBuffer &buffer);

	// A simplification of a rather long if statment
	// to test if a character is a "break" character.
	bool IsBreakChar(char testChar);

	// A vector containing all the values in
	// the current scope.
	CUtlVector< HypKeyValue* >	m_subKeyValues;

	// The name of this keyvalue set.
	//CUtlBuffer					m_name;
	char m_name[MAX_KEY];

	// Our parent keyValues.
	HypKeyValues				*m_parent;

	// Our children keyValues.
	CUtlVector< HypKeyValues* >	m_children;

	enum CommentType_t
	{
		COMMENT_NONE,	// NO COMMENT :O
		COMMENT_EOL,	// A regular // comment that stops at the \n.
		COMMENT_FULL,	// A /* */ comment.
	};
};
