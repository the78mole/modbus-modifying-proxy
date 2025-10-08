/*
 * Web Interface Module
 * 
 * Provides HTTP server for configuration management
 */

#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

/**
 * @brief Initialize web interface
 * 
 * @return 0 on success, negative on error
 */
int web_interface_init(void);

/**
 * @brief Stop web interface
 */
void web_interface_stop(void);

#endif /* WEB_INTERFACE_H */
