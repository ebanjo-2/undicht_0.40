add_library("tools" 

		src/images/image_file.h
		src/images/image_file.cpp
		
		extern/stb_implementation.cpp
)

target_include_directories("tools" PUBLIC src extern/glm)


add_subdirectory(extern/stb)
target_link_libraries("tools" PUBLIC core graphics stb_image)
