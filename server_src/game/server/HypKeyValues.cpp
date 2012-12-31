// ************************************************
//  Hypovolemia HypKeyValues implementation by IBeMad
// ************************************************

#include "cbase.h"
#include "HypKeyValues.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


HypKeyValues::HypKeyValues(void)
{
	m_parent = NULL;
}

HypKeyValues::~HypKeyValues(void)
{
	m_subKeyValues.RemoveAll();
	m_children.RemoveAll();
}

bool HypKeyValues::FileToKeyValues(const char *szPath)
{
	CUtlBuffer buffer(0, 0, CUtlBuffer::TEXT_BUFFER);
	if (!filesystem->ReadFile(szPath, "GAME", buffer))
		return false;

	StringToKeyValues(buffer, szPath);
	
	buffer.Purge();

	return true;
}

void HypKeyValues::StringToKeyValues(CUtlBuffer &buffer, const char *szFileName)
{
	int iLevel = 0;	// How far we are into the structure
	//unsigned int charIndex = 0;
	char currentChar = buffer.GetChar();
	bool bInQuotes = false;
	CommentType_t commentStage = COMMENT_NONE;

	bool bHaveKey = false;	// Do we have something in our key vector?
	char key[MAX_KEY];
	unsigned short iKeyIndex = 0;
	bool bHaveValue = false;	// Do we have something in our value vector?
	char value[MAX_VALUE];
	unsigned short iValueIndex = 0;

	HypKeyValues *pCurrent = this;

	while (currentChar != '\0')	// foreach character
	{
		if (commentStage != COMMENT_NONE)
		{
			// Ignore characters and wait for newline or ending char
			char nextChar = *(char *)buffer.PeekGet(0);
			if (currentChar == '*' && nextChar == '/')
			{
				commentStage = COMMENT_NONE;
				buffer.SeekGet(CUtlBuffer::SEEK_CURRENT, 1);	// Skip the next forward slash
			}
			else if ((currentChar == '\r' || currentChar == '\n') && commentStage == COMMENT_EOL)
			{
				commentStage = COMMENT_NONE;
				// Need to go back 1 char because we go forward one 
				// char at the end of this function.
				buffer.SeekGet(CUtlBuffer::SEEK_CURRENT, -1);
			}
		}
		else if (currentChar == '/')
		{
			// See if our next character is also a backslash or an
			// asterisk, and thus we should enter a comment.
			if (IsEscaped(buffer))	// Just treat it as a normal character
			{
				if (!bHaveKey)
				{
					key[iKeyIndex] = currentChar;
					iKeyIndex++;
				}
				else
				{
					value[iValueIndex] = currentChar;
					iValueIndex++;
				}
			}
			else
			{
				char nextChar = *(char *)buffer.PeekGet(0);
				if (nextChar == '/')	// Starting an EOL comment
					commentStage = COMMENT_EOL;
				else if (nextChar == '*')
					commentStage = COMMENT_FULL;
				else	// Just treat it as a normal character.
				{
					if (!bHaveKey)
					{
						key[iKeyIndex] = currentChar;
						iKeyIndex++;
					}
					else
					{
						value[iValueIndex] = currentChar;
						iValueIndex++;
					}
				}
			}
		}
		else if (currentChar == '{')		// Moving down a level
		{
			if (bInQuotes)
			{
				// See if we've escaped this control character
				if (IsEscaped(buffer))	// Just treat it as a normal character
				{
					if (!bHaveKey)
					{
						key[iKeyIndex] = currentChar;
						iKeyIndex++;
					}
					else
					{
						value[iValueIndex] = currentChar;
						iValueIndex++;
					}
				}
				else	// Someone failed with HypKeyValues.
				{
					Warning("Expected closing quote before { at pos %i in file %s!\n", buffer.TellGet(), szFileName);
					Assert(false);
				}
			}
			else
			{
				bHaveKey = false;
				bHaveValue = false;

				// Switch to a new, nested HypKeyValue
				HypKeyValues *pNewHypKeyValues = new HypKeyValues();

				key[iKeyIndex] = '\0';

				pNewHypKeyValues->SetName(key);
				pNewHypKeyValues->SetParent(pCurrent);
				pCurrent = pNewHypKeyValues;

				key[0] = '\0';
				value[0] = '\0';
				iKeyIndex = 0;
				iValueIndex = 0;

				iLevel++;
			}
		}
		else if (currentChar == '}')	// Moving up a level
		{
			if (bInQuotes)
			{
				// See if we've escaped this control character
				if (IsEscaped(buffer))	// Just treat it as a normal character
				{
					if (!bHaveKey)
					{
						key[iKeyIndex] = currentChar;
						iKeyIndex++;
					}
					else
					{
						value[iValueIndex] = currentChar;
						iValueIndex++;
					}
				}
				else	// Someone failed with HypKeyValues.
				{
					Warning("Expected closing quote before } at pos %i in file %s!\n", buffer.TellGet(), szFileName);
					Assert(false);
				}
			}
			else
			{
				// Add this HypKeyValues to the parent
				pCurrent->GetParent()->m_children.AddToTail(pCurrent);
				
				// Switch back up to our previous HypKeyValues
				pCurrent = pCurrent->GetParent();

				key[0] = '\0';
				value[0] = '\0';
				iKeyIndex = 0;
				iValueIndex = 0;

				iLevel--;
			}
		}
		else if (currentChar == '\\')
		{
			// See if we've escaped this control character
			if (IsEscaped(buffer))	// Just treat it as a normal character
			{
				if (!bHaveKey)
				{
					key[iKeyIndex] = currentChar;
					iKeyIndex++;
				}
				else
				{
					value[iValueIndex] = currentChar;
					iValueIndex++;
				}
			}
			else // Just ignore this character
			{
				currentChar = buffer.GetChar();
				continue;
			}
		}
		else if (currentChar == '"')	// Starting/ending text gathering
		{
			if (IsEscaped(buffer))	// Escaped; just treat it as a normal character
			{
				if (!bHaveKey)
				{
					key[iKeyIndex] = currentChar;
					iKeyIndex++;
				}
				else
				{
					value[iValueIndex] = currentChar;
					iValueIndex++;
				}
			}
			else
			{
				// Swap our quoted state
				bInQuotes = !bInQuotes;
			}
		}
		else if (IsBreakChar(currentChar))	// A break in the text. Stop gathering text if we're not in quotes.
		{
			if (bInQuotes)	// Keep gathering text
			{
				if (!bHaveKey)
				{
					key[iKeyIndex] = currentChar;
					iKeyIndex++;
				}
				else
				{
					value[iValueIndex] = currentChar;
					iValueIndex++;
				}
			}
			else if (!bHaveKey && iKeyIndex > 0)
				bHaveKey = true;
			else if (!bHaveValue && iValueIndex > 0)
				bHaveValue = true;
		}
		else							// There's some actual data here
		{
			if (!bHaveKey)
			{
				key[iKeyIndex] = currentChar;
				iKeyIndex++;
			}
			else
			{
				value[iValueIndex] = currentChar;
				iValueIndex++;
			}
		}

		if (bHaveKey && bHaveValue)
		{
			key[iKeyIndex] = '\0';
			value[iValueIndex] = '\0';
			pCurrent->SetValue(key, value, true);

			key[0] = '\0';
			value[0] = '\0';
			iKeyIndex = 0;
			iValueIndex = 0;

			bHaveKey = false;
			bHaveValue = false;
		}
		
		//charIndex++;
		currentChar = buffer.GetChar();
	}

	if (commentStage == COMMENT_FULL)
	{
		Warning("Reached end of file while still in a /* */ comment in file %s!\n", szFileName);
		Assert(false);
	}
}

