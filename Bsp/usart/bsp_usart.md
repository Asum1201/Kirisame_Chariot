# bsp_usart

- ### 基础参数配置

---

### 1.
```c++
#define DEVICE_USART_CNT 3
#define USART_RXBUFF_MAXSIZE 256 
```

DEVICE_USART_CNT : USART串口的接口数(C板为3)

USART_RXBUFF_MAXSIZE : USART串口的接收缓存区 (256字节)

---

### 2.创建USART实例(USARTInstance)

- #### usart_module_callback :函数指针类型, 指向该实例的位于modules层的回调函数，进行数据的解析

```c++
typedef void (*usart_module_callback)();
```

- ##### 发送方式的枚举类型( 主要是DMA )

```c++
typedef enum
{
    USART_TRANSFER_NONE=0,
    USART_TRANSFER_BLOCKING,
    USART_TRANSFER_IT,
    USART_TRANSFER_DMA,
} USART_TRANSFER_MODE;
```

- ##### USART实例
```c++
typedef struct
{
    uint8_t recv_buff[USART_RXBUFF_MAXSIZE];
    uint8_t recv_buff_size;//接收一包数据的大小
    UART_HandleTypeDef *usart_handle; 
    usart_module_caallback module_callback;
}USARTInstance;
```
- ##### USART初始化的参数

  USART_Init_config_s 结构体 :

  1. recv_buff_size：接收一包数据的大小 

  2. UART_HandleTypeDef *usart_handle; 操作句柄

  3. module_callback : 回调函数的指针 

---

- ### 处理函数

#### 1. usartServiceInit() 初始化USART 
#### 2. usart_Register() 利用初始config注册一个usart实例(并返回一个实例的指针)
#### 3. usart_Send() 发送usart数据
#### 4，HAL_UARTEx_RxEventCallback 重写回调函数实现数据接收
#### 5.USARTIsReady 判断串口是否准备好,用于连续或异步的IT/DMA发送

