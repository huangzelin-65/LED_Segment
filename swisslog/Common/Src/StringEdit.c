#include "StringEdit.h"
#include "LogDebugInfo.h"
#include <ctype.h>

/**
 * @brief 将3个uint32_t类型的UID转换为24位16进制字符串
 * @param pUID 输入：指向UID[3]数组的指针（必须包含3个有效uint32_t元素）
 * @param pUuid 输出：指向uuid数组的指针（数组长度需至少为UUID_ID_LENGTH）
 * @param uuid_len uuid长度
 * @return 0：成功；-1：参数为空（错误码）
 */
int uid_to_uuid(const uint32_t *pUID, char *pUuid,uint32_t uuid_len)
{
    // 1. 参数合法性校验（避免空指针导致程序崩溃）
    if (pUID == NULL || pUuid == NULL)
    {
        return -1; // 返回错误码，标识参数无效
    }

    // 2. 可选：初始化uuid数组（防止残留脏数据，根据需求选择）
    memset(pUuid, 0, uuid_len);

    // 3. 核心转换逻辑：逐个将UID转为8位大写16进制字符串，填入对应位置
    // 每个uint32_t占8个字符，偏移量分别为0、8、16，snprintf确保不越界
    snprintf(&pUuid[0], 9, "%08lX", pUID[0]);  // 第1个UID：0-7位（8个字符）
    snprintf(&pUuid[8], 9, "%08lX", pUID[1]);  // 第2个UID：8-15位（8个字符）
    snprintf(&pUuid[16], 9, "%08lX", pUID[2]); // 第3个UID：16-23位（8个字符）

    pUuid[uuid_len] = '\0';

    return 0; // 转换成功
}

/**
 * 提取字符串末尾的数字部分
 * @param src 源字符串
 * @param dest 存储结果的缓冲区
 * @param dest_len 结果缓冲区的长度
 * @return 成功返回1，失败返回0（无数字/无下划线等）
 */
int extract_last_numbers(char *src, char *dest, int dest_len) {
    if (src == NULL || dest == NULL || dest_len <= 0) {
        return 0;  // 入参非法
    }

    // 1. 找到最后一个下划线的位置
    char *last_underline = strrchr(src, '_');
    if (last_underline == NULL) {
        return 0;
    }

    // 2. 从下划线后一位开始遍历，提取连续数字
    const char *num_start = last_underline + 1;
    int i = 0;
    while (*num_start != '\0' && isdigit((unsigned char)*num_start) && i < dest_len - 1) {
        dest[i++] = *num_start++;
    }
    dest[i] = '\0';  // 字符串结束符

    // 3. 检查是否提取到数字
    if (i == 0) {
        return 0;
    }

    return 1;
}










