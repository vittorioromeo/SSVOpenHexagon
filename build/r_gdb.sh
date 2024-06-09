#!/bin/bash
(cp ./SSVOpenHexagon-Console.exe ../_RELEASE ; cd ../_RELEASE && gdb -- ./SSVOpenHexagon-Console.exe "$@")
