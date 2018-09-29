#ifdef _APPLE_
    pthread_threadid_np(thread,&tcb.id)
#else
#ifdef unix

#endif
#endif

#include <stdio.h>