void HypKeyValues::KeyValuesToFile(const char *szPath)
{
	CUtlBuffer buffer;
	buffer.SetBufferType(true, false);

	KeyValuesToString(buffer);

	filesystem->WriteFile(szPath, "MOD", buffer);

	buffer.Purge();
}

void HypKeyValues::EscapeString(const char *szString, char szDest[])
{
	int strSize = V_strlen(szString);
	int currentPos = 0;

	for (int i = 0; i < strSize; i++)
	{
		char currentChar = szString[i];

		if (currentChar == '"' ||
			currentChar == '}' ||
			currentChar == '{')
		{
			// Escape this char
			szDest[currentPos] = '\\';
			currentPos++;
		}

		szDest[currentPos] = currentChar;
		currentPos++;
	}

	szDest[currentPos] = '\0';
}

bool HypKeyValues::IsEscaped(CUtlBuffer &buffer)
{
	int maxSpaceBehind = buffer.TellGet();

	int maxOffset = 0;
	for (; abs(maxOffset) < maxSpaceBehind; maxOffset--)
	{
		if ((maxSpaceBehind + maxOffset - 2) < 0)
			break;

		char currentChar = *(char *)buffer.PeekGet(maxOffset - 2);
		if (currentChar != '\\')
			break;
	}

	// If we have an odd number of '\' preceeding, then we're escaped.
	// If we have an even number, then we're not escaped.
	int diff = abs(maxOffset);
	bool bEscaped = !!(diff % 2);

	return bEscaped;
}

