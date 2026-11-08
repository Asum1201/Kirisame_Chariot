# remote_ctrl 

## 主要用于：
- 1. 解析 uart3 接收到的遥控器信息
- 2. 发送至message_center

## 代码思路：
 
  - ### 初始化 
      1. 利用结构体传递参数到bsp层的usart得到一个USARTInstnce实例
      
      2. 注册一个 Publisher
  - ### 中断回调函数解析数据包  (归一化)
  - ### 发送至message_center
  - ### 板间通讯传至Chassis (can_comm) -> can_comm订阅这个信息 