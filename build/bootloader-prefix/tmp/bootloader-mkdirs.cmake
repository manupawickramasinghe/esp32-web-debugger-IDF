# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/manupa/esp/v5.1.3/esp-idf/components/bootloader/subproject"
  "/home/manupa/Documents/Freelance/esp32-web-debugger-IDF/build/bootloader"
  "/home/manupa/Documents/Freelance/esp32-web-debugger-IDF/build/bootloader-prefix"
  "/home/manupa/Documents/Freelance/esp32-web-debugger-IDF/build/bootloader-prefix/tmp"
  "/home/manupa/Documents/Freelance/esp32-web-debugger-IDF/build/bootloader-prefix/src/bootloader-stamp"
  "/home/manupa/Documents/Freelance/esp32-web-debugger-IDF/build/bootloader-prefix/src"
  "/home/manupa/Documents/Freelance/esp32-web-debugger-IDF/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/manupa/Documents/Freelance/esp32-web-debugger-IDF/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/manupa/Documents/Freelance/esp32-web-debugger-IDF/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
