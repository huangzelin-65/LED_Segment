################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../swisslog/Tasks/Src/Task_BoxCtrl.c \
../swisslog/Tasks/Src/Task_BoxRfid.c \
../swisslog/Tasks/Src/Task_Can.c \
../swisslog/Tasks/Src/Task_CarRfid.c \
../swisslog/Tasks/Src/Task_HMI.c \
../swisslog/Tasks/Src/Task_Init.c \
../swisslog/Tasks/Src/Task_IntProcess.c \
../swisslog/Tasks/Src/Task_Json.c \
../swisslog/Tasks/Src/Task_MotionCtrl.c \
../swisslog/Tasks/Src/Task_Mqtt.c \
../swisslog/Tasks/Src/Task_Ntp.c \
../swisslog/Tasks/Src/Task_Robot.c \
../swisslog/Tasks/Src/Task_Sensor.c \
../swisslog/Tasks/Src/Task_Tcp.c \
../swisslog/Tasks/Src/Task_Test.c \
../swisslog/Tasks/Src/Task_Wifi.c 

OBJS += \
./swisslog/Tasks/Src/Task_BoxCtrl.o \
./swisslog/Tasks/Src/Task_BoxRfid.o \
./swisslog/Tasks/Src/Task_Can.o \
./swisslog/Tasks/Src/Task_CarRfid.o \
./swisslog/Tasks/Src/Task_HMI.o \
./swisslog/Tasks/Src/Task_Init.o \
./swisslog/Tasks/Src/Task_IntProcess.o \
./swisslog/Tasks/Src/Task_Json.o \
./swisslog/Tasks/Src/Task_MotionCtrl.o \
./swisslog/Tasks/Src/Task_Mqtt.o \
./swisslog/Tasks/Src/Task_Ntp.o \
./swisslog/Tasks/Src/Task_Robot.o \
./swisslog/Tasks/Src/Task_Sensor.o \
./swisslog/Tasks/Src/Task_Tcp.o \
./swisslog/Tasks/Src/Task_Test.o \
./swisslog/Tasks/Src/Task_Wifi.o 

C_DEPS += \
./swisslog/Tasks/Src/Task_BoxCtrl.d \
./swisslog/Tasks/Src/Task_BoxRfid.d \
./swisslog/Tasks/Src/Task_Can.d \
./swisslog/Tasks/Src/Task_CarRfid.d \
./swisslog/Tasks/Src/Task_HMI.d \
./swisslog/Tasks/Src/Task_Init.d \
./swisslog/Tasks/Src/Task_IntProcess.d \
./swisslog/Tasks/Src/Task_Json.d \
./swisslog/Tasks/Src/Task_MotionCtrl.d \
./swisslog/Tasks/Src/Task_Mqtt.d \
./swisslog/Tasks/Src/Task_Ntp.d \
./swisslog/Tasks/Src/Task_Robot.d \
./swisslog/Tasks/Src/Task_Sensor.d \
./swisslog/Tasks/Src/Task_Tcp.d \
./swisslog/Tasks/Src/Task_Test.d \
./swisslog/Tasks/Src/Task_Wifi.d 


# Each subdirectory must supply rules for building sources it contributes
swisslog/Tasks/Src/%.o swisslog/Tasks/Src/%.su swisslog/Tasks/Src/%.cyclo: ../swisslog/Tasks/Src/%.c swisslog/Tasks/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Peripherals/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Tasks/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Common/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Adaptor/Inc" -I../wolfMQTT -I../Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/ -I../Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/ -I../wolfSSL -I../LWIP/App -I../LWIP/Target -I../LWIP/LwIP/src/include -I../LWIP/LwIP/src/include/lwip -I../LWIP/LwIP/src/include/lwip/apps -I../LWIP/LwIP/src/include/lwip/prot -I../LWIP/LwIP/src/include/lwip/priv -I../LWIP/LwIP/src/include/compat/posix -I../LWIP/LwIP/src/include/compat/posix/arpa -I../LWIP/LwIP/src/include/compat/posix/net -I../LWIP/LwIP/src/include/compat/posix/sys -I../LWIP/LwIP/src/include/compat/stdc -I../LWIP/LwIP/src/include/netif -I../LWIP/LwIP/src/include/netif/ppp -I../LWIP/LwIP/system -I../LWIP/LwIP/system/arch -I../LWIP/LwIP/src/include/netif/ppp/polarssl -I../LWIP/lan8742 -I../swisslog/Robot/Inc -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog" -I../swisslog/Ota/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-swisslog-2f-Tasks-2f-Src

