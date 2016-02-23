include(yamail_find_dependencies)
function(find_yreflection_dependencies)

find_package(MYBOOST)

set(YREFLECTION_DEPENDENCY_LIBRARIES
  ${MYBOOST_LIBRARIES}
  CACHE INTERNAL "")

set(YREFLECTION_DEPENDENCY_INCLUDE_DIRS
  ${MYBOOST_INCLUDE_DIR}
  CACHE INTERNAL "")

endfunction()
