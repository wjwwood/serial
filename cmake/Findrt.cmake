include(FindPackageHandleStandardArgs)
 
find_library(rt_LIBRARY rt)

find_package_handle_standard_args(rt
 DEFAULT_MSG
  rt_LIBRARY
)
mark_as_advanced(rt_LIBRARY)

if(rt_FOUND)
  #message("lib rt FOUND!")
  set(rt_LIBRARIES    ${rt_LIBRARY})
else(rt_FOUND)
  #message("lib rt NOT FOUND!")
endif()
