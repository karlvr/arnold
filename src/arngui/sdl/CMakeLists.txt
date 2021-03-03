
set(platformSpecific_FILES 
  SDLPlatformSpecific.cpp
  )

include_directories(sdl)
link_libraries( ${SDL_LIBRARY} )

IF(USE_GTK2 MATCHES "Yes")
	link_libraries(${GTK2_LIBRARIES} )
endif(USE_GTK2 MATCHES "Yes")

add_library (platformspecific ${platformSpecific_FILES}) 
