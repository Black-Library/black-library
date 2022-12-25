set(SOURCES_TESTS
    config_test.cc
    file_test.cc
    log_test.cc
    source_test.cc
    string_test.cc
    time_test.cc
    )

string( REPLACE ".cc" "" BASENAMES_TESTS "${SOURCES_TESTS}" )
set( TARGETS_LIST ${BASENAMES_TESTS} )
    
foreach( name ${TARGETS_LIST} )
    add_executable( ${name} ${name}.cc )
endforeach()

foreach( name ${TARGETS_LIST} )
    target_link_libraries( ${name} Catch2::Catch2WithMain blacklibrarycommon )
    include_directories( ${PROJECT_SOURCE_DIR}/test/util )
    set_property( TARGET ${name} PROPERTY CXX_STANDARD 14 )
    set_property( TARGET ${name} PROPERTY CXX_EXTENSIONS OFF )
    set_property( TARGET ${name} PROPERTY INSTALL_RPATH "$LD_LIBRARY_PATH;${CMAKE_INSTALL_PREFIX}/lib" )
endforeach()

foreach( name ${TARGETS_LIST} )
    install(TARGETS
        ${name}
        RUNTIME DESTINATION test
    )
endforeach()
