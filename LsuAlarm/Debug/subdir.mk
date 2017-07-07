################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
INO_SRCS += \
../LsuAlarm.ino 

OBJS += \
./LsuAlarm.o 

INO_DEPS += \
./LsuAlarm.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.ino
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DARDUINO=100 -DUBRRH -I/home/lsuciu/arduino-ide/hardware/arduino/avr/cores/arduino -I/home/lsuciu/arduino/libraries/LsuUtilsLib -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


