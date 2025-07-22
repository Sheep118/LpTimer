#ifndef LPTIMER_H
#define LPTIMER_H

/**************include******************/
#include <stdint.h>
#include <stdio.h>

/**************Type def******************/
typedef enum
{
    E_LPTIMERMODE_ONCE = 0x00, // 单次模式
    E_LPTIMERMODE_PERIODIC = 0x01, // 周期模式
} E_LPTIMER_MODE; 
typedef struct _LpTimer
{
    void (*taskFunction)(void); // 任务函数指针
    struct _LpTimer *next; // 指向下一个任务的指针
    uint64_t deadline; // 定时器超时时间戳
    uint32_t period; // 定时器周期(唤醒的时间间隔)
    E_LPTIMER_MODE mode:4; // 定时器模式
    uint8_t is_running; // 定时器是否在列表中
} LpTimer;

/**************Macro Setting******************/
// <<< Use Configuration Wizard in Context Menu >>>

// <h> Hardware low-power timer configurations and settings

// <o> Define maximum timeout (32bit)
// <i> Default: 0xFFFFFFFFUL, Hardware timer overflow value
#define HWLPTIMER_MAX_TIMEOUT 0xFFFFFFFFUL

// <o> Define timer frequency (Hz) <1000-100000:1000>
// <i> Default: 40000 (40kHz), Hardware timer Frequency
#define HWLPTIMER_FREQ        40000

// <o> Define safety window (ms) <10-100:1>
// <i> Default: 20ms, Safety window duration, recommended between 10ms to 100ms (tasks with deadlines close to safety window will be executed together)
#define MS_OF_SAFETY_WINDOW   20

// <o> Define initial timestamp 
// <i> Default: 0x00000000, Initialize the timestamp and set it to 0 by default(The range is between 0 and HWLPTIMER_MAX_TIMEOUT)
#define INIT_TIME_STAMP       0x00000000

// <o> Define hardware timer mode
//      <1=> RTC Timer (Continuous counting RTC)
//      <2=> Normal overflow event timer
// <i> Default: 1, There are two types of hardware timers: continuous counting RTC and general overflow interrupt timer, which are different from whether the interrupt will start counting again from 0 after the interrupt occurs
#define HWLPTIMER_MODE        1

// </h>

// <<< end of configuration section >>>

/**************Macro Function******************/
/**
 * @brief 用于LpTimer_Start的参数，将毫秒转为tick
 * @param ms 毫秒 
 */
#define MS2TICK(ms)             (((ms) * (HWLPTIMER_FREQ)) / 1000) // 毫秒转换为tick
/**
 * @brief 用于判断该定时器是否停止
 * @param timer 定时器指针
 * @return 1已经停止，0该定时器还在运行 
 */
#define LPTIMER_IS_STOPED(timer) (!((timer)->is_running))

/**************Function******************/
#ifdef __cplusplus
extern "C" {
#endif
    
/**
 * @brief 创建一个新的低功耗定时器实例。
 * 
 * @param timer 指向要初始化的低功耗定时器结构体指针。
 * @param mode 定时器模式（参见 E_LPTIMER_MODE）。
 * @param hook 定时器到期时调用的回调函数。
 */
void LpTimer_Create(LpTimer * timer, E_LPTIMER_MODE mode, void (*hook)(void));
/**
 * @brief 启动指定的低功耗定时器，并设置超时时间。
 * 
 * @param timer 指向低功耗定时器结构体的指针。
 * @param tick_of_timeout 超时时间，单位为tick, 需要使用MS2TICK宏进行毫秒到tick的转换。
 */
void LpTimer_Start(LpTimer *timer, uint32_t tick_of_timeout);
/**
 * @brief 停止指定的低功耗定时器。
 * 
 * @param timer 指向低功耗定时器结构体的指针。
 * @return uint8_t 返回0表示定时器成功停止，返回1表示失败。
 */
uint8_t LpTimer_Stop(LpTimer *timer);
/**
 * @brief 执行定时器调度器，处理定时器事件(放在中断函数中处理)
 */
void LpTimer_Execute(void);

#ifdef __cplusplus
}
#endif

