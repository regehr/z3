include(CheckCXXCompilerFlag)
include(CMakeParseArguments)

function(z3_add_cxx_flag flag)
  CMAKE_PARSE_ARGUMENTS(z3_add_flag "REQUIRED;GLOBAL" "" "" ${ARGN})
  string(REPLACE "-" "_" SANITIZED_FLAG_NAME "${flag}")
  string(REPLACE "/" "_" SANITIZED_FLAG_NAME "${SANITIZED_FLAG_NAME}")
  string(REPLACE "=" "_" SANITIZED_FLAG_NAME "${SANITIZED_FLAG_NAME}")
  string(REPLACE " " "_" SANITIZED_FLAG_NAME "${SANITIZED_FLAG_NAME}")
  string(REPLACE ":" "_" SANITIZED_FLAG_NAME "${SANITIZED_FLAG_NAME}")
  string(REPLACE "+" "_" SANITIZED_FLAG_NAME "${SANITIZED_FLAG_NAME}")
  unset(HAS_${SANITIZED_FLAG_NAME})
  CHECK_CXX_COMPILER_FLAG("${flag}" HAS_${SANITIZED_FLAG_NAME})
  if (z3_add_flag_REQUIRED AND NOT HAS_${SANITIZED_FLAG_NAME})
    message(FATAL_ERROR "The flag \"${flag}\" is required but your C++ compiler doesn't support it")
  endif()
  if (HAS_${SANITIZED_FLAG_NAME})
    message(STATUS "C++ compiler supports ${flag}")
    if (z3_add_flag_GLOBAL)
      # Set globally
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag} " PARENT_SCOPE)
    else()
      list(APPEND Z3_COMPONENT_CXX_FLAGS "${flag}")
      set(Z3_COMPONENT_CXX_FLAGS "${Z3_COMPONENT_CXX_FLAGS}" PARENT_SCOPE)
    endif()
  else()
    message(STATUS "C++ compiler does not support ${flag}")
  endif()
endfunction()
