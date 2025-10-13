/*
 * Modbus Modifying Proxy
 * 
 * A bidirectional Modbus proxy for ESP32 that can modify values
 * between two RS485 interfaces with web-based configuration.
 */

#include <stdio.h>
#include <string.h>
#include "thread.h"
#include "xtimer.h"
#include "shell.h"
#include "msg.h"

#include "modbus_proxy.h"
#include "web_interface.h"
#include "wifi_manager.h"

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

int main(void)
{
    /* Initialize message queue */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    puts("Modbus Modifying Proxy starting...");

    /* Initialize WiFi */
    wifi_manager_init();
    
    /* Initialize Modbus proxy */
    modbus_proxy_init();
    
    /* Initialize web interface */
    web_interface_init();

    puts("System initialized successfully");

    /* Start the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