#ifdef LPTIMER_IMPLEMENT

//以下函数为硬件相关的接口函数定义，需要用户实现函数体内容
static void HwLpTimer_Puase(void);  // 暂停定时器
static void HwLpTimer_Start(void); // 启动定时器
static void HwLpTimer_SetWakeUpTime(uint32_t period); // 设置定时器周期
static void HwLpTimer_SetCnt(uint32_t counter); // 设置定时器周期
static uint32_t HwLpTimer_GetCnt(void); //得到当前定时器的计数值
static void HwLpTimer_WakeUpHook(void* p_context); //唤醒任何一个LpTImer后会执行的hook函数 (可以用于低功耗模式下配置唤醒后的时钟)


//以下是具体的实现代码
#define TICK_OF_SAFETY_WINDOW   MS2TICK(MS_OF_SAFETY_WINDOW)
// 定义安全窗口(为了防止hook函数执行时间太长，导致定时器头结点的时间已经小于硬件定时器当前时间戳)
// 在执行hook时，如果发现其时间距离当前时间戳小于安全窗口，则立即执行该函数(即在这个窗口内的hook都会被认为同一时间执行)
// 同时，当发现设置的目标时间小于硬件定时器当前时间时，会直接将当前时间加上一个安全窗口作为唤醒时间
#define GET_BASE_COUNTER(x) ((x)&(~HWLPTIMER_MAX_TIMEOUT))
#define HWLPTIMER_MODE_RTC_TIMER        0x01
#define HWLPTIMER_MODE_UPDATE_TIMER     0x02 
//用于保存数据的全局变量
static LpTimer *timerList = NULL; // 定时器列表
static uint64_t currentTime = INIT_TIME_STAMP; // 当前时间戳
static uint64_t targetTime = 0; // 设定硬件定时器的目标时间
#if(HWLPTIMER_MODE == HWLPTIMER_MODE_UPDATE_TIMER)
static uint64_t lastInterruptTime = 0; //上次中断的时间戳 
#endif //(HWLPTIMER_MODE == HWLPTIMER_MODE_UPDATE_TIMER)

static uint8_t LpTimer_Insert2List(LpTimer * timer)
{
    LpTimer *p = timerList;
    if(!(timer->is_running)){ //没有正在运行的定时器才可以加入列表中
        timer->is_running = 1;
        if(p == NULL){
            timerList = timer; // 如果任务列表为空，将当前任务设置为第一个任务
        } else {
            LpTimer * q = p->next;
            if(p->deadline >= timer->deadline){ // 如果当前任务的deadline小于等于第一个任务的deadline
                timer->next = p;
                timerList = timer; //插入到头部
            }else {
                while(q != NULL) {
                    if(q->deadline < timer->deadline){ // 如果当前任务的deadline大于下一个任务的deadline
                        p = q;
                        q = q->next; //指针后移
                    }else { //timer > p; timer <= q
                        p->next = timer;
                        timer->next = q; //插入了
                        break; //退出循环
                    }
                }
                if(q == NULL){ // 遍历到末尾
                    p->next = timer; //那就直接插在末尾
                }
            }
        }
        return 0; //返回0表示插入成功
    } 
    return 1; //返回1表示插入失败
}

static uint8_t LpTimer_DelFormList(LpTimer * timer)
{
    if(timer == NULL || (timer->is_running == 0)) {
        return 1; // 如果传入的任务为空(或者该定时器根本没有在列表中)，直接返回1 表示删除该定时器失败
    }
    LpTimer * p = timerList;
    if(p == timer){
        timerList = p->next;
        timer->next = NULL; // 从链表中摘除
        timer->is_running = 0; //标志不在列表中
        return 0; //摘除该定时器成功
    }
    while(p->next != NULL){
        if(p->next == timer){
            p->next = p->next->next;
            timer->next = NULL; // 从链表中摘除
            timer->is_running = 0; //标志不在列表中
            return 0; //找到并从链表中摘除
        } else {
            p = p->next;
        }
    }
    return 1; //没有找到该定时器(说明该定时器没有启动)
}

