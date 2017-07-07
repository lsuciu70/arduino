################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
INO_SRCS += \
../LsuHeating.ino \
../LsuHeatingNew.ino 

CPP_SRCS += \
../ProgrammP1.cpp \
../ProgrammP2.cpp \
../ProgrammP3.cpp 

OBJS += \
./LsuHeating.o \
./LsuHeatingNew.o \
./ProgrammP1.o \
./ProgrammP2.o \
./ProgrammP3.o 

INO_DEPS += \
./LsuHeating.d \
./LsuHeatingNew.d 

CPP_DEPS += \
./ProgrammP1.d \
./ProgrammP2.d \
./ProgrammP3.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.ino
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DARDUINO=100 -DUBRRH -I/home/lsuciu/arduino-ide/hardware/arduino/avr/cores/arduino -I/home/lsuciu/arduino/libraries/ESP8266WiFi/src -I/home/lsuciu/arduino/libraries/DallasTemperature -I/home/lsuciu/arduino/libraries/OneWire -I/home/lsuciu/arduino/libraries/Time -I/home/lsuciu/arduino/libraries/Timezone -I/home/lsuciu/arduino/libraries/LsuScheduler -I/home/lsuciu/arduino/libraries/LsuWiFi -I/home/lsuciu/arduino/libraries/LsuNtpTime -I/home/lsuciu/arduino/libraries/LsuLogger -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DARDUINO=100 -DUBRRH -I/home/lsuciu/arduino-ide/hardware/arduino/avr/cores/arduino -I/home/lsuciu/arduino/libraries/ESP8266WiFi/src -I/home/lsuciu/arduino/libraries/DallasTemperature -I/home/lsuciu/arduino/libraries/OneWire -I/home/lsuciu/arduino/libraries/Time -I/home/lsuciu/arduino/libraries/Timezone -I/home/lsuciu/arduino/libraries/LsuScheduler -I/home/lsuciu/arduino/libraries/LsuWiFi -I/home/lsuciu/arduino/libraries/LsuNtpTime -I/home/lsuciu/arduino/libraries/LsuLogger -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


