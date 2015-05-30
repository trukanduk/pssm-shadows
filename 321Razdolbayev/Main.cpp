#include "Application.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "BoundingBox.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>


namespace std {

std::ostream& operator <<(std::ostream& out, const glm::vec3& v)
{
    return out << "vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
}

}

struct LightInfo
{
	glm::vec3 position; //Будем здесь хранить координаты в мировой системе координат, а при копировании в юниформ-переменную конвертировать в систему виртуальной камеры
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

/**
Пример с тенями
*/
class SampleApplication : public Application
{
public:
    enum { SPLIT_NUMBER = 2 };

	Mesh cube;
	Mesh sphere;
	Mesh bunny;
	Mesh ground;
	Mesh backgroundCube;

	Mesh quad;

	Mesh marker; //Меш - маркер для источника света

	//Идентификатор шейдерной программы
	ShaderProgram _commonShader;
	ShaderProgram _markerShader;
	ShaderProgram _skyboxShader;
	ShaderProgram _quadShader;
    ShaderProgram _renderToShadowMapShader;
    ShaderProgram _commonWithShadowsShader;
    ShaderProgram _commonWithShadowsShaderVar2;
    ShaderProgram _debugShader;
    ShaderProgram _boundsShader;

	//Переменные для управления положением одного источника света
	float _lr;
	float _phi;
	float _theta;

	LightInfo _light;
    CameraInfo _lightCamera[SPLIT_NUMBER];
    Mesh _lightBound[SPLIT_NUMBER]; // Меши для отображения формы и расположения камер

	GLuint _worldTexId;
	GLuint _brickTexId;
	GLuint _grassTexId;
	GLuint _chessTexId;
	GLuint _myTexId;
	GLuint _cubeTexId;

	GLuint _sampler;
	GLuint _cubeTexSampler;
    GLuint _depthSampler;
    GLuint _depthSamplerLinear;

    GLuint _framebufferId;
    GLuint _depthTexId[SPLIT_NUMBER];
    unsigned int _fbWidth;
    unsigned int _fbHeight;

    int _showDepthQuad;
    bool _isLinearSampler;
    bool _cullFrontFaces;
    bool _randomPoints;
    bool _useDebugShader;
    bool _showLightBounds[SPLIT_NUMBER];

    void initFramebuffer()
    {
        _fbWidth = 1024;
        _fbHeight = 1024;


        //Создаем фреймбуфер
        glGenFramebuffers(1, &_framebufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId);


        ////Создаем текстуру, куда будем впоследствии копировать буфер глубины
        glGenTextures(SPLIT_NUMBER, &_depthTexId[0]);
        for (int i = 0; i < SPLIT_NUMBER; ++i)
        {
            glBindTexture(GL_TEXTURE_2D, _depthTexId[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, _fbWidth, _fbHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            // glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthTexId[index], 0);

        }


        //Указываем куда именно мы будем рендерить
        GLenum buffers[] = { GL_NONE };
        glDrawBuffers(1, buffers);

        // if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        // {
        //     std::cerr << "Failed to setup framebuffer\n";
        //     exit(1);
        // }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

	virtual void makeScene()
	{
		Application::makeScene();

        _showDepthQuad = -1;
        _isLinearSampler = false;
        _cullFrontFaces = false;
        _randomPoints = false;
        _useDebugShader = false;
        _showLightBounds[0] = false;
        _showLightBounds[1] = false;
        _showLightBounds[2] = false;

		//=========================================================
		//Создание и загрузка мешей

		cube.makeCube(0.5);
		cube.modelMatrix() = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.5f));

		sphere.makeSphere(0.5);
		sphere.modelMatrix() = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.5f));

		bunny.loadFromFile("models/bunny.obj");
		bunny.modelMatrix() = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		ground.makeGroundPlane(5.0f, 2.0f);

		marker.makeSphere(0.1);

		backgroundCube.makeCube(10.0f);

		quad.makeScreenAlignedQuad();

		//=========================================================
		//Инициализация шейдеров

