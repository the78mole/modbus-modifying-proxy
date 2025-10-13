/*
 * Modbus Proxy Module
 * 
 * Handles bidirectional Modbus communication between two RS485 interfaces
 * and applies configured modifications to register values.
 */

#ifndef MODBUS_PROXY_H
#define MODBUS_PROXY_H

#include <stdint.h>
#include <stdbool.h>

/* Maximum number of modification rules */
#define MAX_MODIFY_RULES    32

/* Modification types */
typedef enum {
    MOD_TYPE_NONE = 0,
    MOD_TYPE_OVERWRITE,
    MOD_TYPE_ADD,
    MOD_TYPE_SUBTRACT,
    MOD_TYPE_MULTIPLY,
    MOD_TYPE_DIVIDE,
    MOD_TYPE_REMAP        /* Register address remapping */
} modify_type_t;

/* Modification rule structure */
typedef struct {
    bool active;
    uint8_t modbus_addr;      /* Modbus device address */
    uint16_t reg_addr;        /* Register address */
    modify_type_t mod_type;   /* Type of modification */
    int32_t param;            /* Modification parameter (value, offset, multiplier, etc.) */
                              /* For MOD_TYPE_REMAP: target register address */
} modify_rule_t;

/* Configuration structure */
typedef struct {
    modify_rule_t rules[MAX_MODIFY_RULES];
    uint16_t rule_count;
} modbus_config_t;

/**
 * @brief Initialize the Modbus proxy
 * 
 * @return 0 on success, negative on error
 */
int modbus_proxy_init(void);

/**
 * @brief Add a modification rule
 * 
 * @param addr Modbus device address
 * @param reg Register address
 * @param type Modification type
 * @param param Modification parameter
 * @return 0 on success, negative on error
 */
int modbus_add_rule(uint8_t addr, uint16_t reg, modify_type_t type, int32_t param);

/**
 * @brief Remove a modification rule
 * 
 * @param addr Modbus device address
 * @param reg Register address
 * @return 0 on success, negative on error
 */
int modbus_remove_rule(uint8_t addr, uint16_t reg);

/**
 * @brief Get all modification rules
 * 
 * @return Pointer to configuration structure
 */
const modbus_config_t* modbus_get_config(void);

/**
 * @brief Clear all modification rules
 */
void modbus_clear_rules(void);

#endif /* MODBUS_PROXY_H */
