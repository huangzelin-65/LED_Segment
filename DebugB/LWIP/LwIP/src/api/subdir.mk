################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LWIP/LwIP/src/api/api_lib.c \
../LWIP/LwIP/src/api/api_msg.c \
../LWIP/LwIP/src/api/err.c \
../LWIP/LwIP/src/api/if_api.c \
../LWIP/LwIP/src/api/netbuf.c \
../LWIP/LwIP/src/api/netdb.c \
../LWIP/LwIP/src/api/netifapi.c \
../LWIP/LwIP/src/api/sockets.c \
../LWIP/LwIP/src/api/tcpip.c 

OBJS += \
./LWIP/LwIP/src/api/api_lib.o \
./LWIP/LwIP/src/api/api_msg.o \
./LWIP/LwIP/src/api/err.o \
./LWIP/LwIP/src/api/if_api.o \
./LWIP/LwIP/src/api/netbuf.o \
./LWIP/LwIP/src/api/netdb.o \
./LWIP/LwIP/src/api/netifapi.o \
./LWIP/LwIP/src/api/sockets.o \
./LWIP/LwIP/src/api/tcpip.o 

C_DEPS += \
./LWIP/LwIP/src/api/api_lib.d \
./LWIP/LwIP/src/api/api_msg.d \
./LWIP/LwIP/src/api/err.d \
./LWIP/LwIP/src/api/if_api.d \
./LWIP/LwIP/src/api/netbuf.d \
./LWIP/LwIP/src/api/netdb.d \
./LWIP/LwIP/src/api/netifapi.d \
./LWIP/LwIP/src/api/sockets.d \
./LWIP/LwIP/src/api/tcpip.d 


# Each subdirectory must supply rules for building sources it contributes
LWIP/LwIP/src/api/%.o LWIP/LwIP/src/api/%.su LWIP/LwIP/src/api/%.cyclo: ../LWIP/LwIP/src/api/%.c LWIP/LwIP/src/api/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Peripherals/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Tasks/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Common/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Adaptor/Inc" -I../wolfMQTT -I../Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/ -I../Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/ -I../wolfSSL -I../LWIP/App -I../LWIP/Target -I../LWIP/LwIP/src/include -I../LWIP/LwIP/src/include/lwip -I../LWIP/LwIP/src/include/lwip/apps -I../LWIP/LwIP/src/include/lwip/prot -I../LWIP/LwIP/src/include/lwip/priv -I../LWIP/LwIP/src/include/compat/posix -I../LWIP/LwIP/src/include/compat/posix/arpa -I../LWIP/LwIP/src/include/compat/posix/net -I../LWIP/LwIP/src/include/compat/posix/sys -I../LWIP/LwIP/src/include/compat/stdc -I../LWIP/LwIP/src/include/netif -I../LWIP/LwIP/src/include/netif/ppp -I../LWIP/LwIP/system -I../LWIP/LwIP/system/arch -I../LWIP/LwIP/src/include/netif/ppp/polarssl -I../LWIP/lan8742 -I../swisslog/Robot/Inc -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog" -I../swisslog/Ota/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-LWIP-2f-LwIP-2f-src-2f-api

clean-LWIP-2f-LwIP-2f-src-2f-api:
	-$(RM) ./LWIP/LwIP/src/api/api_lib.cyclo ./LWIP/LwIP/src/api/api_lib.d ./LWIP/LwIP/src/api/api_lib.o ./LWIP/LwIP/src/api/api_lib.su ./LWIP/LwIP/src/api/api_msg.cyclo ./LWIP/LwIP/src/api/api_msg.d ./LWIP/LwIP/src/api/api_msg.o ./LWIP/LwIP/src/api/api_msg.su ./LWIP/LwIP/src/api/err.cyclo ./LWIP/LwIP/src/api/err.d ./LWIP/LwIP/src/api/err.o ./LWIP/LwIP/src/api/err.su ./LWIP/LwIP/src/api/if_api.cyclo ./LWIP/LwIP/src/api/if_api.d ./LWIP/LwIP/src/api/if_api.o ./LWIP/LwIP/src/api/if_api.su ./LWIP/LwIP/src/api/netbuf.cyclo ./LWIP/LwIP/src/api/netbuf.d ./LWIP/LwIP/src/api/netbuf.o ./LWIP/LwIP/src/api/netbuf.su ./LWIP/LwIP/src/api/netdb.cyclo ./LWIP/LwIP/src/api/netdb.d ./LWIP/LwIP/src/api/netdb.o ./LWIP/LwIP/src/api/netdb.su ./LWIP/LwIP/src/api/netifapi.cyclo ./LWIP/LwIP/src/api/netifapi.d ./LWIP/LwIP/src/api/netifapi.o ./LWIP/LwIP/src/api/netifapi.su ./LWIP/LwIP/src/api/sockets.cyclo ./LWIP/LwIP/src/api/sockets.d ./LWIP/LwIP/src/api/sockets.o ./LWIP/LwIP/src/api/sockets.su ./LWIP/LwIP/src/api/tcpip.cyclo ./LWIP/LwIP/src/api/tcpip.d ./LWIP/LwIP/src/api/tcpip.o ./LWIP/LwIP/src/api/tcpip.su

.PHONY: clean-LWIP-2f-LwIP-2f-src-2f-api

