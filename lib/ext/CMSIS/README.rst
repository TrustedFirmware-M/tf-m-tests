###############
CMSIS Libraries
###############

The source codes under this folder are copied from:
https://github.com/ARM-software/CMSIS-RTX.git, tag v5.9.1, WITH modifications.

The modifications have been made to RTX_Config.h because the default values in
CMSIS-RTX are different from the standard TF-M requirements.

CMakeLists.txt file is added for TF-M test build purpose.

The prebuilt libraries are taken from the tag 5.9.1:
https://github.com/ARM-software/CMSIS-RTX/releases/tag/v5.9.1

The compilers version are listed in vcpkg-configuration.json in CMSIS-RTX.

The original License for CMSIS is provided  in lib/ext/CMSIS.


--------------

*Copyright (c) The TrustedFirmware-M Contributors. All rights reserved.*