#define PUT_TABS() \
	for (int tabCount = 0; tabCount < level; tabCount++) \
		buffer.PutChar('\t');

void HypKeyValues::KeyValuesToString(CUtlBuffer &buffer, int level)
{
	// Write the name of the current keyvalues
	PUT_TABS();
	buffer.PutChar('"');
	char temp[MAX_VALUE];
	EscapeString(GetName(), temp);
	buffer.PutString(temp);
	buffer.PutChar('"');
	buffer.PutChar('\n');
	PUT_TABS();
	buffer.PutChar('{');
	{
		level++;
		// Write all values
		for (int i = 0; i < m_subKeyValues.Count(); i++)
		{
			HypKeyValue *pValue = m_subKeyValues[i];

			buffer.PutChar('\n');
			PUT_TABS();
			buffer.PutChar('"');
			EscapeString(pValue->GetName(), temp);
			buffer.PutString(temp);
			buffer.PutChar('"');

			buffer.PutChar('\t');

			buffer.PutChar('"');
			EscapeString(pValue->GetString(), temp);
			buffer.PutString(temp);
			buffer.PutChar('"');
		}

		// Write all subkeys
		for (int i = 0; i < m_children.Count(); i++)
		{
			buffer.PutChar('\n');
			HypKeyValues *pKV = m_children[i];
			pKV->KeyValuesToString(buffer, level);
		}

		level--;
	}
	buffer.PutChar('\n');
	PUT_TABS();
	buffer.PutChar('}');
}

bool HypKeyValues::IsBreakChar(char testChar)
{
	return (

		testChar == ' ' ||
		testChar == '\t' || 
		testChar == '\n' || 
		testChar == '\r' || 
		testChar == char(13)

		);
}

bool HypKeyValues::SetValue(const char *keyName, bool value, bool forceNew)
{
	// Loop through our existing values and try to find
	// one with the specified name.
	if (!forceNew)
	{
		for (int i = 0; i < m_subKeyValues.Count(); i++)
		{
			if (!strcmp(keyName, m_subKeyValues[i]->GetName()))
			{
				m_subKeyValues[i]->SetValue(value);
				return false;
			}
		}
	}

	// If we made it here, we haven't found an existing key
	// with the specified keyName.
	HypKeyValue *pKV = new HypKeyValue();
	pKV->SetName(keyName);
	pKV->SetValue(value);
	m_subKeyValues.AddToTail(pKV);

	return true;
}

bool HypKeyValues::SetValue(const char *keyName, float value, bool forceNew)
{
	// Loop through our existing values and try to find
	// one with the specified name.
	if (!forceNew)
	{
		for (int i = 0; i < m_subKeyValues.Count(); i++)
		{
			if (!strcmp(keyName, m_subKeyValues[i]->GetName()))
			{
				m_subKeyValues[i]->SetValue(value);
				return false;
			}
		}
	}

	// If we made it here, we haven't found an existing key
	// with the specified keyName.
	HypKeyValue *pKV = new HypKeyValue();
	pKV->SetName(keyName);
	pKV->SetValue(value);
	m_subKeyValues.AddToTail(pKV);

	return true;
}

bool HypKeyValues::SetValue(const char *keyName, int value, bool forceNew)
{
	// Loop through our existing values and try to find
	// one with the specified name.
	if (!forceNew)
	{
		for (int i = 0; i < m_subKeyValues.Count(); i++)
		{
			if (!strcmp(keyName, m_subKeyValues[i]->GetName()))
			{
				m_subKeyValues[i]->SetValue(value);
				return false;
			}
		}
	}

	// If we made it here, we haven't found an existing key
	// with the specified keyName.
	HypKeyValue *pKV = new HypKeyValue();
	pKV->SetName(keyName);
	pKV->SetValue(value);
	m_subKeyValues.AddToTail(pKV);

	return true;
}

