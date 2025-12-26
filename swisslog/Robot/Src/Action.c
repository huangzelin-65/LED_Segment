#include "JsonCommon.h"
#include "LogDebugInfo.h"
#include "Action.h"
#include "clist.h"
#include "main.h"
#include "common.h"

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
//寻找对应cmdId的数据
int Action_FindCmdId(const void* action, const void* input_action)
{
    // 强制类型转换为int*，取值后比较
    const  Action_t* val_action = (const  Action_t*)action;
    const  Action_t* val_input_action = (const  Action_t*)input_action;

    return strcmp(val_action->cmd.cmdId,val_input_action->cmd.cmdId);
}
//事件发生，需更新动作，id为0，针对于单机器状态使用
void Action_Event(Action_t *action)
{
    DEBUGINFO("id:%d",action->id);
    //第一次使用需创建链表
    if(action_list == NULL)
    {
        Action_ListInit();
    }    
    //创建状态数据
    Action_t* new_action = pvPortMalloc(sizeof(Action_t));

    if(new_action == NULL)return;

    memset(new_action,0,sizeof(Action_t));

    memcpy(new_action,action,sizeof(Action_t));

    Action_t* find_action = (Action_t*)list_find_by_value(action_list,new_action,Action_FindCmdId);

    if(find_action != NULL)
    {
        DEBUGINFO("cmd id has exist");
    }
    else
    {
        list_insert_tail(action_list,new_action);
    }

    Action_Execute();

    DEBUGINFO("list_size:%d",list_size(action_list));
}

//增加最新链表执行动作
void Action_Execute(void)
{
    for(int i = 0; i < action_list->size;i++)
    {
        Action_t* action = list_find_at(action_list, i);
        if(action->execute == 0)
        {   
            //需要回复ack
            memcpy(action->cmd.status,"ack",4);
            Json_GenerateMsg(JSON_G_ACTION,action);
            //执行动作
            Action_ToCmd(action);
            //标记动作已经执行
            action->execute = 1;
        }
    }
}
//动作实际执行
void Action_ToCmd(Action_t *action)
{
    ServerToCarData.xStationStatus = OutStation;//测试用

    char *res = strstr(action->cmd.cmd, "forward");
    if (res != NULL) {
        DEBUGINFO("forward\n"); 
        ServerToCarData.xDirection = Forward; // 小车运行方向 1=正转 2=反转 
        ServerToCarData.xMotorEnable = MotorEnable;                                                      
    } 
    else
    {
        char *res = strstr(action->cmd.cmd, "back");
        if (res != NULL) {
            DEBUGINFO("backward\n"); 
            ServerToCarData.xDirection = Backward; // 小车运行方向 1=正转 2=反转 
            ServerToCarData.xMotorEnable = MotorEnable;
        }
        else
        {
            char *res = strstr(action->cmd.cmd, "stop");
            if (res != NULL) {
                DEBUGINFO("stop\n"); 
                ServerToCarData.xDirection = NoDirection;
                ServerToCarData.xMotorEnable = MotorDisable;
            }
        }
    }
    {
        char *res = strstr(action->cmd.params.model, "auto");
        if(res != NULL)
        {
            ServerToCarData.xAutoMode = Auto; // 自动模式 
        }
        else
        {
            ServerToCarData.xAutoMode = Manual; // 手动模式 
        }            
    }
    {
        char *res = strstr(action->cmd.params.speedLevel, "0");
        if(res != NULL)
        {
            DEBUGINFO("speed level 0\n"); 
            ServerToCarData.xSetSpeed = ZeroSpeed;
        }
        else
        {
            char *res = strstr(action->cmd.params.speedLevel, "1");
            if(res != NULL)
            {
                DEBUGINFO("speed level 1\n");
                ServerToCarData.xSetSpeed = LowSpeed; 
            }
            else
            {
                char *res = strstr(action->cmd.params.speedLevel, "2");
                if(res != NULL)
                {
                    DEBUGINFO("speed level 2\n"); 
                    ServerToCarData.xSetSpeed = NormalSpeed;
                }
                else
                {
                    char *res = strstr(action->cmd.params.speedLevel, "3");
                    if(res != NULL)
                    {
                        DEBUGINFO("speed level 3\n"); 
                        ServerToCarData.xSetSpeed = HighSpeed;
                    }                      
                }
            }
        }
    }
    vParseCommandToCar();     
}








