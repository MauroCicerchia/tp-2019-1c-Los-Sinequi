################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Lissandra/memtable/memtable.c 

OBJS += \
./Lissandra/memtable/memtable.o 

C_DEPS += \
./Lissandra/memtable/memtable.d 


# Each subdirectory must supply rules for building sources it contributes
Lissandra/memtable/%.o: ../Lissandra/memtable/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2019-1c-Los-Sinequi/sharedLib" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


