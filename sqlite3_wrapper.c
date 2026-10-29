#define _WIN32_WINNT 0x0600
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
WINBASEAPI ULONGLONG WINAPI GetTickCount64(VOID);
WINBASEAPI BOOLEAN WINAPI TryAcquireSRWLockExclusive(PSRWLOCK);
#ifdef __cplusplus
}
#endif
#include "sqlite3.c"
