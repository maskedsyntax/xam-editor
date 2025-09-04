#!/bin/bash

echo "Building XamEditor..."

# Navigate to source directory
cd xameditor

# Create .pro file
cat > xameditor.pro << 'EOF'
QT += core widgets
CONFIG += c++17
TARGET = xam-editor
SOURCES += xam.cpp
EOF

# Build
qmake xameditor.pro
make

# Move executable to bin directory
if [ $? -eq 0 ]; then
    mv xam-editor ../bin/
    echo "Build successful! Executable is in bin/xam-editor"

    # Clean up build files
    rm -f xameditor.pro Makefile *.o moc_*

    cd ..
    echo "Run with: ./bin/xam-editor"
else
    echo "Build failed!"
    exit 1
fi
