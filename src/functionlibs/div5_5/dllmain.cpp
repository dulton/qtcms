#include <libpcom.h>
#include "div5_5.h"

EXTERN_C IPcomBase * CreateInstance()
{
	div5_5 * pInstance = new div5_5;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}
