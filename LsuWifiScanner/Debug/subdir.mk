################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
INO_SRCS += \
../LsuWifiScanner.ino 

OBJS += \
./LsuWifiScanner.o 

INO_DEPS += \
./LsuWifiScanner.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.ino
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DUBRRH -DARDUINO=100 -I/home/lsuciu/arduino-ide/hardware/arduino/avr/cores/arduino -I/home/lsuciu/arduino/libraries/ESP8266WiFi/src -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


