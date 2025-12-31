################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../swisslog/Peripherals/Src/BoxRfidReader.c \
../swisslog/Peripherals/Src/CAN_Car.c \
../swisslog/Peripherals/Src/DwinHMI.c \
../swisslog/Peripherals/Src/Encoder.c \
../swisslog/Peripherals/Src/NumDisplay.c \
../swisslog/Peripherals/Src/UV_Clean.c \
../swisslog/Peripherals/Src/eeprom_M24C64_i2c.c \
../swisslog/Peripherals/Src/motor_LD25B60G.c \
../swisslog/Peripherals/Src/sensors.c 

OBJS += \
./swisslog/Peripherals/Src/BoxRfidReader.o \
./swisslog/Peripherals/Src/CAN_Car.o \
./swisslog/Peripherals/Src/DwinHMI.o \
./swisslog/Peripherals/Src/Encoder.o \
./swisslog/Peripherals/Src/NumDisplay.o \
./swisslog/Peripherals/Src/UV_Clean.o \
./swisslog/Peripherals/Src/eeprom_M24C64_i2c.o \
./swisslog/Peripherals/Src/motor_LD25B60G.o \
./swisslog/Peripherals/Src/sensors.o 

C_DEPS += \
./swisslog/Peripherals/Src/BoxRfidReader.d \
./swisslog/Peripherals/Src/CAN_Car.d \
./swisslog/Peripherals/Src/DwinHMI.d \
./swisslog/Peripherals/Src/Encoder.d \
./swisslog/Peripherals/Src/NumDisplay.d \
./swisslog/Peripherals/Src/UV_Clean.d \
./swisslog/Peripherals/Src/eeprom_M24C64_i2c.d \
./swisslog/Peripherals/Src/motor_LD25B60G.d \
./swisslog/Peripherals/Src/sensors.d 


# Each subdirectory must supply rules for building sources it contributes
swisslog/Peripherals/Src/%.o swisslog/Peripherals/Src/%.su swisslog/Peripherals/Src/%.cyclo: ../swisslog/Peripherals/Src/%.c swisslog/Peripherals/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Peripherals/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Tasks/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Common/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Adaptor/Inc" -I../wolfMQTT -I../Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/ -I../Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/ -I../wolfSSL -I../LWIP/App -I../LWIP/Target -I../LWIP/LwIP/src/include -I../LWIP/LwIP/src/include/lwip -I../LWIP/LwIP/src/include/lwip/apps -I../LWIP/LwIP/src/include/lwip/prot -I../LWIP/LwIP/src/include/lwip/priv -I../LWIP/LwIP/src/include/compat/posix -I../LWIP/LwIP/src/include/compat/posix/arpa -I../LWIP/LwIP/src/include/compat/posix/net -I../LWIP/LwIP/src/include/compat/posix/sys -I../LWIP/LwIP/src/include/compat/stdc -I../LWIP/LwIP/src/include/netif -I../LWIP/LwIP/src/include/netif/ppp -I../LWIP/LwIP/system -I../LWIP/LwIP/system/arch -I../LWIP/LwIP/src/include/netif/ppp/polarssl -I../LWIP/lan8742 -I../swisslog/Robot/Inc -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog" -I../swisslog/Ota/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-swisslog-2f-Peripherals-2f-Src

clean-swisslog-2f-Peripherals-2f-Src:
	-$(RM) ./swisslog/Peripherals/Src/BoxRfidReader.cyclo ./swisslog/Peripherals/Src/BoxRfidReader.d ./swisslog/Peripherals/Src/BoxRfidReader.o ./swisslog/Peripherals/Src/BoxRfidReader.su ./swisslog/Peripherals/Src/CAN_Car.cyclo ./swisslog/Peripherals/Src/CAN_Car.d ./swisslog/Peripherals/Src/CAN_Car.o ./swisslog/Peripherals/Src/CAN_Car.su ./swisslog/Peripherals/Src/DwinHMI.cyclo ./swisslog/Peripherals/Src/DwinHMI.d ./swisslog/Peripherals/Src/DwinHMI.o ./swisslog/Peripherals/Src/DwinHMI.su ./swisslog/Peripherals/Src/Encoder.cyclo ./swisslog/Peripherals/Src/Encoder.d ./swisslog/Peripherals/Src/Encoder.o ./swisslog/Peripherals/Src/Encoder.su ./swisslog/Peripherals/Src/NumDisplay.cyclo ./swisslog/Peripherals/Src/NumDisplay.d ./swisslog/Peripherals/Src/NumDisplay.o ./swisslog/Peripherals/Src/NumDisplay.su ./swisslog/Peripherals/Src/UV_Clean.cyclo ./swisslog/Peripherals/Src/UV_Clean.d ./swisslog/Peripherals/Src/UV_Clean.o ./swisslog/Peripherals/Src/UV_Clean.su ./swisslog/Peripherals/Src/eeprom_M24C64_i2c.cyclo ./swisslog/Peripherals/Src/eeprom_M24C64_i2c.d ./swisslog/Peripherals/Src/eeprom_M24C64_i2c.o ./swisslog/Peripherals/Src/eeprom_M24C64_i2c.su ./swisslog/Peripherals/Src/motor_LD25B60G.cyclo ./swisslog/Peripherals/Src/motor_LD25B60G.d ./swisslog/Peripherals/Src/motor_LD25B60G.o ./swisslog/Peripherals/Src/motor_LD25B60G.su ./swisslog/Peripherals/Src/sensors.cyclo ./swisslog/Peripherals/Src/sensors.d ./swisslog/Peripherals/Src/sensors.o ./swisslog/Peripherals/Src/sensors.su

.PHONY: clean-swisslog-2f-Peripherals-2f-Src

