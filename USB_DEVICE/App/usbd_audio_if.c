/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_audio_if.c
  * @version        : v2.0_Cube
  * @brief          : Generic media access layer.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
 /* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_AUDIO_IF
  * @{
  */

/** @defgroup USBD_AUDIO_IF_Private_TypesDefinitions USBD_AUDIO_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Private_Defines USBD_AUDIO_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Private_Macros USBD_AUDIO_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Private_Variables USBD_AUDIO_IF_Private_Variables
  * @brief Private variables.
  * @{
  */

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Exported_Variables USBD_AUDIO_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */
extern uint32_t Volume_Modifier;
/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Private_FunctionPrototypes USBD_AUDIO_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t AUDIO_Init_FS(uint32_t AudioFreq, uint32_t Volume, uint32_t options);
static int8_t AUDIO_DeInit_FS(uint32_t options);
static int8_t AUDIO_AudioCmd_FS(size_t offset, uint8_t cmd);
static int8_t AUDIO_VolumeCtl_FS(uint8_t channel, uint8_t vol);
static int8_t AUDIO_MuteCtl_FS(uint8_t channel, uint8_t cmd);
static int8_t AUDIO_VolumeGet_FS(uint8_t channel, uint8_t *vol);
static int8_t AUDIO_MuteGet_FS(uint8_t channel, uint8_t *cmd);
static int8_t AUDIO_PeriodicTC_FS(uint8_t cmd);
static int8_t AUDIO_GetState_FS(void);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_AUDIO_ItfTypeDef USBD_AUDIO_fops_FS =
{
  AUDIO_Init_FS,
  AUDIO_DeInit_FS,
  AUDIO_AudioCmd_FS,
  AUDIO_VolumeCtl_FS,
  AUDIO_MuteCtl_FS,
  AUDIO_VolumeGet_FS,
  AUDIO_MuteGet_FS,
  AUDIO_PeriodicTC_FS,
  AUDIO_GetState_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the AUDIO media low layer over USB FS IP
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  options: Reserved for future use
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_Init_FS(uint32_t AudioFreq, uint32_t Volume, uint32_t options)
{
  /* USER CODE BEGIN 0 */
  if (AudioFreq != 48000) return USBD_FAIL;
  memset(&haudio.buffer, 0, sizeof haudio.buffer);
  volume_all = Volume;
  volume_l = Volume;
  volume_r = Volume;
  return (USBD_OK);
  /* USER CODE END 0 */
}

/**
  * @brief  De-Initializes the AUDIO media low layer
  * @param  options: Reserved for future use
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_DeInit_FS(uint32_t options)
{
  /* USER CODE BEGIN 1 */
  haudio.started = 0;
  return (USBD_OK);
  /* USER CODE END 1 */
}

/**
  * @brief  Handles AUDIO command.
  * @param  pbuf: Pointer to buffer of data to be sent
  * @param  size: Number of data to be sent (in bytes)
  * @param  cmd: Command opcode
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_AudioCmd_FS(size_t offset, uint8_t cmd)
{
  /* USER CODE BEGIN 2 */
  switch(cmd)
  {
    case AUDIO_CMD_START:
      ConvertS16LEStereoToPWM(haudio.buffer, pwm_ch1_buffer, pwm_ch2_buffer, BUFFER_SIZE);
      volume_all = 100;
      volume_l = 100;
      volume_r = 100;
      is_muted_all = 0;
      is_muted_l = 0;
      is_muted_r = 0;
    break;

    case AUDIO_CMD_PLAY:
    break;

    case AUDIO_CMD_STOP:
    break;
  }
  return (USBD_OK);
  /* USER CODE END 2 */
}

/**
  * @brief  Controls AUDIO Volume.
  * @param  channel: audio channel, 0=left, 1=right
  * @param  vol: volume level (0..100)
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_VolumeCtl_FS(uint8_t channel, uint8_t vol)
{
  /* USER CODE BEGIN 3 */
  switch (channel)
  {
  case 0: volume_all = vol; return USBD_OK;
  case 1: volume_l = vol; return USBD_OK;
  case 2: volume_r = vol; return USBD_OK;
  default: return USBD_FAIL;
  }
  /* USER CODE END 3 */
}

/**
  * @brief  Controls AUDIO Mute.
  * @param  channel: audio channel, 0=left, 1=right
  * @param  cmd: command opcode
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_MuteCtl_FS(uint8_t channel, uint8_t cmd)
{
  /* USER CODE BEGIN 4 */
  switch (channel)
  {
  case 0: is_muted_all = cmd; return USBD_OK;
  case 1: is_muted_l = cmd; return USBD_OK;
  case 2: is_muted_r = cmd; return USBD_OK;
  default: return USBD_FAIL;
  }
  /* USER CODE END 4 */
}

/**
  * @brief  Controls AUDIO Volume
  * @param  channel: audio channel, 0=left, 1=right
  * @param  vol: pointer to output the volume level (0..100)
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_VolumeGet_FS(uint8_t channel, uint8_t *vol)
{
  switch(channel)
  {
  case 0:
	*vol = volume_all;
	return USBD_OK;
  case 1:
	*vol = volume_l;
	return USBD_OK;
  case 2:
	*vol = volume_r;
	return USBD_OK;
  default:
    return USBD_FAIL;
  }
}

/**
  * @brief  Controls AUDIO Mute.
  * @param  channel: audio channel, 0=left, 1=right
  * @param  mute: pointer to output the state of muted or not
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_MuteGet_FS(uint8_t channel, uint8_t *mute)
{
  switch(channel)
  {
  case 0:
	*mute = is_muted_all;
	return USBD_OK;
  case 1:
	*mute = is_muted_l;
	return USBD_OK;
  case 2:
	*mute = is_muted_r;
	return USBD_OK;
  default:
    return USBD_FAIL;
  }
}
/**
  * @brief  AUDIO_PeriodicT_FS
  * @param  cmd: Command opcode
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_PeriodicTC_FS(uint8_t cmd)
{
  /* USER CODE BEGIN 5 */
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Gets AUDIO State.
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_GetState_FS(void)
{
  /* USER CODE BEGIN 6 */
  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  Manages the DMA full transfer complete event.
  * @retval None
  */
void TransferComplete_CallBack_FS(void)
{
  /* USER CODE BEGIN 7 */
  assert(0);
  /* USER CODE END 7 */
}

/**
  * @brief  Manages the DMA Half transfer complete event.
  * @retval None
  */
void HalfTransfer_CallBack_FS(void)
{
  /* USER CODE BEGIN 8 */
  assert(0);
  /* USER CODE END 8 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