clean-swisslog-2f-Tasks-2f-Src:
	-$(RM) ./swisslog/Tasks/Src/Task_BoxCtrl.cyclo ./swisslog/Tasks/Src/Task_BoxCtrl.d ./swisslog/Tasks/Src/Task_BoxCtrl.o ./swisslog/Tasks/Src/Task_BoxCtrl.su ./swisslog/Tasks/Src/Task_BoxRfid.cyclo ./swisslog/Tasks/Src/Task_BoxRfid.d ./swisslog/Tasks/Src/Task_BoxRfid.o ./swisslog/Tasks/Src/Task_BoxRfid.su ./swisslog/Tasks/Src/Task_Can.cyclo ./swisslog/Tasks/Src/Task_Can.d ./swisslog/Tasks/Src/Task_Can.o ./swisslog/Tasks/Src/Task_Can.su ./swisslog/Tasks/Src/Task_CarRfid.cyclo ./swisslog/Tasks/Src/Task_CarRfid.d ./swisslog/Tasks/Src/Task_CarRfid.o ./swisslog/Tasks/Src/Task_CarRfid.su ./swisslog/Tasks/Src/Task_HMI.cyclo ./swisslog/Tasks/Src/Task_HMI.d ./swisslog/Tasks/Src/Task_HMI.o ./swisslog/Tasks/Src/Task_HMI.su ./swisslog/Tasks/Src/Task_Init.cyclo ./swisslog/Tasks/Src/Task_Init.d ./swisslog/Tasks/Src/Task_Init.o ./swisslog/Tasks/Src/Task_Init.su ./swisslog/Tasks/Src/Task_IntProcess.cyclo ./swisslog/Tasks/Src/Task_IntProcess.d ./swisslog/Tasks/Src/Task_IntProcess.o ./swisslog/Tasks/Src/Task_IntProcess.su ./swisslog/Tasks/Src/Task_Json.cyclo ./swisslog/Tasks/Src/Task_Json.d ./swisslog/Tasks/Src/Task_Json.o ./swisslog/Tasks/Src/Task_Json.su ./swisslog/Tasks/Src/Task_MotionCtrl.cyclo ./swisslog/Tasks/Src/Task_MotionCtrl.d ./swisslog/Tasks/Src/Task_MotionCtrl.o ./swisslog/Tasks/Src/Task_MotionCtrl.su ./swisslog/Tasks/Src/Task_Mqtt.cyclo ./swisslog/Tasks/Src/Task_Mqtt.d ./swisslog/Tasks/Src/Task_Mqtt.o ./swisslog/Tasks/Src/Task_Mqtt.su ./swisslog/Tasks/Src/Task_Ntp.cyclo ./swisslog/Tasks/Src/Task_Ntp.d ./swisslog/Tasks/Src/Task_Ntp.o ./swisslog/Tasks/Src/Task_Ntp.su ./swisslog/Tasks/Src/Task_Robot.cyclo ./swisslog/Tasks/Src/Task_Robot.d ./swisslog/Tasks/Src/Task_Robot.o ./swisslog/Tasks/Src/Task_Robot.su ./swisslog/Tasks/Src/Task_Sensor.cyclo ./swisslog/Tasks/Src/Task_Sensor.d ./swisslog/Tasks/Src/Task_Sensor.o ./swisslog/Tasks/Src/Task_Sensor.su ./swisslog/Tasks/Src/Task_Tcp.cyclo ./swisslog/Tasks/Src/Task_Tcp.d ./swisslog/Tasks/Src/Task_Tcp.o ./swisslog/Tasks/Src/Task_Tcp.su ./swisslog/Tasks/Src/Task_Test.cyclo ./swisslog/Tasks/Src/Task_Test.d ./swisslog/Tasks/Src/Task_Test.o ./swisslog/Tasks/Src/Task_Test.su ./swisslog/Tasks/Src/Task_Wifi.cyclo ./swisslog/Tasks/Src/Task_Wifi.d ./swisslog/Tasks/Src/Task_Wifi.o ./swisslog/Tasks/Src/Task_Wifi.su

.PHONY: clean-swisslog-2f-Tasks-2f-Src

