/**
  ******************************************************************************
  * @file    usbd_audio.c
  * @author  MCD Application Team
  * @brief   This file provides the Audio core functions.
  *
  * @verbatim
  *
  *          ===================================================================
  *                                AUDIO Class  Description
  *          ===================================================================
  *           This driver manages the Audio Class 1.0 following the "USB Device Class Definition for
  *           Audio Devices V1.0 Mar 18, 98".
  *          The current audio class version supports the following audio features:
  *             - Pulse Coded Modulation (PCM) format
  *             - sampling rate: 48KHz.
  *             - Bit resolution: 16
  *             - Number of channels: 2
  *             - Volume control capability
  *             - Mute/Unmute capability
  *             - Asynchronous Endpoints
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
- "stm32xxxxx_{eval}{discovery}_audio.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_AUDIO
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_AUDIO_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_AUDIO_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_AUDIO_Private_Macros
  * @{
  */
#define AUDIO_SAMPLE_FREQ(frq)         (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

#define AUDIO_PACKET_SZE(frq)          (uint8_t)(((frq * 2U * 2U)/1000U) & 0xFFU), \
                                       (uint8_t)((((frq * 2U * 2U)/1000U) >> 8) & 0xFFU)

/**
  * @}
  */


/** @defgroup USBD_AUDIO_Private_FunctionPrototypes
  * @{
  */
static uint8_t USBD_AUDIO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_AUDIO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);

static uint8_t USBD_AUDIO_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

static uint8_t *USBD_AUDIO_GetCfgDesc(uint16_t *length);
static uint8_t *USBD_AUDIO_GetDeviceQualifierDesc(uint16_t *length);
static uint8_t USBD_AUDIO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_AUDIO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_AUDIO_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_AUDIO_EP0_TxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_AUDIO_SOF(USBD_HandleTypeDef *pdev);

