#include "AddressBook.h"
#include "BkAddressGroupExtractOption.h"
#include <windows.h>

AddressBook::AddressBook(const TCHAR *szDataFolder)
{
	initialize(szDataFolder, NULL);
}

AddressBook::AddressBook(const TCHAR *szDataFolder, const TCHAR *szAddressGroupName)
{
	initialize(szDataFolder, szAddressGroupName);
}

AddressBook::~AddressBook(void)
{
	if (_szDataFolder) {
		free(_szDataFolder);
		_szDataFolder = NULL;
	}
	if (_ifsAddressBook && _ifsAddressBook.is_open()) {
		_ifsAddressBook.close();
	}
}

void AddressBook::Lookup(const TCHAR *szAddresses, ABCALLBACK callback, void *data = NULL)
{
	CPointerList listTargetAddress;
	TCHAR *szDupAddresses = _tcsdup(szAddresses);
	LPSTR lpTok = TokenAddr(szDupAddresses);
	while (lpTok) {
		TCHAR szName[1024] = _T("");
		TCHAR szEMailAddress[256] = _T("");
		GetNameAndAddr(szName, sizeof(szName) / sizeof(szName[0]), szEMailAddress,
			           sizeof(szEMailAddress) / sizeof(szEMailAddress[0]), lpTok);
		listTargetAddress.AddTail(szEMailAddress);
		lpTok = TokenAddr(NULL);
	}

	std::string strLine;
	while (_ifsAddressBook && std::getline(_ifsAddressBook, strLine)) {
		AddressRecord *addressRecord = AddressRecord::Parse(strLine.c_str());
		for (CPointerItem *item = listTargetAddress.GetTop(); item != NULL; item = item->GetNext()) {
			if (_tcscmp(item->GetData(), addressRecord->GetEMailAddress()) == 0)
				callback(this, addressRecord, data);
		}
		delete addressRecord;
	}
	free(szDupAddresses);
}

/** 
  * private
  */
void AddressBook::initialize(const TCHAR *szDataFolder, const TCHAR *szAddressGroupName)
{
	_szDataFolder = NULL;
	_format = AF_ADDRESS;

	if (szDataFolder) {
		_szDataFolder = _tcsdup(szDataFolder);
	}
	if (szAddressGroupName) {
		Open(szAddressGroupName);
	}
}

void AddressBook::Open(const TCHAR *szAddressGroupName)
{
	std::string strSubPath = szAddressGroupName;
	while (true) {
		std::string::size_type pos = strSubPath.find(_T('"'));
		if (pos == std::string::npos) break;
		strSubPath.erase(pos, 1);
	}
	while (true) {
		std::string::size_type pos = strSubPath.find(_T(':'));
		if (pos == std::string::npos) break;
		strSubPath.replace(pos, 1, _T("\\"));
	}
	std::string strPathAddressBook = _szDataFolder;
	strPathAddressBook.append(_T("AddrBook\\"));
	strPathAddressBook.append(strSubPath);
	strPathAddressBook.append(_T("\\Group.idx"));
	_ifsAddressBook.open(strPathAddressBook.c_str());
}