void LpTimer_Create(LpTimer * timer, E_LPTIMER_MODE mode, void (*hook)(void))
{
    if(timer == NULL || hook == NULL) {
        return; // 如果传入的任务或函数指针为空，直接返回
    }
    if(timerList == NULL){ //第一个定时器创建的时候，初始化硬件定时器为初始时间戳
        HwLpTimer_SetCnt(INIT_TIME_STAMP); 
    }
    timer->mode = mode; // 设置定时器模式
    timer->taskFunction = hook; // 设置任务函数指针
    timer->next = NULL; // 初始化下一个任务指针为NULL
}


void LpTimer_Start(LpTimer *timer, uint32_t tick_of_timeout)
{
    if(timer == NULL) {
        return; // 如果传入的任务为空，直接返回
    }
    uint32_t hw_current_cnt = HwLpTimer_GetCnt(); // 获取当前硬件定时器计数值
    if(timerList != NULL){ // 第一次因为硬件定时器没有开启，不需要更新时间戳
        uint64_t base_counter = GET_BASE_COUNTER(currentTime);
        uint64_t hw64_current_cnt = base_counter + (uint64_t)hw_current_cnt; // 将当前硬件计数值转换为64位并加上累计的回环次数
        if(currentTime <= hw64_current_cnt) {
            currentTime = hw64_current_cnt; // 更新当前时间戳
        } //这里可以不用考虑回环的处理，毕竟只要回环就会溢出中断，在中断中处理
    }
    timer->deadline = currentTime + tick_of_timeout; // 设置超时时间
    if(timer->mode == E_LPTIMERMODE_PERIODIC) {
        timer->period = tick_of_timeout; // 如果是周期模式，设置周期
    } else {
        timer->period = 0; // 如果是单次模式，周期为0
    }
    LpTimer_Insert2List(timer); // 将任务插入到链表中
    if(hw_current_cnt == HWLPTIMER_MAX_TIMEOUT) hw_current_cnt = 0;
    targetTime = hw_current_cnt + timerList->deadline - currentTime; // 计算下一个任务的超时时间
    if(targetTime > HWLPTIMER_MAX_TIMEOUT) targetTime = HWLPTIMER_MAX_TIMEOUT; // 如果超时时间大于最大超时时间，则设置为最大超时时间
    HwLpTimer_Puase(); // 停止定时器中断
    HwLpTimer_SetWakeUpTime((uint32_t)targetTime); // 设置硬件定时器唤醒时间 
    HwLpTimer_Start(); //重新开始定时器
}

uint8_t LpTimer_Stop(LpTimer *timer)
{
    uint8_t ret = 0;
    if(timer == timerList){ //如果删除的是将要触发的定时器
        LpTimer* temp = timerList; //暂时保存将要触发的定时器
        ret = LpTimer_DelFormList(timer); // 从链表中删除该定时器
        if(ret == 0){ //删除成功的话
            if(timerList == NULL){ //删后如果没有定时器了，直接停止硬件定时器
                HwLpTimer_Puase(); // 如果没有任务，暂停定时器
            }else if(timer == temp){ //删除的是将要触发的定时器才需要重新获取时间设定target 
                //先更新一次时间，才能设定target值
                uint32_t hw_current_cnt = HwLpTimer_GetCnt(); // 获取当前硬件定时器计数值
                uint64_t base_counter = GET_BASE_COUNTER(currentTime);
                uint64_t hw64_current_cnt = base_counter + (uint64_t)hw_current_cnt; // 将当前硬件计数值转换为64位并加上累计的回环次数
                if(currentTime <= hw64_current_cnt) {
                    currentTime = hw64_current_cnt; // 更新当前时间戳
                } //这里可以不用考虑回环的处理，毕竟只要回环就会溢出中断，在中断中处理
                if(hw_current_cnt == HWLPTIMER_MAX_TIMEOUT) hw_current_cnt = 0;
                targetTime = hw_current_cnt + timerList->deadline - currentTime; // 计算下一个任务的超时时间
                if(targetTime > HWLPTIMER_MAX_TIMEOUT) targetTime = HWLPTIMER_MAX_TIMEOUT; // 如果超时时间大于最大超时时间，则设置为最大超时时间
                HwLpTimer_Puase(); // 停止定时器中断
                HwLpTimer_SetWakeUpTime((uint32_t)targetTime); // 设置硬件定时器唤醒时间 
                HwLpTimer_Start(); //重新开始定时器
            }
        }
    }
    return ret; // 返回0表示成功删除，返回1表示失败
}    