static uint8_t USBD_AUDIO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_AUDIO_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
static void AUDIO_REQ_GetCur(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_GetMin(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_GetMax(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_GetRes(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_SetCur(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

/**
  * @}
  */

/** @defgroup USBD_AUDIO_Private_Variables
  * @{
  */

USBD_ClassTypeDef  USBD_AUDIO =
{
  USBD_AUDIO_Init,
  USBD_AUDIO_DeInit,
  USBD_AUDIO_Setup,
  USBD_AUDIO_EP0_TxReady,
  USBD_AUDIO_EP0_RxReady,
  USBD_AUDIO_DataIn,
  USBD_AUDIO_DataOut,
  USBD_AUDIO_SOF,
  USBD_AUDIO_IsoINIncomplete,
  USBD_AUDIO_IsoOutIncomplete,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetDeviceQualifierDesc,
};

#pragma pack(push, 1)

/* USB AUDIO device Configuration Descriptor */
#define AUDIO_CONFIG_DESC_SIZE                        0x09U
#define AUDIO_INTERFACE_DESC_SIZE                     0x09U
#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE             0x09U
#define AUDIO_STREAMING_ENDPOINT_DESC_SIZE            0x07U
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE               0x09U
#define AUDIO_FEATURE_UNIT_DESC_SIZE                  0x0AU
#define AUDIO_TYPE_I_FORMAT_INTERFACE_DESC_SIZE       0x0BU
#define AUDIO_CFGDESC_LENGTH \
  AUDIO_CONFIG_DESC_SIZE + \
  AUDIO_INTERFACE_DESC_SIZE + \
  AUDIO_INTERFACE_DESC_SIZE + \
  AUDIO_INPUT_TERMINAL_DESC_SIZE + \
  AUDIO_FEATURE_UNIT_DESC_SIZE + \
  AUDIO_OUTPUT_TERMINAL_DESC_SIZE + \
  AUDIO_INTERFACE_DESC_SIZE + \
  AUDIO_INTERFACE_DESC_SIZE + \
  AUDIO_STREAMING_INTERFACE_DESC_SIZE + \
  AUDIO_TYPE_I_FORMAT_INTERFACE_DESC_SIZE + \
  AUDIO_STREAMING_ENDPOINT_DESC_SIZE + \
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE

#define AUDIO_AC_HEADER_TOTAL_LENGTH \
  AUDIO_INPUT_TERMINAL_DESC_SIZE + \
  AUDIO_FEATURE_UNIT_DESC_SIZE + \
  AUDIO_OUTPUT_TERMINAL_DESC_SIZE

static const size_t AUDIO_CfgDescLength = AUDIO_CFGDESC_LENGTH;
__ALIGN_BEGIN static const uint8_t USBD_AUDIO_CfgDesc[AUDIO_CFGDESC_LENGTH] __ALIGN_END =
{
  /* Configuration 1 */
  AUDIO_CONFIG_DESC_SIZE,               /* bLength */
  USB_DESC_TYPE_CONFIGURATION,          /* bDescriptorType */
  AUDIO_CfgDescLength & 0xFF,           /* wTotalLength */
  AUDIO_CfgDescLength >> 8,
  0x02,                                 /* bNumInterfaces */
  0x01,                                 /* bConfigurationValue */
  0x00,                                 /* iConfiguration */
  0xC0,                                 /* bmAttributes  BUS Powred*/
  0x32,                                 /* bMaxPower = 100 mA*/
  /* 09 byte*/

  /* USB Speaker Standard interface descriptor */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  USB_DESC_TYPE_INTERFACE,              /* bDescriptorType */
  0x00,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x00,                                 /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOCONTROL,          /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* USB Speaker Class-specific AC Interface Descriptor */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_HEADER,                 /* bDescriptorSubtype */
  0x00,          /* 1.00 */             /* bcdADC */
  0x01,
  AUDIO_AC_HEADER_TOTAL_LENGTH,         /* wTotalLength */
  0x00,
  0x01,                                 /* bInCollection */
  0x01,                                 /* baInterfaceNr */
  /* 09 byte*/

  /* USB Speaker Input Terminal Descriptor */
  AUDIO_INPUT_TERMINAL_DESC_SIZE,       /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_INPUT_TERMINAL,         /* bDescriptorSubtype */
  0x01,                                 /* bTerminalID */
  0x01,                                 /* wTerminalType AUDIO_TERMINAL_USB_STREAMING   0x0101 */
  0x01,
  0x00,                                 /* bAssocTerminal */
  0x02,                                 /* bNrChannels */
  0x03,                                 /* wChannelConfig 0x0003  Stereo */
  0x00,
  0x00,                                 /* iChannelNames */
  0x00,                                 /* iTerminal */
  /* 12 byte*/

  /* USB Speaker Audio Feature Unit Descriptor */
  AUDIO_FEATURE_UNIT_DESC_SIZE,         /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_FEATURE_UNIT,           /* bDescriptorSubtype */
  AUDIO_OUT_STREAMING_CTRL,             /* bUnitID */
  0x01,                                 /* bSourceID */
  0x01,                                 /* bControlSize */
  AUDIO_CONTROL_MUTE | AUDIO_CONTROL_VOLUME, /* bmaControls(0) */
  AUDIO_CONTROL_VOLUME,                 /* bmaControls(1) */
  AUDIO_CONTROL_VOLUME,                 /* bmaControls(2) */
  0x00,                                 /* iTerminal */

  /*USB Speaker Output Terminal Descriptor */
  AUDIO_OUTPUT_TERMINAL_DESC_SIZE,      /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_OUTPUT_TERMINAL,        /* bDescriptorSubtype */
  0x03,                                 /* bTerminalID */
  0x01,                                 /* wTerminalType  0x0301*/
  0x03,
  0x00,                                 /* bAssocTerminal */
  0x02,                                 /* bSourceID */
  0x00,                                 /* iTerminal */
  /* 09 byte*/

  /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Zero Bandwith */
  /* Interface 1, Alternate Setting 0                                             */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  USB_DESC_TYPE_INTERFACE,              /* bDescriptorType */
  0x01,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x00,                                 /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Operational */
  /* Interface 1, Alternate Setting 1                                           */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  USB_DESC_TYPE_INTERFACE,              /* bDescriptorType */
  0x01,                                 /* bInterfaceNumber */
  0x01,                                 /* bAlternateSetting */
  0x01,                                 /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* USB Speaker Audio Streaming Interface Descriptor */
  AUDIO_STREAMING_INTERFACE_DESC_SIZE,  /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_STREAMING_GENERAL,              /* bDescriptorSubtype */
  0x01,                                 /* bTerminalLink */
  0x01,                                 /* bDelay */
  0x01,                                 /* wFormatTag AUDIO_FORMAT_PCM  0x0001*/
  0x00,
  /* 07 byte*/

  /* USB Speaker Audio Type I Format Interface Descriptor */
  AUDIO_TYPE_I_FORMAT_INTERFACE_DESC_SIZE, /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_STREAMING_FORMAT_TYPE,          /* bDescriptorSubtype */
  AUDIO_FORMAT_TYPE_I,                  /* bFormatType */
  0x02,                                 /* bNrChannels */
  0x02,                                 /* bSubFrameSize :  2 Bytes per frame (16bits) */
  16,                                   /* bBitResolution (16-bits per sample) */
  0x01,                                 /* bSamFreqType only one frequency supported */
  AUDIO_SAMPLE_FREQ(USBD_AUDIO_FREQ),   /* Audio sampling frequency coded on 3 bytes */
  /* 11 byte*/

  /* Endpoint 0 - Audio Streaming Descriptor */
  AUDIO_STREAMING_ENDPOINT_DESC_SIZE,   /* bLength */
  AUDIO_ENDPOINT_DESCRIPTOR_TYPE,       /* bDescriptorType */
  AUDIO_ENDPOINT_GENERAL,               /* bDescriptor */
  0x00,                                 /* bmAttributes */
  0x00,                                 /* bLockDelayUnits */
  0x00,                                 /* wLockDelay */
  0x00,
  /* 07 byte*/

  /* Endpoint 1 - Standard Descriptor */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,    /* bLength */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType */
  AUDIO_OUT_EP,                         /* bEndpointAddress 1 out endpoint*/
  USBD_EP_TYPE_ADAPTISOC,               /* bmAttributes */
  AUDIO_PACKET_SZE(USBD_AUDIO_FREQ),    /* wMaxPacketSize */
  0x01,                                 /* bInterval */
  0x00,                                 /* bRefresh */
  0x00,                                 /* bSynchAddress */
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

#pragma pack(pop)

/**
  * @}
  */

/** @defgroup USBD_AUDIO_Private_Functions
  * @{
  */
extern USBD_AUDIO_HandleTypeDef haudio;
/**
  * @brief  USBD_AUDIO_Init
  *         Initialize the AUDIO interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_AUDIO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  /* Open EP OUT */
  USBD_LL_OpenEP(pdev, AUDIO_OUT_EP, USBD_EP_TYPE_ADAPTISOC, AUDIO_OUT_PACKET);
  pdev->ep_out[AUDIO_OUT_EP & 0xFU].is_used = 1U;

  /* Allocate Audio structure */
  pdev->pClassData = &haudio;

  if (pdev->pClassData == NULL)
  {
    return USBD_FAIL;
  }
  else
  {
    USBD_AUDIO_HandleTypeDef *haudio = pdev->pClassData;
    USBD_AUDIO_ItfTypeDef *userdata = pdev->pUserData;
    haudio->alt_setting = 0U;
    haudio->offset = AUDIO_OFFSET_NONE;
    haudio->wr_ptr = 0U;

    /* Initialize the Audio output Hardware layer */
    if (userdata->Init(USBD_AUDIO_FREQ, AUDIO_DEFAULT_VOLUME, 0U) != 0)
    {
      return USBD_FAIL;
    }

    /* Prepare Out endpoint to receive 1st packet */
    USBD_LL_PrepareReceive(pdev, AUDIO_OUT_EP, &haudio->buffer[haudio->wr_ptr], AUDIO_OUT_PACKET);
  }

  return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_Init
  *         DeInitialize the AUDIO layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_AUDIO_DeInit(USBD_HandleTypeDef *pdev,
                                  uint8_t cfgidx)
{
  /* Open EP OUT */
  USBD_LL_CloseEP(pdev, AUDIO_OUT_EP);
  pdev->ep_out[AUDIO_OUT_EP & 0xFU].is_used = 0U;

  USBD_AUDIO_ItfTypeDef *userdata = pdev->pUserData;

  /* DeInit  physical Interface components */
  if (pdev->pClassData != NULL)
  {
    userdata->DeInit(0U);
    pdev->pClassData = NULL;
  }

  return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_Setup
  *         Handle the AUDIO specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_AUDIO_Setup(USBD_HandleTypeDef *pdev,
                                 USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef *haudio;
  uint16_t len;
  uint8_t *pbuf;
  uint16_t status_info = 0U;
  uint8_t ret = USBD_OK;

  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
      switch (req->bRequest)
      {
        case AUDIO_REQ_GET_CUR:
          AUDIO_REQ_GetCur(pdev, req);
          break;

        case AUDIO_REQ_GET_MIN:
          AUDIO_REQ_GetMin(pdev, req);
          break;

        case AUDIO_REQ_GET_MAX:
          AUDIO_REQ_GetMax(pdev, req);
          break;

        case AUDIO_REQ_GET_RES:
          AUDIO_REQ_GetRes(pdev, req);
          break;

        case AUDIO_REQ_SET_CUR:
          AUDIO_REQ_SetCur(pdev, req);
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_DESCRIPTOR:
          if ((req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE)
          {
            pbuf = (uint8_t*)USBD_AUDIO_CfgDesc + AUDIO_CONFIG_DESC_SIZE + AUDIO_INTERFACE_DESC_SIZE;
            len = MIN(AUDIO_INTERFACE_DESC_SIZE, req->wLength);

            USBD_CtlSendData(pdev, pbuf, len);
          }
          break;

        case USB_REQ_GET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&haudio->alt_setting, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            if ((uint8_t)(req->wValue) <= USBD_MAX_NUM_INTERFACES)
            {
              haudio->alt_setting = (uint8_t)(req->wValue);
            }
            else
            {
              /* Call the error management function (command will be nacked */
              USBD_CtlError(pdev, req);
              ret = USBD_FAIL;
            }
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;
    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }

  return ret;
}


/**
  * @brief  USBD_AUDIO_GetCfgDesc
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_AUDIO_GetCfgDesc(uint16_t *length)
{
  assert(sizeof(USBD_AUDIO_CfgDesc) == AUDIO_CfgDescLength);
  *length = sizeof(USBD_AUDIO_CfgDesc);
  return (uint8_t *)USBD_AUDIO_CfgDesc;
}

/**
  * @brief  USBD_AUDIO_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_AUDIO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  /* Only OUT data are processed */
  return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_EP0_RxReady
  *         handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_AUDIO_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  USBD_AUDIO_HandleTypeDef *haudio = pdev->pClassData;
  USBD_AUDIO_ItfTypeDef *userdata = pdev->pUserData;

  if (haudio->control.unit != AUDIO_OUT_STREAMING_CTRL) return USBD_FAIL;

  switch(haudio->control.cmd)
  {
    case AUDIO_REQ_SET_CUR:
      switch (haudio->control.feature_control)
      {
        case AUDIO_CONTROL_MUTE:
          return userdata->MuteCtl(haudio->control.data[0]);
        case AUDIO_CONTROL_VOLUME:
          return userdata->VolumeCtl(haudio->control.channel, haudio->control.data[0]);
      }
      USBD_CtlSendStatus(pdev);
      return USBD_OK;
    case AUDIO_REQ_SET_MIN:
    case AUDIO_REQ_SET_MAX:
    case AUDIO_REQ_SET_RES:
      USBD_CtlSendStatus(pdev);
      return USBD_OK;
    case AUDIO_REQ_GET_CUR:
      AUDIO_REQ_GetCur(pdev, &pdev->request);
      return USBD_OK;
    case AUDIO_REQ_GET_MIN:
      AUDIO_REQ_GetMin(pdev, &pdev->request);
      return USBD_OK;
    case AUDIO_REQ_GET_MAX:
      AUDIO_REQ_GetMax(pdev, &pdev->request);
      return USBD_OK;
    case AUDIO_REQ_GET_RES:
      AUDIO_REQ_GetRes(pdev, &pdev->request);
      return USBD_OK;
    default:
      return USBD_FAIL;
  }
}
/**
  * @brief  USBD_AUDIO_EP0_TxReady
  *         handle EP0 TRx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_AUDIO_EP0_TxReady(USBD_HandleTypeDef *pdev)
{
  /* Only OUT control data are processed */
  return USBD_OK;
}
/**
  * @brief  USBD_AUDIO_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_AUDIO_SOF(USBD_HandleTypeDef *pdev)
{
  return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */
void  USBD_AUDIO_Sync(USBD_HandleTypeDef *pdev, AUDIO_OffsetTypeDef offset)
{
  USBD_AUDIO_HandleTypeDef *haudio = pdev->pClassData;
  USBD_AUDIO_ItfTypeDef *userdata = pdev->pUserData;
  haudio->offset = offset;

  switch (haudio->offset)
  {
  case AUDIO_OFFSET_HALF:
	  userdata->AudioCmd(0, AUDIO_CMD_PLAY);
	  break;
  case AUDIO_OFFSET_FULL:
    userdata->AudioCmd(AUDIO_HALF_BUF_SIZE, AUDIO_CMD_PLAY);
    break;
  case AUDIO_OFFSET_NONE:
    userdata->AudioCmd(0, AUDIO_CMD_START);
	  break;
  }
}

/**
  * @brief  USBD_AUDIO_IsoINIncomplete
  *         handle data ISO IN Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_AUDIO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  return USBD_OK;
}
/**
  * @brief  USBD_AUDIO_IsoOutIncomplete
  *         handle data ISO OUT Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_AUDIO_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  return USBD_OK;
}
/**
  * @brief  USBD_AUDIO_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_AUDIO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_AUDIO_HandleTypeDef *haudio = pdev->pClassData;

  if (epnum == AUDIO_OUT_EP)
  {
    USBD_CtlSendStatus(pdev);
    haudio->wr_ptr += AUDIO_OUT_PACKET;
    if (haudio->wr_ptr == AUDIO_HALF_BUF_SIZE)
    {
      if (haudio->offset != AUDIO_OFFSET_NONE)
      {
        haudio->offset = AUDIO_OFFSET_HALF;
        USBD_AUDIO_Sync(pdev, haudio->offset);
      }
    }
    else if (haudio->wr_ptr == AUDIO_TOTAL_BUF_SIZE)
    {
      if (haudio->offset == AUDIO_OFFSET_NONE)
        USBD_AUDIO_Sync(pdev, haudio->offset);
      else
        USBD_AUDIO_Sync(pdev, AUDIO_OFFSET_FULL);
      haudio->offset = AUDIO_OFFSET_FULL;
      haudio->wr_ptr = 0U;
    }

    /* Prepare Out endpoint to receive next audio packet */
    USBD_LL_PrepareReceive(pdev, AUDIO_OUT_EP, &haudio->buffer[haudio->wr_ptr], AUDIO_OUT_PACKET);
  }

  return USBD_OK;
}

/**
  * @brief  AUDIO_Req_GetCur
  *         Handles the GET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_GetCur(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_ItfTypeDef *userdata = pdev->pUserData;

  uint8_t command = HIBYTE(req->wValue);
  uint8_t channel = LOBYTE(req->wValue);
  uint8_t data = 0;

  switch (command)
  {
  case AUDIO_CONTROL_MUTE:
    userdata->MuteGet(&data);
    USBD_CtlSendData(pdev, &data, 1);
    break;
  case AUDIO_CONTROL_VOLUME:
    userdata->VolumeGet(channel, &data);
    USBD_CtlSendData(pdev, &data, 1);
    break;
  default:
	assert(0);
  }
}

/**
  * @brief  AUDIO_Req_GetMin
  *         Handles the GET_MIN Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_GetMin(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  uint8_t command = HIBYTE(req->wValue);
  uint8_t data = 0;

  switch (command)
  {
  case AUDIO_CONTROL_MUTE:
    data = 0;
    USBD_CtlSendData(pdev, &data, 1);
    break;
  case AUDIO_CONTROL_VOLUME:
    data = 0;
    USBD_CtlSendData(pdev, &data, 1);
    break;
  default:
  assert(0);
  }
}

/**
  * @brief  AUDIO_REQ_GetMax
  *         Handles the GET_MAX Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_GetMax(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  uint8_t command = HIBYTE(req->wValue);
  uint8_t data = 0;

  switch (command)
  {
  case AUDIO_CONTROL_MUTE:
    data = 1;
    USBD_CtlSendData(pdev, &data, 1);
    break;
  case AUDIO_CONTROL_VOLUME:
    data = 100;
    USBD_CtlSendData(pdev, &data, 1);
    break;
  default:
  assert(0);
  }
}

/**
  * @brief  AUDIO_REQ_GetRes
  *         Handles the GET_RES Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_GetRes(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  uint8_t command = HIBYTE(req->wValue);
  uint8_t data = 0;

  switch (command)
  {
  case AUDIO_CONTROL_MUTE:
    data = 1;
    USBD_CtlSendData(pdev, &data, 1);
    break;
  case AUDIO_CONTROL_VOLUME:
    data = 1;
    USBD_CtlSendData(pdev, &data, 1);
    break;
  default:
  assert(0);
  }
}

/**
  * @brief  AUDIO_Req_SetCurrent
  *         Handles the SET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_SetCur(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef *) pdev->pClassData;

  if (req->wLength)
  {
    /* Prepare the reception of the buffer over EP0 */
    USBD_CtlPrepareRx(pdev, haudio->control.data, req->wLength);

    haudio->control.cmd = AUDIO_REQ_SET_CUR;     /* Set the request value */
    haudio->control.len = (uint8_t)req->wLength; /* Set the request data length */
    haudio->control.unit = HIBYTE(req->wIndex);  /* Set the request target unit */
    haudio->control.channel = LOBYTE(req->wValue);
    haudio->control.feature_control = HIBYTE(req->wValue);
  }
}


/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t  *USBD_AUDIO_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = sizeof(USBD_AUDIO_DeviceQualifierDesc);

  return USBD_AUDIO_DeviceQualifierDesc;
}

/**
* @brief  USBD_AUDIO_RegisterInterface
* @param  fops: Audio interface callback
* @retval status
*/
uint8_t  USBD_AUDIO_RegisterInterface(USBD_HandleTypeDef *pdev,
                                      USBD_AUDIO_ItfTypeDef *fops)
{
  if (fops != NULL)
  {
    pdev->pUserData = fops;
  }

  return USBD_OK;
}
/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
