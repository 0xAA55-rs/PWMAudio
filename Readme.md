# STM32F103C8T6/CBT6 Sound Card

## Language 语言

English | [简体中文](Readme-CN.md)

## Overview

* Code was generated with STM32CubeMX and modified by the author.
* MCU operates at 72 MHz as a USB Audio Class (UAC) device supporting 48 kHz 16-bit stereo PCM playback.
* Implementation uses 48 kHz PWM to drive MOSFET-based amplifiers for speaker output.
* USB delivers 48 kHz PCM audio, converted to PWM via a timer (TIM) peripheral.
* PWM resolution limited by MCU clock: Theoretically requires 65,536 clock cycles per period (48 kHz × 65,536 = 3.15 GHz) for full 16-bit resolution at 48 kHz, which exceeds MCU capabilities. Practical implementation uses 1,500 clock cycles per PWM period (72 MHz / 48 kHz = 1,500) with optimized signal processing.

## Wiring

* PA0: PWM output to MOSFET amplifier (Channel 1)
* PA1: PWM output to MOSFET amplifier (Channel 2)
* PA11: USB D- (white wire in standard USB cables)
* PA12: USB D+ (green wire) with 1.5kΩ pull-up to 3.3V
* PA13: SWDIO (ST-Link debugger, optional for programming/debugging)*
* PA14: SWCLK (ST-Link debugger, optional for programming/debugging)*
* PC13: Open-drain output to status LED (blinks during operation, steady on fault)

### Debugging Note

PA13/PA14 connections are optional but enable advanced debugging features when using ST-LINK:
* Real-time variable monitoring
* Breakpoint debugging
* Single-step execution
* Register inspection

## Inspiration

This project explores PWM-based audio generation, inspired by EV motor sound systems. While PWM typically drives motors, its harmonic content can produce audible tones. Electric vehicles use similar techniques to generate pedestrian warning sounds at low speeds. By optimizing PWM parameters, this implementation achieves acceptable audio reproduction through speakers despite hardware constraints. Theoretically.
