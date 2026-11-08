//
// Created by 33269 on 2026/10/11.
//

#include "message_center.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>


//C 库函数 void *memcpy(void *str1, const void *str2, size_t n) 从存储区 str2 复制 n 个字节到存储区 str1。

static Publisher_t m_center = {
    .topic_name = "Message_Manager",
    .first_subs = NULL,
    .next_topic_node = NULL};
//链表的虚拟头指针


static void check_len(uint8_t len1, uint8_t len2)
{
    if (len1 != len2)
        while (1)
            ;
}

Subscriber_t *sub_register(char *topic_name,uint8_t data_len)
{
    Publisher_t *pub = pub_register(topic_name,data_len);
    // 创建新的订阅者结点,申请内存,注意要memset因为新空间不一定是空的,可能有之前留存的垃圾值
    Subscriber_t *ret = (Subscriber_t *)malloc(sizeof(Subscriber_t));
    //malloc 后没检查返回值，如果内存耗尽返回 NULL，后续 memset(NULL, ...) 会崩溃。
    if (ret == NULL) {
        while (1)
            ;
    }
    memset(ret, 0, sizeof(Subscriber_t));
    ret->data_len = data_len;

    for (size_t i = 0; i < QUEUE_SIZE; i++)
    {
        ret->queue[i] = malloc(data_len);
        memset(ret->queue[i], 0, data_len);
    }
    // 如果是第一个订阅者,特殊处理一下,将first_subs指针指向新建的订阅者(详见文档)
    if (pub->first_subs == NULL)
    {
        pub->first_subs = ret;
        return ret;
    }
    // 若该话题已经有订阅者, 遍历订阅者链表,直到到达尾部
    Subscriber_t *sub = pub->first_subs; // 作为iterator
    while (sub->next_subs_queue)         // 遍历订阅了该话题的订阅者链表
    {
        sub = sub->next_subs_queue; // 移动到下一个订阅者,遇到空指针停下,说明到了链表尾部
    }
    sub->next_subs_queue = ret; // 把刚刚创建的订阅者接上
    return ret;
}

Publisher_t *pub_register(char *topic_name,uint8_t data_len)
{
    Publisher_t *node = &m_center;
    while (node ->next_topic_node != NULL)
    {
        if (strcmp(node->next_topic_node->topic_name,topic_name) == 0)
        {
           check_len(data_len,node->next_topic_node->data_len);
            node ->pub_registered_flag = 1;
            return node->next_topic_node;
        }
        node = node->next_topic_node;
    }// 遍历完发现尚未创建name对应的话题

    // 在链表尾部创建新的话题并初始化
    node ->next_topic_node = (Publisher_t *)malloc(sizeof(Publisher_t));
    memset(node->next_topic_node,0,sizeof(Publisher_t));
    node->next_topic_node->data_len = data_len;
    strcpy(node->next_topic_node->topic_name,topic_name);
    node->next_topic_node->pub_registered_flag = 1;
    return node->next_topic_node;
}

uint8_t sub_getmessage(Subscriber_t *sub,void * data_ptr)
{
    if (sub->temp_size == 0)
    {
        return 0;
    }
    memcpy(data_ptr,sub->queue[sub->front_idx],sub->data_len);
    sub->front_idx = (sub->front_idx + 1) % QUEUE_SIZE;
    sub->temp_size--;
    return 1;
}

void pub_sendmessage(Publisher_t *pub,void *data_ptr)
{
    Subscriber_t * node;
    node = pub ->first_subs;

    while (node != NULL)
    {
        if (node->temp_size == QUEUE_SIZE)
        {
            node->temp_size --;
            node->front_idx = (node->front_idx + 1) % QUEUE_SIZE;
        }
        memcpy(node->queue[node->back_idx],data_ptr,node->data_len);
        node->back_idx = (node->back_idx + 1) % QUEUE_SIZE;
        node->temp_size++;
        node = node ->next_subs_queue;
    }
}