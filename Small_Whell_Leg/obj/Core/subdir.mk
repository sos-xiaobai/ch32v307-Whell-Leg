################################################################################
# MRS Version: 1.9.2
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/core_riscv.c 

C_DEPS += \
./Core/core_riscv.d 

OBJS += \
./Core/core_riscv.o 


# Each subdirectory must supply rules for building sources it contributes
Core/%.o: ../Core/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"E:\files\2025funny program\jumping_jumping\ch32v307-Whell-Leg\Small_Whell_Leg\Debug" -I"E:\files\2025funny program\jumping_jumping\ch32v307-Whell-Leg\Small_Whell_Leg\Core" -I"E:\files\2025funny program\jumping_jumping\ch32v307-Whell-Leg\Small_Whell_Leg\User" -I"E:\files\2025funny program\jumping_jumping\ch32v307-Whell-Leg\Small_Whell_Leg\Peripheral\inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

