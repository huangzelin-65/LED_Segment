################################################################################
# 自动生成的文件。不要编辑！
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# 将这些工具调用的输入和输出添加到构建变量 
C_SRCS += \
../swisslog/Task/Src/Calculate_Crc16.c \
../swisslog/Task/Src/LogDebugInfo.c \
../swisslog/Task/Src/Task_Init.c \
../swisslog/Task/Src/Task_LED_Segment.c \
../swisslog/Task/Src/Task_Parse.c \
../swisslog/Task/Src/Task_rs485.c 

OBJS += \
./swisslog/Task/Src/Calculate_Crc16.o \
./swisslog/Task/Src/LogDebugInfo.o \
./swisslog/Task/Src/Task_Init.o \
./swisslog/Task/Src/Task_LED_Segment.o \
./swisslog/Task/Src/Task_Parse.o \
./swisslog/Task/Src/Task_rs485.o 

C_DEPS += \
./swisslog/Task/Src/Calculate_Crc16.d \
./swisslog/Task/Src/LogDebugInfo.d \
./swisslog/Task/Src/Task_Init.d \
./swisslog/Task/Src/Task_LED_Segment.d \
./swisslog/Task/Src/Task_Parse.d \
./swisslog/Task/Src/Task_rs485.d 


# 每个子目录必须为构建它所贡献的源提供规则
swisslog/Task/Src/%.o swisslog/Task/Src/%.su swisslog/Task/Src/%.cyclo: ../swisslog/Task/Src/%.c swisslog/Task/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I"C:/Users/e3huara25g/STM32CubeIDE/workspace_1.19.0/LED_Segment/swisslog/Task/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-swisslog-2f-Task-2f-Src

clean-swisslog-2f-Task-2f-Src:
	-$(RM) ./swisslog/Task/Src/Calculate_Crc16.cyclo ./swisslog/Task/Src/Calculate_Crc16.d ./swisslog/Task/Src/Calculate_Crc16.o ./swisslog/Task/Src/Calculate_Crc16.su ./swisslog/Task/Src/LogDebugInfo.cyclo ./swisslog/Task/Src/LogDebugInfo.d ./swisslog/Task/Src/LogDebugInfo.o ./swisslog/Task/Src/LogDebugInfo.su ./swisslog/Task/Src/Task_Init.cyclo ./swisslog/Task/Src/Task_Init.d ./swisslog/Task/Src/Task_Init.o ./swisslog/Task/Src/Task_Init.su ./swisslog/Task/Src/Task_LED_Segment.cyclo ./swisslog/Task/Src/Task_LED_Segment.d ./swisslog/Task/Src/Task_LED_Segment.o ./swisslog/Task/Src/Task_LED_Segment.su ./swisslog/Task/Src/Task_Parse.cyclo ./swisslog/Task/Src/Task_Parse.d ./swisslog/Task/Src/Task_Parse.o ./swisslog/Task/Src/Task_Parse.su ./swisslog/Task/Src/Task_rs485.cyclo ./swisslog/Task/Src/Task_rs485.d ./swisslog/Task/Src/Task_rs485.o ./swisslog/Task/Src/Task_rs485.su

.PHONY: clean-swisslog-2f-Task-2f-Src

