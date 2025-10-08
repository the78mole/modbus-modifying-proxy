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

/* HTML response for main page */
static const char html_page[] = 
    "<!DOCTYPE html><html><head><title>Modbus Proxy Config</title>"
    "<style>"
    "body{font-family:Arial,sans-serif;margin:20px;background:#f0f0f0;}"
    "h1{color:#333;}"
    ".container{background:white;padding:20px;border-radius:8px;max-width:800px;margin:0 auto;}"
    "table{width:100%;border-collapse:collapse;margin:20px 0;}"
    "th,td{border:1px solid #ddd;padding:12px;text-align:left;}"
    "th{background-color:#4CAF50;color:white;}"
    "tr:nth-child(even){background-color:#f2f2f2;}"
    "button{background-color:#4CAF50;color:white;padding:10px 20px;border:none;cursor:pointer;border-radius:4px;}"
    "button:hover{background-color:#45a049;}"
    "input,select{padding:8px;margin:5px 0;border:1px solid #ddd;border-radius:4px;}"
    ".form-group{margin:15px 0;}"
    "label{display:inline-block;width:150px;font-weight:bold;}"
    "</style></head><body>"
    "<div class='container'>"
    "<h1>Modbus Modifying Proxy</h1>"
    "<h2>Configuration</h2>"
    "<div class='form-group'>"
    "<label>Device Address:</label><input type='number' id='addr' min='1' max='247' value='1'>"
    "</div>"
    "<div class='form-group'>"
    "<label>Register Address:</label><input type='number' id='reg' min='0' max='65535' value='0'>"
    "</div>"
    "<div class='form-group'>"
    "<label>Modification Type:</label>"
    "<select id='type'>"
    "<option value='1'>Overwrite</option>"
    "<option value='2'>Add</option>"
    "<option value='3'>Subtract</option>"
    "<option value='4'>Multiply</option>"
    "<option value='5'>Divide</option>"
    "</select>"
    "</div>"
    "<div class='form-group'>"
    "<label>Parameter:</label><input type='number' id='param' value='0'>"
    "</div>"
    "<button onclick='addRule()'>Add Rule</button>"
    "<button onclick='clearRules()'>Clear All Rules</button>"
    "<h2>Active Rules</h2>"
    "<table id='rules'><tr><th>Device</th><th>Register</th><th>Type</th><th>Parameter</th><th>Action</th></tr></table>"
    "<script>"
    "function addRule(){"
    "const addr=document.getElementById('addr').value;"
    "const reg=document.getElementById('reg').value;"
    "const type=document.getElementById('type').value;"
    "const param=document.getElementById('param').value;"
    "fetch('/add?addr='+addr+'&reg='+reg+'&type='+type+'&param='+param).then(()=>location.reload());"
    "}"
    "function clearRules(){"
    "fetch('/clear').then(()=>location.reload());"
    "}"
    "function removeRule(addr,reg){"
    "fetch('/remove?addr='+addr+'&reg='+reg).then(()=>location.reload());"
    "}"
    "</script>"
    "</div></body></html>";

#ifdef MODULE_NANOCOAP_SOCK

/**
 * @brief CoAP GET handler for main page
 */
static ssize_t _riot_board_handler(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *context)
{
    (void)context;
    
    /* Create response */
    gcoap_resp_init(pkt, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pkt, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pkt, COAP_OPT_FINISH_PAYLOAD);
    
    /* Add HTML content */
    if (pkt->payload_len >= strlen(html_page)) {
        memcpy(pkt->payload, html_page, strlen(html_page));
        return resp_len + strlen(html_page);
    }
    
    return resp_len;
}

/**
 * @brief CoAP handler for adding rules
 */
static ssize_t _add_rule_handler(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *context)
{
    (void)context;
    
    /* Parse query parameters - simplified version */
    /* In a real implementation, proper URL parameter parsing would be needed */
    
    gcoap_resp_init(pkt, buf, len, COAP_CODE_CHANGED);
    return coap_opt_finish(pkt, COAP_OPT_FINISH_NONE);
}

/**
 * @brief CoAP handler for clearing rules
 */
static ssize_t _clear_rules_handler(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *context)
{
    (void)context;
    
    modbus_clear_rules();
    
    gcoap_resp_init(pkt, buf, len, COAP_CODE_CHANGED);
    return coap_opt_finish(pkt, COAP_OPT_FINISH_NONE);
}

/* CoAP resources */
static const coap_resource_t _resources[] = {
    { "/", COAP_GET, _riot_board_handler, NULL },
    { "/add", COAP_GET, _add_rule_handler, NULL },
    { "/clear", COAP_GET, _clear_rules_handler, NULL },
};

static gcoap_listener_t _listener = {
    &_resources[0],
    ARRAY_SIZE(_resources),
    NULL,
    NULL
};

#endif /* MODULE_NANOCOAP_SOCK */

/**
 * @brief Web server thread
 */
static void *web_server_thread(void *arg)
{
    (void)arg;

#ifdef MODULE_NANOCOAP_SOCK
    gcoap_register_listener(&_listener);
    puts("CoAP server started on port 5683");
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

    puts("Web interface initialized");
    puts("Access the configuration page at http://<device-ip>:5683/");

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
