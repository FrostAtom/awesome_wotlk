project( AwesomeWotlkLib )

add_library(
	${PROJECT_NAME} SHARED
		"Entry.cpp"
		
		GameClient.h
   "NamePlates.h" "NamePlates.cpp" "Hooks.h" "Hooks.cpp" "Misc.h" "Misc.cpp")

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