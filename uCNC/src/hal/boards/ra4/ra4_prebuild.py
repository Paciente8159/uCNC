"""
Pre-build script for RA4 (Renesas RA4M1) builds.

Disables the Arduino core's built-in tinyUSB source files and USB
core files (USB.cpp, SerialUSB.cpp) to avoid symbol conflicts with
the custom uCNC-tinyusb fork.
"""

import os
from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
FRAMEWORK_DIR = env.PioPlatform().get_package_dir("framework-arduinorenesas-uno")
CORE_DIR = os.path.join(FRAMEWORK_DIR, "cores", "arduino")

DIRS_TO_DISABLE = [
    "tinyusb",   # duplicates tinyUSB library
    "usb",       # USB.cpp/SerialUSB.cpp duplicate tinyUSB callbacks
]

def _disable_sources(src_dir):
    """Rename .c/.cpp -> .c.disabled to prevent compilation.
    Leaves .h files intact so includes still resolve."""
    if not os.path.isdir(src_dir):
        return

    for entry in os.listdir(src_dir):
        child = os.path.join(src_dir, entry)
        if os.path.isdir(child):
            _disable_sources(child)  # recurse
        elif entry.endswith('.c') or entry.endswith('.cpp'):
            dst = child + ".disabled"
            if not os.path.isfile(dst):
                try:
                    os.rename(child, dst)
                except OSError:
                    pass

try:
    for subdir in DIRS_TO_DISABLE:
        _disable_sources(os.path.join(CORE_DIR, subdir))
except Exception:
    pass