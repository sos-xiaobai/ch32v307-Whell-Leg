################################################################################
# MRS Version: 1.9.2
# 自动生成的文件。不要编辑！
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include User/subdir.mk
-include Startup/subdir.mk
-include Peripheral/src/subdir.mk
-include Debug/subdir.mk
-include Core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
Whell_Leg.hex \

SECONDARY_LIST += \
Whell_Leg.lst \

SECONDARY_SIZE += \
Whell_Leg.siz \


# 所有目标
all: Whell_Leg.elf secondary-outputs

# 工具调用
Whell_Leg.elf: $(OBJS) $(USER_OBJS)
	@	@	riscv-none-embed-g++ -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -T Link.ld -nostartfiles -Xlinker --gc-sections -L"../LD" -Wl,-Map,"Whell_Leg.map" --specs=nano.specs --specs=nosys.specs -o "Whell_Leg.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@	@
Whell_Leg.hex: Whell_Leg.elf
	@	riscv-none-embed-objcopy -O ihex "Whell_Leg.elf"  "Whell_Leg.hex"
	@	@
Whell_Leg.lst: Whell_Leg.elf
	@	riscv-none-embed-objdump --all-headers --demangle --disassemble -M xw "Whell_Leg.elf" > "Whell_Leg.lst"
	@	@
Whell_Leg.siz: Whell_Leg.elf
	@	riscv-none-embed-size --format=berkeley "Whell_Leg.elf"
	@	@
# 其他目标
clean:
	-$(RM) $(C_UPPER_DEPS)$(SECONDARY_LIST)$(SECONDARY_SIZE)$(S_DEPS)$(C_DEPS)$(CC_DEPS)$(ASM_UPPER_DEPS)$(C++_DEPS)$(OBJS)$(CXX_DEPS)$(SECONDARY_FLASH)$(ASM_DEPS)$(S_UPPER_DEPS)$(CPP_DEPS) Whell_Leg.elf
	-@
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
