#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"

#define WIFI_SSID "ESP32_SERVER"
#define WIFI_PASS ""
#define SERVER_IP "192.168.4.1"
#define POST_INTERVAL_MS 2000

static const char *TAG = "WiFi_Station_Client";
char post_data[100] = {0};
int16_t value = 14;
int8_t i = 0;
int8_t numbers[2] = {127, 127};


void wifi_init_sta(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi STA connecting to SSID:%s", WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_connect());
}

void post_hello_task(void *pvParameters) {
    while (1) {
        esp_http_client_config_t config = {
            .url = "http://" SERVER_IP "/post",
            .method = HTTP_METHOD_POST,
        };

        esp_http_client_handle_t client = esp_http_client_init(&config);
        //const char *post_data = "Hello";
        //const char *post_data = "10";
        
        post_data[0] = '\0';
        
        for(i = 0; i<2; i++)
        {
			char temp[8];
			
			snprintf(temp, 8, "%d", numbers[i]);
			
			strcat(post_data, temp);
			
			if(i < 1) strcat(post_data, "|");
			
		}
        
        //snprintf(post_data, 100, "%d", value);

        esp_http_client_set_post_field(client, post_data, strlen(post_data));
        esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");

        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "POST sent successfully");
        } else {
            ESP_LOGE(TAG, "POST failed: %s", esp_err_to_name(err));
        }

        esp_http_client_cleanup(client);
        vTaskDelay(pdMS_TO_TICKS(POST_INTERVAL_MS));
    }
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();

    // Delay to allow connection
    vTaskDelay(pdMS_TO_TICKS(5000));

    xTaskCreate(&post_hello_task, "post_hello_task", 8192, NULL, 5, NULL);
}
