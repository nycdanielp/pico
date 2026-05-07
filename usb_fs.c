#include "usb_fs.h"
#include "tusb.h"
#include "ff.h"
#include "diskio.h"
#include "tcp_server.h"

// FatFs object for the USB drive
FATFS usb_fatfs;
static volatile bool disk_busy = false;
volatile bool usb_is_mounted = false;

void usb_fs_init(void) {
    // Initialize TinyUSB Host
    tuh_init(0);
}

// ----------------------------------------------------------------------
// TinyUSB MSC Callbacks
// ----------------------------------------------------------------------
static scsi_inquiry_resp_t inquiry_resp;

bool inquiry_complete_cb(uint8_t dev_addr, tuh_msc_complete_data_t const * cb_data) {
    if (cb_data->csw->status != 0) {
        tcp_printf("USB Drive inquiry failed\r\n");
        return false;
    }

    uint32_t block_count = tuh_msc_get_block_count(dev_addr, cb_data->cbw->lun);
    uint32_t block_size = tuh_msc_get_block_size(dev_addr, cb_data->cbw->lun);
    tcp_printf("USB Drive: %.8s %.16s (Size: %lu MB)\r\n", 
               inquiry_resp.vendor_id, inquiry_resp.product_id, 
               block_count / ((1024*1024)/block_size));

    // Mount the drive using FatFs (Drive 0)
    if (f_mount(&usb_fatfs, "0:", 1) != FR_OK) {
        tcp_printf("Failed to mount FAT32 file system.\r\n");
    } else {
        usb_is_mounted = true;
        tcp_printf("USB File System Mounted Successfully!\r\n");
    }

    return true;
}

void tuh_msc_mount_cb(uint8_t dev_addr) {
    tcp_printf("\r\n[USB] Mass Storage Device plugged in!\r\n");
    // Send inquiry to get drive info, then mount
    tuh_msc_inquiry(dev_addr, 0, &inquiry_resp, inquiry_complete_cb, 0);
}

void tuh_msc_umount_cb(uint8_t dev_addr) {
    tcp_printf("\r\n[USB] Mass Storage Device removed!\r\n");
    f_unmount("0:");
    usb_is_mounted = false;
}

// ----------------------------------------------------------------------
// FatFs DiskIO Implementation (Maps FatFs to TinyUSB MSC)
// ----------------------------------------------------------------------
static void wait_for_disk_io(void) {
    while(disk_busy) {
        tuh_task();
    }
}

static bool disk_io_complete(uint8_t dev_addr, tuh_msc_complete_data_t const * cb_data) {
    (void) dev_addr; (void) cb_data;
    disk_busy = false;
    return true;
}

DSTATUS disk_status(BYTE pdrv) {
    uint8_t dev_addr = pdrv + 1;
    return tuh_msc_mounted(dev_addr) ? 0 : STA_NODISK;
}

DSTATUS disk_initialize(BYTE pdrv) {
    return 0; // Handled by tuh_init
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
    uint8_t dev_addr = pdrv + 1;
    disk_busy = true;
    tuh_msc_read10(dev_addr, 0, buff, sector, (uint16_t)count, disk_io_complete, 0);
    wait_for_disk_io();
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
    uint8_t dev_addr = pdrv + 1;
    disk_busy = true;
    tuh_msc_write10(dev_addr, 0, buff, sector, (uint16_t)count, disk_io_complete, 0);
    wait_for_disk_io();
    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    uint8_t dev_addr = pdrv + 1;
    switch (cmd) {
        case CTRL_SYNC: return RES_OK;
        case GET_SECTOR_COUNT:
            *((DWORD*)buff) = (WORD)tuh_msc_get_block_count(dev_addr, 0);
            return RES_OK;
        case GET_SECTOR_SIZE:
            *((WORD*)buff) = (WORD)tuh_msc_get_block_size(dev_addr, 0);
            return RES_OK;
        case GET_BLOCK_SIZE:
            *((DWORD*)buff) = 1;
            return RES_OK;
        default:
            return RES_PARERR;
    }
}
