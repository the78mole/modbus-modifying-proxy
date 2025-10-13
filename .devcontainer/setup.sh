#!/bin/bash
set -e

echo "🚀 Setting up Modbus Modifying Proxy Development Environment..."

# Install ESP-IDF tools if not already installed
echo "🔧 Installing ESP-IDF tools..."
cd /opt/esp/idf
./install.sh esp32

echo "🐚 Setting up automatic ESP-IDF environment activation..."
# Add automatic ESP-IDF activation to .bashrc
cat >> ~/.bashrc << 'EOF'

# Auto-activate ESP-IDF environment
if [ -f "/opt/esp/idf/export.sh" ]; then
    . /opt/esp/idf/export.sh > /dev/null 2>&1
    echo "✅ ESP-IDF environment activated"
fi

# Useful aliases for RIOT development
alias build-esp32='make BOARD=esp32-wroom-32 all'
alias flash-esp32='make BOARD=esp32-wroom-32 flash'
alias monitor-esp32='make BOARD=esp32-wroom-32 term'
alias clean-build='make BOARD=esp32-wroom-32 clean'

EOF

# Also activate for the current session
echo "🔌 Activating ESP-IDF environment for current session..."
. /opt/esp/idf/export.sh

# Verify installation
echo "✅ Verifying ESP-IDF installation..."
idf.py --version
echo "✅ Verifying ESP32 compiler..."
which xtensa-esp32-elf-gcc

echo ""
echo "✨ Development environment setup complete!"
echo ""
echo "🔧 Available commands:"
echo "  build-esp32      - Build for ESP32"
echo "  flash-esp32      - Flash to ESP32" 
echo "  monitor-esp32    - Monitor serial output"
echo "  clean-build      - Clean build artifacts"
echo ""
echo "💡 ESP-IDF environment is automatically activated in new terminals."