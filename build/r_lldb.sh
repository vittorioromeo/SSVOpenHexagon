#!/bin/bash
(cp ./SSVOpenHexagon-Console.exe ../_RELEASE ; cd ../_RELEASE && lldb -- ./SSVOpenHexagon-Console.exe "$@")


