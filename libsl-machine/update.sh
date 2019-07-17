#!/bin/bash

echo "Updating sl-machine library..."
cp state_machine.h /usr/include/sl_machine.h
cp bin/Debug/libsl-machine.so /usr/lib/libsl-machine.so.1
echo "Done!"
