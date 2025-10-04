#pragma once

#include <stdlib.h>
#include <tchar.h>
#include <mbstring.h>

typedef int (*VCARDCB)(int nType, TCHAR *data);

class vcard
{
public:
	vcard(void);
	~vcard(void);
	void parse(const TCHAR *szVCard, VCARDCB callback);

private:
	TCHAR *parseContentLine(TCHAR *lpBeg);
};
