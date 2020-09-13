TARGET=squeezer
HAL=1
CPPSRC=$(shell ls *.cpp)
CSRC=$(shell ls *.c)
CPP=arm-none-eabi-g++
CC=arm-none-eabi-gcc
AS=arm-none-eabi-gcc -x assembler-with-cpp
LD=arm-none-eabi-ld
CP=arm-none-eabi-objcopy
SZ=arm-none-eabi-size
HEX=$(CP) -O ihex
BIN=$(CP) -O binary -S
CUBE=cubeF1
BUILDDIR=build

MCU=-mcpu=cortex-m4 -mthumb
DEFS = -DSTM32F103xB
OPT = -Og

INCLUDE= -I. -I$(CUBE)/Drivers/CMSIS/Device/ST/STM32F1xx/Include -I$(CUBE)/Drivers/CMSIS/Core/Include -I$(CUBE)/Drivers/CMSIS/Include
ifeq ($(HAL),1)
HALDIR=$(CUBE)/Drivers/STM32F1xx_HAL_Driver
HALINCLDIR=$(HALDIR)/Inc
HALSRCDIR=$(HALDIR)/Src
INCLUDE += -I$(HALINCLDIR)
MODS = stm32f1xx_hal \
					stm32f1xx_hal_cortex \
					stm32f1xx_hal_tim \
					stm32f1xx_hal_tim_ex \
					stm32f1xx_hal_gpio \
					stm32f1xx_hal_dac \
	 				stm32f1xx_hal_dma \
	 				stm32f1xx_hal_rcc 
HALOBJS = $(patsubst %,%.o,$(MODS))
endif


ASFLAGS=$(MCU) -c -Wall -fdata-sections -ffunction-sections
CFLAGS=$(MCU) $(OPT) $(INCLUDE) $(DEFS)
ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif
# I don't bother adding the fine-grained dependencies
#CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"
CPPFLAGS=$(CFLAGS)

OBJS=$(CPPSRC:.cpp=.obj) $(CSRC:.c=.o) startup_stm32f103xb.o system_stm32f1xx.o $(HALOBJS)
BUILDOBJS=$(patsubst %,$(BUILDDIR)/%,$(OBJS))

LDSCRIPT = STM32F103XB_FLASH.ld
LIBS=-lc -lm -lnosys
LIBDIR=
LDFLAGS=$(MCU) -specs=nosys.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(TARGET).map,--cref

all: $(TARGET).bin

clean:
	$(RM) -rf *.d *.o *.obj *.map $(TARGET).elf $(TARGET).hex $(TARGET).bin build/*

flash:
	$(FLASH) write $(TARGET).elf 0x8000000

$(TARGET).hex: $(TARGET).elf
	$(HEX) $< $@

$(TARGET).bin: $(TARGET).elf
	$(BIN) $< $@

$(TARGET).elf: $(BUILDOBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	$(SZ) $@

$(BUILDDIR)/%.obj: %.cpp 
	$(CPP) $(CPPFLAGS) -o $@ -c $<

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILDDIR)/%.o: %.s
	$(AS) $(ASFLAGS) -o $@ -c $<

$(BUILDDIR)/%.o: $(HALSRCDIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean all flash

