TARGET=usbexample
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

BUILDDIR=build
FLASH=st-flash
STPROG=/home/joya/bin/stprog

# Names: 1st used for startup.s files, 2nd for driver directory, 3rd for specific system_stm32fxxxyz.c file and HAL device header file
arch_short=stm32f4xx
ARCH_short=STM32F4xx
arch_specific=stm32f407xx
MCU=-mcpu=cortex-m3 -mthumb -mfloat-abi=soft
DEFS = -DSTM32 -DSTM32F407xx -DUSE_FULL_LL_DRIVER -DSWO_DEBUG
OPT = -O0

EXT_PATH = $(shell pwd)/ext
EXT_SRC = $(EXT_PATH)
EXT_INCL = $(EXT_PATH)
EXT_MODULES = stm32f4xx_hal_msp.c stm32f4xx_it.c syscalls.c sysmem.c system_stm32f4xx.c
EXT_OBJECTS = $(addprefix $(BUILDDIR)/,$(notdir $(EXT_MODULES:.c=.o)))

CUBE_PATH=$(EXT_PATH)/cubef4

CMSIS_ARM_PATH = $(CUBE_PATH)/Drivers/CMSIS
CMSIS_ARM_INCL = $(CMSIS_ARM_PATH)/Include

CMSIS_DEVICE_PATH = $(CMSIS_ARM_PATH)/Device/ST/$(ARCH_short)
CMSIS_DEVICE_SRC = $(CMSIS_DEVICE_PATH)/Source/Templates
CMSIS_DEVICE_INCL = $(CMSIS_DEVICE_PATH)/Include
CMSIS_DEVICE_MODULES = system_stm32f4xx.c gcc/startup_stm32f407xx.s
CMSIS_DEVICE_SOURCES = $(addprefix $(CMSIS_DEVICE_PATH)/,$(CMSIS_DEVICE_MODULES))
CMSIS_DEVICE_OBJECTS = $(addprefix $(BUILDDIR)/cmsis/,$(addsuffix .o,$(notdir $(basename $(CMSIS_DEVICE_SOURCES)))))

HAL_PATH = $(CUBE_PATH)/Drivers/$(ARCH_short)_HAL_Driver
HAL_SRC = $(HAL_PATH)/Src
HAL_INCL = $(HAL_PATH)/Inc
HAL_MODULES = gpio.c rcc.c flash.c tim.c cortex.c pcd.c pcd_ex.c
HAL_SOURCES = $(addprefix $(HAL_SRC)/$(arch_short)_hal_,$(HAL_MODULES)) $(HAL_SRC)/$(arch_short)_hal.c
HAL_OBJECTS = $(addprefix $(BUILDDIR)/hal/,$(notdir $(HAL_SOURCES:.c=.o)))

LL_MODULES = gpio.c tim.c utils.c usb.c
LL_SOURCES = $(addprefix $(HAL_SRC)/$(arch_short)_ll_,$(LL_MODULES))
LL_OBJECTS = $(addprefix $(BUILDDIR)/hal/,$(notdir $(LL_SOURCES:.c=.o)))

USB_PATH = $(CUBE_PATH)/Middlewares/ST/STM32_USB_Device_Library
USB_SRC = $(USB_PATH)/Core/Src
USB_INCL = $(USB_PATH)/Core/Inc
USB_MODULES = usbd_core.c usbd_ctlreq.c usbd_ioreq.c
USB_SOURCES = $(addprefix $(USB_SRC)/,$(USB_MODULES))
USB_OBJECTS = $(addprefix $(BUILDDIR)/usb/,$(notdir $(USB_SOURCES:.c=.o))) 

USBCDC_PATH = $(USB_PATH)/Class/CDC
USBCDC_SRC = $(USBCDC_PATH)/Src
USBCDC_INCL = $(USBCDC_PATH)/Inc
USBCDC_MODULES = usbd_cdc.c
USBCDC_SOURCES = $(addprefix $(USBCDC_SRC)/,$(USBCDC_MODULES))
USBCDC_OBJECTS = $(addprefix $(BUILDDIR)/usbcdc/,$(notdir $(USBCDC_MODULES:.c=.o))) 

USBIMPL_PATH = $(EXT_PATH)/usbimpl
USBIMPL_SRC = $(USBIMPL_PATH)
USBIMPL_INCL = $(USBIMPL_PATH)
USBIMPL_MODULES = usbd_conf.c usbd_desc.c usbd_cdc_if.c
USBIMPL_SOURCES = $(addprefix $(USBIMPL_PATH)/,$(USBCDC_MODULES))
USBIMPL_OBJECTS = $(addprefix $(BUILDDIR)/usbimpl/,$(USBIMPL_MODULES:.c=.o)) 

ASFLAGS=$(MCU) -g -Og -c -Wall -fdata-sections -ffunction-sections -fstack-usage
CFLAGS=$(MCU) $(OPT) $(INCLUDE) $(DEFS) --specs=nano.specs
ifeq ($(DEBUG), 1)
CFLAGS += -g
endif

