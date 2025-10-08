# Application name
APPLICATION = modbus-modifying-proxy

# Default board for ESP32
BOARD ?= esp32-wroom-32

# RIOT base directory
RIOTBASE ?= $(CURDIR)/../RIOT

# Include packages
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += xtimer
USEMODULE += periph_uart
USEMODULE += periph_gpio

# WiFi support
USEMODULE += esp_wifi
USEMODULE += esp_now
USEMODULE += esp_eth

# Network stack
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_icmpv6_echo
USEMODULE += gnrc_sock_udp

# HTTP server
USEMODULE += nanocoap_sock

# Storage for configuration
USEMODULE += vfs
USEMODULE += littlefs2

# Enable the use of float in printf
CFLAGS += -DPRINTF_ENABLE_FLOAT

# Set WiFi credentials (can be overridden)
CFLAGS += -DESP_WIFI_SSID=\"ModbusProxy\"
CFLAGS += -DESP_WIFI_PASS=\"modbus123\"

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
