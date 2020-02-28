################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cqueue.c \
../src/ftl.c \
../src/main.c \
../src/nand.c \
../src/nil.c 

OBJS += \
./src/cqueue.o \
./src/ftl.o \
./src/main.o \
./src/nand.o \
./src/nil.o 

C_DEPS += \
./src/cqueue.d \
./src/ftl.d \
./src/main.d \
./src/nand.d \
./src/nil.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


