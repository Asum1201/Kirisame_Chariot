# 电控组第五次作业 - FreeRTOS
## 任务要求
1. 学习FreeRTOS相关内容
2. 阅读工程模板（电机底层），学习其CAN通信，电机驱动等内容的实现，使用markdown编写你的阅读笔记，无字数要求。以下给出一些可以包括的内容供参考：
   1. 模板中你之前不了解的c/c++功能（如函数指针，class类等）
   2. 模板中关于FreeRTOS的使用
   3. 相比自己电机任务/整车调试中的代码，这份模板相对于你的代码有什么优点
   4. 如何使用这个模板
3. 在模板基础上编写FreeRTOS任务：
   1. LED流水灯（所有参数自己定，只要是流水灯就行）
   2. 陀螺仪移植
   3. 电机控制任务，用**位置环**让3508电机不断转动，其速度是正弦函数（PID参数不重要，能转就行）
   4. 自定义CAN通信任务，接收校内赛需要使用的功率计信息
   5. 遥控器接收与数据解算
4. 校内赛的代码可以并建议使用这个模板
5. 作业第一次提交截止时间11.8日，把工程提交到自己的github上，并将仓库网址发到1459054506@qq.com

## 关于AGENT使用
1. 模板中有一个AGENTS.md文件，它会让你的agent不直接帮你写代码，而是指导你自己怎么写。
2. 禁止修改，删掉，删掉后再加回AGENTS.md的行为（防君子不防小人）
3. 如发现存在绕过AGENTS.md使用AI的行为，此次作业为0分并打回重做，若11.15前未通过这次作业，降一档录取（正式——梯队，梯队——取消入队资格）

## 在这里编写你的阅读笔记
（阅读笔记不要求字数，但是需要自己写，AI生成的一律打回）



# 神秘阅读笔记


## FreeRTOS的使用 

![](\Ciallo\FreeRTOS_ioc.png)

## CAN模块的编写 

### 初始化  
```c++
main.c

 void bsp_can_init() {
    HAL_CAN_Start(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1,  CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY);
    HAL_CAN_Start(&hcan2);
    HAL_CAN_ActivateNotification(&hcan2,  CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY);
}
```

### CAN_bsp.h  

```c++
class can_device_receive; //创建can_device_receive类用于接收can数据

typedef void (can_rx_callback)(can_device_receive *can_receive, uint8_t *data); //创建void类型的回调函数，传递参数是类的地址用来标识是谁的信息，和信息内容的地址


struct can_device_transmit_member{
    CAN_HandleTypeDef *hcan;
    uint32_t id;
    uint8_t len;
    uint8_t *buf_data;
};  //结构体成员 - 一些基本的参数配置和信息缓存区

```
 - 类的定义

1. can_device_receive
```c++

class can_device_receive {

public: 
    can_device_receive(CAN_HandleTypeDef *hcan_=&hcan1, uint32_t id_=0, can_rx_callback *callback_ = nullptr); //这是构造函数
    bool can_set_callback(can_rx_callback *callback_); //用来修改/绑定回调函数的接口
    void can_modify_id(uint32_t id_); //用来绑定id
//公有函数，作为接口 
private:
    CAN_HandleTypeDef *hcan;    //通过函数传递的参数
    uint32_t id;        //通过函数传递的参数
    uint32_t index;     //计算出的索引


public:
    uint8_t rx_data[8]; //原始数据
    can_rx_callback *callback;    //需要直接赋值
    uint8_t is_callback_none;     //未定义回执函数
};//公有的数据
```  
2. can_device_transmit
```c++
class can_device_transmit {

public:
    can_device_transmit(CAN_HandleTypeDef *hcan_, uint32_t id_ = 0, uint8_t len_ = 8, uint8_t *buf_data_ = nullptr); //init
    can_device_transmit();
    osStatus_t can_send_msg(); //发送
    bool set_buf_address(uint8_t *buf_address);
    bool set_id(uint32_t id);

public:
    can_device_transmit_member member;
};
```
 关于osStatus_t (~~大肥鱼解释~~):  

