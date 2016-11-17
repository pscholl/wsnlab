#
# This is a CMake makefile.  You can find the cmake utility and
# information about it at http://www.cmake.org
#

CMAKE_MINIMUM_REQUIRED( VERSION 3.0.0 )

PROJECT( platypus )

# make sure cmake does not overwrite existing Makefile
MACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD MSG)
  STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" insource)
  GET_FILENAME_COMPONENT(PARENTDIR ${CMAKE_SOURCE_DIR} PATH)
  STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${PARENTDIR}" insourcesubdir)
  IF(insource OR insourcesubdir)
    MESSAGE(FATAL_ERROR "${MSG}")
  ENDIF(insource OR insourcesubdir)
ENDMACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD)

MACRO_ENSURE_OUT_OF_SOURCE_BUILD( "
Project '${CMAKE_PROJECT_NAME}' requires an out of source build.
Create a build directory and run 'cmake ..' in there.
You may also want to delete CMakeFiles/ and CMakeCache.txt at this point to clean your source tree.
" )



#
# CONFIGURATION
#

SET( CMAKE_VERBOSE_MAKEFILE on )

SET( CMAKE_CXX_STANDARD 11 )

SET( CMAKE_BUILD_TYPE Release )
#SET( CMAKE_C_FLAGS_RELEASE "..." )
#SET( CMAKE_CXX_FLAGS_RELEASE "..." )

# Set compiler and linker flags
SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdiagnostics-color=always" )
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fdiagnostics-color=always" )
# SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lmraa" )

# set binary output directory
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib )
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib )

# Boost specific stuff
#INCLUDE_DIRECTORIES( $ENV{HOME}/boost_1_60_0 )
#LINK_DIRECTORIES( $ENV{HOME}/boost_1_60_0/stage/lib )
#SET( BOOST_ROOT $ENV{HOME}/boost_1_60_0 )

# Find necessary packages
FIND_PACKAGE( Threads )
FIND_PACKAGE( Boost 1.56 COMPONENTS program_options system REQUIRED )

#
# LIBRARIES
#

# display libraries
file( GLOB GRLIB_SRC "GrLib/grlib/*.c" )
file( GLOB GRLIB_FONT_SRC "GrLib/fonts/*.c" )
ADD_LIBRARY( GrLib ${GRLIB_SRC} ${GRLIB_FONT_SRC} )
TARGET_INCLUDE_DIRECTORIES( GrLib PUBLIC GrLib/grlib )

file( GLOB LCDDRIVER_SRC "LcdDriver/*.c" )
SET_SOURCE_FILES_PROPERTIES( ${LCDDRIVER_SRC} PROPERTIES LANGUAGE CXX )
ADD_LIBRARY( LcdDriver ${LCDDRIVER_SRC} )
TARGET_LINK_LIBRARIES( LcdDriver GrLib )
TARGET_INCLUDE_DIRECTORIES( LcdDriver INTERFACE LcdDriver )

ADD_LIBRARY( platypus src/display_edison.cpp src/imu_edison.cpp src/batgauge_edison.cpp src/ldc_edison.cpp src/SharpLCD.cpp)
TARGET_LINK_LIBRARIES( platypus LcdDriver GrLib )
TARGET_INCLUDE_DIRECTORIES( platypus PUBLIC include )

#
# EXECUTABLES
#

ADD_EXECUTABLE( lcdTest src/lcdTest.cpp )
TARGET_LINK_LIBRARIES( lcdTest platypus -lmraa ${CMAKE_THREAD_LIBS_INIT} )

ADD_EXECUTABLE( platypusTest src/platypusTest.cpp )
TARGET_LINK_LIBRARIES( platypusTest platypus -lmraa ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )








