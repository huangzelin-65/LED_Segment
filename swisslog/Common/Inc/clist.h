#ifndef _LIST_H_
#define _LIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 链表节点结构（存储通用数据，用 void* 适配任意类型）
typedef struct ListNode {
    void* data;          // 节点存储的数据（通用指针）
    struct ListNode* next; // 指向下一个节点的指针
} ListNode;

// 链表管理结构（记录头节点、尾节点和链表长度，方便操作）
typedef struct List {
    ListNode* head;      // 头节点（哨兵节点，不存储实际数据）
    ListNode* tail;      // 尾节点（优化尾插效率）
    int size;            // 链表当前节点数量
} List;

// 函数指针类型：用于自定义数据的比较（查找/删除时使用）
typedef int (*CompareFunc)(const void* a, const void* b);
// 函数指针类型：用于自定义数据的销毁（释放节点数据时使用）
typedef void (*DestroyFunc)(void* data);
// 函数指针类型：用于遍历链表时处理数据
typedef void (*TraverseFunc)(void* data);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化链表
 * @return 链表实例指针（失败返回 NULL）
 */
List* list_init(void);

/**
 * @brief 销毁链表（释放所有节点及链表本身，可选释放节点数据）
 * @param list 链表实例
 * @param destroy 数据销毁函数（NULL 表示不释放数据）
 */
void list_destroy(List* list, DestroyFunc destroy);

/**
 * @brief 头插法插入节点
 * @param list 链表实例
 * @param data 待插入的数据（需自行管理内存，或通过 destroy 释放）
 * @return 插入成功返回 0，失败返回 -1
 */
int list_insert_head(List* list, void* data);

/**
 * @brief 尾插法插入节点
 * @param list 链表实例
 * @param data 待插入的数据
 * @return 插入成功返回 0，失败返回 -1
 */
int list_insert_tail(List* list, void* data);

/**
 * @brief 在指定索引位置插入节点（索引从 0 开始）
 * @param list 链表实例
 * @param index 插入位置（0 <= index <= size）
 * @param data 待插入的数据
 * @return 插入成功返回 0，失败返回 -1
 */
int list_insert_at(List* list, int index, void* data);

/**
 * @brief 根据索引删除节点
 * @param list 链表实例
 * @param index 待删除节点的索引（0 <= index < size）
 * @param destroy 数据销毁函数（NULL 表示不释放数据）
 * @return 成功返回被删除的数据，失败返回 NULL
 */
void* list_remove_at(List* list, int index, DestroyFunc destroy);

/**
 * @brief 根据数据值删除节点（需提供比较函数）
 * @param list 链表实例
 * @param data 待匹配的数据
 * @param compare 数据比较函数（返回 0 表示匹配）
 * @param destroy 数据销毁函数（NULL 表示不释放数据）
 * @return 成功返回被删除的数据，失败返回 NULL
 */
void* list_remove_by_value(List* list, const void* data, CompareFunc compare, DestroyFunc destroy);

/**
 * @brief 根据索引查找节点数据
 * @param list 链表实例
 * @param index 节点索引（0 <= index < size）
 * @return 成功返回节点数据，失败返回 NULL
 */
void* list_find_at(List* list, int index);

/**
 * @brief 根据数据值查找节点（需提供比较函数）
 * @param list 链表实例
 * @param data 待匹配的数据
 * @param compare 数据比较函数（返回 0 表示匹配）
 * @return 成功返回第一个匹配的节点数据，失败返回 NULL
 */
void* list_find_by_value(List* list, const void* data, CompareFunc compare);

/**
 * @brief 遍历链表，对每个节点执行自定义操作
 * @param list 链表实例
 * @param traverse 遍历处理函数（传入节点数据）
 */
void list_traverse(List* list, TraverseFunc traverse);

/**
 * @brief 获取链表当前节点数量
 * @param list 链表实例
 * @return 链表长度（失败返回 -1）
 */
int list_size(List* list);

/**
 * @brief 判断链表是否为空
 * @param list 链表实例
 * @return 空返回 1，非空返回 0，失败返回 -1
 */
int list_is_empty(List* list);

#ifdef __cplusplus
}
#endif

#endif // _LIST_H_