#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <tchar.h>
#include "AddressRecord.h"

/**
 * ƒAƒhƒŒƒX’ 
 */
class AddressBook
{
private:
	std::ifstream _ifsAddressBook;
	TCHAR *_szDataFolder;
	enum ADDRESSFORMAT _format;

public:
	AddressBook(const TCHAR *szDataFolder);
	AddressBook(const TCHAR *szDataFolder, const TCHAR *szAddressGroupName);
	~AddressBook(void);

	typedef void (*ABCALLBACK)(AddressBook *const, AddressRecord *, void *);
	void Lookup(const TCHAR *addresses, ABCALLBACK callback, void *data);

private:
	void initialize(const TCHAR *szDataFolder, const TCHAR *szAddressGroupName);
	void Open(const TCHAR *szAddressGruopName);
};
