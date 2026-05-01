# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "ArduinoSimulatorQt_autogen"
  "CMakeFiles\\ArduinoSimulatorQt_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ArduinoSimulatorQt_autogen.dir\\ParseCache.txt"
  )
endif()
