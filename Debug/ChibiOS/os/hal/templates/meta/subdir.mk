################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ChibiOS/os/hal/templates/meta/driver.c \
../ChibiOS/os/hal/templates/meta/driver_lld.c 

OBJS += \
./ChibiOS/os/hal/templates/meta/driver.o \
./ChibiOS/os/hal/templates/meta/driver_lld.o 

C_DEPS += \
./ChibiOS/os/hal/templates/meta/driver.d \
./ChibiOS/os/hal/templates/meta/driver_lld.d 


# Each subdirectory must supply rules for building sources it contributes
ChibiOS/os/hal/templates/meta/%.o: ../ChibiOS/os/hal/templates/meta/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/francois/BMS_PROGProto/bms/ChibiOS" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


