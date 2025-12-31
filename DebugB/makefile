################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include swisslog/Tasks/Src/subdir.mk
-include swisslog/Robot/Src/subdir.mk
-include swisslog/Peripherals/Src/subdir.mk
-include swisslog/Ota/Src/subdir.mk
-include swisslog/Common/Src/subdir.mk
-include swisslog/Adaptor/Src/subdir.mk
-include Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/wolfcrypt/src/port/st/subdir.mk
-include Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/wolfcrypt/src/port/atmel/subdir.mk
-include Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/wolfcrypt/src/port/arm/subdir.mk
-include Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/wolfcrypt/src/subdir.mk
-include Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/src/subdir.mk
-include Middlewares/Third_Party/wolfSSL_wolfMQTT_wolfMQTT/wolfmqtt/src/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/subdir.mk
-include LWIP/lan8742/subdir.mk
-include LWIP/Target/subdir.mk
-include LWIP/LwIP/system/OS/subdir.mk
-include LWIP/LwIP/src/netif/ppp/polarssl/subdir.mk
-include LWIP/LwIP/src/netif/ppp/subdir.mk
-include LWIP/LwIP/src/netif/subdir.mk
-include LWIP/LwIP/src/core/ipv6/subdir.mk
-include LWIP/LwIP/src/core/ipv4/subdir.mk
-include LWIP/LwIP/src/core/subdir.mk
-include LWIP/LwIP/src/apps/sntp/subdir.mk
-include LWIP/LwIP/src/apps/mqtt/subdir.mk
-include LWIP/LwIP/src/api/subdir.mk
-include LWIP/App/subdir.mk
-include Drivers/STM32H5xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := VCB2.1
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
VCB2.1.elf \

MAP_FILES += \
VCB2.1.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
VCB2.1.list \


# All Target
all:
	+@$(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: VCB2.1.elf secondary-outputs

# Tool invocations
VCB2.1.elf VCB2.1.map: $(OBJS) $(USER_OBJS) C:\Users\e3diany25g\Desktop\VCB\transguard2.0-embedded\STM32H563IIKX_FLASHB.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "VCB2.1.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m33 -T"C:\Users\e3diany25g\Desktop\VCB\transguard2.0-embedded\STM32H563IIKX_FLASHB.ld" --specs=nosys.specs -Wl,-Map="VCB2.1.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

VCB2.1.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "VCB2.1.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) VCB2.1.elf VCB2.1.list VCB2.1.map default.size.stdout
	-@echo ' '

post-build:
	arm-none-eabi-objcopy -O binary "VCB2.1.elf" "VCB2.1B.bin"
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified post-build

-include ../makefile.targets
