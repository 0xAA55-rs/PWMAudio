# Implementing a USB Sound Card with STM32F103C8T6

## Language
English | [简体中文](JourneyFromBeginning-CN.md)

## Source Code
* **GitHub**:
  [Main Repo https://github.com/0xAA55/PWMAudio](https://github.com/0xAA55/PWMAudio)
  [Alt Repo https://github.com/0xAA55-rs/PWMAudio](https://github.com/0xAA55-rs/PWMAudio)
* **Gitee (China Mirror)**:
  [https://gitee.com/a5k3rn3l/pwmaudio](https://gitee.com/a5k3rn3l/pwmaudio)
* **Note**: All repositories are fully synchronized. Gitee defaults to Chinese `Readme-CN.md`.

## Development Tools
### Software:
* **STM32CubeMX**: This thing helps you select the chip model, then choose which peripherals and middleware to use, configure chip pins, and finally generates initial code for you to modify. I've already used it to generate the code, you can directly clone my code from the repository.
  * *Note: If you install STM32CubeMX, you can see exactly how I configured various peripherals, the main system clock configuration (USB peripherals must be configured to 48MHz), TIM2 configuration, DMA configuration, and USB configuration.*

* **STM32CubeIDE**: This thing is a code editor + downloader and debugger (requires ST-LINK device). You need to open the `.project` file in my project using this software, wait for it to fully load, and then you can view, edit, modify code, adjust parameters, compile (Debug and Release configurations), download and debug. I've enabled `-flto` optimization in the Release configuration, so the optimization effect should be ideal (may over-optimize, causing the compiler to think your code is useless. You need to use the `volatile` keyword to modify some registers that must be read or stored). For optimization direction, I used `-O3`. The default is `-Os`, mainly afraid the code would be too large and blow up the ROM, but there actually won't be that much code.

* **Audio Player**: We're developing a sound card device. To test this device, we need a player responsible for playing things like music or recordings.

* **Serial Terminal**: The ideal debugging situation is: when you can't step debug or set breakpoints (for example, when debugging USB transaction processing, if you pause the microcontroller, the host will think USB communication has failed, making debugging difficult), you can use `printf()` to debug. When your microcontroller `printf()`s, its output should appear on the serial terminal software. Unless you have a ST-LINK with SWO pin support, then you can use SWO to receive debugging information output from the microcontroller's B3 pin, which will be displayed in STM32CubeIDE's Console.
  * I use [MobaXterm](https://mobaxterm.mobatek.net/download.html), **UNREGISTERED VERSION**. First, there's no real need to register, but it pops up four penguins drilling holes in your terminal interface when idle, preventing the text content on the terminal interface from updating in real time. Second, it doesn't let me register because it requires payment via PayPal, and I don't have a suitable bank card to pay. I need to get a MasterCard. When it supports WeChat Pay, I'll register.
  * *Personal note:* **I have a message for the MobaXterm team**. Could you please enable registration through WeChat Pay? As with the majority of Chinese developers, I don't have access to PayPal.
    * *I guess they won't see this message.*
  * [RealTerm](https://sourceforge.net/projects/realterm/) can actually be used too, but its appearance is very retro, like software from 20 years ago. Occasionally it goes on strike due to driver issues or Windows UAC permission problems.

* **ST-LINK Utility (Optional)**: This software allows you to use counterfeit (and firmware update-incompatible) ST-LINK-V2 to connect to your STM32 microcontroller, then you can flash ROM, view ROM and RAM, and peripheral registers, etc. It uses a table format to display memory data piece by piece. You can directly edit this data. You can even operate peripheral registers by **hand-fiddling with register values** to get peripherals working, although this behavior is somewhat like operating the ENIAC computer about eighty years ago.

### Hardware:
* **STM32F103C8T6**: I use `blue-pill`. Check if its USB D+ pull-up resistor value is correct. Normally, it should be 1KΩ or 1.5KΩ. If it's wrong (like the counterfeit ones sold with 10KΩ), calculate it yourself and parallel a suitable resistor to the 3.3V power supply.

* **ST-LINK-V2**: Used for programming STM32F103C8T6 and debugging. Remember to buy the "firmware-upgradable" version that must support the firmware upgrade functionality required by STM32CubeIDE.
  * With this you can do step debugging, breakpoint debugging, all kinds of debugging. View registers, memory, structs, variables, peripheral registers.
  * *But this thing doesn't have an SWO pin*, so if you plan to use SWO (B3 pin) for `printf()` debug output, you'll probably need to buy a genuine ST-LINK programmer. Better to use USART for debug output.

* **Optocouplers**: Parameter requirements: Must support sufficiently high frequency, at least 48 kHz, and output basically correct PWM duty cycle at this frequency.
  * I tested [6N137](https://www.vishay.com/docs/84732/6n137.pdf), works well. Its switching speed meets my requirements, but it's delicate. The LED side needs to limit current from the STM32 GPIO output—**must not exceed 20mA**. Its output voltage is 5V, output current is low, can't drive speakers, but can drive MOSFETs. Its power supply side needs parallel filter capacitors. Check the [datasheet](https://www.vishay.com/docs/84732/6n137.pdf) yourself for specifics.
    * The STM32 on `blue-pill` uses an onboard voltage regulator to provide 3.3V power. GPIO single port PP output current **maxes at 25mA**. Such current would degrade your 6N137's performance. You need to calculate the resistance value yourself and add a suitable resistor in series with the LED side. Alternatively, you could use OD output with a suitable pull-up resistor, but using OD mode would require you to invert your output data. Also, OD mode might leave a tiny positive voltage output when pulling the pin low, which could accidentally trigger the optocoupler.
  * I've tried other optocouplers, like [PC817C 217N](https://learnabout-electronics.org/Downloads/PC817%20optocoupler.pdf). Its switching speed can't reach 48 kHz. When I output 50% duty cycle PWM square waves, what I see on the oscilloscope is a slightly fluctuating straight line output. But this optocoupler supports larger output current, and in some cases can even directly drive devices with its output current. It's not as delicate as the 6N137, but obviously not as fast either.
  * You don't necessarily have to use optocouplers. I use optocouplers to achieve **electrical isolation**, separating the power supply of the STM32 MCU from the power supply for the speakers. This way the MCU gets stable 5V power, while the speaker sucking large currents uses a separate power supply that doesn't affect my MCU. Without optocouplers, you'd directly connect GPIO to the MOSFET. In that case, your MCU's power supply becomes unstable. Always be prepared with an oscilloscope to see how much fluctuation there is in your MCU's power supply. If the amplitude is large, the MCU will have strange internal problems, like calculation errors, or register value errors, even the PC running away. This situation means the MCU becomes unreliable due to unreliable power supply, so you'll have to limit the MOSFET's output current (then your speaker's volume might be smaller than a mosquito's buzz) and add filter capacitors in parallel at the power supply.

* **MOSFETs**: Requires appropriate power support (e.g., within 10W). As mentioned above, optocouplers have switching speed limits, but MOSFETs mostly respond quickly to Gate pin to Source pin voltage changes, basically supporting switching frequencies far exceeding 48 kHz, so no worries in this regard.
  * I use [IRF510](https://www.vishay.com/docs/91015/irf510.pdf), screwed onto an aluminum heat sink with screws. Since I couldn't find my thermal paste, I just tightened it with screws to make it stick tightly to the heat sink. I thought about soldering it to the heat sink with solder, but that would require sanding off the aluminum oxide layer on the aluminum heat sink surface, then using solder paste and a hot air gun or electric hotplate to heat the whole thing up. But **the heat sink will dissipate the heat energy from your hot air gun or electric hotplate**, and if you turn the temperature of the hot air gun or hotplate too high, it might damage the MOSFET, which is really annoying. It can work normally without a heat sink, but gets *insanely* hot—don't touch it. With a heat sink, it looks better and stays cool.

* **Resistors**: Once during testing I forgot to add a resistor, and the high current from the MOSFET directly exploded my speaker's diaphragm. *"Electrical surge + big membrane = fly away"* literal translation of "电大膜飞".
  * Also, resistors get *insanely* hot too, because of the low resistance value and high current coming from the MOSFET.

* **CH340 Serial to USB**: Used to receive debug content output from your microcontroller via serial port. If you have your own favorite serial-to-USB tool, use that. `115200` baud rate, 8-bit data, 1 start bit, 1 stop bit. Your microcontroller's serial port should communicate with these parameters too, then it's fine.

* Since many USB ports will be needed (at least three), it's best to get a USB Hub, and this Hub must have independent power supply capability.

* Because of using optocouplers for electrical isolation, the speaker side needs a USB charger for power. Plug this USB charger into mains power, then make your own USB cable. Plug one end into this charger, and the other end connect with a connector to the power supply on your prototype board for the speaker side.

* Miscellaneous items: Prototype board, jumper wires, wire strippers, nail clippers (very suitable for cutting cables and DuPont pins and other thin, sturdy metal), soldering station, hot air gun, heat shrink tubing, solder, brass sponge, DuPont wires, DuPont pins, connectors, USB connectors, pliers or other clamps (to hold the circuit board while soldering), a bunch of ceramic capacitors you may or may not use, resistors. I don't like electrolytic capacitors—they explode.

***Ventilation Warning***:
  **Good ventilation environment, or if ventilation is poor but you can skillfully dodge** by moving your head away from the soldering smoke, you won't suffer skin and brain damage from inhaling excessive lead.

## Development Process

### Idea
* The host continuously sends me PCM audio. The format is 16-bit signed integer samples, with left and right channel samples **interleaved** (stored in alternating order: L, R, L, R...).
* I use PWM to drive the speakers. I need to convert the audio samples sent from the host to PWM duty cycle arrays on my end, then use DMA in circular mode to continuously write these arrays to the PWM channel's duty cycle registers in the TIM peripheral.
* Because I need to drive two speakers playing left and right channel audio, I need two PWM channels of TIM. Each PWM channel's duty cycle data must be provided by an independent DMA channel. Therefore, I need two PWM duty cycle arrays as data sources for DMA transfer.
* Set DMA to run in **"circular mode"** (`DMA_CIRCULAR`). When half transferred, call one of my callback functions; when a whole buffer is transferred, call another callback function of mine. What do my callback functions do? My callback functions are responsible for converting the PCM samples sent by the host into PWM duty cycle on my end. They separate left and right channels, scale the value range from `-32768..32767` to my PWM pulse width range, and store them separately in my two PWM buffers. This way DMA can continuously output PCM-converted PWM duty cycle to the PWM pulse width register of TIM.
  * This is the typical double buffering playback mode. While playing one buffer, update the data in the other buffer; when switching to play the other buffer, update the data in the previous buffer, repeating seamlessly for continuous playback.
* Theoretically, STM32F103C8T6 runs at 72 MHz. If you want to play 48,000 Hz audio with PWM, the PWM pulse width value range can only be within 1500. This way, the theoretical audio quality is reduced to about 10 to 11 bits depth.

### My Code Hands-on Process
STM32CubeMX-generated USB Audio Class example code all needs your modifications. Focus on modifying the following key code files:
* `USB_DEVICE\App\usbd_audio_if.c`
* `USB_DEVICE\App\usbd_audio_if.h`
* `Middlewares\ST\STM32_USB_Device_Library\Class\AUDIO\Src\usbd_audio.c`
* `Middlewares\ST\STM32_USB_Device_Library\Class\AUDIO\Inc\usbd_audio.h`

Additionally, there are many bugs to fix in `Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_pcd.c`.

### Setting Up Serial Debugging
You also need your device to output debug information via serial port and use it. I chose USART1, using `A9` and `A10` as TX and RX, but its RX DMA channel is taken. Forget using DMA. The baud rate is only `115200` Hz—using an interrupt handler to read byte by byte is sufficient.

Connect `A9` to CH340's white wire, `A10` to CH340's green wire. Plug CH340 into your host computer's USB port. Launch your serial terminal software and open the COM port of the CH340 USB-to-serial device.

#### Completing the Standard Input/Output System
The `stdio.h` in the STM32's C standard library has implementations of basic standard I/O functions like `printf()` and `scanf()`, but ultimately, at the lowest level, they rely on calling `__io_putchar()` and `__io_getchar()` to write and read data. The initial code generated by STM32CubeMX has `syscalls.c`, which provides weak symbol implementations of these functions that do nothing. Once we implement some of its functions, standard I/O will work.

But first we need to implement a FIFO buffer struct module, with a series of functions for reading and writing FIFOs. After implementing it, create two instances of this struct dedicated to serving as STDIN and STDOUT buffers, named: `fb_in` and `fb_out`. Then implement the following functions (I implemented them directly in `main.c`):
* `int  __io_getchar(void);` Implement it to read one byte from `fb_in` and return it. If `fb_in` has no data, return `EOF`.
  * When data comes in via UART RX transmission, you store the data in your `fb_in`.
* `int  _read(int file, char *ptr, int len);` Also implement it to read data from `fb_in`. This function is for bulk reading. Return the actual number of bytes read. If your `fb_in` is empty, return `0`.
  * If you don't implement this function, it will call the default implementation in `syscalls.c`. The default implementation is looping calls to `__io_getchar()`, which is inefficient.
* `int  __io_putchar(int ch);` Implement it to store `ch` in your `fb_out`.
  * In the main loop of `main()`, check if your `fb_out` has data. If yes, take it out and output it via serial.
* `int  _write(int file, char *ptr, int len);` Implement it to write a block of data of length `len` pointed to by `ptr` into your `fb_out`. Return the actual number of bytes written. If `fb_out` gets full, return `0`.
  * If you don't implement this function, it will call the default implementation in `syscalls.c`. The default implementation is looping calls to `__io_putchar()`, which is inefficient.

In this way, your `printf()` will call your `_write()` to store the output content in your FIFO, and your main loop will output all data in the FIFO that needs to be sent to the serial port every time it loops. The serial terminal software on your host computer will display the content printed by your `printf()`.

When you type in the serial terminal software on your host computer, the CH340 connected to your host will transmit your input to your RX. Your UART receive data callback function is responsible for storing this data in your FIFO. Then you can use functions like `scanf()` in your main loop to read data from the FIFO. This way you achieve serial interaction, and then you can invent your own command-line interface.

#### Regarding UART Data Reception
You need to implement the `void  HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);` function, and define a global variable outside: `uint32_t uart_buf = 0;`. When this function gets called, the value of `uart_buf` will be a byte received by the serial RX. You need to write this byte into your `fb_in`, then call `HAL_UART_Receive_IT(huart, (uint8_t*)&uart_buf, 1);` before returning.

Why call `HAL_UART_Receive_IT(huart, (uint8_t*)&uart_buf, 1);`? Because after calling this function, when there's data to receive via UART, the relevant interrupt will trigger. The interrupt handler will write data into `uart_buf` and then call your `HAL_UART_RxCpltCallback()` implementation. Therefore, you must also call `HAL_UART_Receive_IT(huart, (uint8_t*)&uart_buf, 1);` within your `HAL_UART_RxCpltCallback()` implementation to continue receiving the next byte.

So in the initialization process of `main()`, add a line: `HAL_UART_Receive_IT(huart, (uint8_t*)&uart_buf, 1);` to start the UART receive logic.

## Wiring
* **5V**: Connect to USB 5V (red wire in standard USB specs).
* **G (GND)**: Connect to USB GND (black wire in standard USB specs).
* **PA0**: PWM output to optocoupler, optocoupler output to MOSFET amplifier (Channel 1)
* **PA1**: PWM output to optocoupler, optocoupler output to MOSFET amplifier (Channel 2)
* **PA9**: Serial output, connect to CH340's white wire. For debugging.
* **PA10**: Serial input, connect to CH340's green wire. For debugging.
* **PA11**: USB D- (white signal wire in standard USB specs)
* **PA12**: USB D+ (green signal wire in standard USB specs) needs a 1.5kΩ pull-up resistor to 3.3V.
* **PA13**: SWDIO (for ST-Link debugger, used during debugging/programming)*
* **PA14**: SWCLK (for ST-Link debugger, used during debugging/programming)*
* **PC13**: Open-drain mode connected to LED (the `blue_pill` board already has it connected with a pull-up resistor). You can control this LED to blink as a form of debug output.

## Pitfall-Filling Process

### Their Default Audio Receive Buffer Size is Ridiculously Large
* In `usbd_audio.h`, there's a struct, `USBD_AUDIO_HandleTypeDef`. It looks like this:
  ```
  typedef struct
  {
      uint32_t                  alt_setting;
      uint8_t                   buffer[AUDIO_TOTAL_BUF_SIZE];
      AUDIO_OffsetTypeDef       offset;
      uint8_t                    rd_enable;
      uint16_t                   rd_ptr;
      uint16_t                   wr_ptr;
      USBD_AUDIO_ControlTypeDef control;
  }
  USBD_AUDIO_HandleTypeDef;
  ```
  * Guess how big `AUDIO_TOTAL_BUF_SIZE` is? You're right! It's huge! Directly running triggers `HardFault` because you're writing outside RAM.
    * The actual value is `AUDIO_OUT_PACKET * AUDIO_OUT_PACKET_NUM`
    * `AUDIO_OUT_PACKET` is sample rate `48000` × `2` × `2` ÷ `1000` = `192`.
    * `AUDIO_OUT_PACKET_NUM` is eighty.
    * So the buffer size is `15360` bytes, hexadecimal `0x3C00`. Your RAM is only `0x5000` bytes total. You also need to maintain your own PWM buffers. Your PWM buffer's sample count must align with its buffer's sample count. Its buffer holds `7680` samples, `3840` per channel. You need left and right channels, each also `3840` samples. You'll store `7680` samples total. Your PWM buffers combined will occupy `15360` bytes. Your buffer and its buffer added together require `0x7800` bytes, far exceeding your total RAM capacity.
  * Change `AUDIO_OUT_PACKET_NUM` from eighty to ten. Your buffer and its buffer together use `0xC00` bytes. Problem solved.
  * The `rd_enable` and `rd_ptr` in this struct seem pretty useless.

### Its Default Device Configuration Descriptor is Unreasonable, Doesn't Allow Setting Volume
* In `usbd_audio.c`, there's the device configuration descriptor, `USBD_AUDIO_CfgDesc`. You need to read the [USB Audio Class standard specification](https://www.usb.org/sites/default/files/audio10.pdf) to understand this and modify its parameters to what you need. If you mess this up, when you plug your device into the computer, the computer won't recognize it as a USB sound card.
  * Pay attention to this section:
    ```
    /* USB Speaker Input Terminal Descriptor */
    AUDIO_INPUT_TERMINAL_DESC_SIZE,       /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
    AUDIO_CONTROL_INPUT_TERMINAL,         /* bDescriptorSubtype */
    0x01,                                 /* bTerminalID */
    0x01,                                 /* wTerminalType AUDIO_TERMINAL_USB_STREAMING   0x0101 */
    0x01,
    0x00,                                 /* bAssocTerminal */
    0x01,                                 /* bNrChannels */
    0x00,                                 /* wChannelConfig 0x0000  Mono */
    0x00,
    0x00,                                 /* iChannelNames */
    0x00,                                 /* iTerminal */
    /* 12 byte*/
    ```
    * This descriptor is your USB input terminal descriptor. `wChannelConfig` must be changed to `0x0003`. This is a bitfield; `0x0003` means you support left and right channels (so-called left front and right front speakers in Dolby™ stereo systems). Change the corresponding byte to `0x03`, followed by `0x00`, that's correct.
  * Now look at this section:
    ```
    /* USB Speaker Audio Feature Unit Descriptor */
    0x09,                                 /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
    AUDIO_CONTROL_FEATURE_UNIT,           /* bDescriptorSubtype */
    AUDIO_OUT_STREAMING_CTRL,             /* bUnitID */
    0x01,                                 /* bSourceID */
    0x01,                                 /* bControlSize */
    AUDIO_CONTROL_MUTE,                   /* bmaControls(0) */
    0,                                    /* bmaControls(1) */
    0x00,                                 /* iTerminal */
    ```
    * It has a problem: Because the previous `USB Speaker Input Terminal Descriptor` wasn't configured properly, it's filling this randomly too. Its meaning is that your sound card only supports mute on/off. A normal sound card needs to support volume adjustment, and normally you need three `bmaControls` bitfields: one to control master volume and mute switch, and the other two to control left and right channel volumes individually. It needs to be changed like this:
    ```
    /* USB Speaker Audio Feature Unit Descriptor */
    0x0A,                                 /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
    AUDIO_CONTROL_FEATURE_UNIT,           /* bDescriptorSubtype */
    AUDIO_OUT_STREAMING_CTRL,             /* bUnitID */
    0x01,                                 /* bSourceID */
    0x01,                                 /* bControlSize */
    AUDIO_CONTROL_MUTE | AUDIO_CONTROL_VOLUME, /* bmaControls(0) */
    AUDIO_CONTROL_VOLUME,                 /* bmaControls(1) */
    AUDIO_CONTROL_VOLUME,                 /* bmaControls(2) */
    0x00,                                 /* iTerminal */
    ```
  * Did you notice? After changing, it's one byte longer. So you must increase the value of `USB_AUDIO_CONFIG_DESC_SIZ` by `1`.
* This descriptor `USBD_AUDIO_CfgDesc` is fully defined like this:
  ```
  __ALIGN_BEGIN static uint8_t USBD_AUDIO_CfgDesc[USB_AUDIO_CONFIG_DESC_SIZ] __ALIGN_END = { ... };
  ```
  * You see it's modified with `static`, but this content isn't meant to be changed. You might think: Ah! This thing isn't modified with `const`! It will go into the `.data` section, consuming my RAM. Then my ROM also stores a copy, and my RAM stores another copy—how much RAM is that wasting?
    * Actually, if you add `const`, you do save RAM, about `110` bytes. But when the STM32's USB peripheral submits the device configuration descriptor to the host, **it can only submit from RAM**, otherwise it triggers `HardFault`.
    * This table looks long, but it's only because the way it's written makes it seem long.

### Its Default Implementations of `USBD_AUDIO_DataOut()` and `USBD_AUDIO_Sync()` Have Lots of Nonsensical Code
* We receive audio data at the DataOut stage of `ep1`. At this point this function is called:
  ```
  static uint8_t USBD_AUDIO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
  ```
  * Remember the struct `USBD_AUDIO_HandleTypeDef` mentioned above? This struct contains the PCM buffer. And this `USBD_AUDIO_DataOut()` is specifically responsible for receiving PCM audio data into this buffer. I mentioned its `rd_enable` and `rd_ptr` are useless. Indeed, they are useless. Its source code is so garbage I don't even want to put it here taking up space. Plus it has bugs causing buffer overflows. I'm speechless. It actually wants to implement a double buffering logic: check if `wr_ptr` has reached half the buffer, then call `usbd_audio_if.c`'s `AudioCmd()` to notify you that half the buffer is ready; then check if `wr_ptr` has reached the end of the buffer, and also call `AudioCmd()` to notify you the other half is ready. Its `rd_enable` is set to `1` after it receives half a buffer of audio data, meaning you can read now. And its `rd_ptr`—I'm not even sure what it's for—probably tells you where in the buffer it's reading, then calls `AudioCmd()` to let you play the "readable part". It's extremely cumbersome and unnecessary.
* Here's how I wrote it:
  ```
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
  ```
* And the implementation of `USBD_AUDIO_Sync()`:
  ```
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
  ```
  * Isn't it just a double buffering logic? Also, actually my `AudioCmd()` only handles the `AUDIO_CMD_START` command. That is, first pause TIM, reset DMA's read position, convert the entire `buffer` to my PWM buffer, then start TIM to begin playback. I've already implemented the double buffering logic in DMA. This is a function called during USB transactions. Converting sample formats here would cause your transaction processing to be too slow, unable to provide timely feedback to the host, causing USB communication failure.

### Fixing Issues in `stm32f1xx_hal_pcd.c`
* This source file is responsible for responding to USB interrupts, reading status from the USB peripheral, handling Setup, Data In, Data Out transactions, providing a bunch of functions and macros to fiddle with USB peripheral registers. But the USB peripheral has a bug: when you read the "amount of data the host sent me" from its `ep`, you'll read strange amounts, like Setup packet length must be `8` according to spec, but you'll occasionally receive wrong lengths like `40`, `64`, `1023`, etc. And its default implementation is to ignore the received length—like for a Setup packet—and dump everything into `hpcd->Setup`. This `hpcd` is the `PCD_HandleTypeDef` struct. Besides `Setup` (a fixed-length array), it has many other important members. This way, garbage data received due to transmission errors will be dumped over this struct's `Setup` and subsequent other members. And because it has limits when fetching the length register value, you won't even get a `HardFault` from writing outside RAM. You'll just mess up your RAM and find things in this struct are bizarre and not normal values. Programmers unfamiliar with the consequences of array out-of-bounds writes in C will surely take a very, very long time debugging this issue, or suspect ghosts.
* Find this function: `static  HAL_StatusTypeDef  PCD_EP_ISR_Handler(PCD_HandleTypeDef *hpcd)`
  * Find its first call to `USB_ReadPMA()`:
    `USB_ReadPMA(hpcd->Instance, (uint8_t *)hpcd->Setup, ep->pmaadress, ep->xfer_count);`
    * Note, this is when processing Setup transactions, reading the Setup packet. Here `ep->xfer_count` is normally `8`, but if it's not `8`, you need packet discard handling. And you need to check the value of `ep->xfer_count` before this `USB_ReadPMA()` call. You can define a local variable to cap its size, like:
      `size_t max_xfer_count = PCD_MIN(ep->xfer_count, sizeof hpcd->Setup);`
    * Then change the function call to:
      `USB_ReadPMA(hpcd->Instance, (uint8_t *)hpcd->Setup, ep->pmaadress, max_xfer_count);`
    * This way at least you can analyze what content it read out, and you need packet discard handling. Packet discard handling means: don't let it call `HAL_PCD_SetupStageCallback()` for Setup transaction processing. So you need to use a `goto` to jump past this function.
  * Find all subsequent calls to `USB_ReadPMA()`. Their last parameter is sometimes `ep->xfer_count`, sometimes `count`. Cap all of them with `PCD_MIN()` to limit their read length not exceeding `ep->maxpacket`. This way, during Data Out stage when `ep1` transmits audio, even if it receives a wrong audio packet length, your restriction will prevent it from writing data outside your buffer.

### During Development, You Might Need to Open the `.ioc` File with STM32CubeMX to Reconfigure Peripherals and Parameters
* If you click "Generate Code", all your modifications to `usbd_audio.c`, `usbd_audio.h`, `usbd_audio_if.c`, `usbd_audio_if.h`, `stm32f1xx_hal_pcd.c` will be overwritten. So you must use Git well, using Git to restore its overwrites.

## Outcome: Playing PCM Music with PWM Becomes "Ghost Music", Extremely Unpleasant
This is because PWM is fundamentally square waves. When its pulse width duty cycle is 50%, 48,000 Hz ultrasonic sound is inaudible to humans. But once you start using it to play music, its pulse width duty cycle changes. As a square wave, it introduces various other frequencies. According to Fourier, any discrete waveform point sequence can be represented by the same number of sinusoidal functions with different amplitudes, phases, and frequencies superimposed. So I've actually introduced a large number of harmonics, making it completely impossible to discern the melody in the music. Its frequency is chaotic: pitches that should be high become low-pitched; parts that should be low-pitched become high-pitched. Human voices can be heard enunciating words, you can hear "zh ch sh z c s" in pinyin, but the pitch is also very strange.

I probably need to find a way to specifically make a signal processing unit to convert PWM to PCM. I'm thinking about whether to use RC filtering or another method to process it, at least to first turn the square wave into a "rounded wave".
