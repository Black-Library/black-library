set(SOURCES_TESTS
    db_test.cc
    sqlite_db_test.cc
    )

string( REPLACE ".cc" "" BASENAMES_TESTS "${SOURCES_TESTS}" )
set( TARGETS_LIST ${BASENAMES_TESTS} )
    
foreach( name ${TARGETS_LIST} )
    add_executable( ${name} ${name}.cc )
endforeach()

foreach( name ${TARGETS_LIST} )
    target_link_libraries( ${name} Catch2::Catch2WithMain blacklibrarydb )
    include_directories( ${PROJECT_SOURCE_DIR}/test )
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
