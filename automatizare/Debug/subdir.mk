################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
INO_SRCS += \
../automatizare.ino 

OBJS += \
./automatizare.o 

INO_DEPS += \
./automatizare.d 


# Each subdirectory must supply rules for building sources it contributes
automatizare.o: ../automatizare.ino
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DARDUINO=100 -DUBRRH -I/home/lsuciu/arduino-ide/hardware/arduino/avr/cores/arduino -I/home/lsuciu/arduino/libraries/DallasTemperature -I/home/lsuciu/arduino/libraries/LsuScheduler -I/home/lsuciu/arduino/libraries/OneWire -I/home/lsuciu/arduino/libraries/Time -I/home/lsuciu/arduino/libraries/Timezone -I/home/lsuciu/arduino/libraries/ESP8266WiFi/src -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"automatizare.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


