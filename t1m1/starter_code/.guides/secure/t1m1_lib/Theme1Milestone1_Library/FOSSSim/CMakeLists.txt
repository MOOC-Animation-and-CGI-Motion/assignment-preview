# FOSSSim Executable

append_files (Headers "h" .)
append_files (Sources "cpp" .)

#find_package (TCLAP REQUIRED)
#include_directories (${TCLAP_INCLUDE_DIR})

# Locate OpenGL
find_package (OpenGL REQUIRED)
if (OPENGL_FOUND)
  include_directories (${OPENGL_INCLUDE_DIR})
  set (FOSSSIM_LIBRARIES ${FOSSSIM_LIBRARIES} ${OPENGL_LIBRARIES})
else (OPENGL_FOUND)
  message (SEND_ERROR "Unable to locate OpenGL")
endif (OPENGL_FOUND)

# Locate GLUT
find_package (GLUT REQUIRED glut)
if (GLUT_FOUND)
  include_directories (${GLUT_INCLUDE_DIR})
  set (FOSSSIM_LIBRARIES ${FOSSSIM_LIBRARIES} ${GLUT_glut_LIBRARY})
else (GLUT_FOUND)
  message (SEND_ERROR "Unable to locate GLUT")
endif (GLUT_FOUND)

# RapidXML library is required
find_package (RapidXML REQUIRED)
if (RAPIDXML_FOUND)
  include_directories (${RAPIDXML_INCLUDE_DIR})
else (RAPIDXML_FOUND)
  message (SEND_ERROR "Unable to locate RapidXML")
endif (RAPIDXML_FOUND)

# TCLAP library is required
find_package (TCLAP REQUIRED)
if (TCLAP_FOUND)
  include_directories (${TCLAP_INCLUDE_PATH})
else (TCLAP_FOUND)
  message (SEND_ERROR "Unable to locate TCLAP")
endif (TCLAP_FOUND)

find_package (PNG)
if (PNG_FOUND)
  add_definitions (${PNG_DEFINITIONS})
  include_directories (${PNG_INCLUDE_DIR})
  set (FOSSSIM_LIBRARIES ${FOSSSIM_LIBRARIES} ${PNG_LIBRARIES})
endif (PNG_FOUND)

#message(STATUS "Extra libs in FOSSSim: ${FOSSSIM_LIBRARIES}")
#message(STATUS "INSTALL: $CMAKE_INSTALL_PREFIX}")

add_library (FOSSSimT1M1base ${Headers} ${Templates} ${Sources})
target_link_libraries (FOSSSimT1M1base ${FOSSSIM_LIBRARIES})

install(TARGETS FOSSSimT1M1base ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/../Theme1Milestone1_Student/lib)
