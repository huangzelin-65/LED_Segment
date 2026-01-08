################################################################################
# 自动生成的文件。不要编辑！
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# 将这些工具调用的输入和输出添加到构建变量 
C_SRCS += \
../swisslog/Task/Inc/Task_LED_Segment.c 

OBJS += \
./swisslog/Task/Inc/Task_LED_Segment.o 

C_DEPS += \
./swisslog/Task/Inc/Task_LED_Segment.d 


# 每个子目录必须为构建它所贡献的源提供规则
swisslog/Task/Inc/%.o swisslog/Task/Inc/%.su swisslog/Task/Inc/%.cyclo: ../swisslog/Task/Inc/%.c swisslog/Task/Inc/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-swisslog-2f-Task-2f-Inc

clean-swisslog-2f-Task-2f-Inc:
	-$(RM) ./swisslog/Task/Inc/Task_LED_Segment.cyclo ./swisslog/Task/Inc/Task_LED_Segment.d ./swisslog/Task/Inc/Task_LED_Segment.o ./swisslog/Task/Inc/Task_LED_Segment.su

.PHONY: clean-swisslog-2f-Task-2f-Inc

