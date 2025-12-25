#include "JsonGenerate.h"
#include "LogDebugInfo.h"
#include "app_freertos.h"
#include "queue.h"
#include "semphr.h"
#include "State.h"


void Json_GenerateMsg(JsonGenerateType_t type,void *data)
{
    if(JsonGenerateQueueHandle != NULL)
    {
        JsonGenerate_t * msg = pvPortMalloc(sizeof(JsonGenerate_t));
        if(msg == NULL)return;
        msg->type = type;
        msg->data = data;        
        DEBUGINFO("uxQueueGetQueueLength:%d uxQueueSpacesAvailable:%d\n",uxQueueGetQueueLength(JsonGenerateQueueHandle),uxQueueSpacesAvailable(JsonGenerateQueueHandle));
        if (xQueueSend(JsonGenerateQueueHandle, &msg, portMAX_DELAY) == pdPASS) 
        {
            DEBUGINFO("type :%d\n",type);
        } 
    }     
}
