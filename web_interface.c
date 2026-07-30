/*
 * Web Interface Module Implementation
 */

#include <stdio.h>
#include <string.h>
#include "web_interface.h"
#include "modbus_proxy.h"
#include "thread.h"

#ifdef MODULE_NANOCOAP_SOCK
#include "net/nanocoap_sock.h"
#include "net/sock/udp.h"
#endif

#define WEB_SERVER_PORT     5683  /* CoAP port */
#define WEB_THREAD_STACKSIZE (THREAD_STACKSIZE_LARGE)

static char web_server_stack[WEB_THREAD_STACKSIZE];

/* Simple HTML response for main page - will be used in future implementation */
/*
static const char html_page[] = 
    "<!DOCTYPE html><html><head><title>Modbus Proxy</title></head><body>"
    "<h1>Modbus Modifying Proxy</h1>"
    "<p>Configuration interface placeholder</p>"
    "<p>Use CoAP client to interact:</p>"
    "<ul>"
    "<li>GET /status - Show proxy status</li>"
    "<li>GET /rules - Show active rules</li>"
    "<li>POST /clear - Clear all rules</li>"
    "</ul>"
    "</body></html>";
*/

/**
 * @brief Web server thread
 */
static void *web_server_thread(void *arg)
{
    (void)arg;

    puts("Web interface: Starting simplified server");

#ifdef MODULE_NANOCOAP_SOCK
    /* TODO: Implement nanocoap_sock server when ready */
    puts("CoAP functionality available");
#else
    puts("Web interface: CoAP module not available");
#endif

    /* Keep thread alive */
    while (1) {
        thread_sleep();
    }

    return NULL;
}

/**
 * @brief Initialize web interface
 */
int web_interface_init(void)
{
    puts("Initializing web interface...");

    /* Create web server thread */
    thread_create(web_server_stack, sizeof(web_server_stack),
                  THREAD_PRIORITY_MAIN - 2, THREAD_CREATE_STACKTEST,
                  web_server_thread, NULL, "web_server");

    puts("Web interface initialized (simplified version)");
    puts("Full CoAP interface will be implemented later");

    return 0;
}

/**
 * @brief Stop web interface
 */
void web_interface_stop(void)
{
    /* Cleanup if needed */
    puts("Web interface stopped");
}
