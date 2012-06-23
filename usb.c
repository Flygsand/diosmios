/* 
   This file implements libogc usb API, but with ehci direct call
   
   most of the file credit goes to libogc devs 
*/

#define __usb_control_message(fd, b, c,d, e, f, g, h, i) ehci_control_message(fd,b,c,d,e,f,g)

static  s32 __usb_getdesc(struct ehci_device * fd, u8 *buffer, u8 type, u8 index, u8 size)
{
        //printk("usb_get desc %X %X %p\n",type,index,buffer);
	return __usb_control_message(fd, USB_ENDPOINT_IN ,USB_REQ_GETDESCRIPTOR, (type << 8) | index, 0, size, buffer, NULL, NULL);
}

s32 USB_GetDescriptors(struct ehci_device * fd, usb_devdesc *udd)
{
	u8 *buffer = NULL;
	u8 *ptr = NULL;
	usb_configurationdesc *ucd = NULL;
	usb_interfacedesc *uid = NULL;
	usb_endpointdesc *ued = NULL;
	s32 retval = 0;
	u32 iConf, iInterface, iEndpoint;

	buffer = USB_Alloc(sizeof(*udd));
	if(buffer == NULL)
	{
		retval = -ENOMEM;
		goto free_and_error;
	}

	retval = __usb_getdesc(fd, buffer, USB_DT_DEVICE, 0, USB_DT_DEVICE_SIZE);
	if(retval < 0)
	{
		dbgprintf("USB:__usb_getdesc():%d failed\n", retval );
		goto free_and_error;
	}

	memcpy( udd, buffer, USB_DT_DEVICE_SIZE );
	USB_Free(buffer);

	u32 *_udd = (u32*)udd;

	_udd[1] = ( udd->bLength << 24 ) | ( udd->bDescriptorType << 16 ) | cpu_to_le16(udd->bcdUSB);
	_udd[2] = ( cpu_to_le16(udd->idVendor) << 16 ) | cpu_to_le16(udd->idProduct);
	_udd[3] = ( cpu_to_le16(udd->bcdDevice) << 16 ) | (udd->iManufacturer<<8) | udd->iProduct;
		
	u32 _ptr = (u32)USB_Alloc( udd->bNumConfigurations * sizeof(*udd->configurations) );

	//udd->configurations = USB_Alloc(udd->bNumConfigurations* sizeof(*udd->configurations));
	if( _ptr == 0)
	{
		dbgprintf("USB:USB_Alloc():failed:%u\n", __LINE__ );
		retval = -ENOMEM;
		goto free_and_error;
	}

	_udd[4] = ( udd->iSerialNumber << 24 ) | ( udd->bNumConfigurations << 16 ) | (_ptr>>16);
	_udd[5] = ((_ptr & 0xFFFF) << 16) | (_udd[5]&0xFFFF);

	memset( udd->configurations, 0, udd->bNumConfigurations * sizeof(*udd->configurations) );
	
	for( iConf = 0; iConf < udd->bNumConfigurations; iConf++)
	{
		buffer = USB_Alloc( USB_DT_CONFIG_SIZE );
		if(buffer == NULL)
		{
			retval = -ENOMEM;
			goto free_and_error;
		}

		retval = __usb_getdesc(fd, buffer, USB_DT_CONFIG, iConf, USB_DT_CONFIG_SIZE);
		ucd = &udd->configurations[iConf];
		memcpy( ucd, buffer, USB_DT_CONFIG_SIZE );
		USB_Free( buffer );

		u32 *_ucd = (u32*)ucd;

		_ucd[0] = ( ucd->bLength << 24 ) | ( ucd->bDescriptorType << 16 ) | cpu_to_le16(ucd->wTotalLength);

		//ucd->wTotalLength = cpu_to_le16(ucd->wTotalLength);

		buffer = USB_Alloc( ucd->wTotalLength);
		if(buffer == NULL)
		{
			retval = -ENOMEM;
			goto free_and_error;
		}

		retval = __usb_getdesc(fd, buffer, USB_DT_CONFIG, iConf, ucd->wTotalLength);
		if(retval < 0)
			goto free_and_error;

		ptr = buffer;
		ptr += ucd->bLength;

		retval = -ENOMEM;
		//ucd->interfaces = USB_Alloc(ucd->bNumInterfaces* sizeof(*ucd->interfaces));
		//if(ucd->interfaces == NULL)
		//	goto free_and_error;

		

		u32 _ptrB = (u32)USB_Alloc(ucd->bNumInterfaces* sizeof(*ucd->interfaces));
		if( _ptrB == 0 )
			goto free_and_error;

		_ucd[2] = ( ucd->bMaxPower << 24 ) | (_ptrB>>8);
		_ucd[3] = ((_ptrB & 0xFF) << 24) | (_ucd[3]&0xFFFFFF);
		
		//dbgprintf("ucd->interfaces:%p\n", ucd->interfaces );
		memset( ucd->interfaces, 0, ucd->bNumInterfaces * sizeof(*ucd->interfaces) );

		for(iInterface = 0; iInterface < ucd->bNumInterfaces; iInterface++)
		{
			uid = &ucd->interfaces[iInterface];
			memcpy(uid, ptr, USB_DT_INTERFACE_SIZE);
			ptr += uid->bLength;

			//uid->endpoints = USB_Alloc(uid->bNumEndpoints* sizeof(*uid->endpoints));
			//if(uid->endpoints == NULL)
			//	goto free_and_error;

			u32 *_uid = (u32*)uid;
			u32 _ptrC = (u32)USB_Alloc(uid->bNumEndpoints* sizeof(*uid->endpoints));

			_uid[2] = (uid->iInterface<<24) | (_ptrC >> 8);
			_uid[3] = (_ptrC<<24) | (_uid[3]&0xFFFFFF);
			
			memset( uid->endpoints, 0, uid->bNumEndpoints * sizeof(*uid->endpoints) );

			for( iEndpoint = 0; iEndpoint < uid->bNumEndpoints; iEndpoint++)
			{
				ued = &uid->endpoints[iEndpoint];
				memcpy( ued, ptr, USB_DT_ENDPOINT_SIZE );
				ptr += ued->bLength;

				//ued->wMaxPacketSize = cpu_to_le16(ued->wMaxPacketSize);
				u32 *_ued = (u32*)ued;
				_ued[1] = (cpu_to_le16(ued->wMaxPacketSize) << 16 ) | (_ued[1] & 0xFFFF);
			}

			USB_Free((void*)_ptrC);
		}
		
		USB_Free((void*)_ptrB);
		
		USB_Free( buffer);
		buffer = (u8*)NULL;
	}
	retval = 0;

free_and_error:
	if( _ptr != 0 )
		USB_Free((void*)_ptr);

	if(buffer != NULL)
		USB_Free(buffer);

	if(retval < 0)
		USB_FreeDescriptors(udd);

	return retval;
}