```
1. 返回值 osStatus_t 是什么？
osStatus_t 是 ARM 官方定义的 CMSIS-RTOS（实时操作系统）标准返回类型。它是一个枚举（Enum），用来告诉调用者：“这个函数执行得怎么样了”。
它最常用的几个状态包括：

osOK：发送成功（或者成功放入了发送队列）。

osErrorTimeout：超时了（比如发送队列满了，等了很久都没发出去）。

osErrorResource：资源不可用（比如 CAN 邮箱被占用，或者总线断开）。

osErrorParameter：参数错误（比如传入了非法的 CAN ID）。

osError：发生未指定的错误。

2. 为什么一个发 CAN 的函数要返回 RTOS 的状态？
如果你自己写过 STM32 的 CAN 发送，你可能会用标准的 HAL 库函数 HAL_CAN_AddTxMessage()，它返回的是 HAL_StatusTypeDef（HAL_OK、HAL_BUSY 等）。

但如果你在 FreeRTOS 环境下，为了防止多个任务同时抢用 CAN 总线导致数据错乱，通常会做一个“中间层”。这个 can_send_msg 内部可能做了这些事：

先等待一个信号量/互斥锁（防止总线冲突）。

如果等待超时，直接返回 osErrorTimeout。

拿到锁后，调用 HAL 库把数据发出去。

释放锁。

返回 osOK 告诉上层业务“数据已经成功发了”。
```

#### 其他
```c++
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} // extern "C"
#endif//使用C风格的函数声明避免C++声明函数因为重载Name Mangling

```

~~DeppSsek~~
```
#ifdef __cplusplus：因为 extern "C" 是 C++ 独有的语法。
如果纯 C 语言的编译器（比如 arm-none-eabi-gcc）看到 extern "C"，它会直接报错（语法错误）。
加上 #ifdef __cplusplus，意思是：“如果当前编译器是 C++（也就是定义了 __cplusplus 宏），才执行下面的 extern "C"；
如果是纯 C 编译器，就假装没看见这段代码。”
```

---

## CAN_bsp.cpp

~~11.4号还没读，要来不及了！！！~~

```c++
can_device_transmit::can_device_transmit(CAN_HandleTypeDef *hcan_, uint32_t id_, uint8_t len_, uint8_t *buf_data_)
               : member{hcan_,id_,len_,buf_data_} {}

can_device_transmit::can_device_transmit()
               : member{&hcan1, 0x000, 8, nullptr} {}

```

构造函数 构造一个类，把函数里传递的参数填入member成员里面  
第二种默认构造，如果没有填参数就默认为hcan1 0x000 8 空的指针  

```c++
osStatus_t can_device_transmit::can_send_msg() { //实际上是发送进队列中
    osStatus_t state;
    if (member.hcan == &hcan1) {
        state = osMessageQueuePut(can1_send_fifoHandle, this, 0, 0);
    } else {
        state = osMessageQueuePut(can2_send_fifoHandle, this, 0, 0);
    }
    return state;
}
```  

📝 函数原型与参数解析
```c++
osStatus_t osMessageQueuePut(osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout);
```

```mq_id```：消息队列的句柄（你创建队列时拿到的那个“传送带凭证”）。

```msg_ptr```：指向你要发送的数据的指针。这里极其重要，系统会从这个指针指向的地址，拷贝你预设大小的数据放进队列。

```msg_prio```：消息优先级（CMSIS-RTOS v2 预留的，在基于 FreeRTOS 的实现中通常直接填 0 即可，FreeRTOS 原生不支持消息插队）。

```timeout```：超时时间。

* ```0```：不等待。如果队列满了，立刻返回失败。  
* ```osWaitForever```：死等。如果队列满了，任务就卡在这里（阻塞），直到队列有空位。  
* 具体数字 ：比如 10，代表最多等 10 个 tick，如果还没空位就返回失败  

`osSemaphoreAcquire` 是 CMSIS-RTOS v2 标准库中，用来**获取（拿走）信号量**的函数。

底层调用的通常是 FreeRTOS 的 `xSemaphoreTake`。它的核心作用是：**申请占用某个公共资源**。

### 📝 函数原型与参数解析

```c
osStatus_t osSemaphoreAcquire(osSemaphoreId_t sem_id, uint32_t timeout);
```

*   **`sem_id`**：信号量的句柄（你创建信号量时拿到的“凭证”）。
*   **`timeout`**：超时时间。
   *   `0`：不等待。如果钥匙不在，立刻走人，返回失败。
   *   `osWaitForever`：死等。如果钥匙不在，就在门口一直睡（阻塞），直到有人把钥匙还回来。
   *   具体数字：比如 `10`，代表最多等 10 个 tick，还没等到就放弃。

### 🔄 返回值 `osStatus_t`
*   **`osOK`**：成功
*   **`osErrorTimeout`**：等超时了，还是没拿到钥匙，没进入临界区。
*   **`osErrorResource`**：信号量无效或不可用。
*   **`osErrorParameter`**：参数错误（比如句柄是空的）。

---

```c++
state = osMessageQueuePut(can1_send_fifoHandle, this, 0, 0);
```
等价于
```c++
osMessageQueuePut(can1_send_fifoHandle, &(this->member), 0, 0);
```
```
在 C++ 的内存模型里，当 member 是这个类的第一个（或者唯一一个）成员变量时，
这个对象的首地址（this）和它内部成员的首地址（&member），在内存上是完全重合的！
```

