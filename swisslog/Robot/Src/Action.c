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
//寻找对应流水号的数据
int Action_FindId(const void* action, const void* input_action)
{
    // 强制类型转换为int*，取值后比较
    const  Action_t* val_action = (const  Action_t*)action;
    const  Action_t* val_input_action = (const  Action_t*)input_action;

    if(val_action->id == val_input_action->id)
    {
        return 0;
    }
    return -1;
}
//寻找对应cmdId的数据
int Action_FindCmdId(const void* action, const void* input_action)
{
    // 强制类型转换为int*，取值后比较
    const  Action_t* val_action = (const  Action_t*)action;
    const  Action_t* val_input_action = (const  Action_t*)input_action;

    return strcmp(val_action->cmd.cmdId,val_input_action->cmd.cmdId);
}
//事件发生，需更新动作，id为0，针对于单机器状态使用 (此接口json解析函数中使用)
void Action_Event(Action_t *action)
{
    DEBUGINFO("id:%d headerId:%s cmd:%s cmdId:%s",action->id,action->headerId,action->cmd.cmd,action->cmd.cmdId);
    //第一次使用需创建链表
    if(action_list == NULL)
    {
        Action_ListInit();
    }  
    if(action_list == NULL) return;    
    //创建状态数据
    Action_t* new_action = pvPortMalloc(sizeof(Action_t));

    if(new_action == NULL)return;

    memset(new_action,0,sizeof(Action_t));

    memcpy(new_action,action,sizeof(Action_t));

    for(int i = 0; i < action_list->size;i++)
    {
        Action_t* action = list_find_at(action_list, i);
        if(action->id == new_action->id)
        {   
            if(strcmp(action->cmd.cmdId,new_action->cmd.cmdId) == 0)
            {
                DEBUGINFO("cmd id has exist");
                vPortFree(new_action);
                return;
            }
        }
    }    

    list_insert_tail(action_list,new_action);

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

            Action_t* json_action = pvPortMalloc(sizeof(Action_t));
            memcpy(json_action,action,sizeof(Action_t));            
            Json_GenerateMsg(JSON_G_ACTION,json_action);
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
    DEBUGINFO("id:%d",action->id);//执行动作需要根据流水号发到对应机器中

    ServerToCarData.xAutoMode = Auto; //默认为自动
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
//根据实际机器状态编辑action状态
void Action_Edit(Action_t *action)
{
    //前进、后退、停止等状态
    DEBUGINFO("id:%d cmd:%s cmdId:%s running:%d new_running:%d",action->id,action->cmd.cmd,action->cmd.cmdId,action->running,CarStatus.xIsCarRunning);
    if(strcmp(action->cmd.cmd,"forward") == 0 || strcmp(action->cmd.cmd,"back") == 0 || strcmp(action->cmd.cmd,"stop") == 0)
    {
        if(action->id == 0)//单设备，id为0
        {
            if(CarStatus.xIsCarRunning == CarRunning)//当前为running
            {
                if(action->running != CarRunning)//之前为stop
                {
                    memcpy(action->cmd.status,"running",8);
                    //发送状态更新
                    Action_t* json_action = pvPortMalloc(sizeof(Action_t));
                    memcpy(json_action,action,sizeof(Action_t));                    
                    Json_GenerateMsg(JSON_G_ACTION,json_action);         
                }
            }
            else//当前为stop或readytoRun
            {
                if(action->running == CarRunning)//之前为running
                {
                    if(CarStatus.xIsCarRunning == CarStop)//不是触发标签停止
                    {
                        if(CarStatus.xMotorStopReason == BySensorError ||
                            CarStatus.xMotorStopReason == BySensorTrigger ||
                            CarStatus.xMotorStopReason == ByBoxUnlock ||
                            CarStatus.xMotorStopReason == ByReset ||
                            CarStatus.xMotorStopReason == ByMotorError
                        ) 
                        {
                            DEBUGINFO("FrontProxStatus:%d RearProxStatus:%d",CarStatus.FrontProxStatus,CarStatus.RearProxStatus);
                            if(CarStatus.FrontProxStatus == SensorTrigger || CarStatus.RearProxStatus == SensorTrigger)
                            {
                                memcpy(action->cmd.status,"pause",6);
                            }
                            else
                            {
                                memcpy(action->cmd.status,"failed",7);
                            }
                        }
                        else
                        {
                            memcpy(action->cmd.status,"finished",9);
                        }
                        //发送状态更新
                        Action_t* json_action = pvPortMalloc(sizeof(Action_t));
                        memcpy(json_action,action,sizeof(Action_t));                    
                        Json_GenerateMsg(JSON_G_ACTION,json_action);  
                        
                        //动作结束后，从链表中删除
                        list_remove_by_value(action_list,action,Action_FindCmdId,vPortFree);
                    }
                    else//触发标签停止
                    {
                        memcpy(action->cmd.status,"finished",9);
                        //发送状态更新
                        Action_t* json_action = pvPortMalloc(sizeof(Action_t));
                        memcpy(json_action,action,sizeof(Action_t));                    
                        Json_GenerateMsg(JSON_G_ACTION,json_action); 
                        
                        //动作结束后，从链表中删除
                        list_remove_by_value(action_list,action,Action_FindCmdId,vPortFree);                        
                    }             
                }
            }
            action->running  = CarStatus.xIsCarRunning;            
        }
        else//多设备则需要做根据id获取状态的接口
        {

        }
    }

    //处理暂停后，接近传感器恢复未触发状态时，上报动作id给服务器使用
    if(strcmp(action->cmd.cmd,"forward") == 0 || strcmp(action->cmd.cmd,"back") == 0)
    {
        if(strcmp(action->cmd.status,"pause") == 0)
        {
            if(action->id == 0)//单设备，id为0
            {
                //暂时场景目前只有一种，是机器触发接近传感器
                //此时判断传感器是否已经恢复
                if(CarStatus.FrontProxStatus != SensorTrigger && CarStatus.RearProxStatus != SensorTrigger)
                {
                    memcpy(action->cmd.status,"ready",6);
                    Action_t* json_action = pvPortMalloc(sizeof(Action_t));
                    memcpy(json_action,action,sizeof(Action_t));                    
                    Json_GenerateMsg(JSON_G_ACTION,json_action);                    
                }
            }
            else
            {

            }
        }
    }
    //其他


}
//动作状态的更新(根据流水号)
void Action_Update(int id)
{
    DEBUGINFO("id:%d",id);
    //第一次使用需创建链表
    if(action_list == NULL)
    {
        Action_ListInit();
    }  
    if(action_list == NULL) return;   
    for(int i = 0; i < action_list->size;i++)
    {
        Action_t* action = list_find_at(action_list, i);
        if(action->id == id)//更新所有对应id(流水号)的动作状态
        {   
            //获取id对应的实际状态
            Action_Edit(action);
        }
    }    
}




