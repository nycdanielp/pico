#ifndef USB_FS_H
#define USB_FS_H

#include <stdbool.h>

void usb_fs_init(void);

extern volatile bool usb_is_mounted;

#endif
