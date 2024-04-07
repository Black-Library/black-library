
add_library(blacklibraryrestapi BlackLibraryRESTAPI.cc)
target_link_libraries(blacklibraryrestapi blacklibrarydb pistache)
target_include_directories(blacklibraryrestapi PUBLIC ${PROJECT_SOURCE_DIR}/include/rest-api)

install(
  TARGETS blacklibraryrestapi
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set_target_properties(blacklibraryrestapi PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH:${CMAKE_INSTALL_PREFIX}/lib")
