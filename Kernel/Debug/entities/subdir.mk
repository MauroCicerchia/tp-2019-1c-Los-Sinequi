################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../entities/Memory.c \
../entities/Operation.c \
../entities/Process.c \
../entities/QueryExec.c \
../entities/Table.c 

OBJS += \
./entities/Memory.o \
./entities/Operation.o \
./entities/Process.o \
./entities/QueryExec.o \
./entities/Table.o 

C_DEPS += \
./entities/Memory.d \
./entities/Operation.d \
./entities/Process.d \
./entities/QueryExec.d \
./entities/Table.d 


# Each subdirectory must supply rules for building sources it contributes
entities/%.o: ../entities/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2019-1c-Los-Sinequi/sharedLib" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


