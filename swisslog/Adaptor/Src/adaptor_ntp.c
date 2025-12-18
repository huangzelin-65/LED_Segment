/*
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Dirk Ziegelmeier <dziegel@gmx.de>
 *
 */

#include "lwip/opt.h"
#include "lwip/apps/sntp.h"
#include "adaptor_ntp.h"
#include "lwip/netif.h"
#include "LogDebugInfo.h"
#include "app_freertos.h"

struct tm current_time_val;

void sntp_set_system_time(u32_t sec)
{
  char buf[32];
  sec += (8 * 60 * 60);
  time_t current_time = (time_t)sec;//北京时间是东8区需偏移8小时
#ifdef _MSC_VER
  localtime_s(&current_time_val, &current_time);
#else
  localtime_r(&current_time, &current_time_val);
#endif
  
  strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S", &current_time_val);
  DEBUGINFO("SNTP time: %s\n", buf);

  sntp_notify(NTP_NOTIFY_UPDATE);
}

struct tm sntp_get_system_time(void)
{
    return current_time_val;
}

void sntp_normal_init(void)
{
  DEBUGINFO("start");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
#if LWIP_DHCP
  sntp_servermode_dhcp(1); /* get SNTP server via DHCP */
#else /* LWIP_DHCP */
#if LWIP_IPV4
  sntp_setserver(0, netif_ip_gw4(netif_default));
#endif /* LWIP_IPV4 */
#endif /* LWIP_DHCP */
  sntp_init();
  DEBUGINFO("end");  
}

//消息通知主线程
void sntp_notify(uint32_t value)
{
    if(NtpManagerTaskHandle != NULL)
    {
        DEBUGINFO("value:%lx",value);
        BaseType_t xReturn = pdPASS;
        xReturn = xTaskNotify(NtpManagerTaskHandle, 
                    value, 
                    eSetValueWithoutOverwrite);
        if(xReturn != pdPASS)
        {
            DEBUGINFO("xReturn is not pdPASS:%ld\n",xReturn);
        } 
    }
    else
    {
        DEBUGINFO("NtpManagerTaskHandle NULL");
    }
}
