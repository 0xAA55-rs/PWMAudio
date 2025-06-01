# 使用 STM32F103C8T6 实现一个 USB 声卡

## 语言 Language

[English](JourneyFromBeginning.md)|简体中文

## 源码地址：

* GitHub：

[主号 https://github.com/0xAA55/PWMAudio](https://github.com/0xAA55/PWMAudio)
[小号 https://github.com/0xAA55-rs/PWMAudio](https://github.com/0xAA55-rs/PWMAudio)

* 国内可下载的源码地址：

[Gitee https://gitee.com/a5k3rn3l/pwmaudio](https://gitee.com/a5k3rn3l/pwmaudio)

三个仓库都是完全同步的，无差异。Gitee 上的仓库会默认显示中文版的 `Readme-CN.md`。

## 开发工具
### 软件：
* STM32CubeMX，这玩意儿帮助你选择芯片型号，然后选择要使用哪些外设和中间件，以及对芯片管脚的的配置等，最终帮你生成一份初始代码，供你进行修改。我已经用它生成了代码了，你从仓库直接 clone 我的代码就行了。
	* 如果你安装了 STM32CubeMX，你可以具体看到我对各种外设的配置，总系统时钟配置（USB 外设一定要配置成 48 MHz），TIM2 的配置，DMA 的配置，USB 的配置。
* STM32CubeIDE，这玩意儿是代码编辑器 + 下载器和调试器（配合 ST-LINK 设备）。你需要使用这玩意儿打开我的工程里面的 `.project` 文件，等它全部加载完，你就可以进行代码查看、编辑、修改、参数调整、编译（有 Debug 和 Release），下载和调试。我开启了 Release 配置的 `-flto` 优化，因此优化的效果应该是很理想的（可能会过度优化导致编译器认为你的代码一句也没卵用。要用好 `volatile` 关键字修饰一些必须读取或存储的寄存器）。优化方向上我开的是 `-O3`，它默认是 `-Os`，主要怕代码太大了撑爆 ROM，但其实不会有那么多代码。
* 音频播放器。咱开发的是声卡设备，要测试这个设备，我们就需要有一个播放器负责播放一些东西比如音乐或者录音等。
* 串口终端，理想的调试情况是：当你没条件进行单步调试和断点调试的时候（比如你要调试 USB 处理事务的时候，你把单片机停住的话，主机就认为 USB 通信失败了，你就不好调试）你可以用 `printf()` 来调试。你的单片机要 `printf()` 的时候，它的字儿从串口终端软件上显示出来。除非你有条件使用带有 SWO pin 的 ST-LINK，这个时候你可以用 SWO 接收单片机的 B3 pin 输出的调试信息，然后在 STM32CubeIDE 的 Console 里面显示出来。
	* 我用的是 [MobaXterm](https://mobaxterm.mobatek.net/download.html)，**UNREGISTERED VERSION**。一个是没啥必要去 Register，但是它会在闲置的时候冒出四个企鹅在你的终端界面上钻孔、挖地，导致终端界面上的文字内容不随时更新。另一个是它不给我 Register，因为它要我使用 PayPal 来支付，我没有合适的银行卡来支付，我得整一张 MasterCard。啥时候它支持微信支付了，我就 Register。
	* Hello! **I have a message for the MobaXterm team**. Could you please enable registration through WeChat Pay? As with the majority of Chinese developers, I don't have access to PayPal.
		* 我估计他们看不到我的这段消息。
	* [RealTerm](https://sourceforge.net/projects/realterm/) 其实也可以，但是它样子非常复古，像 20 年前的软件一样。偶尔它会因为驱动的问题或者 Windows UAC 权限的问题罢工。
* ST-LINK Utility（可选）这个软件允许你使用山寨（且不支持更新固件）的 ST-LINK-V2 去连接你的 STM32 单片机，然后可以烧录 ROM，查看 ROM 和 RAM，以及外设寄存器等，它使用表格的方式来显示一个一个的内存数据，你可以直接编辑这些数据，通过 **手艹外设寄存器** 的数值来使外设工作也是可以实现的，虽然这行为有点类似于约八十年前人们操作 ENIAC 计算机一样。

### 硬件：
* STM32F103C8T6 我用的是 `blue-pill`，检查它的 USB D+ 的上拉电阻值对不对，正常来讲，应该是 1KΩ 或者 1.5KΩ ，如果不对的话（比如你买到的是 10KΩ），你自己计算一下然后并联一个合适的电阻到 3.3V 电源。
* ST-LINK-V2，用于下载程序到 STM32F103C8T6，以及调试程序，记得一定要购买“可支持固件升级版”，要能支持 STM32CubeIDE 要求的固件升级功能。
	* 有这个就能单步调试，断点调试，各种调试。看寄存器，看内存，看结构体，看变量，看外设寄存器。
	* 但是这玩意儿没有 SWO 针，所以如果打算用 SWO（B3 管脚）输出 `printf()` 调试信息的话，你估计得买正版的 ST-LINK 下载器。不如用 USART 输出调试信息。
* 光耦合管，参数要求：一定要支持足够高的频率，至少要支持 48 kHz 频率，在此频率下要能输出基本正确的 PWM 脉宽。
	* 我测试了 [6N137](https://www.vishay.com/docs/84732/6n137.pdf)，好用，它的开关速度符合我的要求，但是娇贵，LED 端需要限制从 STM32 GPIO 出来的电流， **不得超过 20mA** 。它的输出电压是 5V，输出电流很低，不能驱动扬声器，但是可以驱动 MOS 管。对它的供电端需要并联滤波电容。具体自己看 [datasheet](https://www.vishay.com/docs/84732/6n137.pdf) 去。
		* `blue-pill` 的STM32 用的是板载的稳压管提供 3.3V 供电，GPIO 单口 PP 方式输出电流 **最大有 25mA**，这样的电流会使你的 6N137 性能降低。你需要自己计算一下阻值，然后串联一个合适大小的电阻到 LED 端，要不就使用 OD 方式输出，使用一个合适的上拉电阻，但是用 OD 方式的话你得让你输出的数据是反的才行，并且 OD 方式会导致你拉低管脚的时候它依然有微小的正电压输出，搞不好就可能会导致光耦合管被这点电压触发了。
	* 别的光耦合管我也试过，比如 [PC817C 217N](https://learnabout-electronics.org/Downloads/PC817%20optocoupler.pdf)，它的开关速度达不到 48 kHz，我给它输出 50% 占空比的 PWM 方波，示波器上看到的它的输出是一条轻微波动的直线。但是这款光耦合管支持比较大的电流的输出，某些场合甚至能直接用它输出的电流去驱动一些设备。它没有 6N137 那么娇贵，当然也没有 6N137 快。
	* 不一定要用光耦合管。我使用光耦合管是为了做到 **电气隔离**，也就是隔离开 STM32 MCU 的供电和给扬声器的供电，这样的话 MCU 可以获得稳定的 5V 供电，扬声器大口吮吸的大电流是额外的供电，不会影响到我的 MCU 的供电。不用光耦合管的话，那就直接用 GPIO 去怼 MOS 管，这个时候你的 MCU 的供电是不稳的。时刻准备好示波器，看你的 MCU 的供电的波动幅度怎么样，如果幅度比较大，MCU 它内部就会出奇奇怪怪的问题，比如运算错误，或者寄存器值错误等，甚至 PC 跑飞。这种情况等于是 MCU 因为供电不靠谱而变得不靠谱了，此时就得限制 MOS 管的输出电流（然后你的扬声器就有可能音量会比蚊子声音还小），还要在供电处并联滤波电容。
* MOS 管，要求支持的功率合适（比如 10W 以内）。上文有提到光耦合管的开关速度是有极限的，但是 MOS 管大多数都是能快速响应 Gate 管脚到 Source 管脚的电压变化的，基本都支持远超 48 kHz 的开关频率，没有这方面的顾虑。
	* 我用 [IRF510](https://www.vishay.com/docs/91015/irf510.pdf)，用螺丝固定到铝散热片上。我因为导热硅脂找不见了就干脆用螺丝拧紧它，让它紧贴散热片。有想过用焊锡去把它焊到散热片上，但是这样就需要用砂纸把铝散热片表面的氧化铝刮擦掉，然后再用锡膏和热风枪或者铁板烧来整个加热这坨玩意儿，可是**散热片又能把你的热风枪和铁板烧加热的热能给你散走**，而如果你把热风枪温度调太高、铁板烧温度调太高的话又有可能把 MOS 烫坏了，就很蛋疼。不加散热片也可以正常工作，但是弔烫，摸不得。加了散热片的话，它的样子都会好看一些，而且它会一直都是凉凉的。
* 电阻。我有一次试验的时候忘了加电阻，从 MOS 管出来的大电流直接把我的扬声器的振膜炸了。电大膜飞。
	* 另外电阻也弔烫，因为阻值低，MOS 管出来的电流大。
* CH340 串口转 USB 用来接收你的单片机用串口输出的调试内容。你要是有你自己爱用的串口转 USB 工具的话你就用你的。`115200` 波特率，8-bit 数据，1 起始 bit，1 停止 bit。你的单片机串口也以这样的参数通信，就可以了。
* 因为会需要很多的 USB 口（至少三个），最好弄一个 USB Hub，并且这个 Hub 要能具备独立的供电。
* 因为用光耦合管做电气隔离，扬声器端的供电需要一个 USB 充电头来供电。这个 USB 充电头插到市电上，然后你自己做一条 USB 线，插到这个充电头上，另一头用连接器连接到你的洞洞板上的扬声器端的供电上。
* 杂项：洞洞板，飞线，剥线钳，剪甲沟炎用的钳子（非常适合用来剪线缆和杜邦针等细的结实的金属），焊台，热风枪，热收缩管，焊锡，铜丝球，杜邦线，杜邦针，接线器，USB 头，老虎钳或者其它夹具（用于固定电路板进行焊接），一堆可能用得到用不到的瓷片电容，电阻。我不爱用电解电容，它炸。
* 良好的通风环境，或者通风环境不良好但是 **你的头部可以灵活躲位** 、躲开焊锡冒出来的烟的话，你就不会因为吸入过多的铅而导致皮肤和大脑受损了。

## 开发的过程

### 思路

* 主机不断给我发送 PCM 音频，格式是 16 位有符号整数作为音频样本，左右声道交替存储样本。
* 我使用 PWM 来驱动扬声器，我要把主机上传递给我的音频样本转换为我这边的 PWM 的脉宽值的数组，然后使用 DMA 转圈圈模式把脉宽值数组不断地写入到 TIM 的 PWM 通道里面的脉宽值寄存器上。
* 因为我要驱动两个扬声器播放这左右声道的音频，所以我需要 TIM 的两个 PWM 通道，每个 PWM 通道的脉宽数据又要由独立的 DMA 通道来提供，这样的话我就需要两个 PWM 脉宽数组，作为 DMA 传输的数据来源。
* 设定 DMA 以「转圈圈模式」`DMA_CIRCULAR` 运行，每传输到一半的时候，调用我的一个回调函数；传输完一整个缓冲区后，再次调用我的另一个回调函数。我的回调函数干嘛呢？我回调函数负责把主机发过来的 PCM 样本转换为我这边的 PWM 脉宽，分成左右两个声道，将其数值范围从 `-32768..32767` 缩放到我的 PWM 脉宽范围内，按声道分别存放到我的两个 PWM 缓冲区里。这样 DMA 就可以源源不断地把 PCM 转换成的 PWM 脉宽输出给 TIM 的 PWM 脉宽寄存器上了。
	* 典型的双缓冲播放模式。在播放一个缓冲区的时候，更新另一个缓冲区的数据；等到播放到另一个缓冲区了，再去更新上一个缓冲区的数据，如此往复就可以无缝连续播放。
* 理论上来讲，STM32F103C8T6 工作在 72 MHz，如果要用 PWM 播放 48000 Hz 的音频，PWM 的脉宽数值范围就只能在 1500 以内。这样一来，理论上音质是降低到差不多 10 bit 到 11 bit 位深的程度。

### 我的代码上手过程
STM32CubeMX 生成的 USB Audio Class 的例子代码都是需要你来修改的。重点看以下几个代码，这几个代码是需要修改的：
* `USB_DEVICE\App\usbd_audio_if.c`
* `USB_DEVICE\App\usbd_audio_if.h`
* `Middlewares\ST\STM32_USB_Device_Library\Class\AUDIO\Src\usbd_audio.c`
* `Middlewares\ST\STM32_USB_Device_Library\Class\AUDIO\Inc\usbd_audio.h`

除此以外，`Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_pcd.c` 这玩意儿里面有很多 BUG 要修。

### 串口调试条件的搭建
你还需要你的设备能走串口输出调试信息，并使用。我选了 USART1，使用 `A9`、`A10` 作为 TX 和 RX，但是它的 RX 的 DMA 通道被占了。干脆不用 DMA。波特率才 `115200` Hz，使用中断处理程序一个字节一个字节读取是足够的。

`A9` 接 CH340 的白线，`A10` 接 CH340 的绿线。CH340 插你主机 USB 口上。启动你的串口终端软件，打开 CH340 USB 转串口设备的 COM 口。

#### 完成标准输入输出系统
STM32 使用的 C 标准库的 `stdio.h` 有一些基础的标准输入输出函数的实现，比如 `printf()` 和 `scanf()`，但是它们到最终、最底层是靠调用 `__io_putchar()` 和 `__io_getchar()` 来写入和读取数据的。STM32CubeMX 生成的初始代码里有 `syscalls.c`，里面提供了这两个函数的弱符号实现，其行为就是啥也不干。我们把它里面的一些函数实现出来以后，标准输入输出就能用了。

不过我们首先需要实现一个 FIFO 缓冲区结构体模块，实现一系列的函数用于读写 FIFO。实现后，整两个这个结构体实例，专门负责充当你的 STDIN 和 STDOUT 的缓冲区，起名：`fb_in` 和 `fb_out`。再把以下几个函数实现了（我直接把它们实现在了 `main.c` 里）：
* `int  __io_getchar(void);` 把它实现为从 `fb_in` 里面读取一个字节返回出去，如果 `fb_in` 里没有数据，就返回 `EOF`。
	* UART 那边 RX 传输数据进来的时候，你把数据存到你的 `fb_in` 里。
* `int  _read(int file, char *ptr, int len);` 把它也实现为从 `fb_in` 里面读数据出去。这个函数用于批量的读数据。返回实际能读取的字节数。如果你的 `fb_in` 空了，就返回 `0` 。
	* 你如果不实现这个函数，它就会调用 `syscalls.c` 里面的默认实现。默认实现的内容就是循环调用 `__io_getchar()`，效率比较低。
* `int  __io_putchar(int ch);` 把它实现为把 `ch` 存到你的 `fb_out` 里。
	* 你在 `main()` 的主循环里负责判断你的 `fb_out` 是否有数据，是的话取出来用串口输出。
* `int  _write(int file, char *ptr, int len);` 把它实现为把 `ptr` 指向的长度为 `len` 的一段数据写入到你的 `fb_out` 里，返回实际写入的字节数量。如果 `fb_out` 被写满了，就返回 `0`。
	* 你如果不实现这个函数，它就会调用 `syscalls.c` 里面的默认实现。默认实现的内容就是循环调用 `__io_putchar()`，效率比较低。

这样一来，你的 `printf()` 就会调用你的 `_write()` 把需要输出的内容存入你的 FIFO，而你的主循环则会每循环一次就把你 FIFO 里的需要输出到串口的数据全部都输出出去，你的主机上的串口终端软件就会显示你 `printf()` 打印出来的内容。

而你在主机上的串口终端软件上打字的时候，主机上插着的 CH340 就会把你输入的内容传输到你的 RX 上，你的 UART 收数据的回调函数负责把这些数据存到你的 FIFO 里。然后你在主循环里用 `scanf()` 之类的函数就可以把数据从 FIFO 里读出来，这样的话你就实现了串口的交互了，然后你就可以自己发明一个命令行界面。

#### 关于 UART 的数据接收：
你需要实现 `void  HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);` 这个函数，并且在外面定义一个全局变量 `uint32_t uart_buf = 0;`。当你的这个函数被调用的时候，你的 `uart_buf` 的值就是串口 RX 收到的一个字节。你要把这个字节写入到你的 `fb_in` 里，然后调用 `HAL_UART_Receive_IT(huart, (uint8_t*)&uart_buf, 1);` 后再返回。

为什么要调用 `HAL_UART_Receive_IT(huart, (uint8_t*)&uart_buf, 1);` 呢？因为调用这个函数后，一旦 UART 那边有数据要接收了，相关中断就会触发，这个中断处理过程就会把数据写入到 `uart_buf` 里，再调用你的 `HAL_UART_RxCpltCallback()` 实现。因此你在实现 `HAL_UART_RxCpltCallback()` 的时候也要调用 `HAL_UART_Receive_IT(huart, (uint8_t*)&uart_buf, 1);` 来继续接收下一个字节。

所以在 `main()` 的初始化过程中，要加一句 `HAL_UART_Receive_IT(huart, (uint8_t*)&uart_buf, 1);` 来启动 UART 的接收逻辑。

### 接线

- 5V: 接 USB 5V（标准 USB 规范里的红线）。
- G: 接 USB GND（标准 USB 规范里的黑线）。
- PA0: PWM 输出到光耦合管，光耦合管输出到场效应管放大器（声道1）
- PA1: PWM 输出到光耦合管，光耦合管输出到场效应管放大器（声道2）
- PA9: 串口输出，接 CH340 的白线。调试用。
- PA10: 串口输入，接 CH340 的绿线。调试用。
- PA11: USB D- （标准 USB 规范里的白色信号线）
- PA12: USB D+ （标准 USB 规范里的绿色信号线）需要 1.5kΩ 上拉电阻到 3.3V 电源。
- PA13: SWDIO (ST-Link 调试器用，调试、烧写程序的时候用)*
- PA14: SWCLK (ST-Link 调试器用，调试、烧写程序的时候用)*
- PC13: 开漏模式接 LED（`blue_pill` 的板子已经给你接好了，并上拉了电阻），这个 LED 你可以控制它闪烁，用作一种调试输出的方式。


### 填坑的过程

#### 它默认的音频接收缓冲区的大小大到离谱
* 在 `usbd_audio.h` 里面有个结构体，`USBD_AUDIO_HandleTypeDef` 它是这样的：
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
	* 你猜 `AUDIO_TOTAL_BUF_SIZE` 是几？猜对啦！是一个亿！直接运行就触发 `HardFault`，因为你写到了 RAM 的外面去了。
		* 实际数值是 `AUDIO_OUT_PACKET * AUDIO_OUT_PACKET_NUM`
		* `AUDIO_OUT_PACKET` 是采样率 `48000` 乘以 `2` 再乘以 `2` 再除以 `1000` 等于 `192`。
		* `AUDIO_OUT_PACKET_NUM` 是八十。
		* 因此这个缓冲区的大小是 `15360` 字节，十六进制是 `0x3C00`，你的 RAM 本来就只有 `0x5000` 个字节的大小，而你自己还要维护你自己的 PWM 缓冲区，你的 PWM 缓冲区的样本数要和它的缓冲区的样本数对齐，它的缓冲区样本数是 `7680`，每声道 `3840` 个样本，你需要左右两个声道也是每个声道 `3840` 个样本，你也要存 `7680` 个样本，你的 PWM 缓冲区总共加起来也要占 `15360` 字节，你的 buffer 和它的 buffer 加起来要占 `0x7800` 字节，远超你的 RAM 总量。
	* 将 `AUDIO_OUT_PACKET_NUM` 从八十改到十，你的 buffer 和 它的 buffer 加起来是 `0xC00` 字节，问题解决。
	* 这个结构体的 `rd_enable`、`rd_ptr` 没有什么卵用。

#### 它默认的设备配置描述符不合理，不让设置音量
* 在 `usbd_audio.c` 有设备配置描述符，也就是 `USBD_AUDIO_CfgDesc` 这个东西。你需要阅读 [USB Audio Class 的标准规范](https://www.usb.org/sites/default/files/audio10.pdf) 去理解这个东西，并将其参数改为你需要的参数。这玩意儿改错了的话，你的设备插电脑上，电脑就不认你这个设备作为一个 USB 声卡。
	* 注意看这一段：
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
		* 这段描述符是你的 USB 输入终端描述符，`wChannelConfig` 得改成 `0x0003`。这个是一个位域，`0x0003` 表示你支持左声道和右声道（Dolby™立体声系统里面的所谓左前和右前扬声器）。对应的字节改成 `0x03`，后面跟着 `0x00`，就对了。
	* 再来看这一段：
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
		* 它有问题：因为之前的 `USB Speaker Input Terminal Descriptor` 没配置好，所以这里它也是瞎几把填的，它的意思就是你的声卡只支持是否开启静音模式。正常的声卡肯定是要支持调节音量的呀，而且正常你需要三个 `bmaControls` 位域，一个控制主音量和静音开关，另外两个分别控制左右声道的音量。需要改成下面这样：
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
	* 发现没，改了后，它多了一个字节。那你就要把 `USB_AUDIO_CONFIG_DESC_SIZ` 的值增加 `1`。
* 这个描述符 `USBD_AUDIO_CfgDesc` 的完整定义是这样写的：
```
__ALIGN_BEGIN static uint8_t USBD_AUDIO_CfgDesc[USB_AUDIO_CONFIG_DESC_SIZ] __ALIGN_END = { ... };
```
* 看到它有 `static` 修饰，但是这个东西的内容是不会去被改动的，你是不是会想：哎！这东西它不是 `const` 修饰的呀，它会被放到 `.data` 段里，然后就会占用我的 RAM，那我 ROM 也存一份这个，RAM 也存一份这个，这得浪费我多少 RAM？
	* 实际上你要是加上了 `const`，你确实节省了 RAM，也就 `110` 个字节。但是 STM32 的 USB 外设在向主机提交设备配置描述符的时候，**它只能从 RAM 里提交**，否则就触发 `HardFault`。
	* 这表看起来长，但其实它只是因为这种写法显得好像很长。
* 按照这个表的描述，你的 USB 端点 `0` 是你的控制端点，负责接收控制信号，在 Setup 阶段响应主机的请求，然后把主机需要的数据发送出去。你的 USB 端点 `1` 是你用来接收音频数据的端点，这个端点的同步方式是 `USBD_EP_TYPE_ISOC`，也就是只要主机有软件要播放音频，它就开始不断地通过 USB 传输音频数据到你的端点 `1` 上。
	* 这个表有一个 `Interface`，但是有两个 `Interface Descriptor`，其中一个是 `Audio Streaming Zero Bandwith`，另一个是 `Audio Streaming Operational`。这个 `Audio Streaming Zero Bandwith` 的作用就是当主机没有任何的音频在播放的时候，它就不发送音频数据到你的 `ep1` 端点；而当主机有音频要播放的时候，它就开始发送音频数据到你的 `ep1` 端点。

#### 它默认的 Setup 事务处理是瞎几把写的
* 在 `static uint8_t  USBD_AUDIO_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)` 这个函数里，你需要注意这一段代码：
	```
	switch (req->bmRequest & USB_REQ_TYPE_MASK)
	{
		case USB_REQ_TYPE_CLASS :
			switch (req->bRequest)
			{
				case AUDIO_REQ_GET_CUR:
					AUDIO_REQ_GetCurrent(pdev, req);
					break;

				case AUDIO_REQ_SET_CUR:
					AUDIO_REQ_SetCurrent(pdev, req);
					break;

				default:
					USBD_CtlError(pdev, req);
					ret = USBD_FAIL;
					break;
			}
			break;
	```
	* 哎哟哟，它这里的 `AUDIO_REQ_GET_CUR` 和 `AUDIO_REQ_SET_CUR` 简直就是应付了事。这部分的代码是在 USB 端点 `ep0` 的 Setup 阶段收到数据后执行的代码，还记得之前我改了设备配置描述符表里面的 `USB Speaker Audio Feature Unit Descriptor` 吗？我设置我可以调主音量和静音，以及左音量和右音量。这里的 `Get Cur` 和 `Set Cur` 就是负责响应主机要调音量、要调是否静音的请求的。`Get Cur` 负责从你这里读取你的当前音量或者静音等参数，`Set Cur` 负责写入这些参数。
	* 按照 [USB Audio Class 规范](https://www.usb.org/sites/default/files/audio10.pdf) ，你除了可以实现 `Get Cur`、`Set Cur` 以外，你还可以让它能够 `Get Min`、`Get Max`、`Get Res`（Res 指的是 Resolution，也就是你的数值的颗粒度），如果你足够闲的蛋疼，你还可以把 `Set Min`、`Set Max`、`Set Res` 的请求都做了。
	* 来到它的 `AUDIO_REQ_GetCurrent()` 的实现这里，你会看到这样的不明所以的代码：
	```
	static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
	{
		USBD_AUDIO_HandleTypeDef   *haudio;
		haudio = (USBD_AUDIO_HandleTypeDef *) pdev->pClassData;

		memset(haudio->control.data, 0, 64U);

		/* Send the current mute state */
		USBD_CtlSendData(pdev, haudio->control.data, req->wLength);
	}
	```
	* 它根本就没有判断主机想要 get 什么东西的 current 值，直接发送一堆零字节。
	* 我这边使用了几个全局变量存储了我的参数，其中包括：
		* `is_muted_all`
		* `volume_all`
		* `volume_l`
		* `volume_r`
		* `max_volume`
	* 这部分我的代码是这样写的：
	```
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
	```
	* 上面的那部分代码是插入 USB 的时候在 Setup 阶段执行的。而在已经插好了 USB，主机认了声卡后，依然也会有 `Get`、`Set` 的请求的，因为你要调音量呀，你要调是否静音啊。这个时候，以下的代码被调用：
	```
	static uint8_t  USBD_AUDIO_EP0_RxReady(USBD_HandleTypeDef *pdev)
	```
	* 默认的实现写得非常烂，是这样的：
	```
	static uint8_t  USBD_AUDIO_EP0_RxReady(USBD_HandleTypeDef *pdev)
	{
		USBD_AUDIO_HandleTypeDef   *haudio;
		haudio = (USBD_AUDIO_HandleTypeDef *) pdev->pClassData;

		if (haudio->control.cmd == AUDIO_REQ_SET_CUR)
		{
			/* In this driver, to simplify code, only SET_CUR request is managed */

			if (haudio->control.unit == AUDIO_OUT_STREAMING_CTRL)
			{
				((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->MuteCtl(haudio->control.data[0]);
				haudio->control.cmd = 0U;
				haudio->control.len = 0U;
			}
		}

		return USBD_OK;
	}
	```
	* 功能也就只有一个设置是否静音的功能，而且代码写得很繁琐，用一个局部指针变量存储一下 `userdata` 不好么？我的实现是这样的：
	```
	static uint8_t USBD_AUDIO_EP0_RxReady(USBD_HandleTypeDef *pdev)
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
	```
	* 这样的话，基本上各种请求都正常回应了。然后再看，我的 `Get Cur` 是这样写的：
	```
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
	```
	* 它请求我 `Set Cur` 的代码我是这样写的：
	```
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
	```
	* 我给它的 `control` 结构体加料了，它结构体我是这样声明的：
	```
	typedef struct
	{
		uint8_t cmd;
		uint8_t data[USB_MAX_EP0_SIZE];
		uint8_t len;
		uint8_t unit;
		uint8_t channel;
		uint8_t feature_control;
	}
	USBD_AUDIO_ControlTypeDef;
	```
	* 至于那些 `Get Max`、`Get Min`、`Get Res` 之类的也是一样的逻辑。音量的话，它 `Get Max` 的时候我返回 `100`；静音开关的话，毕竟只是个开关，它 `Get Max` 的时候我返回 `1` 就行了。当它要 `Set Cur` 设置音量的时候，`1` 表示开启静音，`0` 表示关闭静音。
	* 值得注意的是我调用了 `userdata->MuteGet(&data);`、`userdata->VolumeGet(channel, &data);` 这样的函数。这个 `userdata` 是这样一个结构体：
	```
	typedef struct
	{
		int8_t (*Init)(uint32_t  AudioFreq, uint32_t Volume, uint32_t options);
		int8_t (*DeInit)(uint32_t options);
		int8_t (*AudioCmd)(uint8_t *pbuf, uint32_t size, uint8_t cmd);
		int8_t (*VolumeCtl)(uint8_t vol);
		int8_t (*MuteCtl)(uint8_t cmd);
		int8_t (*PeriodicTC)(uint8_t cmd);
		int8_t (*GetState)(void);
	} USBD_AUDIO_ItfTypeDef;
	```
	* 看见没，就是一堆回调函数。我给它增加了两个成员，一个是 `MuteGet`，一个是 `VolumeGet`。这样就既能设置音量和静音，又能获取音量和静音了。
	```
	typedef struct
	{
		int8_t (*Init)(uint32_t  AudioFreq, uint32_t Volume, uint32_t options);
		int8_t (*DeInit)(uint32_t options);
		int8_t (*AudioCmd)(size_t offset, uint8_t cmd);
		int8_t (*VolumeCtl)(uint8_t channel, uint8_t vol);
		int8_t (*MuteCtl)(uint8_t cmd);
		int8_t (*VolumeGet)(uint8_t channel, uint8_t *vol);
		int8_t (*MuteGet)(uint8_t *cmd);
		int8_t (*PeriodicTC)(uint8_t cmd);
		int8_t (*GetState)(void);
	} USBD_AUDIO_ItfTypeDef;
	```
	* 这里面对应的每一个函数都要在 `usbd_audio_if.c` 里面实现，并填写这个结构体传递回去。
	* 再看我的 `VolumeCtl` 的实现：（在 `usbd_audio_if.c`）我从 `USBD_SetupReqTypedef *req` 里的 `req->wValue` 里读出了它要 `Get` 的东西，以及对应的声道（`0` 是主声道，`1` 是左声道，`2` 是右声道）。
	```
	static int8_t AUDIO_VolumeCtl_FS(uint8_t channel, uint8_t vol)
	{
		/* USER CODE BEGIN 3 */
		printf("AUDIO_VolumeCtl_FS: %u, %u\r\n", (unsigned  int)channel, (unsigned  int)vol);
		switch (channel)
		{
		case 0: volume_all = vol; return  USBD_OK;
		case 1: volume_l = vol; return  USBD_OK;
		case 2: volume_r = vol; return  USBD_OK;
		default: return  USBD_FAIL;
		}
		/* USER CODE END 3 */
	}
	```
	* 其余函数的实现也类似，就是判断一下它要 `Get`、`Set` 啥，然后哪个声道，处理一下就好了，比如设置我的全局变量，或者返回我的全局变量值给主机。

#### 它默认实现的 `USBD_AUDIO_DataOut()` 和 `USBD_AUDIO_Sync()` 写了很多不明所以的代码
* 我们接收音频数据是在 `ep1` 的 DataOut 阶段，此时这个函数被调用：
	```
	static uint8_t USBD_AUDIO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
	```
	* 还记得上面我提到的结构体 `USBD_AUDIO_HandleTypeDef` 吗？这个结构体里有 PCM buffer，而这个 `USBD_AUDIO_DataOut()` 就专门负责接收 PCM 音频数据到这个 buffer 里。我提到它的 `rd_enable` 和 `rd_ptr` 没卵用。确实就是没卵用。它的源码太垃圾了我都懒得放在这里霸屏。而且还有 BUG，会缓冲区越界。我也是服了。它其实想实现一个双缓冲的逻辑，也就是判断 `wr_ptr` 是否到达了缓冲区的一半，是的话调用 `usbd_audio_if.c` 里面的 `AudioCmd()` 告诉你半个缓冲区好了；然后再看 `wr_ptr` 是否到达了缓冲区末尾，是的话也是调用 `AudioCmd()` 告诉你另外半个缓冲区好了。它的 `rd_enable` 就是在它接收了半个缓冲区的音频数据后，设为 `1`，意思是你可以读取了。而它的 `rd_ptr` 是干嘛的我还有点说不上来，大概就是告诉你读取到缓冲区的哪里了，然后调用  `AudioCmd()` 让你去播放「可读的部分」。非常繁琐而且没必要。
* 我的是这样写的：
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
* 还有 `USBD_AUDIO_Sync()` 的实现：
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
	* 不就是个双缓冲逻辑么？另外其实我的 `AudioCmd()` 只处理 `AUDIO_CMD_START` 命令，也就是先暂停 TIM，重置 DMA 的读取位置，把整个 `buffer` 转换为我的 PWM buffer，再启动 TIM 开始播放。双缓冲逻辑我已经在 DMA 那里做好了，而这里是 USB 事务调用的函数，在这里转换样本格式会导致你处理事务太慢，无法及时向主机反馈你的情况，导致 USB 通讯失败。

#### 修理 `stm32f1xx_hal_pcd.c` 的问题
* 这个源码文件负责响应 USB 中断，从 USB 外设读取状态，处理 Setup、Data In、Data Out 事务，提供一堆函数和宏负责艹 USB 外设寄存器。但是 USB 外设有 bug，你从它的 `ep` 里读取「主机发给我的数据的量」的时候，会读出奇奇怪怪的量，比如 Setup 包的长度按照规范必须是 `8`，但是你偶尔会收到比如 `40`、`64`、`1023` 等错误的长度。而它的默认实现就是不管收到的长度是多少，就比如是 Setup 包，它就一股脑地全部往 `hpcd->Setup` 里面写。这 `hpcd` 是 `PCD_HandleTypeDef` 结构体，它里面除了 `Setup` 这个成员（定长数组）以外，还有别的很多重要的成员，这样一来，这些因传输错误而得到的垃圾数据就会一股脑覆盖掉这个结构体的 `Setup` 以及后面的其它的成员，而且因为取长度寄存器的值的时候它有限制，你甚至不会因为写出到 RAM 外而引发 `HardFault`，你只是把你的 RAM 整的乱七八糟，并且发现这个结构体里面的东西奇奇怪怪的，不像是正常的数值。不熟悉 C 语言的数组越界写入的后果的程序员朋友们遇到这种问题肯定是要调试很久很久很久才能发现问题的了，要不就是会怀疑有鬼。
* 找到这个函数：`static  HAL_StatusTypeDef  PCD_EP_ISR_Handler(PCD_HandleTypeDef *hpcd)`
	* 找到它对 `USB_ReadPMA()` 的第一个调用：
	`USB_ReadPMA(hpcd->Instance, (uint8_t *)hpcd->Setup, ep->pmaadress, ep->xfer_count);`
		* 注意，这里是在处理 Setup 事务的时候，读取 Setup 包的。这里的 `ep->xfer_count` 正常情况下是 `8`，但是它如果不是 `8`，你得进行丢包处理。而且你需要在这句 `USB_ReadPMA()` 的函数调用的前面，去判断这个 `ep->xfer_count` 的值。你可以定义一个局部变量来限制它的大小，比如这样：
		`size_t max_xfer_count = PCD_MIN(ep->xfer_count, sizeof hpcd->Setup);`
		* 然后将函数调用改为：
	`USB_ReadPMA(hpcd->Instance, (uint8_t *)hpcd->Setup, ep->pmaadress, max_xfer_count);`
		* 这样至少你可以分析它到底读出了什么内容，并且你要做丢包处理。所谓丢包处理，就是不要让它去调用 `HAL_PCD_SetupStageCallback()` 来进行 Setup 事务的处理，因此你要用一个 `goto` 去跳到这个函数后面去。
	* 找到后面的所有的 `USB_ReadPMA()` 的调用。它的最后一个参数有的是 `ep->xfer_count`，有的是 `count`，全部做限制，用 `PCD_MIN()` 去限定它的读取长度不能超过 `ep->maxpacket`，这样的话，在 Data Out 阶段走 `ep1` 传输音频的时候，即使它读到了错误的音频数据包长度，也会因为你的限制而防止它越界把数据写出你的缓冲区之外。

#### 开发的中途你可能需要使用 STM32CubeMX 打开 `.ioc` 文件重新调整外设、参数
* 你如果点了「Generate Code」，你对 `usbd_audio.c`、`usbd_audio.h`、`usbd_audio_if.c`、`usbd_audio_if.h`、`stm32f1xx_hal_pcd.c` 的全部改动都会被它覆写。因此一定要用好 Git，利用 Git 恢复它的覆写。

## 结局：用 PWM 播放 PCM 音乐会变成「阴乐」，非常难听

这是因为 PWM 归根结底是方波，当它脉冲长度占空比 50% 的时候，48000 Hz 超声波是人听不到的，但是你一旦开始用它播放音乐，它的脉冲长度占空比就会发生变化，作为方波，它就会引入其它各种频率的波形进来。按照傅里叶的说法，任何离散的波形点构成的波形都可以用一样数量但振幅、相位、频率不同的正弦函数叠加而成。所以我其实引入了大量的谐波，导致完全无法听出音乐里面的旋律，它的频率是混乱的，本来应该是高频的音调它很低沉；本来应该是低沉的部分它很高亢，而人声则能听出人吐字、能听出 zh ch sh z c s 的拼音，但是音调也是非常奇怪的。

估计我得想办法专门做一个信号处理单元把 PWM 转换为 PCM，这样就成了。我在想是使用 RC 方式还是使用别的方式来处理，至少先把方波变成「圆波」吧。
