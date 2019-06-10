################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Fs/bitarray.c \
../Fs/blocks.c \
../Fs/fs.c 

OBJS += \
./Fs/bitarray.o \
./Fs/blocks.o \
./Fs/fs.o 

C_DEPS += \
./Fs/bitarray.d \
./Fs/blocks.d \
./Fs/fs.d 


# Each subdirectory must supply rules for building sources it contributes
Fs/%.o: ../Fs/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2019-1c-Los-Sinequi/sharedLib" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


