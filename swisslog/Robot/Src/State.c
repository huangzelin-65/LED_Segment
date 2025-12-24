#include "LogDebugInfo.h"
#include "State.h"
#include "clist.h"

List *state_list = NULL;

void State_ListInit(void)
{
    state_list = list_init();
    if(state_list != NULL)
    {
        DEBUGINFO("list init Success");
    }
}

















