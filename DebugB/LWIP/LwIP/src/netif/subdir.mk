################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LWIP/LwIP/src/netif/bridgeif.c \
../LWIP/LwIP/src/netif/bridgeif_fdb.c \
../LWIP/LwIP/src/netif/ethernet.c \
../LWIP/LwIP/src/netif/lowpan6.c \
../LWIP/LwIP/src/netif/lowpan6_ble.c \
../LWIP/LwIP/src/netif/lowpan6_common.c \
../LWIP/LwIP/src/netif/slipif.c \
../LWIP/LwIP/src/netif/zepif.c 

OBJS += \
./LWIP/LwIP/src/netif/bridgeif.o \
./LWIP/LwIP/src/netif/bridgeif_fdb.o \
./LWIP/LwIP/src/netif/ethernet.o \
./LWIP/LwIP/src/netif/lowpan6.o \
./LWIP/LwIP/src/netif/lowpan6_ble.o \
./LWIP/LwIP/src/netif/lowpan6_common.o \
./LWIP/LwIP/src/netif/slipif.o \
./LWIP/LwIP/src/netif/zepif.o 

C_DEPS += \
./LWIP/LwIP/src/netif/bridgeif.d \
./LWIP/LwIP/src/netif/bridgeif_fdb.d \
./LWIP/LwIP/src/netif/ethernet.d \
./LWIP/LwIP/src/netif/lowpan6.d \
./LWIP/LwIP/src/netif/lowpan6_ble.d \
./LWIP/LwIP/src/netif/lowpan6_common.d \
./LWIP/LwIP/src/netif/slipif.d \
./LWIP/LwIP/src/netif/zepif.d 


# Each subdirectory must supply rules for building sources it contributes
LWIP/LwIP/src/netif/%.o LWIP/LwIP/src/netif/%.su LWIP/LwIP/src/netif/%.cyclo: ../LWIP/LwIP/src/netif/%.c LWIP/LwIP/src/netif/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Peripherals/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Tasks/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Common/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Adaptor/Inc" -I../wolfMQTT -I../Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/ -I../Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/ -I../wolfSSL -I../LWIP/App -I../LWIP/Target -I../LWIP/LwIP/src/include -I../LWIP/LwIP/src/include/lwip -I../LWIP/LwIP/src/include/lwip/apps -I../LWIP/LwIP/src/include/lwip/prot -I../LWIP/LwIP/src/include/lwip/priv -I../LWIP/LwIP/src/include/compat/posix -I../LWIP/LwIP/src/include/compat/posix/arpa -I../LWIP/LwIP/src/include/compat/posix/net -I../LWIP/LwIP/src/include/compat/posix/sys -I../LWIP/LwIP/src/include/compat/stdc -I../LWIP/LwIP/src/include/netif -I../LWIP/LwIP/src/include/netif/ppp -I../LWIP/LwIP/system -I../LWIP/LwIP/system/arch -I../LWIP/LwIP/src/include/netif/ppp/polarssl -I../LWIP/lan8742 -I../swisslog/Robot/Inc -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog" -I../swisslog/Ota/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-LWIP-2f-LwIP-2f-src-2f-netif

clean-LWIP-2f-LwIP-2f-src-2f-netif:
	-$(RM) ./LWIP/LwIP/src/netif/bridgeif.cyclo ./LWIP/LwIP/src/netif/bridgeif.d ./LWIP/LwIP/src/netif/bridgeif.o ./LWIP/LwIP/src/netif/bridgeif.su ./LWIP/LwIP/src/netif/bridgeif_fdb.cyclo ./LWIP/LwIP/src/netif/bridgeif_fdb.d ./LWIP/LwIP/src/netif/bridgeif_fdb.o ./LWIP/LwIP/src/netif/bridgeif_fdb.su ./LWIP/LwIP/src/netif/ethernet.cyclo ./LWIP/LwIP/src/netif/ethernet.d ./LWIP/LwIP/src/netif/ethernet.o ./LWIP/LwIP/src/netif/ethernet.su ./LWIP/LwIP/src/netif/lowpan6.cyclo ./LWIP/LwIP/src/netif/lowpan6.d ./LWIP/LwIP/src/netif/lowpan6.o ./LWIP/LwIP/src/netif/lowpan6.su ./LWIP/LwIP/src/netif/lowpan6_ble.cyclo ./LWIP/LwIP/src/netif/lowpan6_ble.d ./LWIP/LwIP/src/netif/lowpan6_ble.o ./LWIP/LwIP/src/netif/lowpan6_ble.su ./LWIP/LwIP/src/netif/lowpan6_common.cyclo ./LWIP/LwIP/src/netif/lowpan6_common.d ./LWIP/LwIP/src/netif/lowpan6_common.o ./LWIP/LwIP/src/netif/lowpan6_common.su ./LWIP/LwIP/src/netif/slipif.cyclo ./LWIP/LwIP/src/netif/slipif.d ./LWIP/LwIP/src/netif/slipif.o ./LWIP/LwIP/src/netif/slipif.su ./LWIP/LwIP/src/netif/zepif.cyclo ./LWIP/LwIP/src/netif/zepif.d ./LWIP/LwIP/src/netif/zepif.o ./LWIP/LwIP/src/netif/zepif.su

.PHONY: clean-LWIP-2f-LwIP-2f-src-2f-netif

