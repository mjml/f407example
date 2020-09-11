TARGET=squeezer
CPPSRC=$(shell ls *.cpp)
CSRC=$(shell ls *.c)
OBJS=$(CPPSRC:.cpp=.obj) $(CSRC:.c=.o) startup_stm32f103xb.o system_stm32f1xx.o
CPP=arm-none-eabi-g++
CC=arm-none-eabi-gcc
AS=arm-none-eabi-gcc -x assembler-with-cpp
LD=arm-none-eabi-ld
CP=arm-none-eabi-objcopy
SZ=arm-none-eabi-size
HEX=$(CP) -O ihex
BIN=$(CP) -O binary -S
CUBE=cubeF1

MCU=-mcpu=cortex-m4 -mthumb

INCLUDE= -I$(CUBE)/Drivers/CMSIS/Device/ST/STM32F1xx/Include -I$(CUBE)/Drivers/CMSIS/Core/Include -I$(CUBE)/Drivers/CMSIS/Include
DEFS = -DSTM32F103xB

ASFLAGS=$(MCU) -c -Wall -fdata-sections -ffunction-sections
CFLAGS=$(MCU) $(INCLUDE) $(DEFS)
CPPFLAGS=$(MCU) $(INCLUDE) $(DEFS)
ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif
CFLAGS += -Og -MMD -MP -MF"$(@:%.o=%.d)"

LDSCRIPT = STM32F103XB_FLASH.ld
LIBS=-lc -lm -lnosys
LIBDIR=
LDFLAGS=$(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(TARGET).map,--cref -Wl,--gc-sections

all: $(TARGET).hex

clean:
	$(RM) -rf *.d *.o *.obj *.map $(TARGET).elf $(TARGET).hex

$(TARGET).hex: $(TARGET).elf
	$(HEX) $< $@

$(TARGET).bin: $(TARGET).elf
	$(BIN) $< $@

$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $<
	$(SZ) $@

%.obj: %.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $<

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.s
	$(AS) $(ASFLAGS) -o $@ -c $<



