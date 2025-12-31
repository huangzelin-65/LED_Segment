################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_client.c \
../Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_packet.c \
../Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_socket.c 

OBJS += \
./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_client.o \
./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_packet.o \
./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_socket.o 

C_DEPS += \
./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_client.d \
./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_packet.d \
./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_socket.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/%.o Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/%.su Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/%.cyclo: ../Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/%.c Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Peripherals/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Tasks/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Common/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Adaptor/Inc" -I../wolfMQTT -I../Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/ -I../Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/ -I../wolfSSL -I../LWIP/App -I../LWIP/Target -I../LWIP/LwIP/src/include -I../LWIP/LwIP/src/include/lwip -I../LWIP/LwIP/src/include/lwip/apps -I../LWIP/LwIP/src/include/lwip/prot -I../LWIP/LwIP/src/include/lwip/priv -I../LWIP/LwIP/src/include/compat/posix -I../LWIP/LwIP/src/include/compat/posix/arpa -I../LWIP/LwIP/src/include/compat/posix/net -I../LWIP/LwIP/src/include/compat/posix/sys -I../LWIP/LwIP/src/include/compat/stdc -I../LWIP/LwIP/src/include/netif -I../LWIP/LwIP/src/include/netif/ppp -I../LWIP/LwIP/system -I../LWIP/LwIP/system/arch -I../LWIP/LwIP/src/include/netif/ppp/polarssl -I../LWIP/lan8742 -I../swisslog/Robot/Inc -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog" -I../swisslog/Ota/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-Third_Party-2f-wolfSSL_wolfMQTT_wolfMQTT-2f-wolfmqtt-2f-src

clean-Middlewares-2f-Third_Party-2f-wolfSSL_wolfMQTT_wolfMQTT-2f-wolfmqtt-2f-src:
	-$(RM) ./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_client.cyclo ./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_client.d ./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_client.o ./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_client.su ./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_packet.cyclo ./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_packet.d ./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_packet.o ./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_packet.su ./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_socket.cyclo ./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_socket.d ./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_socket.o ./Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/mqtt_socket.su

.PHONY: clean-Middlewares-2f-Third_Party-2f-wolfSSL_wolfMQTT_wolfMQTT-2f-wolfmqtt-2f-src

