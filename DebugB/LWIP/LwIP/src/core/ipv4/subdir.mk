################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LWIP/LwIP/src/core/ipv4/autoip.c \
../LWIP/LwIP/src/core/ipv4/dhcp.c \
../LWIP/LwIP/src/core/ipv4/etharp.c \
../LWIP/LwIP/src/core/ipv4/icmp.c \
../LWIP/LwIP/src/core/ipv4/igmp.c \
../LWIP/LwIP/src/core/ipv4/ip4.c \
../LWIP/LwIP/src/core/ipv4/ip4_addr.c \
../LWIP/LwIP/src/core/ipv4/ip4_frag.c 

OBJS += \
./LWIP/LwIP/src/core/ipv4/autoip.o \
./LWIP/LwIP/src/core/ipv4/dhcp.o \
./LWIP/LwIP/src/core/ipv4/etharp.o \
./LWIP/LwIP/src/core/ipv4/icmp.o \
./LWIP/LwIP/src/core/ipv4/igmp.o \
./LWIP/LwIP/src/core/ipv4/ip4.o \
./LWIP/LwIP/src/core/ipv4/ip4_addr.o \
./LWIP/LwIP/src/core/ipv4/ip4_frag.o 

C_DEPS += \
./LWIP/LwIP/src/core/ipv4/autoip.d \
./LWIP/LwIP/src/core/ipv4/dhcp.d \
./LWIP/LwIP/src/core/ipv4/etharp.d \
./LWIP/LwIP/src/core/ipv4/icmp.d \
./LWIP/LwIP/src/core/ipv4/igmp.d \
./LWIP/LwIP/src/core/ipv4/ip4.d \
./LWIP/LwIP/src/core/ipv4/ip4_addr.d \
./LWIP/LwIP/src/core/ipv4/ip4_frag.d 


# Each subdirectory must supply rules for building sources it contributes
LWIP/LwIP/src/core/ipv4/%.o LWIP/LwIP/src/core/ipv4/%.su LWIP/LwIP/src/core/ipv4/%.cyclo: ../LWIP/LwIP/src/core/ipv4/%.c LWIP/LwIP/src/core/ipv4/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Peripherals/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Tasks/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Common/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Adaptor/Inc" -I../wolfMQTT -I../Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/ -I../Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/ -I../wolfSSL -I../LWIP/App -I../LWIP/Target -I../LWIP/LwIP/src/include -I../LWIP/LwIP/src/include/lwip -I../LWIP/LwIP/src/include/lwip/apps -I../LWIP/LwIP/src/include/lwip/prot -I../LWIP/LwIP/src/include/lwip/priv -I../LWIP/LwIP/src/include/compat/posix -I../LWIP/LwIP/src/include/compat/posix/arpa -I../LWIP/LwIP/src/include/compat/posix/net -I../LWIP/LwIP/src/include/compat/posix/sys -I../LWIP/LwIP/src/include/compat/stdc -I../LWIP/LwIP/src/include/netif -I../LWIP/LwIP/src/include/netif/ppp -I../LWIP/LwIP/system -I../LWIP/LwIP/system/arch -I../LWIP/LwIP/src/include/netif/ppp/polarssl -I../LWIP/lan8742 -I../swisslog/Robot/Inc -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog" -I../swisslog/Ota/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-LWIP-2f-LwIP-2f-src-2f-core-2f-ipv4

clean-LWIP-2f-LwIP-2f-src-2f-core-2f-ipv4:
	-$(RM) ./LWIP/LwIP/src/core/ipv4/autoip.cyclo ./LWIP/LwIP/src/core/ipv4/autoip.d ./LWIP/LwIP/src/core/ipv4/autoip.o ./LWIP/LwIP/src/core/ipv4/autoip.su ./LWIP/LwIP/src/core/ipv4/dhcp.cyclo ./LWIP/LwIP/src/core/ipv4/dhcp.d ./LWIP/LwIP/src/core/ipv4/dhcp.o ./LWIP/LwIP/src/core/ipv4/dhcp.su ./LWIP/LwIP/src/core/ipv4/etharp.cyclo ./LWIP/LwIP/src/core/ipv4/etharp.d ./LWIP/LwIP/src/core/ipv4/etharp.o ./LWIP/LwIP/src/core/ipv4/etharp.su ./LWIP/LwIP/src/core/ipv4/icmp.cyclo ./LWIP/LwIP/src/core/ipv4/icmp.d ./LWIP/LwIP/src/core/ipv4/icmp.o ./LWIP/LwIP/src/core/ipv4/icmp.su ./LWIP/LwIP/src/core/ipv4/igmp.cyclo ./LWIP/LwIP/src/core/ipv4/igmp.d ./LWIP/LwIP/src/core/ipv4/igmp.o ./LWIP/LwIP/src/core/ipv4/igmp.su ./LWIP/LwIP/src/core/ipv4/ip4.cyclo ./LWIP/LwIP/src/core/ipv4/ip4.d ./LWIP/LwIP/src/core/ipv4/ip4.o ./LWIP/LwIP/src/core/ipv4/ip4.su ./LWIP/LwIP/src/core/ipv4/ip4_addr.cyclo ./LWIP/LwIP/src/core/ipv4/ip4_addr.d ./LWIP/LwIP/src/core/ipv4/ip4_addr.o ./LWIP/LwIP/src/core/ipv4/ip4_addr.su ./LWIP/LwIP/src/core/ipv4/ip4_frag.cyclo ./LWIP/LwIP/src/core/ipv4/ip4_frag.d ./LWIP/LwIP/src/core/ipv4/ip4_frag.o ./LWIP/LwIP/src/core/ipv4/ip4_frag.su

.PHONY: clean-LWIP-2f-LwIP-2f-src-2f-core-2f-ipv4

