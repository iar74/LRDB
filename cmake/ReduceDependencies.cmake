# We statically link to reduce dependencies
if(MSVC)
foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
   string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
   string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
   string(REGEX REPLACE "/MD" "/MT" ${flag_var}_INIT "${${flag_var}_INIT}")
   string(REGEX REPLACE "/MDd" "/MTd" ${flag_var}_INIT "${${flag_var}_INIT}")
endforeach(flag_var)
endif(MSVC)
