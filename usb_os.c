#include <string.h>
#include "ehci_types.h"
#include "usb.h"
#include "ehci.h"
#include "alloc.h"

void *VirtualToPhysical( void *address )
{
	if( ((u32)address & 0xFFFF0000) == 0xFFFF0000 )
		return (void*)address;

	return (void*)((u32)address & 0x7FFFFFFF);
}
void sync_after_write( void *a, u32 v )
{
	dc_flushrange( a, v );
}
void sync_before_read( void *a, u32 v )
{
	dc_invalidaterange( a, v );
}


int usb_os_init(void)
{
	return 0;
}
void *ehci_maligned(int size,int alignement,int crossing)
{
	return (void*)malloca( size, alignement );
}
dma_addr_t ehci_virt_to_dma(void *a)
{
	return (dma_addr_t)VirtualToPhysical(a);
}
dma_addr_t ehci_dma_map_to(void *buf,size_t len)
{
    sync_after_write(buf, len);
    return (dma_addr_t)VirtualToPhysical(buf);
}
dma_addr_t ehci_dma_map_from(void *buf,size_t len)
{
    sync_after_write(buf, len);
    return (dma_addr_t)VirtualToPhysical(buf);
}
dma_addr_t ehci_dma_map_bidir(void *buf,size_t len)
{ 
    sync_after_write(buf, len);
    return (dma_addr_t)VirtualToPhysical(buf); 
}
void ehci_dma_unmap_to(dma_addr_t buf,size_t len)
{
	sync_before_read((void*)buf, len);
}
void ehci_dma_unmap_from(dma_addr_t buf,size_t len)
{
	sync_before_read((void*)buf, len);
}
void ehci_dma_unmap_bidir(dma_addr_t buf,size_t len)
{
	sync_before_read((void*)buf, len);
}
void *USB_Alloc(int size)
{
	//u32 val;
	//__asm("mov	%0,lr": "=r" (val) );
	//dbgprintf("USB_Alloc(%u) LR:%08x\n", size, val );

	//void *ptr = malloc(size);
	//memset( ptr, 0, size );
	//return ptr;

	return malloc(size);
}
void USB_Free(void *ptr)
{
	return free(ptr);
}
