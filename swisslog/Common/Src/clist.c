#include "clist.h"

/**
 * @brief 创建一个新的链表节点
 * @param data 节点数据
 * @return 节点指针（失败返回 NULL）
 */
static ListNode* list_node_create(void* data) {
    ListNode* node = (ListNode*)malloc(sizeof(ListNode));
    if (node == NULL) {
        perror("list_node_create: malloc failed");
        return NULL;
    }
    node->data = data;
    node->next = NULL;
    return node;
}

/**
 * @brief 初始化链表
 */
List* list_init(void) {
    List* list = (List*)malloc(sizeof(List));
    if (list == NULL) {
        perror("list_init: malloc failed");
        return NULL;
    }
    // 创建哨兵头节点（不存储实际数据，简化边界处理）
    list->head = list_node_create(NULL);
    if (list->head == NULL) {
        free(list);
        return NULL;
    }
    list->tail = list->head; // 空链表时，尾节点指向头节点
    list->size = 0;
    return list;
}

/**
 * @brief 销毁链表
 */
void list_destroy(List* list, DestroyFunc destroy) {
    if (list == NULL) return;

    ListNode* curr = list->head;
    ListNode* next = NULL;

    // 遍历释放所有节点
    while (curr != NULL) {
        next = curr->next;
        // 若为数据节点，可选释放数据
        if (curr != list->head && destroy != NULL) {
            destroy(curr->data);
        }
        free(curr);
        curr = next;
    }

    free(list); // 释放链表管理结构
}

/**
 * @brief 头插法插入节点
 */
int list_insert_head(List* list, void* data) {
    if (list == NULL || data == NULL) return -1;

    ListNode* new_node = list_node_create(data);
    if (new_node == NULL) return -1;

    // 新节点插入到头节点之后
    new_node->next = list->head->next;
    list->head->next = new_node;

    // 若链表为空（插入后为第一个节点），更新尾节点
    if (list->size == 0) {
        list->tail = new_node;
    }

    list->size++;
    return 0;
}

/**
 * @brief 尾插法插入节点
 */
int list_insert_tail(List* list, void* data) {
    if (list == NULL || data == NULL) return -1;

    ListNode* new_node = list_node_create(data);
    if (new_node == NULL) return -1;

    // 新节点插入到尾节点之后
    list->tail->next = new_node;
    list->tail = new_node; // 更新尾节点

    list->size++;
    return 0;
}

/**
 * @brief 在指定索引位置插入节点
 */
int list_insert_at(List* list, int index, void* data) {
    if (list == NULL || data == NULL || index < 0 || index > list->size) {
        return -1;
    }

    // 头插
    if (index == 0) {
        return list_insert_head(list, data);
    }
    // 尾插
    if (index == list->size) {
        return list_insert_tail(list, data);
    }

    // 中间插入：找到插入位置的前驱节点
    ListNode* prev = list->head;
    for (int i = 0; i < index; i++) {
        prev = prev->next;
    }

    ListNode* new_node = list_node_create(data);
    if (new_node == NULL) return -1;

    // 插入新节点
    new_node->next = prev->next;
    prev->next = new_node;

    list->size++;
    return 0;
}

/**
 * @brief 根据索引删除节点
 */
void* list_remove_at(List* list, int index, DestroyFunc destroy) {
    if (list == NULL || list->size == 0 || index < 0 || index >= list->size) {
        return NULL;
    }

    ListNode* prev = list->head;
    // 找到待删除节点的前驱节点
    for (int i = 0; i < index; i++) {
        prev = prev->next;
    }

    ListNode* to_remove = prev->next;
    void* data = to_remove->data; // 保存待删除节点的数据

    // 更新指针：前驱节点指向待删除节点的后继节点
    prev->next = to_remove->next;

    // 若删除的是尾节点，更新尾节点
    if (to_remove == list->tail) {
        list->tail = prev;
    }

    // 释放节点内存
    free(to_remove);
    list->size--;

    // 可选释放数据内存
    if (destroy != NULL) {
        destroy(data);
        return NULL;
    }

    return data; // 返回被删除的数据（由调用者管理）
}

/**
 * @brief 根据数据值删除节点
 */
void* list_remove_by_value(List* list, const void* data, CompareFunc compare, DestroyFunc destroy) {
    if (list == NULL || data == NULL || compare == NULL || list->size == 0) {
        return NULL;
    }

    ListNode* prev = list->head;
    ListNode* curr = list->head->next;

    // 遍历查找匹配的节点
    while (curr != NULL) {
        if (compare(curr->data, data) == 0) {
            // 找到匹配节点，更新指针
            prev->next = curr->next;

            // 若删除的是尾节点，更新尾节点
            if (curr == list->tail) {
                list->tail = prev;
            }

            void* removed_data = curr->data;
            free(curr); // 释放节点内存
            list->size--;

            // 可选释放数据内存
            if (destroy != NULL) {
                destroy(removed_data);
                return NULL;
            }

            return removed_data;
        }

        prev = curr;
        curr = curr->next;
    }

    return NULL; // 未找到匹配节点
}

/**
 * @brief 根据索引查找节点数据
 */
void* list_find_at(List* list, int index) {
    if (list == NULL || list->size == 0 || index < 0 || index >= list->size) {
        return NULL;
    }

    ListNode* curr = list->head->next;
    for (int i = 0; i < index; i++) {
        curr = curr->next;
    }

    return curr->data;
}

/**
 * @brief 根据数据值查找节点
 */
void* list_find_by_value(List* list, const void* data, CompareFunc compare) {
    if (list == NULL || data == NULL || compare == NULL || list->size == 0) {
        return NULL;
    }

    ListNode* curr = list->head->next;
    while (curr != NULL) {
        if (compare(curr->data, data) == 0) {
            return curr->data; // 返回第一个匹配的数据
        }
        curr = curr->next;
    }

    return NULL; // 未找到匹配节点
}

/**
 * @brief 遍历链表
 */
void list_traverse(List* list, TraverseFunc traverse) {
    if (list == NULL || traverse == NULL || list->size == 0) {
        return;
    }

    ListNode* curr = list->head->next;
    while (curr != NULL) {
        traverse(curr->data); // 对每个节点执行自定义操作
        curr = curr->next;
    }
}

/**
 * @brief 获取链表长度
 */
int list_size(List* list) {
    return (list == NULL) ? -1 : list->size;
}

/**
 * @brief 判断链表是否为空
 */
int list_is_empty(List* list) {
    if (list == NULL) return -1;
    return (list->size == 0) ? 1 : 0;
}