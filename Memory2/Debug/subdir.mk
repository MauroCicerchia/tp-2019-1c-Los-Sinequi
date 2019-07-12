################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Gossip.c \
../Memory.c \
../Page.c \
../QuerysToFS.c \
../Segment.c 

OBJS += \
./Gossip.o \
./Memory.o \
./Page.o \
./QuerysToFS.o \
./Segment.o 

C_DEPS += \
./Gossip.d \
./Memory.d \
./Page.d \
./QuerysToFS.d \
./Segment.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2019-1c-Los-Sinequi/sharedLib" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


