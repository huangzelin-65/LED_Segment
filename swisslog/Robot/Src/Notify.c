#include "LogDebugInfo.h"
#include "Notify.h"
#include "clist.h"
#include "JsonCommon.h"
#include "main.h"

List *notify_list = NULL;
//链表初始化
void Notify_ListInit(void)
{
    notify_list = list_init();
    if(notify_list != NULL)
    {
        DEBUGINFO("list init Success");
    }
}

void Notify_Init(void)
{
    Notify_ListInit();
}

//寻找对应流水号的数据
int Notify_FindId(const void* notify, const void* input_notify)
{
    // 强制类型转换为int*，取值后比较
    const  Notify_t* val_notify = (const  Notify_t*)notify;
    const  Notify_t* val_input_notify = (const  Notify_t*)input_notify;

    if(val_notify->id == val_input_notify->id)
    {
        return 0;
    }
    return -1;
}
//事件发生，需更新配置，id为0，针对于单机器状态使用 (此接口json解析函数中使用)
void Notify_Event(Notify_t *notify)
{
    DEBUGINFO("id:%d headerId:%s type:%s code:%d message:%s",notify->id,notify->headerId,notify->type,notify->code,notify->message);
    //第一次使用需创建链表
    if(notify_list == NULL)
    {
        Notify_Init();
    }  
    if(notify_list == NULL) return; 
    if(notifyMutexHandle == NULL)return;   
    if (osMutexAcquire(notifyMutexHandle, portMAX_DELAY) == osOK)
    {
        //创建状态数据
        Notify_t* new_notify = pvPortMalloc(sizeof(Notify_t));

        if(new_notify == NULL)return;

        memset(new_notify,0,sizeof(Notify_t));

        memcpy(new_notify,notify,sizeof(Notify_t));

        for(int i = 0; i < notify_list->size;i++)
        {
            Notify_t* notify = list_find_at(notify_list, i);
            if(notify->id == new_notify->id)
            {   
                if(strcmp(notify->type,new_notify->type) == 0
                && notify->code == new_notify->code
                && strcmp(notify->message,new_notify->message) == 0)
                {
                    DEBUGINFO("notify has exist,all params same");
                    vPortFree(new_notify);
                    osMutexRelease(notifyMutexHandle);
                    return;
                }           
            }
        }    

        list_insert_tail(notify_list,new_notify);
        
        Notify_Execute();

        DEBUGINFO("list_size:%d",list_size(notify_list));

        osMutexRelease(notifyMutexHandle);
    }
}

//增加最新链表执行动作
void Notify_Execute(void)
{
    for(int i = 0; i < notify_list->size;i++)
    {
        Notify_t* notify = list_find_at(notify_list, i);
        if(notify->execute == 0)
        {   
            //需要回复ack
            Notify_t* json_notify = pvPortMalloc(sizeof(Notify_t));
            memcpy(json_notify,notify,sizeof(Notify_t));            
            Json_GenerateMsg(JSON_G_NOTIFY,json_notify);
            //执行动作
            Notify_ToCmd(notify);
            //标记动作已经执行
            notify->execute = 1;
        }
    }
    //如不等待配置结果，此处直接释放内存
    while(!list_is_empty(notify_list))
    {
        list_pop_tail(notify_list,vPortFree);
    }
}

//动作实际执行
void Notify_ToCmd(Notify_t* notify)
{
    DEBUGINFO("id:%d",notify->id);//执行需要根据流水号发到对应机器中

}
//根据实际机器状态编辑notify状态
void Notify_Edit(Notify_t *notify)
{
    //其他功能设置
    DEBUGINFO("id:%d",notify->id);
}
//功能的更新(根据流水号)
void Notify_Update(int id)
{
    DEBUGINFO("id:%d",id);
    //第一次使用需创建链表
    if(notify_list == NULL)
    {
        Notify_ListInit();
    }  
    if(notify_list == NULL) return; 
    if(notifyMutexHandle == NULL)return;   
    if (osMutexAcquire(notifyMutexHandle, portMAX_DELAY) == osOK)
    {
        for(int i = 0; i < notify_list->size;i++)
        {
            Notify_t* notify = list_find_at(notify_list, i);
            if(notify->id == id)//更新所有对应id(流水号)的动作状态
            {   
                //执行过后才可以更新状态，防止其他事件出现，提前更新状态
                if(notify->execute == 1)
                {
                    //获取id对应的实际状态
                    Notify_Edit(notify);
                }
            }
        } 
        osMutexRelease(notifyMutexHandle);
    }      
}

