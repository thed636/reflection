include(find_yreflection_dependencies)

if(YREFLECTION_FOUND)
  set(YREFLECTION_FIND_QUIETLY TRUE)
endif()

find_path(YREFLECTION_INCLUDE_DIR
  NAMES yamail/data/reflection/reflection.h
  HINTS ../yreflection/include
        ${CMAKE_INSTALL_PREFIX}/include)

find_yreflection_dependencies()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(YREFLECTION DEFAULT_MSG
  YREFLECTION_INCLUDE_DIR)

if(YREFLECTION_FOUND)
  list(APPEND YREFLECTION_INCLUDE_DIR ${YREFLECTION_DEPENDENCY_INCLUDE_DIRS})
endif()

mark_as_advanced(YREFLECTION_INCLUDE_DIR YREFLECTION_LIBRARIES)