CFLAGS+=-I$(shell pwd) -I$(EXT_INCL) -I$(CMSIS_ARM_INCL) -I$(CMSIS_DEVICE_INCL) -I$(HAL_INCL) -I$(USB_INCL) -I$(USBCDC_INCL) -I$(USBIMPL_INCL)
CPPFLAGS=-std=c++11 $(CFLAGS) 

APP_OBJECTS=$(patsubst %,$(BUILDDIR)/%,$(CPPSRC:.cpp=.obj) $(CSRC:.c=.o) startup_$(arch_specific).o)

BUILDOBJS=$(APP_OBJECTS) $(HAL_OBJECTS) $(LL_OBJECTS) $(USB_OBJECTS) $(USBCDC_OBJECTS) $(USBIMPL_OBJECTS)

LDSCRIPT = STM32F103XB_FLASH.ld
LIBS=-lc -lm
LIBDIR=
LDFLAGS=$(MCU) -specs=nosys.specs -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(TARGET).map,--cref,--gc-sections,--start-group,--end-group -static

all: $(BUILDDIR)/$(TARGET).bin

clean:
	$(RM) -rf *.d *.o *.obj *.map $(TARGET).elf $(TARGET).hex $(TARGET).bin build/* swout.txt
	touch swout.txt

prog: $(BUILDDIR)/$(TARGET).bin
	$(STPROG) -c port=SWD mode=UR -w  $(BUILDDIR)/$(TARGET).bin 0x8000000 -rst

flash:  $(BUILDDIR)/$(TARGET).bin
	$(FLASH) reset
	$(FLASH) --reset write $(BUILDDIR)/$(TARGET).bin 0x08000000

$(BUILDDIR)/$(TARGET).hex: $(BUILDDIR)/$(TARGET).elf
	@echo -n -e "[\033[1;32m$@\033[0m] "
	$(HEX) $< $@

$(BUILDDIR)/$(TARGET).bin: $(BUILDDIR)/$(TARGET).elf
	@echo -n -e "[\033[1;32m$@\033[0m] "
	$(BIN) $< $@

$(BUILDDIR)/$(TARGET).elf $(TARGET).map: $(BUILDOBJS)
	@echo -n -e "[\033[1;32m$@\033[0m] "
	$(CC) $(LDFLAGS) -o $@ $^
	$(SZ) $(BUILDDIR)/$(TARGET).elf

$(BUILDDIR)/%.d: %.cpp $(BUILDDIR)
	$(CPP) $(CPPFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -c $< -o $@

$(BUILDDIR)/%.d: %.c $(BUILDDIR)
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -c $< -o $@

$(BUILDDIR)/%.obj: %.cpp $(BUILDDIR)
	@echo -n -e "[\033[1;32m$@\033[0m] "
	$(CPP) $(CPPFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.c $(BUILDDIR)
	@echo -n -e "[\033[1;32m$@\033[0m] "
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.s $(BUILDDIR)
	@echo -n -e "[\033[1;32m$@\033[0m] "
	$(AS) $(ASFLAGS) -c $< -o $@

$(BUILDDIR)/cmsis/%.o: $(CMSIS_SRC)/%.c $(BUILDDIR)/cmsis
	@echo -n -e "[\033[1;32m$@\033[0m] "
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/hal/%.o: $(HAL_SRC)/%.c $(BUILDDIR)/hal
	@echo -n -e "[\033[1;32m$@\033[0m] "
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/usb/%.o: $(USB_SRC)/%.c $(BUILDDIR)/usb
	@echo -n -e "[\033[1;32m$@\033[0m] "
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/usbcdc/%.o: $(USBCDC_SRC)/%.c $(BUILDDIR)/usbcdc
	@echo -n -e "[\033[1;32m$@\033[0m] "
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/usbimpl/%.o: $(USBIMPL_SRC)/%.c $(BUILDDIR)/usbimpl
	@echo -n -e "[\033[1;32m$@\033[0m] "
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDOBJS): | $(BUILDDIR) $(BUILDDIR)/hal $(BUILDDIR)/cmsis $(BUILDDIR)/usb $(BUILDDIR)/usbcdc $(BUILDDIR)/usbimpl

$(BUILDDIR) $(BUILDDIR)/hal $(BUILDDIR)/cmsis $(BUILDDIR)/usb $(BUILDDIR)/usbcdc $(BUILDDIR)/usbimpl:
	@echo -n -e "[\033[0;32m$@\033[0m] "
	mkdir -p $(BUILDDIR) $(BUILDDIR)/hal $(BUILDDIR)/cmsis $(BUILDDIR)/usb $(BUILDDIR)/usbcdc $(BUILDDIR)/usbimpl


.PHONY: clean all flash prog all

-include $(wildcard $(BUILD_DIR)/*.d)
