
#include <stm32f407xx.h>
#include <usbd_core.h>
#include "usbd_desc.h"
#include "usbd_conf.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"


USBD_HandleTypeDef hUsbDeviceHS;

void init_usb_device ()
{
    if (USBD_Init( &hUsbDeviceHS, &HS_Desc, DEVICE_HS ) != USBD_OK) {
		printf("USBD_Init was unsuccessful.\n");
		return;
	}

	if (USBD_RegisterClass( &hUsbDeviceHS, &USBD_CDC ) != USBD_OK) {
		printf("USBD_RegisterClass was unsuccessful.\n");
		return;
	}

	if (USBD_CDC_RegisterInterface( &hUsbDeviceHS, &USBD_Interface_fops_HS ) != USBD_OK) {
		printf("USBD_CDC_RegisterInterface was unsuccessful.\n");
		return;
	}

	if (USBD_Start( &hUsbDeviceHS ) != USBD_OK) {
		printf("USBD_Start was unsuccessful.\n");
		return;
	}
	
	printf("USBD initialization [OK]\n");
}

