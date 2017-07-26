#include "debug.h"

#ifdef DEBUG
void openDebug()
{
    while (!(*sPortDebug));
    sPortDebug->begin(9600);
}
#endif