bool HypKeyValues::SetValue(const char *keyName, const char *value, bool forceNew)
{
	// Loop through our existing values and try to find
	// one with the specified name.
	if (!forceNew)
	{
		for (int i = 0; i < m_subKeyValues.Count(); i++)
		{
			if (!strcmp(keyName, m_subKeyValues[i]->GetName()))
			{
				m_subKeyValues[i]->SetValue(value);
				return false;
			}
		}
	}

	// If we made it here, we haven't found an existing key
	// with the specified keyName.
	HypKeyValue *pKV = new HypKeyValue();
	pKV->SetName(keyName);
	pKV->SetValue(value);
	m_subKeyValues.AddToTail(pKV);

	return true;
}

void HypKeyValues::AssertHasDuplicates(const char *keyName)
{
	bool bFoundOnce = false;
	for (int i = 0; i < m_subKeyValues.Count(); i++)
	{
		if (!strcmp(keyName, m_subKeyValues[i]->GetName()))
		{
			if (bFoundOnce)
			{
				// We've already bumped into this key before;
				// throw some errors.
				char buf[128];
				Q_snprintf(buf, sizeof(buf), "Found duplicate keyName %s when not calling the singular version of a Get!\n", keyName);
				Warning(buf);
				AssertMsg(!bFoundOnce, buf);
			}
			else
				bFoundOnce = true;
		}
	}
}

void HypKeyValues::SetName(const char *name)
{
	char buffer[MAX_KEY];
	Q_strncpy(buffer, name, sizeof(buffer));
	Q_strlower(buffer);
	Q_strncpy(m_name, buffer, sizeof(m_name));
}

HypKeyValue::HypKeyValue()
{
}

HypKeyValue::~HypKeyValue()
{
}

void HypKeyValue::SetName(const char *name)
{
	char buffer[MAX_KEY];
	Q_strncpy(buffer, name, sizeof(buffer));
	Q_strlower(buffer);
	Q_strncpy(m_name, buffer, sizeof(m_name));
}

void HypKeyValue::SetValue(const char *value)
{
	Q_strncpy(m_content, value, sizeof(m_content));
}

void HypKeyValue::SetValue(int value)
{
	char buf[16];
	Q_snprintf(buf, sizeof(buf), "%i", value);
	SetValue(buf);
}

void HypKeyValue::SetValue(bool value)
{
	if (value)
		SetValue("1");
	else
		SetValue("0");
}

void HypKeyValue::SetValue(float value)
{
	char buf[16];
	Q_snprintf(buf, sizeof(buf), "%f", value);
	SetValue(buf);
}

void HypKeyValues::GetString(const char *keyName, char *out, size_t maxSize, const char *defaultValue)
{
	// Loop through our subHypKeyValues and see if we have
	// any that match the given keyName.
#ifndef FINAL_RELEASE
	AssertHasDuplicates(keyName);
#endif

	char buffer[MAX_VALUE];
	Q_strncpy(buffer, keyName, sizeof(buffer));
	Q_strlower(buffer);

	for (int i = 0; i < m_subKeyValues.Count(); i++)
	{
		if (!strcmp(buffer, m_subKeyValues[i]->GetName()))
		{
			Q_strncpy(out, m_subKeyValues[i]->GetString(), maxSize);
			return;
		}
	}

	// Couldn't find our key.
	Q_strncpy(out, defaultValue, maxSize);
}

void HypKeyValues::GetInt(const char *keyName, int &iOut, int defaultValue)
{
	// Loop through our subHypKeyValues and see if we have
	// any that match the given keyName.
#ifndef FINAL_RELEASE
	AssertHasDuplicates(keyName);
#endif

	char buffer[MAX_KEY];
	Q_strncpy(buffer, keyName, sizeof(buffer));
	Q_strlower(buffer);

	for (int i = 0; i < m_subKeyValues.Count(); i++)
	{
		if (!strcmp(buffer, m_subKeyValues[i]->GetName()))
		{
			iOut = m_subKeyValues[i]->GetInt();
			return;
		}
	}

	// Couldn't find our key.
	iOut = defaultValue;
}

void HypKeyValues::GetFloat(const char *keyName, float &flOut, float defaultValue)
{
	// Loop through our subHypKeyValues and see if we have
	// any that match the given keyName.
#ifndef FINAL_RELEASE
	AssertHasDuplicates(keyName);
#endif

	char buffer[MAX_KEY];
	Q_strncpy(buffer, keyName, sizeof(buffer));
	Q_strlower(buffer);

	for (int i = 0; i < m_subKeyValues.Count(); i++)
	{
		if (!strcmp(buffer, m_subKeyValues[i]->GetName()))
		{
			flOut = m_subKeyValues[i]->GetFloat();
			return;
		}
	}

	// Couldn't find our key.
	flOut = defaultValue;
}

