################################################################################
# MRS Version: 1.9.2
# �Զ����ɵ��ļ�����Ҫ�༭��
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Debug/debug.c 

C_DEPS += \
./Debug/debug.d 

OBJS += \
./Debug/debug.o 


# Each subdirectory must supply rules for building sources it contributes
Debug/%.o: ../Debug/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"E:\files\2025funny program\jumping_jumping\ch32v307-Whell-Leg\Small_Whell_Leg\Debug" -I"E:\files\2025funny program\jumping_jumping\ch32v307-Whell-Leg\Small_Whell_Leg\Core" -I"E:\files\2025funny program\jumping_jumping\ch32v307-Whell-Leg\Small_Whell_Leg\User" -I"E:\files\2025funny program\jumping_jumping\ch32v307-Whell-Leg\Small_Whell_Leg\Peripheral\inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

