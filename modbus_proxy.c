/*
 * Modbus Proxy Module Implementation
 */

#include <stdio.h>
#include <string.h>
#include "modbus_proxy.h"
#include "thread.h"
#include "xtimer.h"
#include "periph/uart.h"
#include "periph/gpio.h"

/* UART configuration for RS485 interfaces */
#define UART_IF1            UART_DEV(1)
#define UART_IF2            UART_DEV(2)
#define UART_BAUDRATE       9600
/* Use valid GPIO pins for ESP32 - disable RS485 control for now */
/*#define RS485_DE_IF1        GPIO_PIN(0, 4)*/     /* Driver Enable for Interface 1 */
/*#define RS485_DE_IF2        GPIO_PIN(0, 5)*/     /* Driver Enable for Interface 2 */

/* Thread stack sizes */
#define PROXY_THREAD_STACKSIZE  (THREAD_STACKSIZE_LARGE)

/* Thread stacks */
static char if1_to_if2_stack[PROXY_THREAD_STACKSIZE];
static char if2_to_if1_stack[PROXY_THREAD_STACKSIZE];

/* Configuration storage */
static modbus_config_t config = {0};

/* Mutex for configuration access */
static mutex_t config_mutex = MUTEX_INIT;

/* Buffer for Modbus frames */
#define MODBUS_MAX_FRAME_SIZE   256
static uint8_t if1_buffer[MODBUS_MAX_FRAME_SIZE];
static uint8_t if2_buffer[MODBUS_MAX_FRAME_SIZE];
static volatile size_t if1_rx_len = 0;
static volatile size_t if2_rx_len = 0;
static volatile bool if1_frame_ready = false;
static volatile bool if2_frame_ready = false;

/* Forward declarations */
static void *if1_to_if2_thread(void *arg);
static void *if2_to_if1_thread(void *arg);
static int32_t apply_modification(uint8_t addr, uint16_t reg, int32_t value);
static uint16_t modbus_crc(uint8_t *buf, int len);
static void uart1_rx_callback(void *arg, uint8_t data);
static void uart2_rx_callback(void *arg, uint8_t data);

/**
 * @brief Initialize RS485 interfaces
 */
static int init_rs485_interfaces(void)
{
    /* Initialize UART devices with RX callbacks */
    if (uart_init(UART_IF1, UART_BAUDRATE, uart1_rx_callback, NULL) < 0) {
        puts("Error: Failed to initialize UART Interface 1");
        return -1;
    }

    if (uart_init(UART_IF2, UART_BAUDRATE, uart2_rx_callback, NULL) < 0) {
        puts("Error: Failed to initialize UART Interface 2");
        return -2;
    }

    /* TODO: Initialize GPIO for RS485 driver enable when GPIO issues are resolved */
    /*
    if (gpio_init(RS485_DE_IF1, GPIO_OUT) < 0) {
        puts("Error: Failed to initialize RS485 DE pin for Interface 1");
        return -3;
    }

    if (gpio_init(RS485_DE_IF2, GPIO_OUT) < 0) {
        puts("Error: Failed to initialize RS485 DE pin for Interface 2");
        return -4;
    }

    set_rs485_mode(RS485_DE_IF1, false);
    set_rs485_mode(RS485_DE_IF2, false);
    */

    puts("RS485 interfaces initialized");
    return 0;
}

/**
 * @brief Set RS485 transceiver mode (placeholder for future implementation)
 */
static void set_rs485_mode(bool transmit)
{
    /* TODO: Implement RS485 driver enable control */
    (void)transmit; /* Suppress unused parameter warning */
}

/**
 * @brief UART 1 receive callback
 */
static void uart1_rx_callback(void *arg, uint8_t data)
{
    (void)arg;
    if (if1_rx_len < MODBUS_MAX_FRAME_SIZE) {
        if1_buffer[if1_rx_len++] = data;
        /* TODO: Implement proper frame detection with timeout */
    }
}

/**
 * @brief UART 2 receive callback  
 */
static void uart2_rx_callback(void *arg, uint8_t data)
{
    (void)arg;
    if (if2_rx_len < MODBUS_MAX_FRAME_SIZE) {
        if2_buffer[if2_rx_len++] = data;
        /* TODO: Implement proper frame detection with timeout */
    }
}

