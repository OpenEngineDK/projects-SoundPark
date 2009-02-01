# Project name. Should be the same as the project directory name and
# will become the name of the binary target.
SET( PROJECT_NAME "SoundPark")

# Project source code list
SET( PROJECT_SOURCES
  # Add all the cpp source files here
  main.cpp
  Factory.cpp
#  TransformationUpdater.cpp
#  CircleUpdate.cpp
#  WaypointUpdate.cpp
)

# Include if using SDL under Mac OS X
IF(APPLE)
  SET(PROJECT_SOURCES ${PROJECT_SOURCES} ${SDL_MAIN_FOR_MAC})
ENDIF(APPLE)

# Project executable
ADD_EXECUTABLE(${PROJECT_NAME}
  ${PROJECT_SOURCES}
)

# Project dependencies
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
  # Core library dependencies
  OpenEngine_Core
  OpenEngine_Display
  OpenEngine_Resources
  OpenEngine_Scene
  OpenEngine_Logging
  OpenEngine_Devices
  OpenEngine_Scene
  OpenEngine_Utils
  OpenEngine_Geometry
  # Extension dependencies
  Extensions_SDL
  Extensions_GenericHandlers
  Extensions_OpenALSoundSystem
  Extensions_OpenGLRenderer
  Extensions_VorbisResource
  Extensions_Shapes
  Extensions_PropertyList
  Extensions_Sky
  Extensions_TGAResource
)
