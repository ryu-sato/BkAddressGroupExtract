#include "vcard.h"

vcard::vcard(void)
{
}

vcard::~vcard(void)
{
}

void vcard::parse(const TCHAR *szVCard, VCARDCB callback)
{
	TCHAR *szDupVCard = _tcsdup(szVCard);
	const TCHAR *lpBegLine = szDupVCard;
	for (TCHAR *lp = szDupVCard; *lp != _T('\0'); lp++) {
	}
	free((void *) szDupVCard);
}

TCHAR *parseContentLine(TCHAR *lpBeg)
{
	bool bFolded = false;

	TCHAR *lpBegToken = lpBeg;
	TCHAR *lp = lpBeg;

	while (*lp != _T('\0')) {
		if (bFolded) {
			if (*lp != _T(' ')) break;  // end of content line
			while (*lp != _T('\0') && *lp == _T(' ')) lp++;
		}

		if (_ismbblead(*lp))  // multi byte string
			break;

		switch (*lp) {
		case _T('\r'):
		case _T('\n'):
			while (*(lp + 1) == _T('\0') 
				   && (*(lp + 1) == _T('\r') || *(lp + 1) == _T('\n'))) lp++;
			bFolded = true;
			break;

		case _T('.'):
			*lp = _T('\0');
			break;
		}
	}

	return lpBeg;
}
