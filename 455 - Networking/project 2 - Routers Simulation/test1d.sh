#!/bin/bash

x-terminal-emulator --noclose -e gdb -quiet -ex r --args router test1 A &
x-terminal-emulator --noclose -e gdb -quiet -ex r --args router test1 B &
x-terminal-emulator --noclose -e gdb -quiet -ex r --args router test1 C &
x-terminal-emulator --noclose -e gdb -quiet -ex r --args router test1 D &
x-terminal-emulator --noclose -e gdb -quiet -ex r --args router test1 E 