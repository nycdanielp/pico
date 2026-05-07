#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------
#define CFG_TUSB_RHPORT0_MODE       OPT_MODE_HOST
#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS                 OPT_OS_NONE
#endif

//--------------------------------------------------------------------
// HOST CONFIGURATION
//--------------------------------------------------------------------
#define CFG_TUH_ENUMERATION_BUFSIZE 256
#define CFG_TUH_HUB                 0
#define CFG_TUH_MSC                 1
#define CFG_TUH_HID                 0
#define CFG_TUH_CDC                 0

// Max devices supported
#define CFG_TUH_DEVICE_MAX          1

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_CONFIG_H_ */