		_commonShader.createProgram("shaders/common.vert", "shaders/common.frag");
		_markerShader.createProgram("shaders/marker.vert", "shaders/marker.frag");
		_skyboxShader.createProgram("shaders/skybox.vert", "shaders/skybox.frag");
		_quadShader.createProgram("shaders/quadDepth.vert", "shaders/quadDepth.frag");
        _renderToShadowMapShader.createProgram("shaders/toshadow.vert", "shaders/toshadow.frag");
        _commonWithShadowsShader.createProgram("shaders/shadow.vert", "shaders/shadow.frag");
        _debugShader.createProgram("shaders/shadow.vert", "shaders/shadowd.frag");
        _boundsShader.createProgram("shaders/marker.vert", "shaders/marker.frag");

        // :TODO: fix shader2.frag shader
        _commonWithShadowsShaderVar2.createProgram("shaders/shadow.vert", "shaders/shadow.frag");
        // _commonWithShadowsShaderVar2.createProgram("shaders/shadow.vert", "shaders/shadow2.frag");

		//=========================================================
		//Инициализация значений переменных освщения
		_lr = 10.0;
		_phi = 0.0f;
		_theta = 0.48f;

		_light.position = glm::vec3(glm::cos(_phi) * glm::cos(_theta), glm::sin(_phi) * glm::cos(_theta), glm::sin(_theta)) * (float)_lr;
		_light.ambient = glm::vec3(0.2, 0.2, 0.2);
		_light.diffuse = glm::vec3(0.8, 0.8, 0.8);
		_light.specular = glm::vec3(1.0, 1.0, 1.0);

		//=========================================================
		//Загрузка и создание текстур
		_worldTexId = Texture::loadTexture("images/earth_global.jpg");
		_brickTexId = Texture::loadTexture("images/brick.jpg");
		_grassTexId = Texture::loadTexture("images/grass.jpg");
		_chessTexId = Texture::loadTextureWithMipmaps("images/chess.dds");
		_myTexId = Texture::makeProceduralTexture();
		_cubeTexId = Texture::loadCubeTexture("images/cube");

