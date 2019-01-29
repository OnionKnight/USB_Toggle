
#include <ogc/usb.h>
#include <ogc/usbstorage.h>
#include <ogc/ipc.h>
#include <string.h>

#define DEVICELIST_MAXIMUM 8



s32 _devid;
u16 _vid;
u16 _pid;
usbstorage_handle _usbh;
usb_device_entry _devices[DEVICELIST_MAXIMUM];

u8 _maxlun;
u8 _devcount;
s32 ret = -1;
bool usb_initialized;
bool usb_mounted;

inline bool USB_Init(void)
{
	/* Initialize USB and Initialize USB Storage */
		ret = USB_Initialize();
			if(ret< 0)	{	return false;	}
		ret = USBStorage_Initialize();
			if(ret< 0)	{	return false;	}
	
	
	usb_initialized = true;
		
	return true;
}

inline u8 Interrogate_Devices(usb_device_entry *buffer)
{

	ret = USB_GetDeviceList(_devices, DEVICELIST_MAXIMUM, 0x08/*USB MASS STORAGE*/, &_devcount);
	if(ret < 0) {return false;}
	
	buffer = _devices;
	return _devcount;

}

inline bool Mount_USB(s32 dev)
{
	_devid = _devices[dev].device_id;
	_vid = _devices[dev].vid;
	_pid = _devices[dev].pid;


	if(!usb_initialized){	return false;}
	usb_mounted = false;
	
	ret = USBStorage_Open(&_usbh, _devid,_vid,_pid);
	if(ret < 0) {	return false;	}
	
	ret = USBStorage_GetMaxLUN(&_usbh);
	if(ret < 0) {	return false;	}
	
	_maxlun = _usbh.max_lun;
	
	ret = USBStorage_MountLUN(&_usbh,0);
	if(ret < 0) {	return false;	}
	
	usb_mounted = true;	
	
	return true;
}



inline bool Get_Capacity(u32 *sector_size, u32 *sectors)
{	
	if(!usb_initialized){return false;}
	if(!usb_mounted){return false;}
	
	ret = USBStorage_ReadCapacity(&_usbh,0,sector_size,sectors);
	if(ret < 0){ return false; }
	
	return true;

}
inline bool Read_Sectors(u32 sector, u16 count, u8 *buffer)
{
	if(!usb_initialized){return false;}
	if(!usb_mounted){return false;}
	
	ret = USBStorage_Read(&_usbh,0,sector,count,buffer);
	if(ret < 0)	{return false;}
	
	return true;
}
inline bool Write_Sectors(u32 sector, u16 count, u8 *buffer)
{
	if(!usb_initialized){	return false;}
	if(!usb_mounted){return false;}
	
	ret = USBStorage_Write(&_usbh,0,sector,count,buffer);
	if(ret < 0)	{return false;}
	
	return true;
}
inline bool Close_Storage()
{
	if(!usb_initialized){	return false;}
	if(!usb_mounted){return false;}
	
	ret = USBStorage_Close(&_usbh);
	if(ret < 0) {return false;}
	
	return true;
}