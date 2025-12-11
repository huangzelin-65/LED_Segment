#ifndef ADAPTOR_INC_ADAPTOR_NTP_H_
#define ADAPTOR_INC_ADAPTOR_NTP_H_

#ifdef __cplusplus
extern "C" {
#endif

void sntp_normal_init(void);

void sntp_set_system_time(u32_t sec);

#ifdef __cplusplus
}
#endif

#endif /* SNTP_EXAMPLE_H */
