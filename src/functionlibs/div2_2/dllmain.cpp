#include <libpcom.h>
#include "div2_2.h"

EXTERN_C IPcomBase * CreateInstance()
{
	div2_2 * pInstance = new div2_2;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}
