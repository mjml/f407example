TARGET=squeezer
HAL=1
DEBUG=1
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
FLASH=st-flash
STPROG=/home/joya/bin/stprog

# Names: 1st used for startup.s files, 2nd for driver directory, 3rd for specific system_stm32fxxxyz.c file and HAL device header file
arch_short=stm32f1xx
ARCH_short=STM32F1xx
arch_specific=stm32f103xb
MCU=-mcpu=cortex-m3 -mthumb -mfloat-abi=soft
DEFS = -DSTM32F103xB
OPT = -O0

ifeq ($(HAL),1)
INCLUDE= -I. -I$(CUBE)/Drivers/CMSIS/Device/ST/$(ARCH_short)/Include -I$(CUBE)/Drivers/CMSIS/Core/Include -I$(CUBE)/Drivers/CMSIS/Include
HALDIR=$(CUBE)/Drivers/$(ARCH_short)_HAL_Driver
HALINCLDIR=$(HALDIR)/Inc
HALSRCDIR=$(HALDIR)/Src
INCLUDE += -I$(HALINCLDIR)
#HALMODULES = cortex tim tim_ex gpio gpio_ex flash flash_ex dac pwr dma rcc rcc_ex exti
LLMODULES = gpio tim rcc utils pwr
ifneq ($(LLMODULES),)
#DEFS += -DUSE_HAL_DRIVER -DUSE_FULL_LL_DRIVER
DEFS += -DUSE_FULL_LL_DRIVER
endif
#HALOBJS = $(arch_short)_hal.o $(patsubst %,$(arch_short)_hal_%.o,$(HALMODULES)) $(patsubst %,$(arch_short)_ll_%.o,$(LLMODULES))
HALOBJS = $(patsubst %,$(arch_short)_hal_%.o,$(HALMODULES)) $(patsubst %,$(arch_short)_ll_%.o,$(LLMODULES))
endif

ifeq ($(DEBUG),1)
DEFS += -DDEBUG
endif


ASFLAGS=$(MCU) -g -Og -c -Wall -fdata-sections -ffunction-sections -fstack-usage
CFLAGS=$(MCU) $(OPT) $(INCLUDE) $(DEFS) --specs=nano.specs
ifeq ($(DEBUG), 1)
CFLAGS += -g3 -gdwarf-2
endif

CPPFLAGS = $(CFLAGS)
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"
CPPFLAGS += -MMD -MP -MF"$(@:%.obj=%.d)"

OBJS=$(CPPSRC:.cpp=.obj) $(CSRC:.c=.o) startup_$(arch_specific).o system_$(arch_short).o $(HALOBJS)
BUILDOBJS=$(patsubst %,$(BUILDDIR)/%,$(OBJS))

LDSCRIPT = STM32F103XB_FLASH.ld
LIBS=-lc -lm
LIBDIR=
LDFLAGS=$(MCU) -specs=nosys.specs -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(TARGET).map,--cref,--gc-sections,--start-group,--end-group -static

all: $(TARGET).bin

clean:
	$(RM) -rf *.d *.o *.obj *.map $(TARGET).elf $(TARGET).hex $(TARGET).bin build/*

prog: $(TARGET).bin
	$(STPROG) -c port=SWD mode=UR -w $(TARGET).bin 0x8000000 -rst

flash: $(TARGET).bin
	$(FLASH) reset
	$(FLASH) --reset write $(TARGET).bin 0x08000000

$(TARGET).hex: $(TARGET).elf
	$(HEX) $< $@

$(TARGET).bin: $(TARGET).elf
	$(BIN) $< $@

$(TARGET).elf $(TARGET).map: $(BUILDOBJS)
	$(CC) $(LDFLAGS) -o $(TARGET).elf $^
	$(SZ) $(TARGET).elf

$(BUILDDIR)/%.d: %.cpp
	;

$(BUILDDIR)/%.d: %.c
	;

$(BUILDDIR)/%.obj: %.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $<

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILDDIR)/%.o: %.s
	$(AS) $(ASFLAGS) -o $@ -c $<

$(BUILDDIR)/%.o: $(HALSRCDIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean all flash prog all

-include $(wildcard $(BUILD_DIR)/*.d)
