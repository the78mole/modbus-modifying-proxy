/*
 * WiFi Manager Module
 * 
 * Manages WiFi connection in both AP and STA modes
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

/* Use ESP32 WiFi types if available */
#ifdef ESP_PLATFORM
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#else
/**
 * @brief WiFi operation modes
 */
typedef enum {
    WIFI_MODE_AP = 0,   /* Access Point mode */
    WIFI_MODE_STA,      /* Station mode */
    WIFI_MODE_APSTA     /* Both AP and STA mode */
} wifi_mode_t;
#endif

/**
 * @brief Initialize WiFi manager
 * 
 * @return 0 on success, negative on error
 */
int wifi_manager_init(void);

/**
 * @brief Set WiFi mode
 * 
 * @param mode WiFi mode to set
 * @return 0 on success, negative on error
 */
int wifi_set_mode(wifi_mode_t mode);

/**
 * @brief Connect to WiFi network in STA mode
 * 
 * @param ssid Network SSID
 * @param password Network password
 * @return 0 on success, negative on error
 */
int wifi_connect(const char *ssid, const char *password);

/**
 * @brief Start WiFi AP mode
 * 
 * @param ssid AP SSID
 * @param password AP password
 * @return 0 on success, negative on error
 */
int wifi_start_ap(const char *ssid, const char *password);

/**
 * @brief Get current WiFi mode
 * 
 * @return Current WiFi mode
 */
wifi_mode_t wifi_get_mode(void);

/**
 * @brief Check if WiFi is connected (in STA mode)
 * 
 * @return true if connected, false otherwise
 */
bool wifi_is_connected(void);

#endif /* WIFI_MANAGER_H */
