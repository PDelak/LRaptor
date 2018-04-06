set(CPPFILES
	lr-editor-lib-driver.cpp
  )

set(PRIVATE_HFILES  
  )

include_directories(${PROJECT_SOURCE_DIR}/lr-editor-lib-driver)

add_executable(lr-editor-lib-driver ${CPPFILES} ${PRIVATE_HFILES})
target_link_libraries (lr-editor-lib-driver gtest gtest_main)

set_property(TARGET lr-editor-lib-driver PROPERTY FOLDER "${LRAPTOR_PREFIX}lr-editor-lib-driver")

add_test(NAME lr-editor-lib-driver COMMAND lr-editor-lib-driver)