void USB_FreeDescriptors(usb_devdesc *udd)
{
	int iConf, iInterface;
	usb_configurationdesc *ucd;
	usb_interfacedesc *uid;
	if(udd->configurations != NULL)
	{
		for(iConf = 0; iConf < udd->bNumConfigurations; iConf++)
		{
			ucd = &udd->configurations[iConf];
			if(ucd->interfaces != NULL)
			{
				for(iInterface = 0; iInterface < ucd->bNumInterfaces; iInterface++)
				{
					uid = &ucd->interfaces[iInterface];
					if(uid->endpoints != NULL)
						USB_Free(uid->endpoints);
				}
				USB_Free(ucd->interfaces);
			}
		}
		USB_Free(udd->configurations);
	}
}


void USB_SuspendDevice(struct ehci_device *fd)
{
        return ;
}
void USB_ResumeDevice(struct ehci_device *fd)
{
        return ;
}


s32 USB_WriteBlkMsg(struct ehci_device *fd,u8 bEndpoint,u16 wLength,void *rpData)
{
	return ehci_bulk_message(fd,bEndpoint,wLength,rpData);
}

s32 USB_WriteCtrlMsg(struct ehci_device *fd,u8 bmRequestType,u8 bmRequest,u16 wValue,u16 wIndex,u16 wLength,void *rpData)
{
	return __usb_control_message(fd,bmRequestType,bmRequest,wValue,wIndex,wLength,rpData,NULL,NULL);
}

s32 USB_GetConfiguration(struct ehci_device *fd, u8 *configuration)
{
	u8 *_configuration;
	s32 retval;

	_configuration = USB_Alloc(1);
	if(_configuration == NULL)
		return -ENOMEM;

	retval = __usb_control_message(fd, (USB_CTRLTYPE_DIR_DEVICE2HOST | USB_CTRLTYPE_TYPE_STANDARD | USB_CTRLTYPE_REC_DEVICE), USB_REQ_GETCONFIG, 0, 0, 1, _configuration, NULL, NULL);
	if(retval >= 0)
		*configuration = *_configuration;
	USB_Free( _configuration);

	return retval;
}
s32 USB_SetConfiguration(struct ehci_device *fd, u8 configuration)
{
	return __usb_control_message(fd, (USB_CTRLTYPE_DIR_HOST2DEVICE | USB_CTRLTYPE_TYPE_STANDARD | USB_CTRLTYPE_REC_DEVICE), USB_REQ_SETCONFIG, configuration, 0, 0, NULL, NULL, NULL);
}
s32 USB_SetAlternativeInterface(struct ehci_device *fd, u8 interface, u8 alternateSetting)
{
	if(alternateSetting == 0)
		return -EINVAL;
	return __usb_control_message(fd, (USB_CTRLTYPE_DIR_HOST2DEVICE | USB_CTRLTYPE_TYPE_STANDARD | USB_CTRLTYPE_REC_INTERFACE),
                                     USB_REQ_SETINTERFACE, alternateSetting, interface, 0, NULL, NULL, NULL);

}
s32 USB_ClearHalt(struct ehci_device *fd, u8 endpoint)
{
	return __usb_control_message(fd, (USB_CTRLTYPE_DIR_HOST2DEVICE | USB_CTRLTYPE_TYPE_STANDARD | USB_CTRLTYPE_REC_ENDPOINT),
                                     USB_REQ_CLEARFEATURE, USB_FEATURE_ENDPOINT_HALT, endpoint, 0, NULL, NULL, NULL);
}
