#ifndef GAME_H
#define GAME_H

#include <string>
#include <sstream>

#include <iostream>
#include <glew-1.13.0\include\GL\glew.h>
#include <glew-1.13.0\include\GL\wglew.h>


#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <Debug.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "time.h"


using namespace std;
using namespace sf;
using namespace glm;

enum class GameState {Playing, GameOver};

class Game
{
public:
	Game();
	Game(sf::ContextSettings settings);
	~Game();
	void run();
private:
	RenderWindow window;
	bool isRunning = false;
	void initialize();
	void update();
	void render();
	void unload();

	void drawIndividual(mat4 model);

	void checkReset(mat4 & modelIn);
	float randomX();
	bool checkCollision(mat4 modelOne, mat4 modelTwo);
	void updateGame();
	void updateGameOver();

	void renderGame();
	void renderGameOver();

	void reInit();

	void checkX(mat4 & matIn);

	const int NO_OF_ENEMIES = 10;
	const int DISTANCE_BETWEEN_ENEMIES = 51;
	const int START_POS = -500;
	const int HALF_CUBE_WIDTH = 1.f;
	const float SPEED_INCREMENT = 0.001f;
	const int MAX_SPEED = 4;
	const float BASE_SPEED = 1.5f;
	GameState gameState;

	sf::Text txtGameOver;
	sf::Text txtGameOverInstructions;
	sf::Text startUpText;
	sf::Text txtTimeEnd;

	int score;
	float timeTotal;
	float speed;

	sf::Clock clock;
	sf::Time timeSinceLastUpdate;
};
#endif