
include_directories(${arnold_SOURCE_DIR}/cpc)
include_directories(${arnold_SOURCE_DIR}/cpc/debugger)

# add arnold z80 core if not using inkz80
IF(USE_INKZ80 MATCHES "Yes")
else(USE_INKZ80 MATCHES "Yes")
include_directories(${arnold_SOURCE_DIR}/cpc/z80)
endif(USE_INKZ80 MATCHES "Yes")

include_directories(${arnold_SOURCE_DIR}/cpc/z80tools)
include_directories(${arnold_SOURCE_DIR}/cpc/diskimage)

# include sdl "platform specific" files
if (USING_SDL MATCHES "Yes")
add_subdirectory(sdl)
include_directories(${arnold_SOURCE_DIR}/arngui/sdl)
link_directories(${arnold_SOURCE_DIR}/arngui/sdl)

# add common sdl files
add_subdirectory(sdlcommon)
include_directories(${arnold_SOURCE_DIR}/arngui/sdlcommon)
link_directories(${arnold_SOURCE_DIR}/arngui/sdlcommon)

endif(USING_SDL MATCHES "Yes")

# include sdl2 "platform specific" files
if (USING_SDL2 MATCHES "Yes")
add_subdirectory(sdl2)
include_directories(${arnold_SOURCE_DIR}/arngui/sdl2)
link_directories(${arnold_SOURCE_DIR}/arngui/sdl2)

# add common files
add_subdirectory(sdlcommon)
include_directories(${arnold_SOURCE_DIR}/arngui/sdlcommon)
link_directories(${arnold_SOURCE_DIR}/arngui/sdlcommon)

endif(USING_SDL2 MATCHES "Yes")

# gui source files
set(gui_FILES
		AboutDialog.cpp
		ArchiveDialog.cpp
		arnguiApp.cpp
		arnguiMain.cpp
		connection.cpp
		AutoTypeDialog.cpp
		CSDDialog.cpp
		DefineKeyDialog.cpp
		DebuggerDialog.cpp
		DriveSettingsDialog.cpp
		DriveStatusDialog.cpp
		EmuFileType.cpp
		EmuWindow.cpp
		OSD.cpp
		ExpansionRomDialog.cpp
		GetKeyDialog.cpp
		GraphicsEditor.cpp
		FileFiler.cpp
		ItemData.cpp
		JoystickDialog.cpp
		KeyJoyDialog.cpp
		KeyStickDialog.cpp
		PokeMemoryDialog.cpp
		PositionalKeyboardDialog.cpp
		LoadBinaryDialog.cpp
		MediaStatusDialog.cpp
		OnBoardRomDialog.cpp
		SaveBinaryDialog.cpp
		SnapshotSettingsDialog.cpp
		SnapshotLoadDialog.cpp
		JoystickButtonDialog.cpp
		LabelManager.cpp
		winape_poke_database.cpp
    FullScreenDialog.cpp
    AudioSettingsDialog.cpp
    DevicesDialog.cpp
		YMDialogs.cpp
		arnmodule.cpp
    SortableListCtrl.cpp
	MonitorSettingsDialog.cpp
		hostg.c
		)

#if using inkz80 then include it's "connection" source file
#otherwise use arnold's z80. inkz80 is more accurate
IF(USE_INKZ80 MATCHES "Yes")
set (gui_FILES ${gui_FILES} InkZ80Connect.cpp)
else(USE_INKZ80 MATCHES "Yes")
set (gui_FILES ${gui_FILES} ArnZ80Connect.cpp)
endif(USE_INKZ80 MATCHES "Yes")

# if windows, include windows specific files (resource and windows sources)
if(WIN32)
  set(gui_FILES ${gui_FILES} windows/resource.rc windows/win.cpp)
endif(WIN32)


link_directories(${arnold_SOURCE_DIR}/cpc)
link_directories(${arnold_SOURCE_DIR}/cpc/debugger)
link_directories(${arnold_SOURCE_DIR}/cpc/z80tools)

# link against arnold's z80 files if not using inkz80
IF(USE_INKZ80 MATCHES "Yes")
else(USE_INKZ80 MATCHES "Yes")
link_directories(${arnold_SOURCE_DIR}/cpc/z80)
endif(USE_INKZ80 MATCHES "Yes")

link_directories(${arnold_SOURCE_DIR}/cpc/diskimage)

#link_libraries( ${EXPAT_LIBRARIES} )
link_libraries( ${wxWidgets_LIBRARIES})
link_libraries( diskimage cpc debugger)

IF(USE_INKZ80 MATCHES "Yes")
link_libraries( inkz80 )
else(USE_INKZ80 MATCHES "Yes")
link_libraries( z80)
endif(USE_INKZ80 MATCHES "Yes")
link_libraries(z80tools)

# these are the resources arnold wants to load
set(arnold_RESOURCES 
	roms.zip 
	disabledbreakpoint.png
	breakpoint.png
	current.png
	arnlogo.png
	GUIFrame.xrc
  icons/32x32/autostart.png
  icons/32x32/cart.png
  icons/32x32/disk.png
  icons/32x32/snapshotload.png
  icons/32x32/snapshotsave.png
  icons/32x32/debugger.png
  icons/32x32/cass.png
  icons/32x32/rom.png
  font/MyFont16.bmp
  windows/arnold.bmp
)

