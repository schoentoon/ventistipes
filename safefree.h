#ifndef _SAFE_FREE_H
#define _SAFE_FREE_H

#define SAFEFREE(p) safefree((void**)&p)

void safefree(void **pp);

#endif //_SAFE_FREE_H