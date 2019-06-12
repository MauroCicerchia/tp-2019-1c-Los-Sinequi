################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../sharedLib/client.c \
../sharedLib/consistency.c \
../sharedLib/console.c \
../sharedLib/packaging.c \
../sharedLib/query.c \
../sharedLib/server.c 

OBJS += \
./sharedLib/client.o \
./sharedLib/consistency.o \
./sharedLib/console.o \
./sharedLib/packaging.o \
./sharedLib/query.o \
./sharedLib/server.o 

C_DEPS += \
./sharedLib/client.d \
./sharedLib/consistency.d \
./sharedLib/console.d \
./sharedLib/packaging.d \
./sharedLib/query.d \
./sharedLib/server.d 


# Each subdirectory must supply rules for building sources it contributes
sharedLib/%.o: ../sharedLib/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


