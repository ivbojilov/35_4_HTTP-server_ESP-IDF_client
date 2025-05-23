#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "esp_timer.h"

#define WIFI_SSID "ESP32_AP"
#define WIFI_PASS "12345678"
#define MAX_NUMBERS 400

static const char *TAG = "ESP32_CLIENT";

static int8_t numbers[MAX_NUMBERS];
static char body[1800];

static void wifi_init_sta(void);
static void post_task(void *pvParameters);

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Init networking stack
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_sta();

    // Fill numbers array
    for (int i = 0; i < MAX_NUMBERS; i++) {
        numbers[i] = (i / 20) % 2 == 0 ? 127 : -128;
    }

    // Format body
    body[0] = '\0';
    for (int i = 0; i < MAX_NUMBERS; i++) {
        char temp[8];
        snprintf(temp, sizeof(temp), "%d", numbers[i]);
        strcat(body, temp);
        if (i < MAX_NUMBERS - 1) {
            strcat(body, "|");
        }
    }

    // Create the POST task
    xTaskCreate(&post_task, "post_task", 8192, NULL, 5, NULL);
}

static void wifi_init_sta(void) {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialization complete.");

    // Wait for connection
    ESP_LOGI(TAG, "Connecting to AP...");
    esp_wifi_connect();

    // Wait for IP event (blocking)
    vTaskDelay(5000 / portTICK_PERIOD_MS);  // Allow time to connect
}

static void post_task(void *pvParameters) {
    while (1) {
        esp_http_client_config_t config = {
            .url = "http://192.168.4.1/",
        };
        esp_http_client_handle_t client = esp_http_client_init(&config);

        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_header(client, "Content-Type", "text/plain");
        esp_http_client_set_post_field(client, body, strlen(body));

        esp_err_t err = esp_http_client_perform(client);

        if (err == ESP_OK) {
            ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %lld",
                     esp_http_client_get_status_code(client),
                     esp_http_client_get_content_length(client));
        } else {
            ESP_LOGE(TAG, "HTTP POST failed: %s", esp_err_to_name(err));
        }

        esp_http_client_cleanup(client);

        // Wait 30ms before next request
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}
