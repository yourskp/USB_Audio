/*******************************************************************************
*  File Name: iAP2_Configuration.c
*
*  Version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
*  Description: Contains user configuration from the iAP component customizer
*               wizard. These configurations would be passed to library object 
*               code.
*
********************************************************************************
* Copyright 2009-2010, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
********************************************************************************/
#include <MyDevice.h>

/* constant accessory information data block */
uint8 iAP2_TransportProtocol                     = (iAP2_IAP_TRANSPORT_PROTOCOL == 1) ? iAP2_IAP_USB: iAP2_IAP_UART;
CYCODE uint8 iAP2_PowerMode                      = iAP2_POWER_MODE;
CYCODE uint8 iAP2_AuthenticationSupported        = iAP2_AUTHENTICATION_SUPPORTED;
CYCODE uint8 iAP2_SlaveAddress                   = (0x11 ^ (iAP2_CP_PROTOCOL & 0x01));
CYCODE uint8 iAP2_biPodPowered                   = iAP2_SELF_OR_IPOD_POWERED;
CYCODE uint8 iAP1_bAccessoryRFCertification      = (iAP1_RF_CLASS5 << iAP1_RF_CLASS5_BIT) | (iAP1_RF_CLASS4 << iAP1_RF_CLASS4_BIT) 
													| (iAP1_RF_CLASS2 << iAP1_RF_CLASS2_BIT) | (iAP1_RF_CLASS1);
CYIDATA uint16 iAP1_LingoBitmask                 = ((uint16) iAP1_GENERAL_LINGO_BITMASK |
                                                  (iAP1_DISPLAY_REMOTE_LINGO_SUPPORT ? iAP1_DISPLAY_REMOTE_LINGO_BITMASK : 0) |
                                                  (iAP1_SIMPLE_REMOTE_LINGO_SUPPORT ? iAP1_SIMPLE_REMOTE_LINGO_BITMASK : 0));

CYCODE char iAP2_AccessoryName[]                  = iAP2_ACC_NAME;
CYCODE char iAP2_Manufacturer[]                   = iAP2_MANUFACTURER_NAME;
CYCODE char iAP2_ModelNumner[]                    = iAP2_MODEL_NUMBER;
CYCODE char iAP2_ProtocolString[][32]             = {iAP2_EA_PROTOCOL_STRING};
CYCODE uint8 iAP2_bProtocolIndex                  = iAP2_EA_PROTOCOL_INDEX;
CYCODE char iAP2_BundleSeedIDString[]             = {iAP2_EA_ID_STRING};
CYCODE uint8 iAP2_NumberOfProtocol                = iAP2_IPHONE_OS_COMMUNICATION;
CYCODE uint8 iAP2_bBundleSeedStringLength         = iAP2_BUNDLE_SEED_ID_STRING_LENGTH;
CYCODE uint8 iAP2_SiSerialNumber                  = iAP2_USE_SILICON_ID_AS_SERIAL_NUMBER;
#if (iAP2_USE_SILICON_ID_AS_SERIAL_NUMBER)
char iAP2_SerialNumber[17];
#else
char iAP2_SerialNumber[]                          = iAP2_SERIAL_NUMBER;
#endif

CYCODE uint8 iAP2_bMajorAccessoryFWVer            = iAP2_ACCESSORY_FIRMWARE_MAJOR_VERSION;
CYCODE uint8 iAP2_bMinorAccessoryFWVer            = iAP2_ACCESSORY_FIRMWARE_MINOR_VERSION;
CYCODE uint8 iAP2_bRevAccessoryFWVer              = iAP2_ACCESSORY_FIRMWARE_REVISION_VERSION;

CYCODE uint8 iAP2_bMajorAccessoryHWVer            = iAP2_ACCESSORY_HARDWARE_MAJOR_VERSION;
CYCODE uint8 iAP2_bMinorAccessoryHWVer            = iAP2_ACCESSORY_HARDWARE_MINOR_VERSION;
CYCODE uint8 iAP2_bRevAccessoryHWVer              = iAP2_ACCESSORY_HARDWARE_REVISION_VERSION;

uint8 iAP1_bAccessoryCapabilityByte3              = (iAP2_APP_LAUNCH_SUPPORT << iAP1_APP_LAUNCH_SUPPORT_BIT) |
                                                   (iAP1_USB_HOST_SUPPORT << iAP1_USB_HOST_SUPPORT_BIT) | 
                                                   (iAP1_USB_AUDIO_IN_SUPPORT << iAP1_USB_AUDIO_IN_SUPPORT_BIT) |
                                                   (iAP1_USB_AUDIO_OUT_SUPPORT << iAP1_USB_AUDIO_OUT_SUPPORT_BIT);
uint8 iAP1_bAccessoryCapabilityByte2              = (iAP1_MULTI_PACKET_HANDLE << iAP1_MULTI_PACKET_HANDLE_BIT) | (iAP1_UI_ACCESSIBILITY << iAP1_UI_CAPABILITY_BIT);
uint8 iAP1_bAccessoryCapabilityByte1              = (iAP2_IPHONE_OS_COMMUNICATION << iAP1_COMMUNICATION_WITH_APP_BIT) | 
                                                  (iAP1_IPOD_VOLUME_CHECK_ENABLE << iAP1_IPOD_VOLUME_CHECK_BIT);
uint8 iAP1_bAccessoryCapabilityByte0              = (0 << iAP1_ANALOG_LINE_OUT_BIT) | 
                                                   (0 << iAP1_ANALOG_VIDEO_OUT_BIT);
uint8 iAP2_bMetaDataType                          = iAP1_METADATA_PROTOCOL;

/* [] END OF FILE */

