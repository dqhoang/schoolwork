#!/bin/bash

x-terminal-emulator --noclose -e ./router test1 A &
x-terminal-emulator --noclose -e ./router test1 B &
x-terminal-emulator --noclose -e ./router test1 C &
x-terminal-emulator --noclose -e ./router test1 D &
x-terminal-emulator --noclose -e ./router test1 E 