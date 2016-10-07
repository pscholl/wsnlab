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

SET( CMAKE_CXX_STANDARD 14 )

# Set compiler and linker flags
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fdiagnostics-color=always" )
# SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lmraa" )

# set binary output directory
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )

# Some include directories
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include )
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/LcdDriver )
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/GrLib/grlib )

# Boost specific stuff
#INCLUDE_DIRECTORIES( $ENV{HOME}/boost_1_60_0 )
#LINK_DIRECTORIES( $ENV{HOME}/boost_1_60_0/stage/lib )
#SET( BOOST_ROOT $ENV{HOME}/boost_1_60_0 )

# Find necessary packages
FIND_PACKAGE( Threads )
FIND_PACKAGE( Boost 1.56 COMPONENTS program_options system REQUIRED )

# Source files that are not executables
SET( DSP src/display_edison.cpp )
SET( IMU src/imu_edison.cpp )
SET( BAT src/batgauge_edison.cpp )
SET( LDC src/ldc_edison.cpp )



#
# LIBRARIES
#

# display libraries
file( GLOB LCDDRIVER_SRC "LcdDriver/*.c" "LcdDriver/*.h" )
SET_SOURCE_FILES_PROPERTIES( ${LCDDRIVER_SRC} PROPERTIES LANGUAGE CXX )
ADD_LIBRARY( LcdDriver ${LCDDRIVER_SRC} )

file( GLOB GRLIB_SRC "GrLib/grlib/*.c" "GrLib/grlib/*.h" )
file( GLOB GRLIB_FONT_SRC "GrLib/fonts/*.c" )
ADD_LIBRARY( GrLib ${GRLIB_SRC} ${GRLIB_FONT_SRC} )

ADD_LIBRARY( dsp ${DSP} )
TARGET_LINK_LIBRARIES( dsp LcdDriver GrLib )

# other libraries
ADD_LIBRARY( imu ${IMU} )
ADD_LIBRARY( bat ${BAT} )
ADD_LIBRARY( ldc ${LDC} )



#
# EXECUTABLES
#

ADD_EXECUTABLE( platypusTest src/platypusTest.cpp )
TARGET_LINK_LIBRARIES( platypusTest dsp imu bat ldc -lmraa ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} )

ADD_EXECUTABLE( batgaugeTest src/batgaugeTest.cpp )
TARGET_LINK_LIBRARIES( batgaugeTest bat -lmraa ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} )








