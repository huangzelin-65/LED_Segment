#include "LogDebugInfo.h"
#include "JsonCommon.h"
#include "State.h"
#include "clist.h"
#include "main.h"

List *state_list = NULL;
//链表初始化
void State_ListInit(void)
{
    state_list = list_init();
    if(state_list != NULL)
    {
        DEBUGINFO("list init Success");
    }
}

void State_Init(void)
{
    State_ListInit();
}
//寻找对应流水号的数据
int State_FindId(const void* state, const void* input_state)
{
    // 强制类型转换为int*，取值后比较
    const  State_t* val_state = (const  State_t*)state;
    const  State_t* val_input_state = (const  State_t*)input_state;

    if(val_state->id == val_input_state->id)
    {
        return 0;
    }
    return -1;
}
// 辅助函数：对比字符串指针（处理NULL场景）
static bool str_ptr_equal(const char *s1, const char *s2) {
    if (s1 == NULL && s2 == NULL) return true;  // 都为NULL
    if (s1 == NULL || s2 == NULL) return false; // 一个NULL一个非NULL
    return strcmp(s1, s2) == 0;                 // 都非NULL，对比内容
}
/**
 * @brief 对比两个State_t结构体是否存在成员差异
 * @param a 结构体A指针（不可为NULL）
 * @param b 结构体B指针（不可为NULL）
 * @return 0：无差异；1：有差异；-1：参数非法（NULL）
 */
