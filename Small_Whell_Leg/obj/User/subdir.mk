################################################################################
# MRS Version: 1.9.2
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../User/mid.cpp \
../User/test.cpp 

C_SRCS += \
../User/ch32v30x_it.c \
../User/main.c \
../User/system_ch32v30x.c 

C_DEPS += \
./User/ch32v30x_it.d \
./User/main.d \
./User/system_ch32v30x.d 

OBJS += \
./User/ch32v30x_it.o \
./User/main.o \
./User/mid.o \
./User/system_ch32v30x.o \
./User/test.o 

CPP_DEPS += \
./User/mid.d \
./User/test.d 


# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"D:\RM\ch32v307-Whell-Leg\Small_Whell_Leg\Debug" -I"D:\RM\ch32v307-Whell-Leg\Small_Whell_Leg\Core" -I"D:\RM\ch32v307-Whell-Leg\Small_Whell_Leg\User" -I"D:\RM\ch32v307-Whell-Leg\Small_Whell_Leg\Peripheral\inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
User/%.o: ../User/%.cpp
	@	@	riscv-none-embed-g++ -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"D:\RM\ch32v307-Whell-Leg\Small_Whell_Leg\Debug" -I"D:\RM\ch32v307-Whell-Leg\Small_Whell_Leg\User" -I"D:\RM\ch32v307-Whell-Leg\Small_Whell_Leg\Peripheral\inc" -I"D:\RM\ch32v307-Whell-Leg\Small_Whell_Leg\Core" -std=gnu++11 -fabi-version=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

