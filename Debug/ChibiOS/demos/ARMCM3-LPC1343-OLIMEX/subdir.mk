################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ChibiOS/demos/ARMCM3-LPC1343-OLIMEX/main.c 

OBJS += \
./ChibiOS/demos/ARMCM3-LPC1343-OLIMEX/main.o 

C_DEPS += \
./ChibiOS/demos/ARMCM3-LPC1343-OLIMEX/main.d 


# Each subdirectory must supply rules for building sources it contributes
ChibiOS/demos/ARMCM3-LPC1343-OLIMEX/%.o: ../ChibiOS/demos/ARMCM3-LPC1343-OLIMEX/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/francois/BMS_PROGProto/bms/ChibiOS" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


