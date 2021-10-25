add_library(FMod INTERFACE)
target_include_directories (OverlordEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_include_directories (PortalGame PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/bin/FMod.dll DESTINATION bin)