void LpTimer_Execute(void)
{
    if(HwLpTimer_WakeUpHook != NULL)HwLpTimer_WakeUpHook(NULL);
    uint32_t hw_current_cnt = HwLpTimer_GetCnt(); // 获取当前硬件定时器计数值
#if(HWLPTIMER_MODE == HWLPTIMER_MODE_RTC_TIMER)
    uint64_t base_counter = GET_BASE_COUNTER(currentTime);
    uint64_t hw64_current_cnt = base_counter + (uint64_t)hw_current_cnt; // 将当前硬件计数值转换为64位并加上累计的回环次数
    if(currentTime <= hw64_current_cnt) { //还在同一个周期内
        currentTime = hw64_current_cnt; // 更新当前时间戳
    } else { //已经回环了一次(因为RTC会一直往上加，和普通定时器溢出不同，触发中断后不会停止)
        currentTime = base_counter + HWLPTIMER_MAX_TIMEOUT + 1 + hw_current_cnt;
    } //这里其实有一个当hw_current_cnt = MAX_TIMEOUT lts的current_time%后也是MAX_TIMEOUT 时候会有一个bug,
    // 不过这种情况不太可能在32bit的RTC计数中出现，如果出现这种情况，意味着，有个定时器周期超过了32bit
#elif(HWLPTIMER_MODE == HWLPTIMER_MODE_UPDATE_TIMER)
    lastInterruptTime = lastInterruptTime + targetTime;
    currentTime = lastInterruptTime;
#endif //(HWLPTIMER_MODE == HWLPTIMER_MODE_RTC_TIMER)
    LpTimer *p = timerList;
    while(p != NULL && p->deadline <= (currentTime + TICK_OF_SAFETY_WINDOW)) {
        p->taskFunction(); // 执行任务函数
        if(p->mode == E_LPTIMERMODE_ONCE) {
            LpTimer_DelFormList(p); // 如果是单次模式，从链表中摘除p
            p = timerList; // 重新获取链表头
        } else {
            LpTimer_DelFormList(p); //从链表中摘除p
            p->deadline = currentTime + (uint64_t)(p->period); // 如果是周期模式，重新设置超时时间
            LpTimer_Insert2List(p); // 重新插入到链表中
            p = timerList; // 重新获取链表头
        }
    }
    if(timerList != NULL) {
        if(hw_current_cnt == HWLPTIMER_MAX_TIMEOUT) hw_current_cnt = 0;
        //当hw_current_cnt恰好等于最大值时，下面可能会有一个bug，导致重复计算一次max_timeout
        targetTime = hw_current_cnt + \
                                (timerList->deadline - currentTime); // 计算下一个任务的超时时间
        if(targetTime < HwLpTimer_GetCnt()){ //由于RTC一直在计数，会自动回环，所以这里直接再获取一次cnt
            // 说明由于任务执行时间过长，导致下一个任务的超时时间已经小于当前硬件计数值(在一个安全窗口后直接唤醒执行)
            targetTime = HwLpTimer_GetCnt() + TICK_OF_SAFETY_WINDOW; // 那么就将目标时间设置为当前硬件计数值加上安全窗口
        }
        if(targetTime > HWLPTIMER_MAX_TIMEOUT) targetTime = HWLPTIMER_MAX_TIMEOUT; // 如果超时时间大于最大超时时间，则设置为最大超时时间
        HwLpTimer_Puase(); // 停止定时器中断
        HwLpTimer_SetWakeUpTime((uint32_t)targetTime); // 设置硬件定时器唤醒时间 
        HwLpTimer_Start(); // 重新开始定时器
    } else {
        HwLpTimer_Puase(); // 如果没有任务，暂停定时器
    }
}

#endif // if LPTIMER_IMPLEMENT


#endif // LPTIMER_H

