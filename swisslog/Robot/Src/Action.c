#include "LogDebugInfo.h"
#include "Action.h"
#include "clist.h"

List *action_list = NULL;

void Action_ListInit(void)
{
    action_list = list_init();
    if(action_list != NULL)
    {
        DEBUGINFO("list init Success");
    }
}

void Action_Init(void)
{
    Action_ListInit();
}















