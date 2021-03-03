
set(platformSpecific_FILES
  SDL2PlatformSpecific.cpp
  )

if (APPLE)
	set(platformSpecific_FILES ${platformSpecific_FILES} SDL2PlatformSpecificMac.m)
endif(APPLE)

include_directories(sdl2)
link_libraries(${SDL2_LIBRARY})

IF(USE_GTK2 MATCHES "Yes")
	link_libraries(${GTK2_LIBRARIES} )
endif(USE_GTK2 MATCHES "Yes")


add_library (platformspecific ${platformSpecific_FILES}) 

