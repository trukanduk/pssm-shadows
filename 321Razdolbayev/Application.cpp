#include "Application.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>

//======================================

const float Application::NEAR_PLANE = 0.1f;
const float Application::FAR_PLANE = 30.0f;
const float Application::VIEW_ANGLE = 45.0f;

const float Application::NEAR_PLANED = 0.1f;
const float Application::FAR_PLANED = 100.0f;
const float Application::VIEW_ANGLED = 45.0f;

//Функция обратного вызова для обработки нажатий на клавиатуре
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Application* app = (Application*)glfwGetWindowUserPointer(window);

	app->handleKey(key, scancode, action, mods);
}

void windowSizeChangedCallback(GLFWwindow* window, int width, int height)
{
	TwWindowSize(width, height);
}

void mouseButtonPressedCallback(GLFWwindow* window, int button, int action, int mods)
{
	TwEventMouseButtonGLFW(button, action);
}

void mouseCursosPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	TwEventMousePosGLFW(xpos, ypos);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	TwEventMouseWheelGLFW(xoffset);
}

//======================================

Application::Application():
_oldTime(0.0),
	_rotateLeft(false),
	_rotateRight(false),
	_phiAng(0.0),
	_rotateUp(false),
	_rotateDown(false),
	_thetaAng(0.0),
	_radiusInc(false),
	_radiusDec(false),
	_r(5.0),
	_useDebugCamera(false)
{
}

Application::~Application()
{
	glfwTerminate();
}

void Application::start()
{
	initContext();
	initGL();
	initGUI();
	makeScene();
	run();
}

void Application::initContext()
{
	if (!glfwInit())
	{
		std::cerr << "ERROR: could not start GLFW3\n";
		exit(1);
	}

#ifdef USE_CORE_PROFILE
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

	_window = glfwCreateWindow(640, 480, "MIPT OpenGL demos", NULL, NULL);
	if (!_window)
	{
		std::cerr << "ERROR: could not open window with GLFW3\n";
		glfwTerminate();
		exit(1);
	}
	glfwMakeContextCurrent(_window);

	glfwSetWindowUserPointer(_window, this); //Регистрируем указатель на данный объект, чтобы потом использовать его в функциях обратного вызова}

	glfwSetKeyCallback(_window, keyCallback); //Регистрирует функцию обратного вызова для обработки событий клавиатуры
	glfwSetWindowSizeCallback(_window, windowSizeChangedCallback);
	glfwSetMouseButtonCallback(_window, mouseButtonPressedCallback);
	glfwSetCursorPosCallback(_window, mouseCursosPosCallback);
	glfwSetScrollCallback(_window, scrollCallback);
}

void Application::initGL()
{
	glewExperimental = GL_TRUE;
	glewInit();

	const GLubyte* renderer = glGetString(GL_RENDERER); //Получаем имя рендерера
	const GLubyte* version = glGetString(GL_VERSION); //Получаем номер версии
	std::cout << "Renderer: " << renderer << std::endl;
	std::cout << "OpenGL version supported: " << version << std::endl;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void Application::initGUI()
{
	int width, height;
	glfwGetFramebufferSize(_window, &width, &height);

#ifdef USE_CORE_PROFILE
	TwInit(TW_OPENGL_CORE, NULL);
#else
	TwInit(TW_OPENGL, NULL);
#endif

	TwWindowSize(width, height);

	_bar = TwNewBar("TweakBar");
	TwDefine("GLOBAL help='This example shows how to integrate AntTweakBar with GLFW and OpenGL.'");
}

void Application::makeScene()
{
	_camera.viewMatrix = glm::lookAt(glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	_camera.projMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.f);
}

void Application::run()
{
	while (!glfwWindowShouldClose(_window)) //Пока окно не закрыто
	{
		glfwPollEvents(); //Проверяем события ввода

		update(); //Обновляем сцену и положение виртуальной камеры

		draw(); //Рисуем один кадр

		TwDraw(); //Рисуем графический интерфейс пользователя

		glfwSwapBuffers(_window); //Переключаем передний и задний буферы
	}
}

void Application::handleKey(int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE)
		{
			glfwSetWindowShouldClose(_window, GL_TRUE);
		}
		else if (key == GLFW_KEY_A)
		{
			_rotateLeft = true;
		}
		else if (key == GLFW_KEY_D)
		{
			_rotateRight = true;
		}
		else if (key == GLFW_KEY_W)
		{
			_rotateUp = true;
		}
		else if (key == GLFW_KEY_S)
		{
			_rotateDown = true;
		}
		else if (key == GLFW_KEY_R)
		{
			_radiusInc = true;
		}
		else if (key == GLFW_KEY_F)
		{
			_radiusDec = true;
		}
		else if (key == GLFW_KEY_P)
		{
			_useDebugCamera = !_useDebugCamera;
			_dr = _r;
			_dphiAng = _phiAng;
			_dthetaAng = _thetaAng;
		}
	}
	else if (action == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_A)
		{
			_rotateLeft = false;
		}
		else if (key == GLFW_KEY_D)
		{
			_rotateRight = false;
		}
		else if (key == GLFW_KEY_W)
		{
			_rotateUp = false;
		}
		else if (key == GLFW_KEY_S)
		{
			_rotateDown = false;
		}
		else if (key == GLFW_KEY_R)
		{
			_radiusInc = false;
		}
		else if (key == GLFW_KEY_F)
		{
			_radiusDec = false;
		}
	}
}

void Application::update()
{
	double dt = glfwGetTime() - _oldTime;
	_oldTime = glfwGetTime();

	double speed = 1.0;

	double& phiAng = _useDebugCamera ? _dphiAng : _phiAng;
	double& thetaAng = _useDebugCamera ? _dthetaAng : _thetaAng;
	double& r = _useDebugCamera ? _dr : _r;

	if (_rotateLeft)
	{
		phiAng -= speed * dt;
	}
	if (_rotateRight)
	{
		phiAng += speed * dt;
	}
	if (_rotateUp)
	{
		thetaAng += speed * dt;
	}
	if (_rotateDown)
	{
		thetaAng -= speed * dt;
	}
	if (_radiusInc)
	{
		r += r * dt;
	}
	if (_radiusDec)
	{
		r -= r * dt;
	}

	_thetaAng = glm::clamp(_thetaAng, -glm::pi<double>() * 0.49, glm::pi<double>() * 0.49);

	//Вычисляем положение виртуальной камеры в мировой системе координат по формуле сферических координат
	glm::vec3 pos = glm::vec3(glm::cos(phiAng) * glm::cos(thetaAng), glm::sin(phiAng) * glm::cos(thetaAng), glm::sin(thetaAng) + 0.5f) * (float)r;

	//Обновляем матрицу вида
	_camera.viewMatrix = glm::lookAt(pos, glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f));


	//-----------------------------------------

	int width, height;
	glfwGetFramebufferSize(_window, &width, &height);

	//Обновляем матрицу проекции на случай, если размеры окна изменились
	if (_useDebugCamera)
		_camera.projMatrix = glm::perspective(glm::radians(VIEW_ANGLED),
											 (float)width / height, NEAR_PLANED, FAR_PLANED);
	else
	{
		_camera.projMatrix = glm::perspective(glm::radians(VIEW_ANGLE),
											 (float)width / height, NEAR_PLANE, FAR_PLANE);
		_mainCamera = _camera;
	}

}