if(APPLE)

# these are the additional required resources
set(mac_RESOURCES macosx/arnold.icns macosx/disk.icns macosx/tape.icns macosx/cartridge.icns macosx/snapshot.icns)

# copy the special plist
CONFIGURE_FILE( macosx/Info.plist.cmake ${arnold_SOURCE_DIR}/arngui/Info.plist)

# add plist to Resources
set(mac_RESOURCES ${mac_RESOURCES} ${arnold_SOURCE_DIR}/arngui/Info.plist)

# indicate where the emu resources (roms etc) go in the bundle
set_source_files_properties(${arnold_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

# indicate where the mac specific resources (icons) go in the bundle
set_source_files_properties(${mac_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

# add the mac resources to the arnold resources
set(arnold_RESOURCES ${arnold_RESOURCES} ${mac_RESOURCES})

endif(APPLE)


add_executable(arnold MACOSX_BUNDLE WIN32 ${gui_FILES} ${arnold_RESOURCES})


# for linux copy files to destination
if(NOT APPLE)

ADD_CUSTOM_COMMAND(TARGET arnold POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# generate GUIFrame.xrc
#add_custom_command(TARGET arnold POST_BUILD COMMAND wxformbuilder -g "${arnold_SOURCE_DIR}/arngui/arngui.fbp" COMMENT "Generating GUIFrame.xrc" VERBATIM)

# copy files to exe path
FOREACH ( resource_file ${arnold_RESOURCES} )
   SET (source "${arnold_SOURCE_DIR}/arngui/${resource_file}")
   GET_FILENAME_COMPONENT(destfile ${arnold_SOURCE_DIR}/arngui/${resource_file} NAME)
   SET (dest "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${destfile}")
   ADD_CUSTOM_COMMAND (
     TARGET arnold
      POST_BUILD
     COMMAND    ${CMAKE_COMMAND} -E copy_if_different ${source} ${dest} 
     COMMENT  " Copying ${source} to ${dest}")
ENDFOREACH (  resource_file ${arnold_RESOURCES} )

endif(NOT APPLE)

if(APPLE)

# copy the appropiate SDL library framework into the bundle
# we don't need to change the directory "links" inside it
# because it's already setup for bundling as it is.
if(USING_SDL MATCHES "Yes")
INSTALL(DIRECTORY /Library/Frameworks/SDL.framework DESTINATION ${CMAKE_INSTALL_PREFIX}/arnold.app/Contents/Frameworks)
endif(USING_SDL MATCHES "Yes")

if(USING_SDL2 MATCHES "Yes")
INSTALL(DIRECTORY /Library/Frameworks/SDL2.framework DESTINATION ${CMAKE_INSTALL_PREFIX}/arnold.app/Contents/Frameworks)
endif(USING_SDL2 MATCHES "Yes")

# indicate the plist needs copying into the bundle
set_target_properties(arnold PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${arnold_SOURCE_DIR}/arngui/Info.plist)

endif(APPLE)

# if linux or windows, change the name of the output binary if building debug version
if(NOT APPLE)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
set_target_properties(arnold PROPERTIES OUTPUT_NAME "arnold_debug")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")
endif(NOT APPLE)

TARGET_LINK_LIBRARIES(arnold cpc)
TARGET_LINK_LIBRARIES(arnold debugger)
TARGET_LINK_LIBRARIES(arnold platformspecific)

# todo: put this into bundle
if (APPLE)
TARGET_LINK_LIBRARIES(arnold expat)
endif(APPLE)

if (USING_SDL MATCHES "Yes")
TARGET_LINK_LIBRARIES(arnold sdlcommon)

endif(USING_SDL MATCHES "Yes")

if (USING_SDL2 MATCHES "Yes")
TARGET_LINK_LIBRARIES(arnold sdlcommon)
endif(USING_SDL2 MATCHES "Yes")


IF(USE_INKZ80 MATCHES "Yes")
TARGET_LINK_LIBRARIES(arnold inkz80)
else(USE_INKZ80 MATCHES "Yes")
TARGET_LINK_LIBRARIES(arnold z80)
endif(USE_INKZ80 MATCHES "Yes")
TARGET_LINK_LIBRARIES(arnold z80tools)

if (APPLE)
set(APPS "${CMAKE_INSTALL_PREFIX}/arnold.app")
set(DIRS "")

# See https://github.com/bvschaik/julius/issues/386#issuecomment-602233717
# if SDL2 library is a framework, we need to indicate to CMake
# the path to its dependencies. SDL2_LIBRARY contains two parts.
# Example: /Library/Frameworks/SDL2.framework;-framework Cocoa
# So strip out everything after the ';'
string(REGEX REPLACE ";.*$" "" SDL2_LIB_DIR "${SDL2_LIBRARY}")
if(EXISTS "${SDL2_LIB_DIR}/Versions/A/Frameworks")
	set(DIRS "${DIRS};${SDL2_LIB_DIR}/Versions/A/Frameworks")
endif()

INSTALL(CODE "
   include(BundleUtilities)
   fixup_bundle(\"${APPS}\" \"\" \"${DIRS}\")
" COMPONENT Runtime)
endif(APPLE)
