################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../swisslog/Adaptor/Src/adaptor_HMI.c \
../swisslog/Adaptor/Src/adaptor_beep.c \
../swisslog/Adaptor/Src/adaptor_box.c \
../swisslog/Adaptor/Src/adaptor_can.c \
../swisslog/Adaptor/Src/adaptor_eeprom.c \
../swisslog/Adaptor/Src/adaptor_motor.c \
../swisslog/Adaptor/Src/adaptor_mqtt.c \
../swisslog/Adaptor/Src/adaptor_ntp.c \
../swisslog/Adaptor/Src/adaptor_power.c \
../swisslog/Adaptor/Src/adaptor_rfid.c \
../swisslog/Adaptor/Src/adaptor_rtc.c \
../swisslog/Adaptor/Src/adaptor_tcp.c \
../swisslog/Adaptor/Src/adaptor_test.c \
../swisslog/Adaptor/Src/adaptor_wifi.c 

OBJS += \
./swisslog/Adaptor/Src/adaptor_HMI.o \
./swisslog/Adaptor/Src/adaptor_beep.o \
./swisslog/Adaptor/Src/adaptor_box.o \
./swisslog/Adaptor/Src/adaptor_can.o \
./swisslog/Adaptor/Src/adaptor_eeprom.o \
./swisslog/Adaptor/Src/adaptor_motor.o \
./swisslog/Adaptor/Src/adaptor_mqtt.o \
./swisslog/Adaptor/Src/adaptor_ntp.o \
./swisslog/Adaptor/Src/adaptor_power.o \
./swisslog/Adaptor/Src/adaptor_rfid.o \
./swisslog/Adaptor/Src/adaptor_rtc.o \
./swisslog/Adaptor/Src/adaptor_tcp.o \
./swisslog/Adaptor/Src/adaptor_test.o \
./swisslog/Adaptor/Src/adaptor_wifi.o 

C_DEPS += \
./swisslog/Adaptor/Src/adaptor_HMI.d \
./swisslog/Adaptor/Src/adaptor_beep.d \
./swisslog/Adaptor/Src/adaptor_box.d \
./swisslog/Adaptor/Src/adaptor_can.d \
./swisslog/Adaptor/Src/adaptor_eeprom.d \
./swisslog/Adaptor/Src/adaptor_motor.d \
./swisslog/Adaptor/Src/adaptor_mqtt.d \
./swisslog/Adaptor/Src/adaptor_ntp.d \
./swisslog/Adaptor/Src/adaptor_power.d \
./swisslog/Adaptor/Src/adaptor_rfid.d \
./swisslog/Adaptor/Src/adaptor_rtc.d \
./swisslog/Adaptor/Src/adaptor_tcp.d \
./swisslog/Adaptor/Src/adaptor_test.d \
./swisslog/Adaptor/Src/adaptor_wifi.d 


# Each subdirectory must supply rules for building sources it contributes
swisslog/Adaptor/Src/%.o swisslog/Adaptor/Src/%.su swisslog/Adaptor/Src/%.cyclo: ../swisslog/Adaptor/Src/%.c swisslog/Adaptor/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Peripherals/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Tasks/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Common/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Adaptor/Inc" -I../wolfMQTT -I../Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/ -I../Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/ -I../wolfSSL -I../LWIP/App -I../LWIP/Target -I../LWIP/LwIP/src/include -I../LWIP/LwIP/src/include/lwip -I../LWIP/LwIP/src/include/lwip/apps -I../LWIP/LwIP/src/include/lwip/prot -I../LWIP/LwIP/src/include/lwip/priv -I../LWIP/LwIP/src/include/compat/posix -I../LWIP/LwIP/src/include/compat/posix/arpa -I../LWIP/LwIP/src/include/compat/posix/net -I../LWIP/LwIP/src/include/compat/posix/sys -I../LWIP/LwIP/src/include/compat/stdc -I../LWIP/LwIP/src/include/netif -I../LWIP/LwIP/src/include/netif/ppp -I../LWIP/LwIP/system -I../LWIP/LwIP/system/arch -I../LWIP/LwIP/src/include/netif/ppp/polarssl -I../LWIP/lan8742 -I../swisslog/Robot/Inc -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog" -I../swisslog/Ota/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-swisslog-2f-Adaptor-2f-Src

