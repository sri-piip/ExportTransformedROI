PROJECT( ExportTransformedROI )
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )
CMAKE_POLICY( SET CMP0053 OLD )

##
## Load the Sedeen dependencies
SET( CMAKE_PREFIX_PATH "C:/Program Files (x86)/Sedeen Viewer SDK/v5.2.1.427/msvc2012" )
FIND_PACKAGE( SEDEENSDK REQUIRED "C:/Program Files (x86)/Sedeen Viewer SDK/v5.2.1.427/msvc2012" )
## Load OpenCV dependency
SET (CMAKE_PREFIX_PATH "D:/Documents/Libraries/opencv-3.1.0_Contrib/build/install" )
FIND_PACKAGE( OPENCV REQUIRED "D:/Documents/Libraries/opencv-3.1.0_Contrib/build/install" )

INCLUDE_DIRECTORIES( "C:/Program Files (x86)/Sedeen Viewer SDK/v5.2.1.427/msvc2012/include" "D:/Documents/Libraries/opencv-3.1.0_Contrib/build/install/include" "D:/Documents/Libraries/opencv-3.1.0_Contrib/build/install/include/opencv" "D:/Documents/Libraries/opencv-3.1.0_Contrib/build/install/include/opencv2")
LINK_DIRECTORIES( "C:/Program Files (x86)/Sedeen Viewer SDK/v5.2.1.427/msvc2012/lib" "D:/Documents/Libraries/opencv-3.1.0_Contrib/build/install/x86/vc12/lib" )

##
## Build the code into a module library
ADD_LIBRARY( ExportTransformedROI MODULE ExportTransformedROI.cpp ExportTransformedROI.h)

# Link the library against the Sedeen libraries
# NOTE: The QT libraries must be linked first.
TARGET_LINK_LIBRARIES( ExportTransformedROI image.lib global.lib algorithm.lib archive.lib Comdlg32.lib ) 

##
## Install the plugin in the sedeen plugins directory
##IF( ${PATHCORE_FOUND} )
  INSTALL( TARGETS ExportTransformedROI LIBRARY DESTINATION "C:/Program Files (x86)/Sedeen Viewer/plugins" )
##ENDIF()
