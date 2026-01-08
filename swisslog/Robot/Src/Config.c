#include "LogDebugInfo.h"
#include "Config.h"
#include "clist.h"
#include "JsonCommon.h"
#include "main.h"

List *config_list = NULL;
//链表初始化
void Config_ListInit(void)
{
    config_list = list_init();
    if(config_list != NULL)
    {
        DEBUGINFO("list init Success");
    }
}

void Config_Init(void)
{
    Config_ListInit();
}

//寻找对应流水号的数据
int Config_FindId(const void* config, const void* input_config)
{
    // 强制类型转换为int*，取值后比较
    const  Config_t* val_config = (const  Config_t*)config;
    const  Config_t* val_input_config = (const  Config_t*)input_config;

    if(val_config->id == val_input_config->id)
    {
        return 0;
    }
    return -1;
}
//寻找对应name的数据
int Config_FindName(const void* config, const void* input_config)
{
    // 强制类型转换为int*，取值后比较
    const  Config_t* val_config = (const  Config_t*)config;
    const  Config_t* val_input_config = (const  Config_t*)input_config;

    return strcmp(val_config->name,val_input_config->name);
}
//事件发生，需更新配置，id为0，针对于单机器状态使用 (此接口json解析函数中使用)
void Config_Event(Config_t *config)
{
    DEBUGINFO("id:%d headerId:%s name:%s value:%s params:%s",config->id,config->headerId,config->name,config->value,config->params);
    //第一次使用需创建链表
    if(config_list == NULL)
    {
        Config_Init();
    }  
    if(config_list == NULL) return;  
    if(configMutexHandle == NULL)return;   

    if (osMutexAcquire(configMutexHandle, portMAX_DELAY) == osOK)
    {
        //创建状态数据
        Config_t* new_config = pvPortMalloc(sizeof(Config_t));

        if(new_config == NULL)return;

        memset(new_config,0,sizeof(Config_t));

        memcpy(new_config,config,sizeof(Config_t));

        int exist = 0;
        for(int i = 0; i < config_list->size;i++)
        {
            Config_t* config = list_find_at(config_list, i);
            if(config->id == new_config->id)
            {   
                if(strcmp(config->name,new_config->name) == 0
                && strcmp(config->value,new_config->value) == 0
                && strcmp(config->params,new_config->params) == 0)
                {
                    DEBUGINFO("config has exist,all params same");
                    vPortFree(new_config);
                    osMutexRelease(configMutexHandle);
                    return;
                }
                //相同的功能，则覆盖
                if(strcmp(config->name,new_config->name) == 0
                &&(strcmp(config->value,new_config->value) != 0 || strcmp(config->params,new_config->params) != 0))
                {
                    DEBUGINFO("config has exist,but something change");
                    memcpy(config,new_config,sizeof(Config_t));
                    exist = 1;
                }            
            }
        }    

        if(exist)//存在，只是参数不同,释放内存
        {
            vPortFree(new_config);
        }
        else
        {
            list_insert_tail(config_list,new_config);
        }
        
        Config_Execute();

        DEBUGINFO("list_size:%d",list_size(config_list));
        osMutexRelease(configMutexHandle);
    }
}

//增加最新链表执行动作
void Config_Execute(void)
{
    for(int i = 0; i < config_list->size;i++)
    {
        Config_t* config = list_find_at(config_list, i);
        if(config->execute == 0)
        {   
            //需要回复ack
            Config_t* json_config = pvPortMalloc(sizeof(Config_t));
            config->code = 0;//默认回复执行成功
            memcpy(json_config,config,sizeof(Config_t));            
            Json_GenerateMsg(JSON_G_CONFIG,json_config);
            //标记动作已经执行
            config->execute = 1;
            //执行动作
            Config_ToCmd(config);            
        }
    }
    //如不等待配置结果，此处直接释放内存
    while(!list_is_empty(config_list))
    {
        list_pop_tail(config_list,vPortFree);
    }
}

//动作实际执行
void Config_ToCmd(Config_t* config)
{
    DEBUGINFO("id:%d",config->id);//执行需要根据流水号发到对应机器中

    //设置速度
    {
        char *res = strstr(config->name, "speedLevel1");
        if (res != NULL) {
            DEBUGINFO("speedLevel1\n"); 
            char *res = strstr(config->value, "1");  
            if (res != NULL) {
                DEBUGINFO("1\n"); 

            }                                                  
        } 
    }
    //设置消毒时间
    {
        char *res = strstr(config->name, "disinfect");
        if (res != NULL) {
            DEBUGINFO("disinfect\n"); 
            DEBUGINFO("time:%s min\n",config->value);                                                    
        }
    }
    //设置洁车污车
    {
        char *res = strstr(config->name, "kind");
        if (res != NULL) {
            DEBUGINFO("kind\n"); 
            DEBUGINFO("value:%s\n",config->value);  
            res = strstr(config->name, "clean");
            if (res != NULL) {
                HMI_Update_DirtyStatus_Req(0);
            }
            else
            {
                HMI_Update_DirtyStatus_Req(1);
            }                                                  
        }

    }
}
//根据实际机器状态编辑config状态
void Config_Edit(Config_t *config)
{
    //配置状态更新
    {
        char *res = strstr(config->name, "speedLevel1");
        if (res != NULL) {
            DEBUGINFO("speedLevel1\n");

            //这里需要从全局变量或接口中获取

            //更新完回复最新状态到服务器

            //删除链表中的状态，释放内存
            list_remove_by_value(config_list,config,Config_FindName,vPortFree);
        }
    }

    //其他功能设置
    
}
//功能的更新(根据流水号)
void Config_Update(int id)
{
    DEBUGINFO("id:%d",id);
    //第一次使用需创建链表
    if(config_list == NULL)
    {
        Config_ListInit();
    }  
    if(config_list == NULL) return;
    if(configMutexHandle == NULL)return;  
    
    if (osMutexAcquire(configMutexHandle, portMAX_DELAY) == osOK)
    {
        for(int i = 0; i < config_list->size;i++)
        {
            Config_t* config = list_find_at(config_list, i);
            if(config->id == id)//更新所有对应id(流水号)的动作状态
            {   
                //执行过后才可以更新状态，防止其他事件出现，提前更新状态
                if(config->execute == 1)
                {
                    //获取id对应的实际状态
                    Config_Edit(config);
                }
            }
        }
        osMutexRelease(configMutexHandle);
    }
}

