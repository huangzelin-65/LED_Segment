#ifndef ADAPTOR_INC_ADAPTOR_NTP_H_
#define ADAPTOR_INC_ADAPTOR_NTP_H_
#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NTP_NOTIFY_INIT =   0x01,
    NTP_NOTIFY_UPDATE = 0x02
} NtpNotify_t;

void sntp_normal_init(void);

void sntp_set_system_time(uint32_t sec);

void sntp_notify(uint32_t value);

struct tm sntp_get_system_time(void);

#ifdef __cplusplus
}
#endif

#endif /* SNTP_EXAMPLE_H */
