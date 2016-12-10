################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
INO_SRCS += \
../lcd_20x4.ino 

OBJS += \
./lcd_20x4.o 

INO_DEPS += \
./lcd_20x4.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.ino
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DARDUINO -I/home/lsuciu/arduino-ide/hardware/arduino/avr/libraries/Wire/src -I/home/lsuciu/arduino/libraries/LiquidCrystal_I2C -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


