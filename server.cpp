
#include <stm32f407xx.h>
#include <usbd_core.h>
#include "usbd_desc.h"
#include "usbd_conf.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"


USBD_HandleTypeDef hUsbDeviceFS;

void init_usb_device ()
{
	
    if (USBD_Init( &hUsbDeviceFS, &FS_Desc, DEVICE_FS ) != USBD_OK) {
		printf("USBD_Init                       [\e[1;31mFAILED\e[0m]\n");
		return;
	} else {
		printf("USBD_Init                       [\e[1;32mOK\e[0m]\n");
	}

	if (USBD_RegisterClass( &hUsbDeviceFS, &USBD_CDC ) != USBD_OK) {
		printf("USBD_RegisterClass              [\e[1;31mFAILED\e[0m]\n");
		return;
	} else {
		printf("USBD_RegisterClass              [\e[1;32mOK\e[0m]\n");
	}

	if (USBD_CDC_RegisterInterface( &hUsbDeviceFS, &USBD_Interface_fops_FS ) != USBD_OK) {
		printf("USBD_CDC_RegisterInterface      [\e[1;31mFAILED\e[0m]\n");
		return;
	} else {
		printf("USBD_CDC_RegisterInterface      [\e[1;32mOK\e[0m]\n");
	}

	if (USBD_Start( &hUsbDeviceFS ) != USBD_OK) {
		printf("USBD_Start                      [\e[1;31mFAILED\e[0m]\n");
		return;
	} else {
		printf("USBD_Start                      [\e[1;32mOK\e[0m]\n");
	}

	(*USBD_Interface_fops_FS.Init)();
	
	printf("USBD Initialization Complete.\n");
}

