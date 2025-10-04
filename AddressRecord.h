#pragma once

#include <stdlib.h>
#include <tchar.h>
#include "PlugInSDK/BkCommon.h"

/**
 * アドレス帳のレコード
 */
class AddressRecord
{
public:
	size_t nFileIndex;
	size_t UNKNOWN;  // unknown data. 
	unsigned long nID;

private:
	TCHAR *_szAddressBookName;
	TCHAR *_szName;
	TCHAR *_szEMailAddress;

public:
	AddressRecord(void);
	~AddressRecord(void);

	const TCHAR *GetAddressBookName();
	void SetAddressBookName(const TCHAR *szAddressBookName);

	const TCHAR *GetName();
	void SetName(TCHAR *szName);

	const TCHAR *GetEMailAddress();
	void SetEMailAddress(const TCHAR *szEMailAddress);

	static AddressRecord *Parse(const TCHAR *szAddressBookMember);
};
