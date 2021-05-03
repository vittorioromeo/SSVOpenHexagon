#!/bin/bash
make check -j8 && make -j8 && cp ./SSVOpenHexagon.exe ../_RELEASE && cp ./OHWorkshopUploader.exe ../_RELEASE && cp ./OHServerControl.exe ../_RELEASE

