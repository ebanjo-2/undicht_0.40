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
	
	src/xml/xml_tag_attribute.h
	src/xml/xml_tag_attribute.cpp
	src/xml/xml_file.h
	src/xml/xml_file.cpp
	src/xml/xml_element.h
	src/xml/xml_element.cpp
	
	src/model_loading/model_loader.h
	src/model_loading/model_loader.cpp
	src/model_loading/collada/collada_file.h
	src/model_loading/collada/collada_file.cpp
	
	src/file_tools.h
	src/file_tools.cpp
	
	extern/stb_implementation.cpp
)

target_include_directories("tools" PUBLIC src extern/glm)


add_subdirectory(extern/stb)

target_link_libraries("tools" PUBLIC core graphics stb_image)
