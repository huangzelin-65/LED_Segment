################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LWIP/LwIP/src/netif/ppp/auth.c \
../LWIP/LwIP/src/netif/ppp/ccp.c \
../LWIP/LwIP/src/netif/ppp/chap-md5.c \
../LWIP/LwIP/src/netif/ppp/chap-new.c \
../LWIP/LwIP/src/netif/ppp/chap_ms.c \
../LWIP/LwIP/src/netif/ppp/demand.c \
../LWIP/LwIP/src/netif/ppp/eap.c \
../LWIP/LwIP/src/netif/ppp/ecp.c \
../LWIP/LwIP/src/netif/ppp/eui64.c \
../LWIP/LwIP/src/netif/ppp/fsm.c \
../LWIP/LwIP/src/netif/ppp/ipcp.c \
../LWIP/LwIP/src/netif/ppp/ipv6cp.c \
../LWIP/LwIP/src/netif/ppp/lcp.c \
../LWIP/LwIP/src/netif/ppp/magic.c \
../LWIP/LwIP/src/netif/ppp/mppe.c \
../LWIP/LwIP/src/netif/ppp/multilink.c \
../LWIP/LwIP/src/netif/ppp/ppp.c \
../LWIP/LwIP/src/netif/ppp/pppapi.c \
../LWIP/LwIP/src/netif/ppp/pppcrypt.c \
../LWIP/LwIP/src/netif/ppp/pppoe.c \
../LWIP/LwIP/src/netif/ppp/pppol2tp.c \
../LWIP/LwIP/src/netif/ppp/pppos.c \
../LWIP/LwIP/src/netif/ppp/upap.c \
../LWIP/LwIP/src/netif/ppp/utils.c \
../LWIP/LwIP/src/netif/ppp/vj.c 

OBJS += \
./LWIP/LwIP/src/netif/ppp/auth.o \
./LWIP/LwIP/src/netif/ppp/ccp.o \
./LWIP/LwIP/src/netif/ppp/chap-md5.o \
./LWIP/LwIP/src/netif/ppp/chap-new.o \
./LWIP/LwIP/src/netif/ppp/chap_ms.o \
./LWIP/LwIP/src/netif/ppp/demand.o \
./LWIP/LwIP/src/netif/ppp/eap.o \
./LWIP/LwIP/src/netif/ppp/ecp.o \
./LWIP/LwIP/src/netif/ppp/eui64.o \
./LWIP/LwIP/src/netif/ppp/fsm.o \
./LWIP/LwIP/src/netif/ppp/ipcp.o \
./LWIP/LwIP/src/netif/ppp/ipv6cp.o \
./LWIP/LwIP/src/netif/ppp/lcp.o \
./LWIP/LwIP/src/netif/ppp/magic.o \
./LWIP/LwIP/src/netif/ppp/mppe.o \
./LWIP/LwIP/src/netif/ppp/multilink.o \
./LWIP/LwIP/src/netif/ppp/ppp.o \
./LWIP/LwIP/src/netif/ppp/pppapi.o \
./LWIP/LwIP/src/netif/ppp/pppcrypt.o \
./LWIP/LwIP/src/netif/ppp/pppoe.o \
./LWIP/LwIP/src/netif/ppp/pppol2tp.o \
./LWIP/LwIP/src/netif/ppp/pppos.o \
./LWIP/LwIP/src/netif/ppp/upap.o \
./LWIP/LwIP/src/netif/ppp/utils.o \
./LWIP/LwIP/src/netif/ppp/vj.o 

C_DEPS += \
./LWIP/LwIP/src/netif/ppp/auth.d \
./LWIP/LwIP/src/netif/ppp/ccp.d \
./LWIP/LwIP/src/netif/ppp/chap-md5.d \
./LWIP/LwIP/src/netif/ppp/chap-new.d \
./LWIP/LwIP/src/netif/ppp/chap_ms.d \
./LWIP/LwIP/src/netif/ppp/demand.d \
./LWIP/LwIP/src/netif/ppp/eap.d \
./LWIP/LwIP/src/netif/ppp/ecp.d \
./LWIP/LwIP/src/netif/ppp/eui64.d \
./LWIP/LwIP/src/netif/ppp/fsm.d \
./LWIP/LwIP/src/netif/ppp/ipcp.d \
./LWIP/LwIP/src/netif/ppp/ipv6cp.d \
./LWIP/LwIP/src/netif/ppp/lcp.d \
./LWIP/LwIP/src/netif/ppp/magic.d \
./LWIP/LwIP/src/netif/ppp/mppe.d \
./LWIP/LwIP/src/netif/ppp/multilink.d \
./LWIP/LwIP/src/netif/ppp/ppp.d \
./LWIP/LwIP/src/netif/ppp/pppapi.d \
./LWIP/LwIP/src/netif/ppp/pppcrypt.d \
./LWIP/LwIP/src/netif/ppp/pppoe.d \
./LWIP/LwIP/src/netif/ppp/pppol2tp.d \
./LWIP/LwIP/src/netif/ppp/pppos.d \
./LWIP/LwIP/src/netif/ppp/upap.d \
./LWIP/LwIP/src/netif/ppp/utils.d \
./LWIP/LwIP/src/netif/ppp/vj.d 


