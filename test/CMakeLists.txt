
set(_test_sources
	testbase.cpp
	stringfromtests.cpp)

if(ENABLE_JSON)
	set(_test_sources
		${_test_sources}
		jsontests.cpp)
endif()
		
add_executable(tests ${_test_sources})
target_include_directories(tests PRIVATE "${BBCAT_COMMON_DIR}/include")
target_link_libraries(tests bbcat-base${LINKTYPE})

# create custom target to run tests
add_custom_target(test ALL
				  DEPENDS tests
				  COMMAND tests --use-colour no
				  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
