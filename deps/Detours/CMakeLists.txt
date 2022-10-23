project(Detours)

add_library(
	${PROJECT_NAME} STATIC
		creatwth.cpp
		detours.cpp
		detours.h
		detver.h
		disasm.cpp
		disolarm.cpp
		disolarm64.cpp
		disolia64.cpp
		disolx64.cpp
		disolx86.cpp
		image.cpp
		modules.cpp
		uimports.cpp
)
set_property(SOURCE uimports.cpp APPEND PROPERTY HEADER_FILE_ONLY true)

target_compile_options(
	${PROJECT_NAME} PRIVATE
		/we4777
		/we4800
		/D_X86_
)