# Each subdirectory must supply rules for building sources it contributes
LWIP/LwIP/src/netif/ppp/%.o LWIP/LwIP/src/netif/ppp/%.su LWIP/LwIP/src/netif/ppp/%.cyclo: ../LWIP/LwIP/src/netif/ppp/%.c LWIP/LwIP/src/netif/ppp/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Peripherals/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Tasks/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Common/Inc" -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog/Adaptor/Inc" -I../wolfMQTT -I../Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/ -I../Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/ -I../wolfSSL -I../LWIP/App -I../LWIP/Target -I../LWIP/LwIP/src/include -I../LWIP/LwIP/src/include/lwip -I../LWIP/LwIP/src/include/lwip/apps -I../LWIP/LwIP/src/include/lwip/prot -I../LWIP/LwIP/src/include/lwip/priv -I../LWIP/LwIP/src/include/compat/posix -I../LWIP/LwIP/src/include/compat/posix/arpa -I../LWIP/LwIP/src/include/compat/posix/net -I../LWIP/LwIP/src/include/compat/posix/sys -I../LWIP/LwIP/src/include/compat/stdc -I../LWIP/LwIP/src/include/netif -I../LWIP/LwIP/src/include/netif/ppp -I../LWIP/LwIP/system -I../LWIP/LwIP/system/arch -I../LWIP/LwIP/src/include/netif/ppp/polarssl -I../LWIP/lan8742 -I../swisslog/Robot/Inc -I"C:/Users/e3diany25g/Desktop/VCB/transguard2.0-embedded/swisslog" -I../swisslog/Ota/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-LWIP-2f-LwIP-2f-src-2f-netif-2f-ppp

