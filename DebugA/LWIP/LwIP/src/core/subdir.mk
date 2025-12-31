################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LWIP/LwIP/src/core/altcp.c \
../LWIP/LwIP/src/core/altcp_alloc.c \
../LWIP/LwIP/src/core/altcp_tcp.c \
../LWIP/LwIP/src/core/def.c \
../LWIP/LwIP/src/core/dns.c \
../LWIP/LwIP/src/core/inet_chksum.c \
../LWIP/LwIP/src/core/init.c \
../LWIP/LwIP/src/core/ip.c \
../LWIP/LwIP/src/core/mem.c \
../LWIP/LwIP/src/core/memp.c \
../LWIP/LwIP/src/core/netif.c \
../LWIP/LwIP/src/core/pbuf.c \
../LWIP/LwIP/src/core/raw.c \
../LWIP/LwIP/src/core/stats.c \
../LWIP/LwIP/src/core/sys.c \
../LWIP/LwIP/src/core/tcp.c \
../LWIP/LwIP/src/core/tcp_in.c \
../LWIP/LwIP/src/core/tcp_out.c \
../LWIP/LwIP/src/core/timeouts.c \
../LWIP/LwIP/src/core/udp.c 

OBJS += \
./LWIP/LwIP/src/core/altcp.o \
./LWIP/LwIP/src/core/altcp_alloc.o \
./LWIP/LwIP/src/core/altcp_tcp.o \
./LWIP/LwIP/src/core/def.o \
./LWIP/LwIP/src/core/dns.o \
./LWIP/LwIP/src/core/inet_chksum.o \
./LWIP/LwIP/src/core/init.o \
./LWIP/LwIP/src/core/ip.o \
./LWIP/LwIP/src/core/mem.o \
./LWIP/LwIP/src/core/memp.o \
./LWIP/LwIP/src/core/netif.o \
./LWIP/LwIP/src/core/pbuf.o \
./LWIP/LwIP/src/core/raw.o \
./LWIP/LwIP/src/core/stats.o \
./LWIP/LwIP/src/core/sys.o \
./LWIP/LwIP/src/core/tcp.o \
./LWIP/LwIP/src/core/tcp_in.o \
./LWIP/LwIP/src/core/tcp_out.o \
./LWIP/LwIP/src/core/timeouts.o \
./LWIP/LwIP/src/core/udp.o 

C_DEPS += \
./LWIP/LwIP/src/core/altcp.d \
./LWIP/LwIP/src/core/altcp_alloc.d \
./LWIP/LwIP/src/core/altcp_tcp.d \
./LWIP/LwIP/src/core/def.d \
./LWIP/LwIP/src/core/dns.d \
./LWIP/LwIP/src/core/inet_chksum.d \
./LWIP/LwIP/src/core/init.d \
./LWIP/LwIP/src/core/ip.d \
./LWIP/LwIP/src/core/mem.d \
./LWIP/LwIP/src/core/memp.d \
./LWIP/LwIP/src/core/netif.d \
./LWIP/LwIP/src/core/pbuf.d \
./LWIP/LwIP/src/core/raw.d \
./LWIP/LwIP/src/core/stats.d \
./LWIP/LwIP/src/core/sys.d \
./LWIP/LwIP/src/core/tcp.d \
./LWIP/LwIP/src/core/tcp_in.d \
./LWIP/LwIP/src/core/tcp_out.d \
./LWIP/LwIP/src/core/timeouts.d \
./LWIP/LwIP/src/core/udp.d 


# Each subdirectory must supply rules for building sources it contributes
LWIP/LwIP/src/core/%.o LWIP/LwIP/src/core/%.su LWIP/LwIP/src/core/%.cyclo: ../LWIP/LwIP/src/core/%.c LWIP/LwIP/src/core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Peripherals/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Tasks/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Common/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Adaptor/Inc" -I../wolfMQTT -I../Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/ -I../Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/ -I../wolfSSL -I../LWIP/App -I../LWIP/Target -I../LWIP/LwIP/src/include -I../LWIP/LwIP/src/include/lwip -I../LWIP/LwIP/src/include/lwip/apps -I../LWIP/LwIP/src/include/lwip/prot -I../LWIP/LwIP/src/include/lwip/priv -I../LWIP/LwIP/src/include/compat/posix -I../LWIP/LwIP/src/include/compat/posix/arpa -I../LWIP/LwIP/src/include/compat/posix/net -I../LWIP/LwIP/src/include/compat/posix/sys -I../LWIP/LwIP/src/include/compat/stdc -I../LWIP/LwIP/src/include/netif -I../LWIP/LwIP/src/include/netif/ppp -I../LWIP/LwIP/system -I../LWIP/LwIP/system/arch -I../LWIP/LwIP/src/include/netif/ppp/polarssl -I../LWIP/lan8742 -I../swisslog/Robot/Inc -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog" -I../swisslog/Ota/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-LWIP-2f-LwIP-2f-src-2f-core