int State_Compare(const State_t *a, const State_t *b) 
{
    // 入参合法性检查
    if (a == NULL || b == NULL) {
        return -1;
    }
    // 1. 字符数组成员对比（只要一个不同，立即返回有差异）
    // if (strcmp(a->headerId, b->headerId) != 0)      return 1;
    // if (strcmp(a->timestamp, b->timestamp) != 0)    return 1;
    // if (strcmp(a->version, b->version) != 0)        return 1;

    // // 2. 字符串指针成员对比
    // if (!str_ptr_equal(a->manufacturer, b->manufacturer))  return 1;
    // if (!str_ptr_equal(a->serialNumber, b->serialNumber))  return 1;
    //以上数据暂不比对
    if (!str_ptr_equal(a->operatingMode, b->operatingMode))return 1;

    // 3. int类型成员
    if (a->id != b->id)              return 1;
    if (a->runtime != b->runtime)    return 1;
    if (a->car_running != b->car_running) return 1;

    // 4. uint32_t类型
    if (a->curPos != b->curPos)      return 1;

    // 5. bool类型
    if (a->lockState1 != b->lockState1)  return 1;
    if (a->lockState2 != b->lockState2)  return 1;
    if (a->lockState != b->lockState)    return 1;

    // 6. 嵌套SensorPos - bumperState
    if (a->bumperState.front != b->bumperState.front)  return 1;
    if (a->bumperState.back != b->bumperState.back)    return 1;

    // 7. 嵌套SensorPos - hallState
    if (a->hallState.front != b->hallState.front)      return 1;
    if (a->hallState.back != b->hallState.back)        return 1;

    // 8. 嵌套MotorState
    if (a->motorState.speedLevel != b->motorState.speedLevel)return 1;
    if (a->motorState.direction != b->motorState.direction)  return 1;

    // 9. 嵌套UDisinfection
    if (a->disinfect.runState != b->disinfect.runState)  return 1;
    if (a->disinfect.runTime != b->disinfect.runTime)    return 1;
    if (a->disinfect.startTime != b->disinfect.startTime)return 1;
    if (a->disinfect.setTime != b->disinfect.setTime)    return 1;

    // 10. 嵌套Error_t（字符串指针）
    if (!str_ptr_equal(a->errors.Type, b->errors.Type))    return 1;
    if (!str_ptr_equal(a->errors.Level, b->errors.Level))  return 1;

    // 11. 数组uid[3]
    if (a->uid[0] != b->uid[0])  return 1;
    if (a->uid[1] != b->uid[1])  return 1;
    if (a->uid[2] != b->uid[2])  return 1;

    // 12. uint16_t类型
    if (a->encode_number != b->encode_number)  return 1;

    // 所有成员都相同
    return 0;
}
//判断状态是否发生变化
//返回值，0：无变化 1:有变化 2：不存在
int State_IsChange(State_t* input_state)
{
    State_t* state = (State_t*)list_find_by_value(state_list,input_state,State_FindId);
    if(state == NULL)//找不到对应state,则是新数据，需要更新链表
    {
        return 2;
    }
    else//找的到数据则对比每个成员
    {
        return State_Compare(state,input_state);
    }    
}
//更新状态数据
void State_Update(State_t* new_state)
{
    if(new_state->id == 0)//只有一份状态则默认获取
    {
        DEBUGINFO("curPos:%d xRealDirection:%d xRealSpeed:%d xAutoMode:%d",CarStatus.dwCurPos,CarStatus.xRealDirection,CarStatus.xRealSpeed,CarStatus.xAutoMode);
        new_state->lockState1 = (CarStatus.xBoxELockStatus1 == Locked ? true : false);
        new_state->lockState2 = (CarStatus.xBoxELockStatus2 == Locked ? true : false);
        new_state->lockState = (CarStatus.xBoxLocked == Locked ? true : false);
        new_state->bumperState.front = (CarStatus.FrontCrashStatus == SensorTrigger ? true : false);
        new_state->bumperState.back = (CarStatus.RearCrashStatus == SensorTrigger ? true : false);
        new_state->hallState.front = (CarStatus.FrontProxStatus == SensorTrigger ? true : false);
        new_state->hallState.back = (CarStatus.RearProxStatus == SensorTrigger ? true : false);
        new_state->curPos = CarStatus.dwCurPos;
        new_state->motorState.direction = CarStatus.xRealDirection;
        new_state->motorState.speedLevel = CarStatus.xRealSpeed;
        new_state->operatingMode = (CarStatus.xAutoMode == Auto ? MODE_TYPE_AUTO : MODE_TYPE_MANUAL);
        DEBUGINFO("front:%d back:%d",new_state->bumperState.front,new_state->bumperState.back);
    }
    else//多份状态则从不同地方获取对应id机器的状态
    {

    }
}
//事件发生，需更新状态，id为0，针对于单机器状态使用
void State_Event(int id)
{
    DEBUGINFO("id:%d",id);
    //第一次使用需创建链表
    if(state_list == NULL)
    {
        State_ListInit();
    }    
    //创建状态数据
    State_t* new_state = pvPortMalloc(sizeof(State_t));

    if(new_state == NULL)return;

    memset(new_state,0,sizeof(State_t));

    new_state->id = id;
    //给每个成员赋值
    State_Update(new_state);
    //对比数组有无发生变化
    int change_state = State_IsChange(new_state);
    DEBUGINFO("change_state:%d",change_state);
    switch(change_state)
    {
        case 0:
                //数据没有变化
        break;
        case 1: //数据有变化，链表中的数据重新赋值
        {
            State_t* state = (State_t*)list_find_by_value(state_list,new_state,State_FindId);
            memcpy(state,new_state,sizeof(State_t));
            vPortFree(new_state);//赋值完释放内存

            //需发送到队列中，创建json数据
            Json_GenerateMsg(JSON_G_STATE,state);
        }        
        break;
        case 2: //数据不存在，需要添加到链表中
        {
            list_insert_tail(state_list,new_state);
            
            //需发送到队列中，创建json数据
            Json_GenerateMsg(JSON_G_STATE,new_state);
        }
        break;        
        default:break;
    }
    DEBUGINFO("list_size:%d",list_size(state_list));
}
//删除某个id状态
void State_DeleteId(int id)
{
    DEBUGINFO("id:%d",id);
    State_t t_state;
    t_state.id = id;
    list_remove_by_value(state_list,&t_state,State_FindId,vPortFree);
    DEBUGINFO("list_size:%d",list_size(state_list));
}







