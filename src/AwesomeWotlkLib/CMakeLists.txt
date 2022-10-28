project( AwesomeWotlkLib )

add_library(
	${PROJECT_NAME} SHARED
		"Entry.cpp"
		
		GameClient.h
   "NamePlates.h" "NamePlates.cpp" "Hooks.h" "Hooks.cpp" "Misc.h" "Misc.cpp" "BugFixes.h" "BugFixes.cpp"  "Utils.h" "Utils.cpp" "CommandLine.cpp" "CommandLine.h")

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