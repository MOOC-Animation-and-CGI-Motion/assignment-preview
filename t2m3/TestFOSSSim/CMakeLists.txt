# TestFOSSSim Executable

append_files (Headers "h" .)
append_files (Sources "cpp" .)

#find_package (wxWidgets REQUIRED base core gl)
#include (${wxWidgets_USE_FILE})

# Locate Google Test
find_package (GoogleTest REQUIRED)
if (GTEST_FOUND)
    include_directories (${GTEST_INCLUDE_DIRS})
    set (TEST_FOSSSIM_LIBRARIES ${TEST_FOSSSIM_LIBRARIES} ${GTEST_LIBRARIES})
else (GTEST_FOUND)
  message (SEND_ERROR "Unable to locate Google Test")
endif (GTEST_FOUND)

#message(STATUS "Extra libs in TestFOSSSim: ${TEST_FOSSSIM_LIBRARIES}")

add_executable (TestFOSSSim ${Headers} ${Templates} ${Sources})
target_link_libraries (TestFOSSSim ${TEST_FOSSSIM_LIBRARIES})