clean-swisslog-2f-Adaptor-2f-Src:
	-$(RM) ./swisslog/Adaptor/Src/adaptor_HMI.cyclo ./swisslog/Adaptor/Src/adaptor_HMI.d ./swisslog/Adaptor/Src/adaptor_HMI.o ./swisslog/Adaptor/Src/adaptor_HMI.su ./swisslog/Adaptor/Src/adaptor_beep.cyclo ./swisslog/Adaptor/Src/adaptor_beep.d ./swisslog/Adaptor/Src/adaptor_beep.o ./swisslog/Adaptor/Src/adaptor_beep.su ./swisslog/Adaptor/Src/adaptor_box.cyclo ./swisslog/Adaptor/Src/adaptor_box.d ./swisslog/Adaptor/Src/adaptor_box.o ./swisslog/Adaptor/Src/adaptor_box.su ./swisslog/Adaptor/Src/adaptor_can.cyclo ./swisslog/Adaptor/Src/adaptor_can.d ./swisslog/Adaptor/Src/adaptor_can.o ./swisslog/Adaptor/Src/adaptor_can.su ./swisslog/Adaptor/Src/adaptor_eeprom.cyclo ./swisslog/Adaptor/Src/adaptor_eeprom.d ./swisslog/Adaptor/Src/adaptor_eeprom.o ./swisslog/Adaptor/Src/adaptor_eeprom.su ./swisslog/Adaptor/Src/adaptor_motor.cyclo ./swisslog/Adaptor/Src/adaptor_motor.d ./swisslog/Adaptor/Src/adaptor_motor.o ./swisslog/Adaptor/Src/adaptor_motor.su ./swisslog/Adaptor/Src/adaptor_mqtt.cyclo ./swisslog/Adaptor/Src/adaptor_mqtt.d ./swisslog/Adaptor/Src/adaptor_mqtt.o ./swisslog/Adaptor/Src/adaptor_mqtt.su ./swisslog/Adaptor/Src/adaptor_ntp.cyclo ./swisslog/Adaptor/Src/adaptor_ntp.d ./swisslog/Adaptor/Src/adaptor_ntp.o ./swisslog/Adaptor/Src/adaptor_ntp.su ./swisslog/Adaptor/Src/adaptor_power.cyclo ./swisslog/Adaptor/Src/adaptor_power.d ./swisslog/Adaptor/Src/adaptor_power.o ./swisslog/Adaptor/Src/adaptor_power.su ./swisslog/Adaptor/Src/adaptor_rfid.cyclo ./swisslog/Adaptor/Src/adaptor_rfid.d ./swisslog/Adaptor/Src/adaptor_rfid.o ./swisslog/Adaptor/Src/adaptor_rfid.su ./swisslog/Adaptor/Src/adaptor_rtc.cyclo ./swisslog/Adaptor/Src/adaptor_rtc.d ./swisslog/Adaptor/Src/adaptor_rtc.o ./swisslog/Adaptor/Src/adaptor_rtc.su ./swisslog/Adaptor/Src/adaptor_tcp.cyclo ./swisslog/Adaptor/Src/adaptor_tcp.d ./swisslog/Adaptor/Src/adaptor_tcp.o ./swisslog/Adaptor/Src/adaptor_tcp.su ./swisslog/Adaptor/Src/adaptor_test.cyclo ./swisslog/Adaptor/Src/adaptor_test.d ./swisslog/Adaptor/Src/adaptor_test.o ./swisslog/Adaptor/Src/adaptor_test.su ./swisslog/Adaptor/Src/adaptor_wifi.cyclo ./swisslog/Adaptor/Src/adaptor_wifi.d ./swisslog/Adaptor/Src/adaptor_wifi.o ./swisslog/Adaptor/Src/adaptor_wifi.su

.PHONY: clean-swisslog-2f-Adaptor-2f-Src