		//=========================================================
		//Инициализация сэмплера, объекта, который хранит параметры чтения из текстуры
		glGenSamplers(1, &_sampler);
		glSamplerParameteri(_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glSamplerParameteri(_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glSamplerParameteri(_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glGenSamplers(1, &_cubeTexSampler);
		glSamplerParameteri(_cubeTexSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(_cubeTexSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glSamplerParameteri(_cubeTexSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(_cubeTexSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(_cubeTexSampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        GLfloat border[] = { 1.0f, 0.0f, 0.0f, 1.0f };

        glGenSamplers(1, &_depthSampler);
        glSamplerParameteri(_depthSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(_depthSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(_depthSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glSamplerParameteri(_depthSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glSamplerParameterfv(_depthSampler, GL_TEXTURE_BORDER_COLOR, border);
        glSamplerParameteri(_depthSampler, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glSamplerParameteri(_depthSampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

        glGenSamplers(1, &_depthSamplerLinear);
        glSamplerParameteri(_depthSamplerLinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(_depthSamplerLinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(_depthSamplerLinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glSamplerParameteri(_depthSamplerLinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glSamplerParameterfv(_depthSamplerLinear, GL_TEXTURE_BORDER_COLOR, border);
        glSamplerParameteri(_depthSamplerLinear, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glSamplerParameteri(_depthSamplerLinear, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);


        //=========================================================
        //Инициализация фреймбуфера для рендера теневой карты

        initFramebuffer();
	}

	virtual void initGUI()
	{
		Application::initGUI();

		TwAddVarRW(_bar, "r", TW_TYPE_FLOAT, &_lr, "group=Light step=0.01 min=0.1 max=100.0");
		TwAddVarRW(_bar, "phi", TW_TYPE_FLOAT, &_phi, "group=Light step=0.01 min=0.0 max=6.28");
		TwAddVarRW(_bar, "theta", TW_TYPE_FLOAT, &_theta, "group=Light step=0.01 min=-1.57 max=1.57");
		TwAddVarRW(_bar, "La", TW_TYPE_COLOR3F, &_light.ambient, "group=Light label='ambient'");
		TwAddVarRW(_bar, "Ld", TW_TYPE_COLOR3F, &_light.diffuse, "group=Light label='diffuse'");
		TwAddVarRW(_bar, "Ls", TW_TYPE_COLOR3F, &_light.specular, "group=Light label='specular'");
	}

    virtual void handleKey(int key, int scancode, int action, int mods)
    {
        Application::handleKey(key, scancode, action, mods);

        if (action == GLFW_PRESS)
        {
            if (key == GLFW_KEY_1)
            {
                _showDepthQuad = (_showDepthQuad == 0 ? -1 : 0);
            }
            if (key == GLFW_KEY_2)
            {
                _showDepthQuad = (_showDepthQuad == 1 ? -1 : 1);
            }
            if (key == GLFW_KEY_3)
            {
                _showDepthQuad = (_showDepthQuad == 2 ? -1 : 2);
            }
            if (key == GLFW_KEY_4)
            {
                _showDepthQuad = (_showDepthQuad == 3 ? -1 : 3);
            }
            if (key == GLFW_KEY_5)
            {
                _showLightBounds[0] = !_showLightBounds[0];
            }
            if (key == GLFW_KEY_6)
            {
                _showLightBounds[1] = !_showLightBounds[1];
            }
            if (key == GLFW_KEY_7)
            {
                _showLightBounds[2] = !_showLightBounds[2];
            }
            if (key == GLFW_KEY_8)
            {
                _showLightBounds[3] = !_showLightBounds[3];
            }
            else if (key == GLFW_KEY_L)
            {
                _isLinearSampler = !_isLinearSampler;
            }
            else if (key == GLFW_KEY_C)
            {
                _cullFrontFaces = !_cullFrontFaces;
            }
            else if (key == GLFW_KEY_P)
            {
                _randomPoints = !_randomPoints;
            }
            else if (key == GLFW_KEY_G)
            {
                _useDebugShader = !_useDebugShader;
            }
        }
    }

    void update()
    {
        using namespace glm;

        Application::update();

        _light.position = vec3(cos(_phi) * cos(_theta), sin(_phi) * cos(_theta), sin(_theta)) * (float)_lr;

        int wwidth, wheight;
        glfwGetFramebufferSize(_window, &wwidth, &wheight);

        float aspect = 1.0f * wwidth / wheight;

        vec3 upos = vec3(cos(_phiAng) * cos(_thetaAng), sin(_phiAng) * cos(_thetaAng), sin(_thetaAng)) * (float)_r;
        vec3 uy = vec3(0.0f, 1.0f, 0.0f);
        vec3 uz = normalize(-upos);
        vec3 ux = cross(uz, uy);
        uy = cross(ux, uz);

        // Берём пирамиду обзора пользователя, разбиваем её на SPLIT_NUMBER кусков.
        // Сейчас на равные, но вообще их надо разбивать более интеллектуально
        // Для каждого куска считаем его bounding box, из него -- bounding sphere
        // Далее подгоняем такой конус обзора из источника света, чтобы эта сфера
        // gолностью в него попадала. описываем этот конус пирамидой и
        // подгоняем model-view-projection матрицы для источника света
        // PROFIT!
        for (int i = 0; i < SPLIT_NUMBER; ++i)
        {
            float nearPlane = (FAR_PLANE - NEAR_PLANE) * i / SPLIT_NUMBER + NEAR_PLANE;
            // float nearPlane = NEAR_PLANE;
            float farPlane = (FAR_PLANE - NEAR_PLANE) * (i + 1) / SPLIT_NUMBER + NEAR_PLANE;

            float nearWidth = tan(VIEW_ANGLE / 2.0f) * nearPlane;
            float nearHeight = nearWidth*aspect;
            float farWidth = tan(VIEW_ANGLE / 2.0f) * farPlane;
            float farHeight = farWidth*aspect;

            // Прикинем Bounding box.
            vec3 minVec = upos + uz*nearPlane;
            vec3 maxVec = upos + uz*nearPlane;

            BoundigBox bbox;
            bbox.expand(upos + uz*nearPlane + ux*nearWidth + uy*nearHeight);
            bbox.expand(upos + uz*nearPlane + ux*nearWidth - uy*nearHeight);
            bbox.expand(upos + uz*nearPlane - ux*nearWidth + uy*nearHeight);
            bbox.expand(upos + uz*nearPlane - ux*nearWidth - uy*nearHeight);

            bbox.expand(upos + uz*farPlane + ux*farWidth + uy*farHeight);
            bbox.expand(upos + uz*farPlane + ux*farWidth - uy*farHeight);
            bbox.expand(upos + uz*farPlane - ux*farWidth + uy*farHeight);
            bbox.expand(upos + uz*farPlane - ux*farWidth - uy*farHeight);

            vec3 boundCenter = bbox.getCenter();
            float boundRadius = bbox.getRadius();

            float viewAngle = std::atan2(boundRadius, distance(_light.position, upos))*2.0f;
            _lightCamera[i].viewMatrix = lookAt(_light.position, boundCenter, vec3(0.0f, 0.0f, 1.0f));

            float light2center = distance(boundCenter, _light.position);
            _lightCamera[i].projMatrix = perspective(viewAngle, 1.0f,
                    light2center - boundRadius, light2center + boundRadius);

            _lightBound[i].makeViewVolume(_light.position,
                                          boundCenter,
                                          glm::vec3(0.0f, 1.0f, 0.0f),
                                          viewAngle,
                                          1.0,
                                          light2center - boundRadius,
                                          light2center + boundRadius);
            // _lightBound[i].makeSphere(boundRadius, boundCenter);
        }
    }

    virtual void draw()
    {
        for (int i = 0; i < SPLIT_NUMBER; ++i)
        {
            drawToShadowMap(_lightCamera[i], i);
        }
        const ShaderProgram& shader =
                (_useDebugShader ? _debugShader
                                 : _randomPoints ? _commonWithShadowsShaderVar2
                                                 : _commonWithShadowsShader);
        drawToScreen(shader, _camera, _lightCamera);

        if (_showDepthQuad >= 0)
        {
            drawDebug(_showDepthQuad);
        }
    }

    void drawToShadowMap(const CameraInfo& lightCamera, int index)
    {
        //=========== Сначала подключаем фреймбуфер и рендерим в текстуру ==========
        glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthTexId[index], 0);

        glViewport(0, 0, _fbWidth, _fbHeight);
        glClear(GL_DEPTH_BUFFER_BIT);

        _renderToShadowMapShader.use();
        _renderToShadowMapShader.setMat4Uniform("viewMatrix", lightCamera.viewMatrix);
        _renderToShadowMapShader.setMat4Uniform("projectionMatrix", lightCamera.projMatrix);

        if (_cullFrontFaces)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
        }

        drawScene(_renderToShadowMapShader, lightCamera);

        if (_cullFrontFaces)
        {
            glDisable(GL_CULL_FACE);
        }

        glUseProgram(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); //Отключаем фреймбуфер
    }

    void drawToScreen(const ShaderProgram& shader, const CameraInfo& camera, const CameraInfo lightCamera[SPLIT_NUMBER])
	{
		//Получаем текущие размеры экрана и выставлям вьюпорт
		int width, height;
		glfwGetFramebufferSize(_window, &width, &height);

		glViewport(0, 0, width, height);

		//Очищаем буферы цвета и глубины от результатов рендеринга предыдущего кадра
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setMat4Uniform("viewMatrix", camera.viewMatrix);
        shader.setMat4Uniform("projectionMatrix", camera.projMatrix);

		glm::vec3 lightPosCamSpace = glm::vec3(camera.viewMatrix * glm::vec4(_light.position, 1.0));

        shader.setVec3Uniform("light.pos", lightPosCamSpace); //копируем положение уже в системе виртуальной камеры
        shader.setVec3Uniform("light.La", _light.ambient);
        shader.setVec3Uniform("light.Ld", _light.diffuse);
        shader.setVec3Uniform("light.Ls", _light.specular);

        for (int i = 0; i < SPLIT_NUMBER; ++i)
        {
            shader.setMat4Uniform(arrItemName("lightViewMatrix", i).c_str(), lightCamera[i].viewMatrix);
            shader.setMat4Uniform(arrItemName("lightProjectionMatrix", i).c_str(), lightCamera[i].projMatrix);

            glm::mat4 projScaleBiasMatrix = glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.5, 0.5, 0.5)), glm::vec3(0.5, 0.5, 0.5));
            shader.setMat4Uniform(arrItemName("lightScaleBiasMatrix", i).c_str(), projScaleBiasMatrix);
        }

        glActiveTexture(GL_TEXTURE0);  //текстурный юнит 0
        glBindTexture(GL_TEXTURE_2D, _brickTexId);
        glBindSampler(0, _sampler);
        shader.setIntUniform("diffuseTex", 0);

        for (int i = 0; i < SPLIT_NUMBER; ++i)
        {
            static GLenum TEX_UNITS[] = { GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE2, GL_TEXTURE3 };
            glActiveTexture(TEX_UNITS[i]);  //текстурный юнит i
            glBindTexture(GL_TEXTURE_2D, _depthTexId[i]);
            glBindSampler(i + 1, _isLinearSampler ? _depthSamplerLinear : _depthSampler);
            shader.setIntUniform(arrItemName("shadowTex", i).c_str(), i + 1);
        }

        drawScene(shader, camera);

		//Рисуем маркеры для всех источников света
		{
			_markerShader.use();

            _markerShader.setMat4Uniform("mvpMatrix", camera.projMatrix * camera.viewMatrix * glm::translate(glm::mat4(1.0f), _light.position));
            _markerShader.setVec4Uniform("color", glm::vec4(_light.diffuse, 1.0f));
            marker.draw();
        }

        // Рисуем bound'ы
        for (int i = 0; i < SPLIT_NUMBER; ++i)
        {
            static const  glm::vec4 colors[] = { glm::vec4(1.0f, 0.0f, 0.0f, 0.3f),
                                                 glm::vec4(0.0f, 1.0f, 0.0f, 0.3f),
                                                 glm::vec4(0.0f, 0.0f, 1.0f, 0.3f),
                                                 glm::vec4(1.0f, 1.0f, 1.0f, 0.3f) };
            if (_showLightBounds[i])
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                _boundsShader.use();
    			_boundsShader.setMat4Uniform("mvpMatrix", camera.projMatrix * camera.viewMatrix);
                _boundsShader.setVec4Uniform("color", colors[i]);
                _lightBound[i].draw();
            }
        }
		//Отсоединяем сэмплер и шейдерную программу
		glBindSampler(0, 0);
		glUseProgram(0);
	}

    void drawScene(const ShaderProgram& shader, const CameraInfo& camera)
    {
        glFrontFace(GL_CW);

        shader.setMat4Uniform("modelMatrix", cube.modelMatrix());
        shader.setMat3Uniform("normalToCameraMatrix", glm::transpose(glm::inverse(glm::mat3(camera.viewMatrix * cube.modelMatrix()))));

        cube.draw();

        shader.setMat4Uniform("modelMatrix", sphere.modelMatrix());
        shader.setMat3Uniform("normalToCameraMatrix", glm::transpose(glm::inverse(glm::mat3(camera.viewMatrix * sphere.modelMatrix()))));

        sphere.draw();

        shader.setMat4Uniform("modelMatrix", ground.modelMatrix());
        shader.setMat3Uniform("normalToCameraMatrix", glm::transpose(glm::inverse(glm::mat3(camera.viewMatrix * ground.modelMatrix()))));

        ground.draw();

        glFrontFace(GL_CCW);

        shader.setMat4Uniform("modelMatrix", bunny.modelMatrix());
        shader.setMat3Uniform("normalToCameraMatrix", glm::transpose(glm::inverse(glm::mat3(camera.viewMatrix * bunny.modelMatrix()))));

        bunny.draw();
    }

    void drawDebug(int bufferInd)
    {
        glViewport(0, 0, 500, 500);

        _quadShader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _depthTexId[bufferInd]);
        glBindSampler(0, _sampler);
        _quadShader.setIntUniform("tex", 0);

        quad.draw();

        glBindSampler(0, 0);
        glUseProgram(0);
    }

private:
    static std::string arrItemName(const std::string& name, int ind)
    {
        return name + "[" + std::to_string(ind) + "]";
    }
};

int main()
{
	SampleApplication app;
	app.start();

	return 0;
}