void HypKeyValues::GetBool(const char *keyName, bool &bOut, bool defaultValue)
{
	// Loop through our subHypKeyValues and see if we have
	// any that match the given keyName.
#ifndef FINAL_RELEASE
	AssertHasDuplicates(keyName);
#endif

	char buffer[MAX_VALUE];
	Q_strncpy(buffer, keyName, sizeof(buffer));
	Q_strlower(buffer);

	for (int i = 0; i < m_subKeyValues.Count(); i++)
	{
		if (!strcmp(buffer, m_subKeyValues[i]->GetName()))
		{
			bOut = m_subKeyValues[i]->GetBool();
			return;
		}
	}

	// Couldn't find our key.
	bOut = defaultValue;
}

void HypKeyValues::GetColor(const char *keyName, Color &out, Color defaultValue)
{
	// Loop through our subHypKeyValues and see if we have
	// any that match the given keyName.
#ifndef FINAL_RELEASE
	AssertHasDuplicates(keyName);
#endif

	char buffer[MAX_VALUE];
	Q_strncpy(buffer, keyName, sizeof(buffer));
	Q_strlower(buffer);

	for (int i = 0; i < m_subKeyValues.Count(); i++)
	{
		if (!strcmp(buffer, m_subKeyValues[i]->GetName()))
		{
			out = m_subKeyValues[i]->GetColor();
			return;
		}
	}

	// Couldn't find our key.
	out = defaultValue;
}

void HypKeyValues::GetColor(const char *keyName, ColorRGBExp32 &out, ColorRGBExp32 defaultValue)
{
	Color value;
	Color colorDefaultValue = Color(defaultValue.r, defaultValue.g, defaultValue.b, defaultValue.exponent);
	GetColor(keyName, value, colorDefaultValue);
	
	out.r = value.r();
	out.g = value.g();
	out.b = value.b();
	out.exponent = value.a();
}

int HypKeyValue::GetInt()
{
	return atoi(m_content);
}

float HypKeyValue::GetFloat()
{
	return atof(m_content);
}

bool HypKeyValue::GetBool()
{
	if (m_content[0] == '0' || !strcmp(m_content, "false"))
		return false;

	// Return true if anything other than 0.
	return true;
}

Color HypKeyValue::GetColor()
{
	char szRaw[(MAX_COLOR_DIGITS * 4) + 3];
	char szValueBuffer[MAX_COLOR_DIGITS + 1];
	Color returnValue = Color();
	bool bAlpha = false;
	Q_strncpy(szRaw, GetString(), sizeof(szRaw));

	// Count the spaces to see if an alpha has been specified
	int len = Q_strlen(szRaw);
	int spaces = 0;
	for (int i = 0; i < len; i++)
	{
		if (szRaw[i] == ' ')
			spaces++;
	}

	// If spaces == 2, then bAlpha = false. We've already initialized 
	// bAlpha to false though, so we don't need to set it again.
	if (spaces < 2)
	{
		// Derp not enough colors specified
		return returnValue;
	}
	else if (spaces == 3)
		bAlpha = true;
	else if (spaces > 3)
	{
		// Derp wtf kind of input are you giving me???
		return returnValue;
	}

	int startChar = 0;
	for (int c = 0; c < (bAlpha ? 4 : 3); c++)
	{
		for (int i = startChar; i < (startChar + MAX_COLOR_DIGITS + 1); i++)
		{
			char current = szRaw[i];
			if (current == ' ' || i >= len)
			{
				szValueBuffer[i - startChar] = '\0';
				switch (c)
				{
				case 0:
					{
						// Red
						returnValue.SetColor(atoi(szValueBuffer), 0, 0, 0);
						break;
					}
				case 1:
					{
						// Green
						returnValue.SetColor(returnValue.r(), atoi(szValueBuffer), 0, 0);
						break;
					}
				case 2:
					{
						// Blue
						returnValue.SetColor(returnValue.r(), returnValue.g(), atoi(szValueBuffer), 0);
						break;
					}
				case 3:
					{
						// Alpha (if we don't have an alpha, we'll never get here,
						// and thus alpha will be set to 0 in the return value.
						returnValue.SetColor(returnValue.r(), returnValue.g(), returnValue.b(), atoi(szValueBuffer));
						break;
					}
				}
				startChar = i + 1;
				break;
			}
			szValueBuffer[i - startChar] = current;
		}
	}
	return returnValue;
}

