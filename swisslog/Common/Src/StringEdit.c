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

/**
 * @brief 从指定字符串中提取IP地址
 * @param input_str 输入字符串（格式如 +LIP=192.168.10.126）
 * @param ip_buffer 存储提取出的IP的缓冲区
 * @param buffer_size 缓冲区大小（建议至少16，IPv4最长15字符+结束符）
 * @return 成功返回IP缓冲区地址，失败返回NULL
 */
char* extract_ip(const char* input_str, char* ip_buffer, int buffer_size) 
{
    // 1. 入参合法性检查
    if (input_str == NULL || ip_buffer == NULL || buffer_size <= 0) {
        return NULL;
    }

    // 2. 找到等号的位置（IP的起始标记）
    const char* equal_sign = strchr(input_str, '=');
    if (equal_sign == NULL) {
        return NULL; // 未找到等号，无IP
    }

    // 3. 从等号下一个字符开始提取IP
    const char* ip_start = equal_sign + 1;
    char* ip_ptr = ip_buffer;
    int char_count = 0;

    // 4. 只提取数字和点，直到缓冲区满或遇到非IP字符
    // 关键修正：将char转换为unsigned char后再传入isdigit，消除警告
    while (*ip_start != '\0' && char_count < buffer_size - 1) {
        if (isdigit((unsigned char)*ip_start) || *ip_start == '.') {
            *ip_ptr++ = *ip_start++;
            char_count++;
        } else {
            break; // 遇到非IP字符（如换行、OK等），停止提取
        }
    }

    // 5. 补充字符串结束符（必须）
    *ip_ptr = '\0';

    // 6. 提取到有效IP才返回，否则返回NULL
    return (char_count > 0) ? ip_buffer : NULL;
}

/**
 * @brief 判断IP是否为无效地址（0.0.0.0）
 * @param ip_str 提取到的IP字符串（需确保非NULL）
 * @return 是0.0.0.0返回1，否则返回0
 */
int is_invalid_ip(const char* ip_str)
{
    // 入参检查：NULL视为无效IP
    if (ip_str == NULL) {
        return 1;
    }
    // 字符串比较，判断是否等于0.0.0.0
    return (strcmp(ip_str, "0.0.0.0") == 0) ? 1 : 0;
}






