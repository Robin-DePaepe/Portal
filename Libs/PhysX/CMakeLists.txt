add_library(PhysX INTERFACE)
target_include_directories (OverlordEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories (PortalGame PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/bin/Release/d3dcompiler_46.dll DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/bin/Release/nvToolsExt32_1.dll DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/bin/Release/PhysX3CharacterKinematic_x86.dll DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/bin/Release/PhysX3Common_x86.dll DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/bin/Release/PhysX3_x86.dll DESTINATION bin)