这段代码是 C++ 面向对象编程里**最硬核、最标准**的写法之一。它主要干了一件事：**当你实例化一个 CAN 接收设备时，自动在底层单片机的硬件上给它分配一个“专属过滤器（Filter）”，并把它登记到系统的接收设备列表里。**

```cpp
can_device_receive::can_device_receive(CAN_HandleTypeDef *hcan_, uint32_t id_, can_rx_callback *callback_)
                            : hcan(hcan_), id(id_), callback(callback_), rx_data{}
```
1.  **`can_device_receive::can_device_receive`**：这是类的构造函数在类外部的定义。前面带类名，后面带函数名。
2.  **冒号 `:` 后面的初始化列表**：这是 C++ 的高效赋值方式。
   *   `hcan(hcan_)`：把外面传进来的硬件句柄赋给内部成员。
   *   `rx_data{}`：**极其巧妙的一笔**。大括号 `{}` 在 C++ 里叫做“零初始化”，意思是直接把 `rx_data[8]` 这个数组全部清零，防止里面有脏数据。
3.  **大括号 `{}` 里面的代码**：这是构造函数真正执行逻辑的地方。

---

### 临界区保护（`taskENTER_CRITICAL`）
```cpp
taskENTER_CRITICAL();
// ... 配置代码 ...
taskEXIT_CRITICAL();
```
**为什么要有这俩？**
假设你的云台任务和底盘任务同时调用这个类创建两个 CAN 设备。如果它俩同时执行 `can1_device_num++`，这个变量可能会少加一次，导致索引错乱。
这俩函数的作用是**关中断**（或挂起调度器），保证在配置过滤器的这几微秒里，绝对没有其他任务来捣乱。**这是一种极其严谨的多任务保护手段。**
![](\Ciallo\nihaoCRITCAL.png)
---

```c++
uint32_t index = rx_header.FilterMatchIndex;
```
`rx_header.FilterMatchIndex`是 STM32 硬件（CAN 外设）自动提供的一个极其强大的“捷径”。它的意思是：硬件告诉你，这帧数据精准匹配到了过滤器的第几个“坑位”。

---

# PID 

```c++
__RAM_FUNC
```
把被这个宏修饰的函数，放到 RAM（内存）里运行，而不是放在 Flash（闪存）里。

这主要是为了解决一个经典的硬件冲突问题：当 CPU 正在擦除或烧写 Flash 时，它就无法同时从 Flash 中读取指令，这会导致程序卡死，所有中断也无法响应。  
### 好高级(~~不会用~~)

```c++
pid::pid(float max_out_, float integral_limit, float kp, float ki, float kd)
    :param{kp, ki, kd, 0, max_out_, integral_limit, -integral_limit},//结构体初始化格式
     enable(true),
     set(0),get(0),
     err(0),last_err(0),penultimate_err(0),
     pout(0),iout(0),dout(0),out(0){ }
```
* ### 成员初始化列表（Member Initializer List）  
  * 这段代码的意思是：在创建 PID 对象的一瞬间，把所有的参数和内部状态变量全部填好、清零，防止出现随机乱码。 

---
# motor_driver 

```C++
virtual float motor_get_speed_forward();  
```  
#### `virtual` 在 C++ 中代表`虚函数（Virtual Function）`  
**它是 C++ 面向对象编程（OOP）最核心的灵魂**：**多态**（Polymorphism）。  
场景：**统一管理各种不同的电机**  

所以说这是利用多态实现对电机类的方法定义具有极高抽象性和普适性  
对不同的电机只需重写里面各个抽象的方法即可，而接口却都是统一的！  
~~_**太神奇了**_~~  
`override` :让编译器帮你检查，你到底有没有成功地重写（覆盖）父类的那个虚函数(同时显式说明这是对父类的覆写增强代码可读性)
---

# DJI_motor_driver

```c++
    while (init_offset_f!=1){
        osDelay(1);//在这里停止
    }
```
在大疆电机初始化时，一般需要等遥控器连接、或者等底盘/云台找零点完成。这是为了保证初始化顺序安全，防止在系统还没准备好时把电机启动飞车。

```c++
DJI_motor *dji_motor = container_of(can_receive, DJI_motor, can_rx);
```
`container_of` 已知一个结构体（或类）里面某个“成员变量”的内存地址，反向推导计算出整个“大结构体”的起始地址  
### #返回的是包含该 can_rx 成员的、那个完整的 DJI_motor 对象的首地址。