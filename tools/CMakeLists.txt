add_library("tools" 

	src/images/image_file.h
	src/images/image_file.cpp
		
	src/fonts/true_type.h
	src/fonts/true_type.cpp
		
	src/math/math_tools.h
	src/math/orientation_3d.h
	src/math/orientation_3d.cpp
	src/math/orthographic_projection.h
	src/math/orthographic_projection.cpp
	src/math/perspective_projection.h
	src/math/perspective_projection.cpp
	
	src/3D/camera/camera_3d.h
	src/3D/camera/camera_3d.cpp
	src/3D/camera/perspective_camera_3d.h
	src/3D/camera/perspective_camera_3d.cpp

	extern/stb_implementation.cpp
)

target_include_directories("tools" PUBLIC src extern/glm)


add_subdirectory(extern/stb)

target_link_libraries("tools" PUBLIC core graphics stb_image)