clean-LWIP-2f-LwIP-2f-src-2f-netif-2f-ppp:
	-$(RM) ./LWIP/LwIP/src/netif/ppp/auth.cyclo ./LWIP/LwIP/src/netif/ppp/auth.d ./LWIP/LwIP/src/netif/ppp/auth.o ./LWIP/LwIP/src/netif/ppp/auth.su ./LWIP/LwIP/src/netif/ppp/ccp.cyclo ./LWIP/LwIP/src/netif/ppp/ccp.d ./LWIP/LwIP/src/netif/ppp/ccp.o ./LWIP/LwIP/src/netif/ppp/ccp.su ./LWIP/LwIP/src/netif/ppp/chap-md5.cyclo ./LWIP/LwIP/src/netif/ppp/chap-md5.d ./LWIP/LwIP/src/netif/ppp/chap-md5.o ./LWIP/LwIP/src/netif/ppp/chap-md5.su ./LWIP/LwIP/src/netif/ppp/chap-new.cyclo ./LWIP/LwIP/src/netif/ppp/chap-new.d ./LWIP/LwIP/src/netif/ppp/chap-new.o ./LWIP/LwIP/src/netif/ppp/chap-new.su ./LWIP/LwIP/src/netif/ppp/chap_ms.cyclo ./LWIP/LwIP/src/netif/ppp/chap_ms.d ./LWIP/LwIP/src/netif/ppp/chap_ms.o ./LWIP/LwIP/src/netif/ppp/chap_ms.su ./LWIP/LwIP/src/netif/ppp/demand.cyclo ./LWIP/LwIP/src/netif/ppp/demand.d ./LWIP/LwIP/src/netif/ppp/demand.o ./LWIP/LwIP/src/netif/ppp/demand.su ./LWIP/LwIP/src/netif/ppp/eap.cyclo ./LWIP/LwIP/src/netif/ppp/eap.d ./LWIP/LwIP/src/netif/ppp/eap.o ./LWIP/LwIP/src/netif/ppp/eap.su ./LWIP/LwIP/src/netif/ppp/ecp.cyclo ./LWIP/LwIP/src/netif/ppp/ecp.d ./LWIP/LwIP/src/netif/ppp/ecp.o ./LWIP/LwIP/src/netif/ppp/ecp.su ./LWIP/LwIP/src/netif/ppp/eui64.cyclo ./LWIP/LwIP/src/netif/ppp/eui64.d ./LWIP/LwIP/src/netif/ppp/eui64.o ./LWIP/LwIP/src/netif/ppp/eui64.su ./LWIP/LwIP/src/netif/ppp/fsm.cyclo ./LWIP/LwIP/src/netif/ppp/fsm.d ./LWIP/LwIP/src/netif/ppp/fsm.o ./LWIP/LwIP/src/netif/ppp/fsm.su ./LWIP/LwIP/src/netif/ppp/ipcp.cyclo ./LWIP/LwIP/src/netif/ppp/ipcp.d ./LWIP/LwIP/src/netif/ppp/ipcp.o ./LWIP/LwIP/src/netif/ppp/ipcp.su ./LWIP/LwIP/src/netif/ppp/ipv6cp.cyclo ./LWIP/LwIP/src/netif/ppp/ipv6cp.d ./LWIP/LwIP/src/netif/ppp/ipv6cp.o ./LWIP/LwIP/src/netif/ppp/ipv6cp.su ./LWIP/LwIP/src/netif/ppp/lcp.cyclo ./LWIP/LwIP/src/netif/ppp/lcp.d ./LWIP/LwIP/src/netif/ppp/lcp.o ./LWIP/LwIP/src/netif/ppp/lcp.su ./LWIP/LwIP/src/netif/ppp/magic.cyclo ./LWIP/LwIP/src/netif/ppp/magic.d ./LWIP/LwIP/src/netif/ppp/magic.o ./LWIP/LwIP/src/netif/ppp/magic.su ./LWIP/LwIP/src/netif/ppp/mppe.cyclo ./LWIP/LwIP/src/netif/ppp/mppe.d ./LWIP/LwIP/src/netif/ppp/mppe.o ./LWIP/LwIP/src/netif/ppp/mppe.su ./LWIP/LwIP/src/netif/ppp/multilink.cyclo ./LWIP/LwIP/src/netif/ppp/multilink.d ./LWIP/LwIP/src/netif/ppp/multilink.o ./LWIP/LwIP/src/netif/ppp/multilink.su ./LWIP/LwIP/src/netif/ppp/ppp.cyclo ./LWIP/LwIP/src/netif/ppp/ppp.d ./LWIP/LwIP/src/netif/ppp/ppp.o ./LWIP/LwIP/src/netif/ppp/ppp.su ./LWIP/LwIP/src/netif/ppp/pppapi.cyclo ./LWIP/LwIP/src/netif/ppp/pppapi.d ./LWIP/LwIP/src/netif/ppp/pppapi.o ./LWIP/LwIP/src/netif/ppp/pppapi.su ./LWIP/LwIP/src/netif/ppp/pppcrypt.cyclo ./LWIP/LwIP/src/netif/ppp/pppcrypt.d ./LWIP/LwIP/src/netif/ppp/pppcrypt.o ./LWIP/LwIP/src/netif/ppp/pppcrypt.su ./LWIP/LwIP/src/netif/ppp/pppoe.cyclo ./LWIP/LwIP/src/netif/ppp/pppoe.d ./LWIP/LwIP/src/netif/ppp/pppoe.o ./LWIP/LwIP/src/netif/ppp/pppoe.su ./LWIP/LwIP/src/netif/ppp/pppol2tp.cyclo ./LWIP/LwIP/src/netif/ppp/pppol2tp.d ./LWIP/LwIP/src/netif/ppp/pppol2tp.o ./LWIP/LwIP/src/netif/ppp/pppol2tp.su ./LWIP/LwIP/src/netif/ppp/pppos.cyclo ./LWIP/LwIP/src/netif/ppp/pppos.d ./LWIP/LwIP/src/netif/ppp/pppos.o ./LWIP/LwIP/src/netif/ppp/pppos.su ./LWIP/LwIP/src/netif/ppp/upap.cyclo ./LWIP/LwIP/src/netif/ppp/upap.d ./LWIP/LwIP/src/netif/ppp/upap.o ./LWIP/LwIP/src/netif/ppp/upap.su ./LWIP/LwIP/src/netif/ppp/utils.cyclo ./LWIP/LwIP/src/netif/ppp/utils.d ./LWIP/LwIP/src/netif/ppp/utils.o ./LWIP/LwIP/src/netif/ppp/utils.su ./LWIP/LwIP/src/netif/ppp/vj.cyclo ./LWIP/LwIP/src/netif/ppp/vj.d ./LWIP/LwIP/src/netif/ppp/vj.o ./LWIP/LwIP/src/netif/ppp/vj.su

.PHONY: clean-LWIP-2f-LwIP-2f-src-2f-netif-2f-ppp

