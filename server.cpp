
#include <stm32f407xx.h>
#include <usbd_core.h>
#include "usbd_desc.h"
#include "usbd_conf.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"


USBD_HandleTypeDef hUsbDeviceHS;

void init_usb_device ()
{
	// Pitfall: Consider changing this to full speed. The USB Clock can only go at 48mbit
    USBD_Init( &hUsbDeviceHS, &HS_Desc, DEVICE_HS );

	USBD_RegisterClass( &hUsbDeviceHS, &USBD_CDC );

	USBD_CDC_RegisterInterface( &hUsbDeviceHS, &USBD_Interface_fops_HS );

	USBD_Start( &hUsbDeviceHS );
}

