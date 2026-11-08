//
// Created by 33269 on 2026/10/11.
//

#ifndef MESSAGE_CENTER_H
#define MESSAGE_CENTER_H

#include <stdint.h>

#define MAX_TOPIC_NAME_LEN 32 //最大话题名长度
#define MAX_TOPIC_COUNT 12 //最多话题数
#define QUEUE_SIZE 3 //信息队列长度

typedef struct ms
{
    void *queue[QUEUE_SIZE];
    uint8_t data_len;
    uint8_t front_idx;  //循环队列的参数 指向队首
    uint8_t back_idx;   //循环队列的参数 指向队尾
    uint8_t temp_size; // 当前队列长度

    struct ms *next_subs_queue; //链表的参数 指向下一个订阅话题
}Subscriber_t;

typedef struct topicnode
{
    char topic_name[MAX_TOPIC_NAME_LEN + 1];
    uint8_t data_len;
    Subscriber_t *first_subs;

    struct topicnode *next_topic_node;//下一个话题
    uint8_t pub_registered_flag;// 用于标记该发布者是否已经注册
}Publisher_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 注册一个订阅者订阅一个话题
 * @param topic_name 指针指向话题的字符串
 * @param data_len 数据的长度， 便于memcpy获取数据，还要用来检测与话题的关系是否正确
 * @note 对于相同话题，其消息长度必须相同
 *       遍历发布者的话题结点，如果发现相同的话题，直接返回指针即可；遍历完成后发现尚未创建则创建新的话题。
 * @return 一个指针返回订阅数据的地址
 */
Subscriber_t *sub_register(char *topic_name,uint8_t data_len);

/**
 * @brief 注册一个发布者
 * @note  需要注意，由于不同应用/模块的初始化顺序不同，可能出现订阅者先于发布者订阅某一消息的情况
 *        所以要进行发布者链表的遍历，判断是否已经存在相同话题名的发布者，不存在则要先创建发布者结点再将新建订阅者结点并挂载到前者上。
 * @param topic_name 话题名
 * @param data_len 数据长度
 * @note 对于相同话题，其消息长度必须相同
 * @return 一个发布者的指针
 */
Publisher_t *pub_register(const char *topic_name,uint8_t data_len);

/**
 * @brief 订阅者获取订阅的数据
 * @param sub 订阅者的位置
 * @param data_ptr 存放数据的指针
 * @return 返回是否获得成功 0/1
 */
uint8_t sub_getmessage(Subscriber_t *sub,void * data_ptr);

/**
 * @brief 发布者发布一个话题对应的消息
 * @param pub 发布者的位置
 * @param data_ptr 存放发布数据位置 的指针
 */
void pub_sendmessage(Publisher_t *pub,void *data_ptr);
#ifdef __cplusplus
}
#endif
#endif //_MESSAGE_CENTER_H
