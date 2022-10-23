project( AwesomeWotlkLib )

add_library(
	${PROJECT_NAME} SHARED
		"Entry.cpp"
		
		GameClient.h
 "LuaAPI.h" "LuaAPI.cpp" "NamePlates.h" "NamePlates.cpp")

target_include_directories(
	${PROJECT_NAME} PRIVATE
		${CMAKE_SOURCE_DIR}/deps
)

target_link_libraries(
	${PROJECT_NAME} PRIVATE
		Detours
)

target_compile_definitions(
	${PROJECT_NAME} PRIVATE
		APP_NAME="${PROJECT_NAME}"
)