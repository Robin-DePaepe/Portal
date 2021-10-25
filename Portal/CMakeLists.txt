
add_executable(PortalGame 
	Main.cpp 
	MainGame.cpp
	ControllerScheme.cpp
	MainMenu.cpp
	Menu.cpp
	MenuButton.cpp
	PauseMenu.cpp
	PortalCharacterController.cpp
	PortalMaterial.cpp
	PortalPrefab.cpp
	PortalPuzzle1.cpp
	stdafx.cpp
	WallPrefab.cpp
		
)
add_definitions(-DUNICODE)
message(STATUS "build dir: ${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(PortalGame PUBLIC OverlordEngine)
target_link_directories(PortalGame
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Libs/PhysX/Lib
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Libs/DX_Effects11/Lib
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Libs/FMod/Lib
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Libs/DX_Tex/Lib
	)
	
install(TARGETS PortalGame DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Resources DESTINATION bin)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")

set(CPACK_NSIS_MODIFY_PATH ON)
include(CPack)
