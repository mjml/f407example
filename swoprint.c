#include <stm32f103xb.h>

int _write (int file, char* ptr, int len) 
{
    int idx=0;
    for (idx=0; idx < len; idx++) {
        ITM_SendChar( *ptr++ );
    }
    return len;
}

