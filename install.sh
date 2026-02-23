#!/bin/bash
set -e

echo "Starting installation of MyStrace..."

if [ -f "Makefile" ]; then
  echo "Compiling binary..."
  make clean
  make
else
  echo "Error: Makefile not found."
  exit 1
fi

echo "Installing to /usr/local/bin..."
sudo make install

echo "-----------------------------------------------"
echo "Installation complete."
echo "You can now run the app by typing 'mystrace' in any terminal."
