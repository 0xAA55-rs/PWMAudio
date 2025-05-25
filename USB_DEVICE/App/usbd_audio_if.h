/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_audio_if.h
  * @version        : v2.0_Cube
  * @brief          : Header for usbd_audio_if.c file.
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_AUDIO_IF_H__
#define __USBD_AUDIO_IF_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief For Usb device.
  * @{
  */

/** @defgroup USBD_AUDIO_IF USBD_AUDIO_IF
  * @brief Usb audio interface device module.
  * @{
  */

/** @defgroup USBD_AUDIO_IF_Exported_Defines USBD_AUDIO_IF_Exported_Defines
  * @brief Defines.
  * @{
  */

/* USER CODE BEGIN EXPORTED_DEFINES */
#define BUFFER_SIZE (AUDIO_BUF_SAMPLE_COUNT / 2)
#define MUTE_BUFFER_SIZE 128
/* USER CODE END EXPORTED_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Exported_Types USBD_AUDIO_IF_Exported_Types
  * @brief Types.
  * @{
  */

/* USER CODE BEGIN EXPORTED_TYPES */

/* USER CODE END EXPORTED_TYPES */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Exported_Macros USBD_AUDIO_IF_Exported_Macros
  * @brief Aliases.
  * @{
  */

/* USER CODE BEGIN EXPORTED_MACRO */

/* USER CODE END EXPORTED_MACRO */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Exported_Variables USBD_AUDIO_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

/** AUDIO_IF Interface callback. */
extern USBD_AUDIO_ItfTypeDef USBD_AUDIO_fops_FS;

/* USER CODE BEGIN EXPORTED_VARIABLES */
extern uint16_t pwm_ch1_buffer[BUFFER_SIZE];
extern uint16_t pwm_ch2_buffer[BUFFER_SIZE];
extern uint16_t* pwm_ch1_buffer_half;
extern uint16_t* pwm_ch2_buffer_half;
extern USBD_AUDIO_HandleTypeDef haudio;
extern uint32_t volume_all;
extern uint32_t volume_l;
extern uint32_t volume_r;
extern int is_muted_all;
extern int is_muted_l;
extern int is_muted_r;
/* USER CODE END EXPORTED_VARIABLES */

/* USER CODE BEGIN EXPORTED_FUNCTIONS */
extern void Main_ResetDMAPosition();
extern void Main_StartPlayTimer();
extern void Main_StopPlayTimer();
extern int Main_IsPlayTimerOn();
extern void ConvertS16LEStereoToPWM(uint8_t *Buffer, uint16_t *Target_L, uint16_t *Target_R, size_t Count);
/* USER CODE END EXPORTED_FUNCTIONS */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_AUDIO_IF_H__ */
