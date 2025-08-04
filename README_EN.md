![LPTimer Framework](./pic/LPTimer框架more.png)

# LpTimer: A Software Timer Supporting Sleep at Fixed Intervals

*NOTE： This README is translated using GPT.* ([Chinese version](./README.md))

## Inspiration Behind LpTimer

After experiencing the `app_timer` module in the nRF SDK, I naturally wished for a similar software timer library when doing bare-metal low-power programming on other MCUs. A library that supports sleeping during fixed intervals would be ideal. However, after some searching, it seemed that only the tick-less mode in FreeRTOS offered similar functionality (yet the tick timer in FreeRTOS typically relies on `SysTick`, which cannot wake the MCU from stop mode on many chips).

As a result, I created the LpTimer software timer library, which has the following features:

- **(Theoretically) supports unlimited software timers with independent timing**
- **Allows the main controller to sleep in stop mode when no timer is running (lowest power mode while maintaining full RAM retention)**
- **Standalone software timer with minimal code footprint — only one `.h` file — suitable for RAM-constrained bare-metal programming**
- **Supports both RTC Alarm and regular LowPower Timer overflow as hardware timer sources**

## Porting LpTimer and Interface Overview

Since LpTimer is small (only the `LpTimer.h` file, ~318 lines including comments and whitespace), it follows the approach of the well-known [single-file C/C++ libraries like stb](https://github.com/nothings/stb/tree/master).

### How to Port LpTimer

1. You need to create a file that implements the underlying hardware timer interface functions for LpTimer (**It is recommended to copy `LpTimer_Implement_Template.c` and implement the bodies of the provided interface functions**). The required hardware timer interface functions are:

   - `static void HwLpTimer_Puase(void)` : Function to pause the hardware timer
   - `static void HwLpTimer_Start(void)` : Function to start the hardware timer
   - `static void HwLpTimer_SetWakeUpTime(uint32_t period)` : Set the timer interrupt period
   - `static void HwLpTimer_SetCnt(uint32_t counter)` : Set the current counter value of the hardware timer
   - `static uint32_t HwLpTimer_GetCnt(void)` : Get the current count value of the hardware timer
   - `static void HwLpTimer_WakeUpHook(void* p_context)` : Hook function called when any LpTimer wakes up (can be used to reconfigure clocks after waking from low-power mode)

   Note: Similar to stb-style single-file libraries, you must define the macro `LPTIMER_IMPLEMENT` **before** including `LpTimer.h` in the implementation file. Refer to `LpTimer_Implement_Template.c` for guidance.

2. Configure the settings in `LpTimer.h` based on your hardware timer. These configurations are also available in Keil's GUI configuration interface, as shown below:

   ![Keil Configuration](./pic/keil配置.png)

   Below is an explanation of each configuration option (they are also annotated in Keil’s UI): 
   
      - `HWLPTIMER_MAX_TIMEOUT` macro (`Define maximum timeout` option): The maximum overflow value of the hardware timer. It must be within 32 bits. The default is the maximum 32-bit value, 0xFFFFFFFF.
   
      - `HWLPTIMER_FREQ` macro (`Define timer frequency` option): The frequency of the hardware timer (i.e., the clock frequency at which the counter increments), in Hz. Default is 40000 (40 kHz). Must be within 100 Hz to 100 kHz.
   
      - `MS_OF_SAFETY_WINDOW` macro (`Define safety window` option): The duration of the safety window, in milliseconds. Default is 20 ms. It is recommended to be between 10 ms and 100 ms. When multiple timers have deadlines within the safety window, their callbacks will be executed together to improve efficiency.
   
      - `INIT_TIME_STAMP` macro (`Define initial timestamp` option): The initial timestamp of the timer system. Default is 0x00000000. It must be between 0 and `HWLPTIMER_MAX_TIMEOUT`. Usually, you can keep the default value.
   
      - `HWLPTIMER_MODE` macro (`Define hardware timer mode` option): Sets the working mode of the hardware timer, depending on its type:
        - Mode 1: RTC timer mode (continuously counting RTC)
        
        - Mode 2: Standard overflow event timer mode  
        
          Default is Mode 1. The main difference is how the counter behaves after an interrupt: in RTC mode, the counter continues counting; in overflow mode, the counter is assumed to reset to zero.
   
   3. Call `LpTimer_Execute()` within the hardware timer’s interrupt callback function to handle software timer events.
   
   At this point, the porting is complete. You can refer to the two porting examples under the `Examples` folder. The section [LpTimer Porting Examples](##LpTimer移植案例) below provides a brief explanation, and more details can be found in the [README file under Examples](./Examples/README.md).
   
   ### LpTimer API Usage
   
   - `MS2TICK(ms)`: Convert milliseconds to timer ticks, used as a parameter for `LpTimer_Start`.
   - `LPTIMER_IS_STOPED(timer)`: Check if a timer has stopped. Returns 1 if stopped, 0 if still running.
   
   - `void LpTimer_Create(LpTimer *timer, E_LPTIMER_MODE mode, void (*hook)(void))`  
     - Create a new low-power timer instance  
     - Parameters:  
       - `timer`: Pointer to the timer structure to initialize  
       - `mode`: Timer mode, either `E_LPTIMERMODE_ONCE` (one-shot) or `E_LPTIMERMODE_PERIODIC` (periodic)  
       - `hook`: Callback function to be called when the timer expires
   
   - `void LpTimer_Start(LpTimer *timer, uint32_t tick_of_timeout)`  
     - Start the specified low-power timer with a timeout  
     - Parameters:  
       - `timer`: Pointer to the timer structure  
       - `tick_of_timeout`: Timeout value in ticks. Use `MS2TICK` to convert milliseconds to ticks
   
   - `uint8_t LpTimer_Stop(LpTimer *timer)`  
     - Stop the specified low-power timer  
     - Parameters:  
       - `timer`: Pointer to the timer structure to stop  
     - Return value:  
       - 0 if the timer was successfully stopped  
       - 1 if stopping failed
   
   - `void LpTimer_Execute(void)`  
     - Execute the timer scheduler to process timer events  
     - Should be called in the hardware timer interrupt handler
   
   ### LpTimer Porting Examples
   
   Two example cases are provided:
   
   - One runs on an STM32F103C8T6 "Blue Pill" board using the RTC with Alarm interrupt as the timing source.
   - The other runs on the Air001 using a dedicated low-power hardware timer and timer overflow interrupts.
   
   RTC Alarm interrupts and low-power timer overflow interrupts were chosen because they are allowed as wakeup sources from STOP mode, according to the microcontroller datasheets.
   
   **Note:** LpTimer can also use general-purpose timers (e.g., TIM6 on STM32F103C8T6) as the timing source, but due to TIM6 cannot be used as a wakeup source in STOP mode, so if the MCU enters STOP sleep mode, LpTimer scheduling will not be able to wake it up.
   
   **LpTimer does not impose any requirement on which low-power mode to use — you can freely select the power mode based on your needs. However, if you use RTC Alarm or the overflow interrupt from most low-power timers in MCUs, it enables ultra-low power consumption while still waking up periodically for scheduled tasks.**
   
   ## Core Concept and Pros/Cons of LpTimer
   
   ### Core Concept
   
   Unlike traditional software timers (which are often based on polling in the main loop), LpTimer only triggers interrupts when a timer expires or when there is a pending task.
   
   **It maintains a sorted list and a 64-bit internal counter. Every time a new software timer is started or a periodic timer restarts, the timer is inserted into the list in chronological order. The hardware timer is then configured to expire at the nearest event's time. This mechanism ensures scheduled execution of tasks with minimal interrupt frequency.**
   
   ### Pros and Cons
   
   #### Pros:
   
   - Lightweight code with minimal footprint. It allows the system to stay in low-power mode and only wakes the MCU via interrupt when necessary to execute a timer callback.
   - Includes a "safety window" to prevent timer misses due to long callback execution durations.
   
   #### Cons:
   
   - Internally relies on a 64-bit counter. If the hardware timer runs at 40 kHz, the LpTimer time base could overflow in about 140 million years.
   - Currently, timer callback functions are executed inside the hardware timer ISR (Interrupt Service Routine). This can be modified to run in the main loop (like traditional polling frameworks), which ensures quick ISR exit but reduces the precision of callback timing.
   - Because the MCU needs to recover system clocks after waking up (LpTimer provides a hook function for this purpose), the actual timing precision after sleep/wake cycles is about 20–30 ms, and also depends on the task execution duration.
   
     For timing accuracy benchmarks, refer to the [README in the Examples folder](./Examples/README.md), where IO signal traces captured by a logic analyzer are used to measure software timer wake-up latency.