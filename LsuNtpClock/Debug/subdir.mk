################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
INO_SRCS += \
../LsuNtpClock.ino 

OBJS += \
./LsuNtpClock.o 

INO_DEPS += \
./LsuNtpClock.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.ino
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DARDUINO=100 -DUBRRH -I/home/lsuciu/arduino-ide/hardware/arduino/avr/cores/arduino -I/home/lsuciu/arduino/libraries/ESP8266WiFi/src -I/home/lsuciu/arduino/libraries/LsuNtpTime -I/home/lsuciu/arduino/libraries/NewliquidCrystal -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


