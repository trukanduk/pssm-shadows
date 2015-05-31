#pragma once

#include <GL/glew.h>

#include <string>
#include <memory>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/**
Описание меша:
- тип примитива (обычно GL_TRIANGLES)
- идентификатор Vertex Array Object (хранит настройки буферов и вершинных атрибутов)
- количество вершин в модели
- матрица модели (LocalToWorld)
*/
class Mesh
{
public:
	/**
	Вспомогательный класс для добавления вершинных атрибутов в буфер
	*/
	template <typename T>
	class Buffer : public std::vector <T>
	{
	public:
		void addVec2(T s, T t)
		{
			this->push_back(s);
			this->push_back(t);
		}

		void addVec2(const glm::vec2& v)
		{
			this->push_back(v.x);
			this->push_back(v.y);
		}

		void addVec3(T x, T y, T z)
		{
			this->push_back(x);
			this->push_back(y);
			this->push_back(z);
		}

		void addVec3(const glm::vec3& v)
		{
			this->push_back(v.x);
			this->push_back(v.y);
			this->push_back(v.z);
		}

		void addVec4(T r, T g, T b, T a)
		{
			this->push_back(r);
			this->push_back(g);
			this->push_back(b);
			this->push_back(a);
		}

		void addVec3(const glm::vec4& v)
		{
			this->push_back(v.x);
			this->push_back(v.y);
			this->push_back(v.z);
			this->push_back(v.w);
		}
	};

	Mesh();

	Mesh(GLuint primType,
		 const std::vector<float>& vertices,
		 const std::vector<float>& normals,
		 const std::vector<float>& texcoords);

	GLuint& primitiveType() { return _primitiveType; }

	/**
	Возвращает матрицу модели, которая описывает положение меша в мировой системе координат
	*/
	glm::mat4& modelMatrix() { return _modelMatrix; }

	/**
	Запускает отрисовку меша
	*/
	void draw();

	//----------------- Методы для инициализации меша

	/**
	Инициализирует меш сферы
	*/
	void makeSphere(float radius, const glm::vec3& center = glm::vec3(0.0f, 0.0f, 0.0f),
					int N = 100);

	/**
	Инициализирует меш куба
	*/
	void makeCube(float size);

	/**
	Нарисовать куб без front и back граней.
	*/
	void makeViewVolume();

	/**
	Создает квадрат из двух треугольников. Координаты в Clip Space
	*/
	void makeScreenAlignedQuad();

	/**
	Создает плоскость земли размером от -size до +size по осям XY
	Генерирует текстурные координаты, так чтобы на плоскости размещалось 2 * numTiles по каждой оси
	*/
	void makeGroundPlane(float size, float numTiles);

	/**
	Загружает меш из внешнего файла с помощью библиотеки Assimp
	*/
	void loadFromFile(const std::string& filename);

	void init(GLuint aPrimitiveType,
			  const std::vector<float>& vertices,
			  const std::vector<float>& normals,
			  const std::vector<float>& texcoords);

	void init(GLuint aPrimitiveType,
			  const std::vector<float>& vertices);

protected:
	GLuint _primitiveType;
	GLuint _vao;
	bool _inited;
	unsigned int _numVertices;
	glm::mat4 _modelMatrix;
};