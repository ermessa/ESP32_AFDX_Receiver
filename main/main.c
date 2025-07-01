#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "afdx.h"
#include "crc32.h"

//TARGET NETWORK DATA
#define WIFI_SSID "ESP32_SENDER_AP"
#define WIFI_PASS "123qweasd"

//CONFIGURE STATIC DEVICE
#define ESP_IP "192.168.1.150"
#define ESP_GATEWAY "192.168.1.1"
#define ESP_SUBNET_MASK "255.255.255.0"

//CONFIGURE TARGET DEVICE
#define TARGET_IP "192.168.1.149"
#define TARGET_PORT 7000

//GLOBAL VARIABLES
static uint32_t numCycles = 0;
static bool start = false;

//PROTOTYPES

/// @brief INIT WI-FI STATION WITH DEFAULT CONFIGURATION
void WifiInitSta(void);

/// @brief GET THE TARGET IP
/// @param arg EMPTY
/// @param eventBase EMPTY
/// @param eventId EMPTY
/// @param eventData EMPTY
void OnGotIp(void *arg, esp_event_base_t eventBase, int32_t eventId, void *eventData);

/// @brief RECEIVE VIRTUAL-LINK
/// @param param EMPTY PARAM
void VlReceiveTask(void *param);

/// @brief CONFIGURE STATIC IP
void ConfigureStaticIp();

/// @brief CREATE AND WRITE TIMESTAMP IN A FILE FOR DEBUG
/// @param fileName FILENAME WITH EXTENSION
/// @param timestamp TIME TO PRINT IN THE FILE
/// @param status STATUS FOR REGISTER
void WriteFile(char* fileName, int64_t timestamp, char status [32]);

/// @brief CLEAR BUFFER
void FlushStdin();

/// @brief WAIT SERIAL COMMAND TO ENABLE THE ROUTINE FLOW
void WaitForStartCommand();

//FUNCTIONS

void ConfigureStaticIp()
{
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif == NULL)
    {
        ESP_LOGE("IP", "DID CANNOT OBTAIN THE NETIF");
        return;
    }
    esp_netif_dhcpc_stop(netif); 

    esp_netif_ip_info_t ipInfo;
    ip4_addr_t ip, gw, netmask;
    ip4addr_aton(ESP_IP, &ip);
    ip4addr_aton(ESP_GATEWAY, &gw);
    ip4addr_aton(ESP_SUBNET_MASK, &netmask);

    ipInfo.ip.addr = ip.addr;
    ipInfo.gw.addr = gw.addr;
    ipInfo.netmask.addr = netmask.addr;

    esp_netif_set_ip_info(netif, &ipInfo);
}

void WifiInitSta()
{
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&config);  
    wifi_config_t wifiConfig =
    {
        .sta = 
        {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void OnGotIp(void *arg, esp_event_base_t eventBase, int32_t eventId, void *eventData)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)eventData;
    ESP_LOGI("NET", "Connected with IP: " IPSTR, IP2STR(&event->ip_info.ip));

    xTaskCreate(VlReceiveTask, "VL_RECEIVE", 4096, NULL, 5, NULL);
}

void VlReceiveTask(void *param)
{
    ESP_LOGI("RECEIVER", "VlReceiveTask started");

    int port = 6000;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if(sock < 0)
    {
        ESP_LOGE("RECEIVER", "Failed to create socket: errno %d", errno);
        vTaskDelete(NULL);
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        ESP_LOGE("RECEIVER", "Bind failed: errno %d", errno);
        close(sock);
        vTaskDelete(NULL);
    }

    int txSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    struct sockaddr_in errorAddr = {0};
    errorAddr.sin_family = AF_INET;
    errorAddr.sin_port = htons(TARGET_PORT);
    inet_pton(AF_INET, TARGET_IP, &errorAddr.sin_addr);

    while (true)
    {
        int64_t startTime = esp_timer_get_time();
        char status [32] = "";
        AfdxFrame_t frame;
        struct timeval timeout = 
        {
            .tv_sec = 5,
            .tv_usec = 0
        };
        
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        int len = recvfrom(sock, &frame, sizeof(frame), 0, NULL, NULL);
        
        if (len > 0)
        {
            if (AfdxValidateFrame(&frame))
            {
                strcpy(status, "VALID_PAYLOAD");
                int missionCode;
                float altitude;
                uint32_t timeMs;

                int payloadLen = len - sizeof(frame.id) - sizeof(frame.timestamp) - sizeof(frame.crc32);

                AfdxPrintFrame(&frame);
                AfdxParsePayload(&frame, payloadLen, &missionCode, &altitude, &timeMs);

                printf(">>> Parsed Data :\n");
                printf("    Mission Code: %d\n", missionCode);
                printf("    Altitude    : %.1f m\n", altitude);
                printf("    Timestamp   : %lu ms\n", timeMs);
            }
            else
            {
                strcpy(status, "INVALID_CRC32");
                ESP_LOGW("RECEIVER", "Invalid CRC32 detected!");

                const char* errorMsg = "ERROR: CRC32 mismatch in received frame.";
                sendto(txSock, errorMsg, strlen(errorMsg), 0, (struct sockaddr*) &errorAddr, sizeof(errorAddr));
            }
        }
        else
        {
            strcpy(status, "TIMEOUT_OR_NON-PACKET_RECEIVED");
            ESP_LOGW("RECEIVER", "No packet received or timeout.");
        }
        int64_t endTime = esp_timer_get_time();
        ESP_LOGI("RECEIVER: VLRECEIVETASK_TIMESTAMP", "%lld ms", (endTime - startTime)/1000);
        WriteFile("RECEIVER_LOG", endTime - startTime, status);
    }
}

void WriteFile(char* fileName, int64_t timestamp, char status [32])
{
    numCycles++;
    printf("%s|%lld|%s|%ld\n", fileName, timestamp, status, numCycles); 
}

void FlushStdin()
{
    while (getchar() != -1) ;
}

void WaitForStartCommand()
{
    char input[16] = {0};
    while(!start)
    {
        printf("ID=RECEIVER\n");
        for (int i = 0; i < 20; i++)
        {
            if (fgets(input, sizeof(input), stdin))
            {
                if (strncmp(input, "start", 5) == 0)
                {
                    start = true;
                    break;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void app_main(void)
{
    FlushStdin();
    WaitForStartCommand();

    Crc32Init();
    nvs_flash_init();
    ESP_ERROR_CHECK(esp_netif_init());
    esp_event_loop_create_default();
    WifiInitSta();
    ConfigureStaticIp();
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &OnGotIp, NULL);
    ESP_ERROR_CHECK(esp_wifi_connect());
}