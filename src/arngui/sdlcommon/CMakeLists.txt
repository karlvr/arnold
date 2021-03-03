
set(sdlcommon_FILES 
  sound.cpp
  )

include_directories(sdlcommon)

# not required on 2.0
#link_libraries( ${SDL_LIBRARY} )

add_library (sdlcommon ${sdlcommon_FILES}) 
