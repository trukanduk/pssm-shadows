#version 330

const int SPLIT_NUMBER = 2;

//стандартные матрицы для преобразования координат
uniform mat4 modelMatrix; //из локальной в мировую
uniform mat4 viewMatrix; //из мировой в систему координат камеры
uniform mat4 projectionMatrix; //из системы координат камеры в усеченные координаты

//матрица для преобразования нормалей из локальной системы координат в систему координат камеры
uniform mat3 normalToCameraMatrix;

//стандартные матрицы для проектора
uniform mat4 lightViewMatrix[SPLIT_NUMBER]; //из мировой в систему координат камеры
uniform mat4 lightProjectionMatrix[SPLIT_NUMBER]; //из системы координат камеры в усеченные координаты
uniform mat4 lightScaleBiasMatrix[SPLIT_NUMBER];

layout(location = 0) in vec3 vertexPosition; //координаты вершины в локальной системе координат
layout(location = 1) in vec3 vertexNormal; //нормаль в локальной системе координат
layout(location = 2) in vec2 vertexTexCoord; //текстурные координаты вершины

struct ShadowCoords
{
	vec4 coord[SPLIT_NUMBER];
};

out vec3 normalCamSpace; //нормаль в системе координат камеры
out vec4 posCamSpace; //координаты вершины в системе координат камеры
out vec2 texCoord; //текстурные координаты
out ShadowCoords shadowTexCoords; //выходные текстурные координаты для проективное текстуры

void main()
{
	texCoord = vertexTexCoord;

	//вычисляем текстурные координаты для теневой карты
	for (int i = 0; i < SPLIT_NUMBER; ++i)
	{
		shadowTexCoords.coord[i] = lightScaleBiasMatrix[i] * lightProjectionMatrix[i] * lightViewMatrix[i] * modelMatrix * vec4(vertexPosition, 1.0);
	}

	posCamSpace = viewMatrix * modelMatrix * vec4(vertexPosition, 1.0); //преобразование координат вершины в систему координат камеры
	normalCamSpace = normalize(normalToCameraMatrix * vertexNormal); //преобразование нормали в систему координат камеры

	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
}
