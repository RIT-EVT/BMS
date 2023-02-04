#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "BMS::BMS" for configuration "Debug"
set_property(TARGET BMS::BMS APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(BMS::BMS PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libBMSd.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS BMS::BMS )
list(APPEND _IMPORT_CHECK_FILES_FOR_BMS::BMS "${_IMPORT_PREFIX}/lib/libBMSd.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
