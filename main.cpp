/* WiFi Example
 * Copyright (c) 2018 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "TCPSocket.h"

// Sensors drivers present in the BSP library
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_magneto.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_accelero.h"
#include <cstdio>
#include <cstring>

#define WIFI_IDW0XX1    2

#if (defined(TARGET_DISCO_L475VG_IOT01A) || defined(TARGET_DISCO_F413ZH))
#include "ISM43362Interface.h"
//ISM43362Interface wifi(MBED_CONF_APP_WIFI_SPI_MOSI, MBED_CONF_APP_WIFI_SPI_MISO, MBED_CONF_APP_WIFI_SPI_SCLK, MBED_CONF_APP_WIFI_SPI_NSS, MBED_CONF_APP_WIFI_RESET, MBED_CONF_APP_WIFI_DATAREADY, MBED_CONF_APP_WIFI_WAKEUP, false);
ISM43362Interface wifi(false);

#else // External WiFi modules

#if MBED_CONF_APP_WIFI_SHIELD == WIFI_IDW0XX1
#include "SpwfSAInterface.h"
SpwfSAInterface wifi(MBED_CONF_APP_WIFI_TX, MBED_CONF_APP_WIFI_RX);
#endif // MBED_CONF_APP_WIFI_SHIELD == WIFI_IDW0XX1

#endif

static BufferedSerial serial_port(USBTX, USBRX);
FileHandle *mbed::mbed_override_console(int fd) {
    return &serial_port;
}

const char *sec2str(nsapi_security_t sec)
{
    switch (sec) {
        case NSAPI_SECURITY_NONE:
            return "None";
        case NSAPI_SECURITY_WEP:
            return "WEP";
        case NSAPI_SECURITY_WPA:
            return "WPA";
        case NSAPI_SECURITY_WPA2:
            return "WPA2";
        case NSAPI_SECURITY_WPA_WPA2:
            return "WPA/WPA2";
        case NSAPI_SECURITY_UNKNOWN:
        default:
            return "Unknown";
    }
}

int main()
{
    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
    printf("\nConnection error\n");
    return -1;
    }

    TCPSocket socket;
    nsapi_error_t response;

    SocketAddress a;
    a.set_ip_address("192.168.43.235");
    printf("IP address: %s\n", a.get_ip_address() ? a.get_ip_address() : "None");
    socket.open(&wifi);
    a.set_port(12345);
    response = socket.connect(a);


    if(0 != response) {
        printf("Error connecting: %d\n", response);
        socket.close();
        return 0;
    }

    int16_t pDataXYZ[3] = {0};

    char sbuffer[] = "";

    printf("Start sensor init\n");
    BSP_ACCELERO_Init();

    while(1) {
        //printf("\nNew loop, LED1 should blink during sensor read\n");
 
        BSP_ACCELERO_AccGetXYZ(pDataXYZ);
        
        sprintf(sbuffer,"%d%s%d%s%d", pDataXYZ[0], ",", pDataXYZ[1], ",", pDataXYZ[2]);
        nsapi_size_t size = strlen(sbuffer);
        response = 0;

        while(size) {
            response = socket.send(sbuffer+response, size);
            if (response < 0) {
                printf("Error sending data: %d\n", response);
                socket.close();
                return 0;
            } else {
                size -= response;
                // Check if entire message was sent or not
                printf("sent %d [%.*s]\n", response, strstr(sbuffer, "\r\n")-sbuffer, sbuffer);
            }
        }

        //ThisThread::sleep_for(100);
    }

    wifi.disconnect();

    printf("\nDone\n");
}
