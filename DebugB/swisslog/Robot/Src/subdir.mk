################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../swisslog/Robot/Src/Action.c \
../swisslog/Robot/Src/Config.c \
../swisslog/Robot/Src/Feature.c \
../swisslog/Robot/Src/Heartbeat.c \
../swisslog/Robot/Src/JsonCommon.c \
../swisslog/Robot/Src/Notify.c \
../swisslog/Robot/Src/Robot.c \
../swisslog/Robot/Src/State.c \
../swisslog/Robot/Src/StringCommon.c 

OBJS += \
./swisslog/Robot/Src/Action.o \
./swisslog/Robot/Src/Config.o \
./swisslog/Robot/Src/Feature.o \
./swisslog/Robot/Src/Heartbeat.o \
./swisslog/Robot/Src/JsonCommon.o \
./swisslog/Robot/Src/Notify.o \
./swisslog/Robot/Src/Robot.o \
./swisslog/Robot/Src/State.o \
./swisslog/Robot/Src/StringCommon.o 

C_DEPS += \
./swisslog/Robot/Src/Action.d \
./swisslog/Robot/Src/Config.d \
./swisslog/Robot/Src/Feature.d \
./swisslog/Robot/Src/Heartbeat.d \
./swisslog/Robot/Src/JsonCommon.d \
./swisslog/Robot/Src/Notify.d \
./swisslog/Robot/Src/Robot.d \
./swisslog/Robot/Src/State.d \
./swisslog/Robot/Src/StringCommon.d 


# Each subdirectory must supply rules for building sources it contributes
swisslog/Robot/Src/%.o swisslog/Robot/Src/%.su swisslog/Robot/Src/%.cyclo: ../swisslog/Robot/Src/%.c swisslog/Robot/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Peripherals/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Tasks/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Common/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Adaptor/Inc" -I../wolfMQTT -I../Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/ -I../Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/ -I../wolfSSL -I../LWIP/App -I../LWIP/Target -I../LWIP/LwIP/src/include -I../LWIP/LwIP/src/include/lwip -I../LWIP/LwIP/src/include/lwip/apps -I../LWIP/LwIP/src/include/lwip/prot -I../LWIP/LwIP/src/include/lwip/priv -I../LWIP/LwIP/src/include/compat/posix -I../LWIP/LwIP/src/include/compat/posix/arpa -I../LWIP/LwIP/src/include/compat/posix/net -I../LWIP/LwIP/src/include/compat/posix/sys -I../LWIP/LwIP/src/include/compat/stdc -I../LWIP/LwIP/src/include/netif -I../LWIP/LwIP/src/include/netif/ppp -I../LWIP/LwIP/system -I../LWIP/LwIP/system/arch -I../LWIP/LwIP/src/include/netif/ppp/polarssl -I../LWIP/lan8742 -I../swisslog/Robot/Inc -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog" -I../swisslog/Ota/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-swisslog-2f-Robot-2f-Src

clean-swisslog-2f-Robot-2f-Src:
	-$(RM) ./swisslog/Robot/Src/Action.cyclo ./swisslog/Robot/Src/Action.d ./swisslog/Robot/Src/Action.o ./swisslog/Robot/Src/Action.su ./swisslog/Robot/Src/Config.cyclo ./swisslog/Robot/Src/Config.d ./swisslog/Robot/Src/Config.o ./swisslog/Robot/Src/Config.su ./swisslog/Robot/Src/Feature.cyclo ./swisslog/Robot/Src/Feature.d ./swisslog/Robot/Src/Feature.o ./swisslog/Robot/Src/Feature.su ./swisslog/Robot/Src/Heartbeat.cyclo ./swisslog/Robot/Src/Heartbeat.d ./swisslog/Robot/Src/Heartbeat.o ./swisslog/Robot/Src/Heartbeat.su ./swisslog/Robot/Src/JsonCommon.cyclo ./swisslog/Robot/Src/JsonCommon.d ./swisslog/Robot/Src/JsonCommon.o ./swisslog/Robot/Src/JsonCommon.su ./swisslog/Robot/Src/Notify.cyclo ./swisslog/Robot/Src/Notify.d ./swisslog/Robot/Src/Notify.o ./swisslog/Robot/Src/Notify.su ./swisslog/Robot/Src/Robot.cyclo ./swisslog/Robot/Src/Robot.d ./swisslog/Robot/Src/Robot.o ./swisslog/Robot/Src/Robot.su ./swisslog/Robot/Src/State.cyclo ./swisslog/Robot/Src/State.d ./swisslog/Robot/Src/State.o ./swisslog/Robot/Src/State.su ./swisslog/Robot/Src/StringCommon.cyclo ./swisslog/Robot/Src/StringCommon.d ./swisslog/Robot/Src/StringCommon.o ./swisslog/Robot/Src/StringCommon.su

.PHONY: clean-swisslog-2f-Robot-2f-Src

