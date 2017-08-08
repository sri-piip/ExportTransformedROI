PROJECT( ExportTransformedROI )
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )
CMAKE_POLICY( SET CMP0053 OLD )

##
## Load the Sedeen dependencies
SET( CMAKE_PREFIX_PATH "C:/Program Files (x86)/Sedeen Viewer SDK/v5.2.2.579/msvc2012" )
FIND_PACKAGE( SEDEENSDK REQUIRED "C:/Program Files (x86)/Sedeen Viewer SDK/v5.2.2.579/msvc2012" )

## Load Boost dependency
SET (CMAKE_PREFIX_PATH "C:/Boost" )
FIND_PACKAGE( BOOST REQUIRED "C:/Boost" )

INCLUDE_DIRECTORIES( "C:/Program Files (x86)/Sedeen Viewer SDK/v5.2.2.579/msvc2012/include" 
					 "C:/Boost/include/boost-1_55")
LINK_DIRECTORIES( "C:/Program Files (x86)/Sedeen Viewer SDK/v5.2.2.579/msvc2012/lib" 
				  "C:/Boost/lib/i386")

##
## Build the code into a module library
ADD_LIBRARY( ExportTransformedROI MODULE ExportTransformedROI.cpp ExportTransformedROI.h)

# Link the library against the Sedeen libraries
# NOTE: The QT libraries must be linked first.
TARGET_LINK_LIBRARIES( ExportTransformedROI image.lib global.lib algorithm.lib archive.lib Comdlg32.lib propsys.lib )

##
## Install the plugin in the sedeen plugins directory
##FIND_PACKAGE( PATHCORE REQUIRED "C:/Program Files (x86)/Sedeen Viewer" )
##IF( ${PATHCORE_FOUND} )
  INSTALL( TARGETS ExportTransformedROI LIBRARY DESTINATION "C:/Program Files (x86)/Sedeen Viewer/plugins/cpp/piip/ExportTransformedROI" )
##ENDIF()
