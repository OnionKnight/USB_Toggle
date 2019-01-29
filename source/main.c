#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogc/ipc.h>
#include <ogc/lwp_watchdog.h>
#include <unistd.h>
#include <ogc/usbstorage.h>
#include "USBI.h"

 
 
void initwii(void) {
 
static void *xfb = NULL;
static GXRModeObj *rmode = NULL;
 
	// Initialise the video system
	VIDEO_Init();
 
	// This function initialises the attached controllers
	WPAD_Init();
 
	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = VIDEO_GetPreferredMode(NULL);
 
	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
 
	// Initialise the console, required for printf
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
 
	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);
 
	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);
 
	// Make the display visible
	VIDEO_SetBlack(FALSE);
 
	// Flush the video register changes to the hardware
	VIDEO_Flush();
 
	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
}





bool hide()
{

	while(1) {
 
		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();
 
		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed = WPAD_ButtonsDown(0);
 
		// We return to the launcher application via exit
		if ( pressed & WPAD_BUTTON_A )
		{
			return true;
		}
		else if(pressed & WPAD_BUTTON_B)
		{
			return false;
		}
		else if(pressed & !WPAD_BUTTON_A  || pressed & !WPAD_BUTTON_B)
		{
			printf("Input Not Recognized\n");
		}
 
		// Wait for the next frame
		VIDEO_WaitVSync();
	}

return false;
}
void decide_to_hide(u8 *mbr)
{
	bool ret = false;
	
	if(mbr[511] == 0xAA)
	{
		printf("currently not hidden, Press A to hide, B to cancel...\n");
		printf("device...");
		ret = hide();
		if(ret)
		{
			mbr[511]++;
			Write_Sectors(0,1,mbr);
			printf("hidden\n");
			return;
		}
		else printf("cancelled\n"); return;
	}
	if(mbr[511] == 0xAB)
	{
		printf("Currently hidden, Press A to unhide, B to cancel\n");
		printf("Device...");
		ret = hide();
		if(ret)
		{
			mbr[511]--;
			Write_Sectors(0,1,mbr);
			printf("unhidden\n");
			return;
		}
		else printf("cancelled\n"); return;

	}
	if(mbr[511] != 0xAA || mbr[511] != 0xAB)
	{
		printf("Device not compatible currently with USB Toggle\n");
		return;
	}

}

/* Main */


int main(int argc, char **argv) {

	initwii();
	
	void __exception_setreload(int t); 
         __exception_setreload(1);     
 
	// The console understands VT terminal escape codes
	// This positions the cursor on row 2, column 0
	// we can use variables for this with format codes too
	// e.g. printf ("\x1b[%d;%dH", row, column );
	printf("\x1b[2;0H");
 
	printf("USB Toggle 2.0\n");
	
	
	/* Initialize Variables */
	bool ret;
	u8 devcount;
	u32 sector_size;
	u32 sectors;
	u8 *mbr;
	u8 retry = 10;
	
	usb_device_entry devices[8];

	
	
	
	/* Initialize USB */
	
	ret = USB_Init();
	if(!ret){printf("Failed to Initialize!\n"); exit(0);}
	
	while(retry)
	{
		devcount = Interrogate_Devices(devices);
		retry--;
		if(devcount > 0) break;
		
		
	}
	
	if(!devcount){ printf("No USB Mass Storage Devices found!"); exit(0);}
	
	printf("Devices Found: %d\n", devcount);

	int i;
	for(i=0;i<devcount;i++)
	{
		ret = Mount_USB(i);
			if(!ret) {printf("Mount Failed\n"); exit(0);}
		ret = Get_Capacity(&sector_size,&sectors);
			if(!ret) {printf("Get Capacity Failed\n"); exit(0);}
		mbr = calloc(sector_size, sizeof(u8));
		ret = Read_Sectors(0,1,mbr);
			if(!ret){ printf("Failed to read MBR\n"); exit(0);}
		printf("Device %d\nVID:%d \t PID:%d\n",i+1, devices[i].vid, devices[i].pid);
		decide_to_hide(mbr);
		free(mbr);
		ret = Close_Storage();
			if(!ret){ printf("Device %d Failed to Close Properly\n", i+1); exit(0);}
	}
	return 0;

#if 1
{
        void USBStorage_Deinitialize();
        printf("Doing IOSReload.\n");

        __io_usbstorage.shutdown();
        USBStorage_Deinitialize();
               
        usleep(500*1000);
        WPAD_Shutdown();
        USB_Deinitialize();
        usleep(50*1000);
        IOS_ReloadIOS(58);
        sleep(1);
        WPAD_Init();

		__io_usbstorage.startup();
        usleep(500*1000);
}
#endif
}