clean-LWIP-2f-LwIP-2f-src-2f-core:
	-$(RM) ./LWIP/LwIP/src/core/altcp.cyclo ./LWIP/LwIP/src/core/altcp.d ./LWIP/LwIP/src/core/altcp.o ./LWIP/LwIP/src/core/altcp.su ./LWIP/LwIP/src/core/altcp_alloc.cyclo ./LWIP/LwIP/src/core/altcp_alloc.d ./LWIP/LwIP/src/core/altcp_alloc.o ./LWIP/LwIP/src/core/altcp_alloc.su ./LWIP/LwIP/src/core/altcp_tcp.cyclo ./LWIP/LwIP/src/core/altcp_tcp.d ./LWIP/LwIP/src/core/altcp_tcp.o ./LWIP/LwIP/src/core/altcp_tcp.su ./LWIP/LwIP/src/core/def.cyclo ./LWIP/LwIP/src/core/def.d ./LWIP/LwIP/src/core/def.o ./LWIP/LwIP/src/core/def.su ./LWIP/LwIP/src/core/dns.cyclo ./LWIP/LwIP/src/core/dns.d ./LWIP/LwIP/src/core/dns.o ./LWIP/LwIP/src/core/dns.su ./LWIP/LwIP/src/core/inet_chksum.cyclo ./LWIP/LwIP/src/core/inet_chksum.d ./LWIP/LwIP/src/core/inet_chksum.o ./LWIP/LwIP/src/core/inet_chksum.su ./LWIP/LwIP/src/core/init.cyclo ./LWIP/LwIP/src/core/init.d ./LWIP/LwIP/src/core/init.o ./LWIP/LwIP/src/core/init.su ./LWIP/LwIP/src/core/ip.cyclo ./LWIP/LwIP/src/core/ip.d ./LWIP/LwIP/src/core/ip.o ./LWIP/LwIP/src/core/ip.su ./LWIP/LwIP/src/core/mem.cyclo ./LWIP/LwIP/src/core/mem.d ./LWIP/LwIP/src/core/mem.o ./LWIP/LwIP/src/core/mem.su ./LWIP/LwIP/src/core/memp.cyclo ./LWIP/LwIP/src/core/memp.d ./LWIP/LwIP/src/core/memp.o ./LWIP/LwIP/src/core/memp.su ./LWIP/LwIP/src/core/netif.cyclo ./LWIP/LwIP/src/core/netif.d ./LWIP/LwIP/src/core/netif.o ./LWIP/LwIP/src/core/netif.su ./LWIP/LwIP/src/core/pbuf.cyclo ./LWIP/LwIP/src/core/pbuf.d ./LWIP/LwIP/src/core/pbuf.o ./LWIP/LwIP/src/core/pbuf.su ./LWIP/LwIP/src/core/raw.cyclo ./LWIP/LwIP/src/core/raw.d ./LWIP/LwIP/src/core/raw.o ./LWIP/LwIP/src/core/raw.su ./LWIP/LwIP/src/core/stats.cyclo ./LWIP/LwIP/src/core/stats.d ./LWIP/LwIP/src/core/stats.o ./LWIP/LwIP/src/core/stats.su ./LWIP/LwIP/src/core/sys.cyclo ./LWIP/LwIP/src/core/sys.d ./LWIP/LwIP/src/core/sys.o ./LWIP/LwIP/src/core/sys.su ./LWIP/LwIP/src/core/tcp.cyclo ./LWIP/LwIP/src/core/tcp.d ./LWIP/LwIP/src/core/tcp.o ./LWIP/LwIP/src/core/tcp.su ./LWIP/LwIP/src/core/tcp_in.cyclo ./LWIP/LwIP/src/core/tcp_in.d ./LWIP/LwIP/src/core/tcp_in.o ./LWIP/LwIP/src/core/tcp_in.su ./LWIP/LwIP/src/core/tcp_out.cyclo ./LWIP/LwIP/src/core/tcp_out.d ./LWIP/LwIP/src/core/tcp_out.o ./LWIP/LwIP/src/core/tcp_out.su ./LWIP/LwIP/src/core/timeouts.cyclo ./LWIP/LwIP/src/core/timeouts.d ./LWIP/LwIP/src/core/timeouts.o ./LWIP/LwIP/src/core/timeouts.su ./LWIP/LwIP/src/core/udp.cyclo ./LWIP/LwIP/src/core/udp.d ./LWIP/LwIP/src/core/udp.o ./LWIP/LwIP/src/core/udp.su

.PHONY: clean-LWIP-2f-LwIP-2f-src-2f-core

