# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\TonyLabClient_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\TonyLabClient_autogen.dir\\ParseCache.txt"
  "TonyLabClient_autogen"
  )
endif()
