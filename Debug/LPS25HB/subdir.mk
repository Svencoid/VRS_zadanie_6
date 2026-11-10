################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LPS25HB/lps25hb.c 

OBJS += \
./LPS25HB/lps25hb.o 

C_DEPS += \
./LPS25HB/lps25hb.d 


# Each subdirectory must supply rules for building sources it contributes
LPS25HB/%.o LPS25HB/%.su LPS25HB/%.cyclo: ../LPS25HB/%.c LPS25HB/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DSTM32F303x8 -DHSE_VALUE=8000000 -DHSE_STARTUP_TIMEOUT=100 -DLSE_STARTUP_TIMEOUT=5000 -DLSE_VALUE=32768 -DEXTERNAL_CLOCK_VALUE=8000000 -DHSI_VALUE=8000000 -DLSI_VALUE=40000 -DVDD_VALUE=3300 -DPREFETCH_ENABLE=1 -c -I../Core/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/sswen/Desktop/VRS/vrs_zadanie_6/HTS221" -I"C:/Users/sswen/Desktop/VRS/vrs_zadanie_6/LPS25HB" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-LPS25HB

clean-LPS25HB:
	-$(RM) ./LPS25HB/lps25hb.cyclo ./LPS25HB/lps25hb.d ./LPS25HB/lps25hb.o ./LPS25HB/lps25hb.su

.PHONY: clean-LPS25HB

