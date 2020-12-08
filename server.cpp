
#include <stm32f407xx.h>
#include <usbd_core.h>
#include "usbd_desc.h"
#include "usbd_conf.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"


USBD_HandleTypeDef hUsbDeviceHS;

void init_usb_device ()
{
    USBD_Init( &hUsbDeviceHS, &HS_Desc, DEVICE_HS );


	// USBD_RegisterClass( giving it the CDC class code )

	// USBD_CDC_RegisterInterface( provide the device structure and the fops_HS interface )

	// USBD_Start( again giving the usb device structure )
}

