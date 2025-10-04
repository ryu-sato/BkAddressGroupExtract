#include "AddressRecord.h"

/**
 * アドレス帳のレコード
 */
AddressRecord::AddressRecord(void)
{
}

AddressRecord::~AddressRecord(void)
{
	if (_szAddressBookName) {
		free(_szAddressBookName);
		_szAddressBookName = NULL;
	}
	if (_szName) {
		free(_szName);
		_szName = NULL;
	}
	if (_szEMailAddress) {
		free(_szEMailAddress);
		_szEMailAddress = NULL;
	}
}

const TCHAR *AddressRecord::GetAddressBookName()
{
	return _szAddressBookName;
}

void AddressRecord::SetAddressBookName(const TCHAR *szAddressBookName)
{
	if (! szAddressBookName) return;
	if (_szAddressBookName) {
		free(_szAddressBookName);
		_szAddressBookName = NULL;
	}
	_szAddressBookName = _tcsdup(szAddressBookName);
}

const TCHAR *AddressRecord::GetName()
{
	return _szName;
}

void AddressRecord::SetName(TCHAR *szName)
{
	if (! szName) return;
	if (_szName) {
		free(_szName);
		_szName = NULL;
	}
	_szName = _tcsdup(szName);
}

const TCHAR *AddressRecord::GetEMailAddress()
{
	return _szEMailAddress;
}

void AddressRecord::SetEMailAddress(const TCHAR *szEMailAddress)
{
	if (! szEMailAddress) return;
	if (_szEMailAddress) {
		free(_szEMailAddress);
		_szEMailAddress = NULL;
	}
	_szEMailAddress = _tcsdup(szEMailAddress);
}

AddressRecord *AddressRecord::Parse(const TCHAR *szAddressBookMember)
{
	TCHAR *szDupAddressBookMember;
	TCHAR *lpBeg, *lpEnd;
	int index = 0;

	AddressRecord *member = new AddressRecord();

	szDupAddressBookMember = _tcsdup(szAddressBookMember);
	lpBeg = szDupAddressBookMember;
	for (lpEnd = lpBeg; *lpEnd != _T('\0'); lpEnd++) {
		if (*lpEnd != _T('\t')) continue;

		*lpEnd = _T('\0');
		switch (index) {
		case 0:
			member->nFileIndex = _tstoi(lpBeg);
			break;
		case 1:
			member->UNKNOWN = _tstoi(lpBeg);
			break;
		case 2:
			member->SetAddressBookName(lpBeg);
			break;
		case 3:
			member->SetName(lpBeg);
			break;
		case 4:
			member->SetEMailAddress(lpBeg);
			break;
		case 5:
			member->nID = _tstoi(lpBeg);
			break;
		}
		lpBeg = lpEnd + 1;
		index++;
	}
	free(szDupAddressBookMember);

	return member;
}
