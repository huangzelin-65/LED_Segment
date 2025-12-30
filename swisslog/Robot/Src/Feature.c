#include "LogDebugInfo.h"
#include "Feature.h"
#include "clist.h"
#include "JsonCommon.h"
#include "main.h"

List *feature_list = NULL;
//链表初始化
void Feature_ListInit(void)
{
    feature_list = list_init();
    if(feature_list != NULL)
    {
        DEBUGINFO("list init Success");
    }
}

void Feature_Init(void)
{
    Feature_ListInit();
}

//寻找对应流水号的数据
int Feature_FindId(const void* feature, const void* input_feature)
{
    // 强制类型转换为int*，取值后比较
    const  Feature_t* val_feature = (const  Feature_t*)feature;
    const  Feature_t* val_input_feature = (const  Feature_t*)input_feature;

    if(val_feature->id == val_input_feature->id)
    {
        return 0;
    }
    return -1;
}
//寻找对应name的数据
int Feature_FindName(const void* feature, const void* input_feature)
{
    // 强制类型转换为int*，取值后比较
    const  Feature_t* val_feature = (const  Feature_t*)feature;
    const  Feature_t* val_input_feature = (const  Feature_t*)input_feature;

    return strcmp(val_feature->name,val_input_feature->name);
}
//事件发生，需更新功能，id为0，针对于单机器状态使用 (此接口json解析函数中使用)
void Feature_Event(Feature_t *feature)
{
    DEBUGINFO("id:%d headerId:%s name:%s value:%s params:%s",feature->id,feature->headerId,feature->name,feature->value,feature->params);
    //第一次使用需创建链表
    if(feature_list == NULL)
    {
        Feature_Init();
    }  
    if(feature_list == NULL) return;    
    //创建状态数据
    Feature_t* new_feature = pvPortMalloc(sizeof(Feature_t));

    if(new_feature == NULL)return;

    memset(new_feature,0,sizeof(Feature_t));

    memcpy(new_feature,feature,sizeof(Feature_t));

    int exist = 0;
    for(int i = 0; i < feature_list->size;i++)
    {
        Feature_t* feature = list_find_at(feature_list, i);
        if(feature->id == new_feature->id)
        {   
            if(strcmp(feature->name,new_feature->name) == 0
            && strcmp(feature->value,new_feature->value) == 0
            && strcmp(feature->params,new_feature->params) == 0)
            {
                DEBUGINFO("feature has exist,all params same");
                vPortFree(new_feature);
                return;
            }
            //相同的功能，则覆盖
            if(strcmp(feature->name,new_feature->name) == 0
            &&(strcmp(feature->value,new_feature->value) != 0 || strcmp(feature->params,new_feature->params) != 0))
            {
                DEBUGINFO("feature has exist,but something change");
                memcpy(feature,new_feature,sizeof(Feature_t));
                exist = 1;
            }            
        }
    }    

    if(exist)//存在，只是参数不同,释放内存
    {
        vPortFree(new_feature);
    }
    else
    {
        list_insert_tail(feature_list,new_feature);
    }
    
    Feature_Execute();

    DEBUGINFO("list_size:%d",list_size(feature_list));
}

//增加最新链表执行动作
void Feature_Execute(void)
{
    for(int i = 0; i < feature_list->size;i++)
    {
        Feature_t* feature = list_find_at(feature_list, i);
        if(feature->execute == 0)
        {   
            //需要回复ack
            Feature_t* json_feature = pvPortMalloc(sizeof(Feature_t));
            feature->code = 0;//默认回复执行成功
            memcpy(json_feature,feature,sizeof(Feature_t));            
            Json_GenerateMsg(JSON_G_FEATURE,json_feature);
            //执行动作
            Feature_ToCmd(feature);
            //标记动作已经执行
            feature->execute = 1;
        }
    }
}

//动作实际执行
void Feature_ToCmd(Feature_t* feature)
{
    DEBUGINFO("id:%d",feature->id);//执行需要根据流水号发到对应机器中

    //设置屏幕
    {
        char *res = strstr(feature->name, "screenLock");
        if (res != NULL) {
            DEBUGINFO("screenLock\n"); 
            char *res = strstr(feature->value, "unlock");  
            if (res != NULL) {
                DEBUGINFO("unlock\n"); 

            }
            else
            {
                DEBUGINFO("lock\n"); 

            }                                                   
        } 
    }
    //设置模式
    {
        char *res = strstr(feature->name, "runModel");
        if (res != NULL) {
            DEBUGINFO("runModel\n"); 
            char *res = strstr(feature->value, "auto");  
            if (res != NULL) {
                DEBUGINFO("auto\n"); 

            }
            else
            {
                DEBUGINFO("manual\n"); 

            }                                                   
        }
    }

}
//根据实际机器状态编辑feature状态
void Feature_Edit(Feature_t *feature)
{
    //屏幕状态更新
    {
        char *res = strstr(feature->name, "screenLock");
        if (res != NULL) {
            DEBUGINFO("screenLock\n");

            //这里需要从全局变量或接口中获取

            //更新完回复最新状态到服务器

            //删除链表中的状态，释放内存

        }
    }

    //更新模式状态
    {
        char *res = strstr(feature->name, "runModel");
        if (res != NULL) {
            DEBUGINFO("runModel\n");

            //这里需要从全局变量或接口中获取
            feature->mode = CarStatus.xAutoMode;
            //更新完回复最新状态到服务器
            if(CarStatus.xAutoMode == Manual)
            {
                memcpy(feature->value,"manual",7);
            }
            else
            {
                memcpy(feature->value,"auto",5);
            }
            Feature_t* json_feature = pvPortMalloc(sizeof(Feature_t));
            feature->code = 0;//默认回复执行成功
            memcpy(json_feature,feature,sizeof(Feature_t));            
            Json_GenerateMsg(JSON_G_FEATURE,json_feature);            
            //删除链表中的状态，释放内存
            list_remove_by_value(feature_list,feature,Feature_FindName,vPortFree);
        }        
    }
    //其他功能设置
    
}
//功能的更新(根据流水号)
void Feature_Update(int id)
{
    DEBUGINFO("id:%d",id);
    //第一次使用需创建链表
    if(feature_list == NULL)
    {
        Feature_ListInit();
    }  
    if(feature_list == NULL) return;   
    for(int i = 0; i < feature_list->size;i++)
    {
        Feature_t* feature = list_find_at(feature_list, i);
        if(feature->id == id)//更新所有对应id(流水号)的动作状态
        {   
            //执行过后才可以更新状态，防止其他事件出现，提前更新状态
            if(feature->execute == 1)
            {
                //获取id对应的实际状态
                Feature_Edit(feature);
            }
        }
    }    
}