HypKeyValues *HypKeyValues::FindKey(const char *keyName, bool bCreate)
{
	HypKeyValues *returnValue = NULL;
	
	char buffer[MAX_KEY];
	Q_strncpy(buffer, keyName, sizeof(buffer));
	Q_strlower(buffer);

	// Loop through all our children and see if any
	// of them have names matching the keyName given.
	for (int i = 0; i < m_children.Count(); i++)
	{
		if (!strcmp(m_children[i]->GetName(), buffer))
		{
			returnValue = m_children[i];
			break;
		}
	}

	if (bCreate && !returnValue)
	{
		// Create a new key.
		HypKeyValues *pNewKV = new HypKeyValues();
		pNewKV->SetName(keyName);
		pNewKV->SetParent(this);
		m_children.AddToTail(pNewKV);

		returnValue = pNewKV;
	}

	return returnValue;
}

CUtlVector<HypKeyValues*> *HypKeyValues::FindKeys(const char *keyName, bool bCreate)
{
	// Make sure we have at least one child
	if (m_children.Count() == 0)
		return NULL;
	
	char buffer[MAX_KEY];
	Q_strncpy(buffer, keyName, sizeof(buffer));
	Q_strlower(buffer);

	// Loop through all of our children and see if any 
	// of them have names matching the keyName given.
	// If they do, add them to a vector then return
	// that vector after we have finished our loop.
	CUtlVector<HypKeyValues*> *foundKeys = new CUtlVector<HypKeyValues*>();
	for (int i = 0; i < m_children.Count(); i++)
	{
		if (!strcmp(m_children[i]->GetName(), buffer))
			foundKeys->AddToTail(m_children[i]);
	}

	// Return our vector.
	return foundKeys;
}

HypKeyValue *HypKeyValues::GetValue(int valueIndex)
{
	if (valueIndex < m_subKeyValues.Count() && valueIndex >= 0)
		return m_subKeyValues[valueIndex];

	return NULL;
}

HypKeyValues *HypKeyValues::GetKey(int keyIndex)
{
	if (keyIndex < m_children.Count() && keyIndex >= 0)
		return m_children[keyIndex];

	return NULL;
}

HKVExistsType HypKeyValues::ExistsInCurrentScope(const char *keyName)
{
	int returnValue = HKVExistsType::DOES_NOT_EXIST;

	char keyNameLower[MAX_KEY];
	V_strncpy(keyNameLower, keyName, sizeof(keyNameLower));
	V_strlower(keyNameLower);

	// Loop through our HypKeyValue array.
	for (int i = 0; i < m_subKeyValues.Count(); i++)
	{
		HypKeyValue *pValue = m_subKeyValues[i];
		if (!V_strcmp(pValue->GetName(), keyNameLower))
		{
			returnValue |= HKVExistsType::EXISTS_KEY;
			break;
		}
	}

	// Loop through our children HypKeyValues.
	for (int i = 0; i < m_children.Count(); i++)
	{
		HypKeyValues *pKV = m_children[i];
		if (!V_strcmp(pKV->GetName(), keyNameLower))
		{
			returnValue |= HKVExistsType::EXISTS_KVSET;
			break;
		}
	}

	return (HKVExistsType)returnValue;
}

CUtlVector<const char*> *HypKeyValues::GetStrings(const char *keyName, const char *defaultValues)
{
	// Make sure we have at least one child.
	if (m_subKeyValues.Count() == 0)
		return NULL;

	char buffer[MAX_KEY];
	Q_strncpy(buffer, keyName, sizeof(buffer));
	Q_strlower(buffer);

	// Loop through all of our subKeys and see if any 
	// of them have names matching the keyName given.
	// If they do, add them to a vector then return
	// that vector after we have finished our loop.
	CUtlVector<const char *> *foundKeys = new CUtlVector<const char *>();
	for (int i = 0; i < m_subKeyValues.Count(); i++)
	{
		if (!strcmp(m_subKeyValues[i]->GetName(), buffer))
			foundKeys->AddToTail(m_subKeyValues[i]->GetString());
	}

	// Return our vector.
	return foundKeys;
}