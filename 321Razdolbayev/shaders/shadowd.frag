#version 330

const int SPLIT_NUMBER = 3;

uniform sampler2D diffuseTex;
uniform sampler2DShadow shadowTex[SPLIT_NUMBER]; //Вариант 2

struct LightInfo
{
	vec3 pos; //положение источника света в системе координат ВИРТУАЛЬНОЙ КАМЕРЫ!
	vec3 La; //цвет и интенсивность окружающего света
	vec3 Ld; //цвет и интенсивность диффузного света
	vec3 Ls; //цвет и интенсивность бликового света
};
uniform LightInfo light;

in vec3 normalCamSpace; //нормаль в системе координат камеры (интерполирована между вершинами треугольника)
in vec4 posCamSpace; //координаты вершины в системе координат камеры (интерполированы между вершинами треугольника)

struct ShadowCoords
{
	vec4 coord[SPLIT_NUMBER];
};

in vec2 texCoord; //текстурные координаты (интерполирована между вершинами треугольника)
in ShadowCoords shadowTexCoords; //выходные текстурные координаты для проективное текстуры

out vec4 fragColor; //выходной цвет фрагмента

const vec3 Ks = vec3(1.0, 1.0, 1.0); //Коэффициент бликового отражения
const float shininess = 128.0;

void main()
{

	float visibility = 1.0;
	for (int i = 0; i < SPLIT_NUMBER; ++i) {
		vec4 shadowCoords = shadowTexCoords.coord[i];
		shadowCoords.xyzw /= shadowCoords.w;

		if (shadowCoords.x >= 0.0 && shadowCoords.x <= 1.0 &&
			shadowCoords.y >= 0.0 && shadowCoords.y <= 1.0)
		{
			visibility = textureProj(shadowTex[i], shadowCoords); //глубина ближайшего фрагмента в пространстве источника света

			if (i == 0)
				fragColor.r = 1.0;
			else if (i == 1)
				fragColor.g = 1.0;
			else
				fragColor.b = 1.0;
		} else {
			if (i == 0)
				fragColor.r = 0.0;
			else if (i == 1)
				fragColor.g = 0.0;
			else
				fragColor.b = 0.0;
		}
	}
	fragColor.a = 1.0;
	// visibility = round(visibility / SPLIT_NUMBER);

}
