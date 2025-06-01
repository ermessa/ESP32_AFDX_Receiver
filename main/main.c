#include <stdio.h>
#include "string.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "afdx.h"
#include "crc32.h"

//INSERT YOUR LOCAL NETWORK DATA
#define WIFI_SSID "ESP32_SENDER_AP"
#define WIFI_PASS "123qweasd"

//CONFIGURE STATIC DEVICE
#define ESP_IP "192.168.1.150"
#define ESP_GATEWAY "192.168.1.1"
#define ESP_SUBNET_MASK "255.255.255.0"

//PROTOTYPES

/// @brief INIT WI-FI STATION WITH DEFAULT CONFIGURATION
void wifiInitSta(void);

/// @brief 
/// @param param 
void vlReceiveTask(void *param);

/// @brief CONFIGURE STATIC IP
void configureStaticIp();

//FUNCTIONS

void configureStaticIp()
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

void wifiInitSta()
{
    esp_netif_init();
    esp_event_loop_create_default();
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

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    esp_wifi_start();
}

void vlReceiveTask(void *param)
{
    int port = 6000;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));

    while (true)
    {
        AfdxFrame_t frame;
        int len = recvfrom(sock, &frame, sizeof(frame), 0, NULL, NULL);
        if (len > 0 && AfdxValidateFrame(&frame))
        {
            int missionCode;
            float altitude;
            uint32_t timeMs;

            AfdxPrintFrame(&frame);
            AfdxParsePayload(&frame, &missionCode, &altitude, &timeMs);

            printf(">>> Parsed Data :\n");
            printf("    Mission Code: %d\n", missionCode);
            printf("    Altitude    : %.1f m\n", altitude);
            printf("    Timestamp   : %lu ms\n", timeMs);
        }
    }
}

void app_main(void)
{
    nvs_flash_init();
    wifiInitSta();
    configureStaticIp();
    esp_wifi_connect();
    crc32Init();

    xTaskCreate(vlReceiveTask, "VL_RECEIVE", 4096, NULL, 5, NULL);
}