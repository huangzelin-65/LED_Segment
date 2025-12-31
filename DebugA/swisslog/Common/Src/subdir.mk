################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../swisslog/Common/Src/Calculate.c \
../swisslog/Common/Src/LogDebugInfo.c \
../swisslog/Common/Src/SystemTime.c \
../swisslog/Common/Src/UserPasswd.c \
../swisslog/Common/Src/cJSON.c \
../swisslog/Common/Src/cJSON_Utils.c \
../swisslog/Common/Src/clist.c \
../swisslog/Common/Src/common.c 

OBJS += \
./swisslog/Common/Src/Calculate.o \
./swisslog/Common/Src/LogDebugInfo.o \
./swisslog/Common/Src/SystemTime.o \
./swisslog/Common/Src/UserPasswd.o \
./swisslog/Common/Src/cJSON.o \
./swisslog/Common/Src/cJSON_Utils.o \
./swisslog/Common/Src/clist.o \
./swisslog/Common/Src/common.o 

C_DEPS += \
./swisslog/Common/Src/Calculate.d \
./swisslog/Common/Src/LogDebugInfo.d \
./swisslog/Common/Src/SystemTime.d \
./swisslog/Common/Src/UserPasswd.d \
./swisslog/Common/Src/cJSON.d \
./swisslog/Common/Src/cJSON_Utils.d \
./swisslog/Common/Src/clist.d \
./swisslog/Common/Src/common.d 


# Each subdirectory must supply rules for building sources it contributes
swisslog/Common/Src/%.o swisslog/Common/Src/%.su swisslog/Common/Src/%.cyclo: ../swisslog/Common/Src/%.c swisslog/Common/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Peripherals/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Tasks/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Common/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Adaptor/Inc" -I../wolfMQTT -I../Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/ -I../Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/ -I../wolfSSL -I../LWIP/App -I../LWIP/Target -I../LWIP/LwIP/src/include -I../LWIP/LwIP/src/include/lwip -I../LWIP/LwIP/src/include/lwip/apps -I../LWIP/LwIP/src/include/lwip/prot -I../LWIP/LwIP/src/include/lwip/priv -I../LWIP/LwIP/src/include/compat/posix -I../LWIP/LwIP/src/include/compat/posix/arpa -I../LWIP/LwIP/src/include/compat/posix/net -I../LWIP/LwIP/src/include/compat/posix/sys -I../LWIP/LwIP/src/include/compat/stdc -I../LWIP/LwIP/src/include/netif -I../LWIP/LwIP/src/include/netif/ppp -I../LWIP/LwIP/system -I../LWIP/LwIP/system/arch -I../LWIP/LwIP/src/include/netif/ppp/polarssl -I../LWIP/lan8742 -I../swisslog/Robot/Inc -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog" -I../swisslog/Ota/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-swisslog-2f-Common-2f-Src

clean-swisslog-2f-Common-2f-Src:
	-$(RM) ./swisslog/Common/Src/Calculate.cyclo ./swisslog/Common/Src/Calculate.d ./swisslog/Common/Src/Calculate.o ./swisslog/Common/Src/Calculate.su ./swisslog/Common/Src/LogDebugInfo.cyclo ./swisslog/Common/Src/LogDebugInfo.d ./swisslog/Common/Src/LogDebugInfo.o ./swisslog/Common/Src/LogDebugInfo.su ./swisslog/Common/Src/SystemTime.cyclo ./swisslog/Common/Src/SystemTime.d ./swisslog/Common/Src/SystemTime.o ./swisslog/Common/Src/SystemTime.su ./swisslog/Common/Src/UserPasswd.cyclo ./swisslog/Common/Src/UserPasswd.d ./swisslog/Common/Src/UserPasswd.o ./swisslog/Common/Src/UserPasswd.su ./swisslog/Common/Src/cJSON.cyclo ./swisslog/Common/Src/cJSON.d ./swisslog/Common/Src/cJSON.o ./swisslog/Common/Src/cJSON.su ./swisslog/Common/Src/cJSON_Utils.cyclo ./swisslog/Common/Src/cJSON_Utils.d ./swisslog/Common/Src/cJSON_Utils.o ./swisslog/Common/Src/cJSON_Utils.su ./swisslog/Common/Src/clist.cyclo ./swisslog/Common/Src/clist.d ./swisslog/Common/Src/clist.o ./swisslog/Common/Src/clist.su ./swisslog/Common/Src/common.cyclo ./swisslog/Common/Src/common.d ./swisslog/Common/Src/common.o ./swisslog/Common/Src/common.su

.PHONY: clean-swisslog-2f-Common-2f-Src