/**
 * @brief Calculate Modbus CRC16
 */
static uint16_t modbus_crc(uint8_t *buf, int len)
{
    uint16_t crc = 0xFFFF;
    
    for (int pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];
        
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

/**
 * @brief Apply modification to a value based on configured rules
 */
static int32_t apply_modification(uint8_t addr, uint16_t reg, int32_t value)
{
    mutex_lock(&config_mutex);
    
    for (uint16_t i = 0; i < config.rule_count; i++) {
        if (config.rules[i].active && 
            config.rules[i].modbus_addr == addr && 
            config.rules[i].reg_addr == reg) {
            
            switch (config.rules[i].mod_type) {
                case MOD_TYPE_OVERWRITE:
                    value = config.rules[i].param;
                    break;
                    
                case MOD_TYPE_ADD:
                    value += config.rules[i].param;
                    break;
                    
                case MOD_TYPE_SUBTRACT:
                    value -= config.rules[i].param;
                    break;
                    
                case MOD_TYPE_MULTIPLY:
                    value *= config.rules[i].param;
                    break;
                    
                case MOD_TYPE_DIVIDE:
                    if (config.rules[i].param != 0) {
                        value /= config.rules[i].param;
                    }
                    break;
                    
                default:
                    break;
            }
            break; /* Apply only first matching rule */
        }
    }
    
    mutex_unlock(&config_mutex);
    return value;
}

/**
 * @brief Process Modbus frame and apply modifications
 * 
 * @param frame Pointer to Modbus frame buffer
 * @param len Pointer to frame length
 * @param is_request true if frame is a request (master->slave), false if response (slave->master)
 */
static void process_modbus_frame(uint8_t *frame, size_t *len, bool is_request)
{
    /* Minimum Modbus frame: addr(1) + func(1) + data(2) + crc(2) = 6 bytes */
    if (*len < 6) {
        return;
    }

    uint8_t addr = frame[0];
    uint8_t func = frame[1];

    /* Handle register remapping in requests (0x03, 0x04 read commands) */
    if (is_request && (func == 0x03 || func == 0x04) && *len >= 8) {
        /* Request format: addr(1) + func(1) + start_addr_H(1) + start_addr_L(1) + count_H(1) + count_L(1) + crc(2) */
        uint16_t start_reg = (frame[2] << 8) | frame[3];
        /* TODO: Implement register remapping if needed */
        (void)start_reg; /* Suppress unused variable warning */
    }
    
    /* Handle value modifications and reverse register remapping in responses */
    if (!is_request && (func == 0x03 || func == 0x04) && *len > 4) {
        uint8_t byte_count = frame[2];
        
        /* Verify frame structure */
        if (*len < (size_t)(3 + byte_count + 2)) {
            return;
        }

        /* Process register values (assuming 16-bit registers) */
        for (int i = 0; i < byte_count / 2; i++) {
            uint16_t reg_offset = i; /* Simplified - actual register address would need to be tracked */
            int32_t value = (frame[3 + i * 2] << 8) | frame[4 + i * 2];
            
            /* Apply value modification */
            int32_t modified = apply_modification(addr, reg_offset, value);
            
            /* Update frame if value changed */
            if (modified != value) {
                frame[3 + i * 2] = (modified >> 8) & 0xFF;
                frame[4 + i * 2] = modified & 0xFF;
            }
        }

        /* Recalculate CRC */
        uint16_t crc = modbus_crc(frame, *len - 2);
        frame[*len - 2] = crc & 0xFF;
        frame[*len - 1] = (crc >> 8) & 0xFF;
    }
}

/**
 * @brief Thread: Forward from Interface 1 to Interface 2
 */
static void *if1_to_if2_thread(void *arg)
{
    (void)arg;
    
    while (1) {
        /* TODO: Implement proper frame reception using callbacks and timeouts */
        /* For now, this is a simplified polling version */
        
        if (if1_frame_ready) {
            size_t len = if1_rx_len;
            
            if (len > 0) {
                /* Process and modify frame - requests from master (IF1) to slave (IF2) */
                process_modbus_frame(if1_buffer, &len, true);

                /* Forward to Interface 2 */
                set_rs485_mode(true);  /* TX mode */
                uart_write(UART_IF2, if1_buffer, len);
                xtimer_usleep(1000 * len); /* Wait for transmission */
                set_rs485_mode(false); /* RX mode */
            }
            
            /* Reset for next frame */
            if1_rx_len = 0;
            if1_frame_ready = false;
        }

        xtimer_usleep(1000); /* Small delay */
    }

    return NULL;
}

/**
 * @brief Thread: Forward from Interface 2 to Interface 1
 */
static void *if2_to_if1_thread(void *arg)
{
    (void)arg;
    
    while (1) {
        /* TODO: Implement proper frame reception using callbacks and timeouts */
        /* For now, this is a simplified polling version */
        
        if (if2_frame_ready) {
            size_t len = if2_rx_len;
            
            if (len > 0) {
                /* Process and modify frame - responses from slave (IF2) to master (IF1) */
                process_modbus_frame(if2_buffer, &len, false);

                /* Forward to Interface 1 */
                set_rs485_mode(true);  /* TX mode */
                uart_write(UART_IF1, if2_buffer, len);
                xtimer_usleep(1000 * len); /* Wait for transmission */
                set_rs485_mode(false); /* RX mode */
            }
            
            /* Reset for next frame */
            if2_rx_len = 0;
            if2_frame_ready = false;
        }

        xtimer_usleep(1000); /* Small delay */
    }

    return NULL;
}

/**
 * @brief Initialize the Modbus proxy
 */
int modbus_proxy_init(void)
{
    puts("Initializing Modbus proxy...");

    /* Initialize RS485 interfaces */
    if (init_rs485_interfaces() < 0) {
        return -1;
    }

    /* Initialize configuration */
    memset(&config, 0, sizeof(config));

    /* Create proxy threads */
    thread_create(if1_to_if2_stack, sizeof(if1_to_if2_stack),
                  THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                  if1_to_if2_thread, NULL, "if1_to_if2");

    thread_create(if2_to_if1_stack, sizeof(if2_to_if1_stack),
                  THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                  if2_to_if1_thread, NULL, "if2_to_if1");

    puts("Modbus proxy initialized");
    return 0;
}

/**
 * @brief Add a modification rule
 */
int modbus_add_rule(uint8_t addr, uint16_t reg, modify_type_t type, int32_t param)
{
    mutex_lock(&config_mutex);

    /* Check if rule already exists */
    for (uint16_t i = 0; i < config.rule_count; i++) {
        if (config.rules[i].modbus_addr == addr && config.rules[i].reg_addr == reg) {
            /* Update existing rule */
            config.rules[i].mod_type = type;
            config.rules[i].param = param;
            config.rules[i].active = true;
            mutex_unlock(&config_mutex);
            return 0;
        }
    }

    /* Add new rule */
    if (config.rule_count < MAX_MODIFY_RULES) {
        config.rules[config.rule_count].active = true;
        config.rules[config.rule_count].modbus_addr = addr;
        config.rules[config.rule_count].reg_addr = reg;
        config.rules[config.rule_count].mod_type = type;
        config.rules[config.rule_count].param = param;
        config.rule_count++;
        mutex_unlock(&config_mutex);
        return 0;
    }

    mutex_unlock(&config_mutex);
    return -1; /* No space for new rule */
}

/**
 * @brief Remove a modification rule
 */
int modbus_remove_rule(uint8_t addr, uint16_t reg)
{
    mutex_lock(&config_mutex);

    for (uint16_t i = 0; i < config.rule_count; i++) {
        if (config.rules[i].modbus_addr == addr && config.rules[i].reg_addr == reg) {
            /* Mark as inactive or shift array */
            config.rules[i].active = false;
            mutex_unlock(&config_mutex);
            return 0;
        }
    }

    mutex_unlock(&config_mutex);
    return -1; /* Rule not found */
}

/**
 * @brief Get all modification rules
 */
const modbus_config_t* modbus_get_config(void)
{
    return &config;
}

/**
 * @brief Clear all modification rules
 */
void modbus_clear_rules(void)
{
    mutex_lock(&config_mutex);
    memset(&config, 0, sizeof(config));
    config.rule_count = 0;
    mutex_unlock(&config_mutex);
}
