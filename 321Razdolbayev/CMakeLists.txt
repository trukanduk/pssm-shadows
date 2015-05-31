SET(APP_NAME 321Razdolbayev)

SET(SRC_FILES
	Main.cpp
	Application.cpp
	Mesh.cpp
	ShaderProgram.cpp
	Texture.cpp
)

SET(HEADER_FILES
	Application.h
	Mesh.h
	ShaderProgram.h
	Texture.h
	BoundingBox.h
)

SET(SHADER_FILES
	# shaders 4
	shaders/marker.vert
	shaders/marker.frag

	# shaders 7
	shaders/quadDepth.vert
	shaders/quadDepth.frag

	# shaders 8
	shaders/toshadow.vert
	shaders/toshadow.frag

	shaders/shadow.vert
	shaders/shadow.frag
	shaders/shadow2.frag
	shaders/shadowd.frag
)

SOURCE_GROUP("Shaders" FILES
	${SHADER_FILES}
)

MAKE_TASK(${APP_NAME})

INSTALL(DIRECTORY ${APP_NAME}/shaders DESTINATION ${CMAKE_INSTALL_PREFIX})