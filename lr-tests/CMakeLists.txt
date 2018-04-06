set(CPPFILES
	lr-basic-driver.cpp
  )

set(PRIVATE_HFILES  
  )

include_directories(${PROJECT_SOURCE_DIR}/lr-tests)

add_executable(lr-tests ${CPPFILES} ${PRIVATE_HFILES})
target_link_libraries (lr-tests gtest gtest_main)

set_property(TARGET lr-tests PROPERTY FOLDER "${LRAPTOR_PREFIX}test")

add_test(NAME lr-tests COMMAND lr-tests)
