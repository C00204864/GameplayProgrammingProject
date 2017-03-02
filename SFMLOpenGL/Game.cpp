#include <Game.h>
#include <Cube.h>

template <typename T>
string toString(T number)
{
	ostringstream oss;
	oss << number;
	return oss.str();
}

GLuint	vsid,		// Vertex Shader ID
		fsid,		// Fragment Shader ID
		progID,		// Program ID
		vao = 0,	// Vertex Array ID
		vbo,		// Vertex Buffer ID
		vib,		// Vertex Index Buffer
		to;			// Texture ID 1 to 32
GLint	positionID,	// Position ID
		colorID,	// Color ID
		textureID,	// Texture ID
		uvID,		// UV ID
		mvpID;		// Model View Projection ID

GLenum	error;		// OpenGL Error Code

//const string filename = ".//Assets//Textures//coordinates.tga";
//const string filename = ".//Assets//Textures//cube.tga";
const string filename = ".//Assets//Textures//grid.tga";
//const string filename = ".//Assets//Textures//grid_wip.tga";
//const string filename = ".//Assets//Textures//minecraft.tga";
//const string filename = ".//Assets//Textures//texture.tga";
//const string filename = ".//Assets//Textures//texture_2.tga";
//const string filename = ".//Assets//Textures//uvtemplate.tga";


int width;						// Width of texture
int height;						// Height of texture
int comp_count;					// Component of texture

unsigned char* img_data;		// image data

mat4 mvp, projection, view, model, leftWall, rightWall;			// Model View Projection

std::vector<mat4> enemyBlocks;


Font font;

Game::Game() : 
	window(VideoMode(800, 600), 
	"Introduction to OpenGL Texturing")
{
}

Game::Game(sf::ContextSettings settings) : 
	window(VideoMode(800, 600), 
	"Introduction to OpenGL Texturing", 
	sf::Style::Default, 
	settings)
{
}

Game::~Game(){}


void Game::run()
{

	initialize();

	Event event;
	clock.restart();
	while (isRunning){

#if (DEBUG >= 2)
		DEBUG_MSG("Game running...");
#endif

		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
			{
				isRunning = false;
			}
		}
		timeSinceLastUpdate += clock.restart();
		if (timeSinceLastUpdate.asSeconds() > 1.f / 60.f)
		{
			update();
			timeSinceLastUpdate = sf::Time::Zero;
		}
		render();
	}

#if (DEBUG >= 2)
	DEBUG_MSG("Calling Cleanup...");
#endif
	unload();

}

void Game::initialize()
{
	timeTotal = 0.f;
	gameState = GameState::Playing;
	speed = BASE_SPEED;
	font.loadFromFile(".//Assets//Fonts//BBrick.ttf");
	txtGameOver.setColor(sf::Color(0, 255, 0, 170));
	txtGameOver.setCharacterSize(60);
	txtGameOver.setPosition(210.f, 250.f);
	txtGameOver.setString("Game Over");
	txtGameOver.setFont(font);

	txtGameOverInstructions.setColor(sf::Color(0, 255, 0, 170));
	txtGameOverInstructions.setCharacterSize(20);
	txtGameOverInstructions.setPosition(120.f, 330.f);
	txtGameOverInstructions.setString("Press Escape To Quit Or Enter To Play Again");
	txtGameOverInstructions.setFont(font);

	txtTimeEnd.setColor(sf::Color(0, 255, 0, 170));
	txtTimeEnd.setCharacterSize(20);
	txtTimeEnd.setPosition(350.f, 360.f);
	txtTimeEnd.setFont(font);

	srand(time(0));
	int pos = -400;
	for (int i = 0; i < NO_OF_ENEMIES; i++)
	{
		pos -= DISTANCE_BETWEEN_ENEMIES;
		model[3][2] = pos;
		model[3][0] = randomX();
		enemyBlocks.push_back(model);
	}
	leftWall = glm::scale(glm::translate(leftWall, glm::vec3(-12, 0, 0)), glm::vec3(1, 1, 10000));
	rightWall = glm::scale(glm::translate(rightWall, glm::vec3(12, 0, 0)), glm::vec3(1, 1, 10000));
	isRunning = true;
	GLint isCompiled = 0;
	GLint isLinked = 0;

	if (!(!glewInit())) { DEBUG_MSG("glewInit() failed"); }

	// Copy UV's to all faces
	for (int i = 1; i < 6; i++)
		memcpy(&uvs[i * 4 * 2], &uvs[0], 2 * 4 * sizeof(GLfloat));

	DEBUG_MSG(glGetString(GL_VENDOR));
	DEBUG_MSG(glGetString(GL_RENDERER));
	DEBUG_MSG(glGetString(GL_VERSION));

	// Vertex Array Buffer
	glGenBuffers(1, &vbo);		// Gen Vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Vertices (3) x,y,z , Colors (4) RGBA, UV/ST (2)
	glBufferData(GL_ARRAY_BUFFER, ((3 * VERTICES) + (4 * COLORS) + (2 * UVS)) * sizeof(GLfloat), NULL, GL_STATIC_DRAW);

	glGenBuffers(1, &vib); //Gen Vertex Index Buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vib);

	// Indices to be drawn
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * INDICES * sizeof(GLuint), indices, GL_STATIC_DRAW);

	const char* vs_src = 
		"#version 400\n\r"
		""
		//"layout(location = 0) in vec3 sv_position; //Use for individual Buffers"
		//"layout(location = 1) in vec4 sv_color; //Use for individual Buffers"
		//"layout(location = 2) in vec2 sv_texel; //Use for individual Buffers"
		""
		"in vec3 sv_position;"
		"in vec4 sv_color;"
		"in vec2 sv_uv;"
		""
		"out vec4 color;"
		"out vec2 uv;"
		""
		"uniform mat4 sv_mvp;"
		""
		"void main() {"
		"	color = sv_color;"
		"	uv = sv_uv;"
		//"	gl_Position = vec4(sv_position, 1);"
		"	gl_Position = sv_mvp * vec4(sv_position, 1);"
		"}"; //Vertex Shader Src

	DEBUG_MSG("Setting Up Vertex Shader");

	vsid = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vsid, 1, (const GLchar**)&vs_src, NULL);
	glCompileShader(vsid);

	// Check is Shader Compiled
	glGetShaderiv(vsid, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_TRUE) {
		DEBUG_MSG("Vertex Shader Compiled");
		isCompiled = GL_FALSE;
	}
	else
	{
		DEBUG_MSG("ERROR: Vertex Shader Compilation Error");
	}

	const char* fs_src =
		"#version 400\n\r"
		""
		"uniform sampler2D f_texture;"
		""
		"in vec4 color;"
		"in vec2 uv;"
		""
		"out vec4 fColor;"
		""
		"void main() {"
		//"	vec4 lightColor = vec4(1.0f, 0.0f, 0.0f, 1.0f); "
		//"	fColor = vec4(0.50f, 0.50f, 0.50f, 1.0f);"
		//"	fColor = texture2D(f_texture, uv);"
		//"	fColor = color * texture2D(f_texture, uv);"
		//"	fColor = lightColor * texture2D(f_texture, uv);"
		"	fColor = texture2D(f_texture, uv);"
		//"	fColor = color - texture2D(f_texture, uv);"
		//"	fColor = color;"
		"}"; //Fragment Shader Src

	DEBUG_MSG("Setting Up Fragment Shader");

	fsid = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fsid, 1, (const GLchar**)&fs_src, NULL);
	glCompileShader(fsid);

	// Check is Shader Compiled
	glGetShaderiv(fsid, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_TRUE) {
		DEBUG_MSG("Fragment Shader Compiled");
		isCompiled = GL_FALSE;
	}
	else
	{
		DEBUG_MSG("ERROR: Fragment Shader Compilation Error");
	}

	DEBUG_MSG("Setting Up and Linking Shader");
	progID = glCreateProgram();
	glAttachShader(progID, vsid);
	glAttachShader(progID, fsid);
	glLinkProgram(progID);

	// Check is Shader Linked
	glGetProgramiv(progID, GL_LINK_STATUS, &isLinked);

	if (isLinked == 1) {
		DEBUG_MSG("Shader Linked");
	}
	else
	{
		DEBUG_MSG("ERROR: Shader Link Error");
	}

	// Set image data
	// https://github.com/nothings/stb/blob/master/stb_image.h
	img_data = stbi_load(filename.c_str(), &width, &height, &comp_count, 4);

	if (img_data == NULL)
	{
		DEBUG_MSG("ERROR: Texture not loaded");
	}

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &to);
	glBindTexture(GL_TEXTURE_2D, to);

	// Wrap around
	// https://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexParameter.xml
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// Filtering
	// https://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexParameter.xml
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Bind to OpenGL
	// https://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexImage2D.xml
	glTexImage2D(
		GL_TEXTURE_2D,			// 2D Texture Image
		0,						// Mipmapping Level 
		GL_RGBA,				// GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA, GL_RGB, GL_BGR, GL_RGBA 
		width,					// Width
		height,					// Height
		0,						// Border
		GL_RGBA,				// Bitmap
		GL_UNSIGNED_BYTE,		// Specifies Data type of image data
		img_data				// Image Data
		);

	// Projection Matrix 
	projection = perspective(
		45.0f,					// Field of View 45 degrees
		4.0f / 3.0f,			// Aspect ratio
		0.1f,					// Display Range Min : 0.1f unit
		500.0f					// Display Range Max : 100.0f unit
		);

	// Camera Matrix
	view = lookAt(
		vec3(0.0f, 4.0f, 10.0f),	// Camera (x,y,z), in World Space
		vec3(0.0f, 0.0f, 0.0f),		// Camera looking at origin
		vec3(0.0f, 1.0f, 0.0f)		// 0.0f, 1.0f, 0.0f Look Down and 0.0f, -1.0f, 0.0f Look Up
		);

	// Model matrix
	model = mat4(
		1.0f					// Identity Matrix
		);
	// Enable Depth Test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	// Load Font
	
}

void Game::update()
{
	switch (gameState)
	{
	case GameState::Playing:
		updateGame();
		break;
	case GameState::GameOver:
		updateGameOver();
		break;
	default:
		break;
	}
}

void Game::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	switch (gameState)
	{
	case GameState::Playing:
		renderGame();
		break;
	case GameState::GameOver:
		renderGameOver();
		break;
	default:
		break;
	}
	
}

void Game::unload()
{
#if (DEBUG >= 2)
	DEBUG_MSG("Cleaning up...");
#endif
	glDetachShader(progID, vsid);	// Shader could be used with more than one progID
	glDetachShader(progID, fsid);	// ..
	glDeleteShader(vsid);			// Delete Vertex Shader
	glDeleteShader(fsid);			// Delete Fragment Shader
	glDeleteProgram(progID);		// Delete Shader
	glDeleteBuffers(1, &vbo);		// Delete Vertex Buffer
	glDeleteBuffers(1, &vib);		// Delete Vertex Index Buffer
	stbi_image_free(img_data);		// Free image stbi_image_free(..)
}

void Game::drawIndividual(mat4 modelIn)
{
	mvp = projection * view * modelIn;

	// VBO Data....vertices, colors and UV's appended
	glBufferSubData(GL_ARRAY_BUFFER, 0 * VERTICES * sizeof(GLfloat), 3 * VERTICES * sizeof(GLfloat), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * VERTICES * sizeof(GLfloat), 4 * COLORS * sizeof(GLfloat), colors);
	glBufferSubData(GL_ARRAY_BUFFER, ((3 * VERTICES) + (4 * COLORS)) * sizeof(GLfloat), 2 * UVS * sizeof(GLfloat), uvs);

	// Send transformation to shader mvp uniform
	glUniformMatrix4fv(mvpID, 1, GL_FALSE, &mvp[0][0]);

	// Set Active Texture .... 32
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(textureID, 0);

	// Set pointers for each parameter (with appropriate starting positions)
	// https://www.khronos.org/opengles/sdk/docs/man/xhtml/glVertexAttribPointer.xml
	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, (VOID*)(3 * VERTICES * sizeof(GLfloat)));
	glVertexAttribPointer(uvID, 2, GL_FLOAT, GL_FALSE, 0, (VOID*)(((3 * VERTICES) + (4 * COLORS)) * sizeof(GLfloat)));

	// Enable Arrays
	glEnableVertexAttribArray(positionID);
	glEnableVertexAttribArray(colorID);
	glEnableVertexAttribArray(uvID);

	// Draw Element Arrays
	glDrawElements(GL_TRIANGLES, 3 * INDICES, GL_UNSIGNED_INT, NULL);
}

float Game::randomX()
{
	return ((rand() % 20) - 9.5f);
}

void Game::checkReset(mat4 & modelIn)
{
	if (modelIn[3][2] > 10)
	{
		modelIn[3][2] = START_POS;
		modelIn[3][0] = randomX();
	}
}

bool Game::checkCollision(mat4 mdlOne, mat4 mdlTwo)
{
	int a = mdlOne[3][0];
	int b = mdlOne[3][2];
	int c = mdlTwo[3][0];
	int d = mdlTwo[3][2];
	if (mdlOne[3][0] + HALF_CUBE_WIDTH < mdlTwo[3][0] - HALF_CUBE_WIDTH || mdlOne[3][0] - HALF_CUBE_WIDTH > mdlTwo[3][0] + HALF_CUBE_WIDTH)
	{
		return false;
	}
	if (mdlOne[3][2] + HALF_CUBE_WIDTH < mdlTwo[3][2] - HALF_CUBE_WIDTH || mdlOne[3][2] - HALF_CUBE_WIDTH > mdlTwo[3][2] + HALF_CUBE_WIDTH)
	{
		return false;
	}
	return true;
}

void Game::updateGame()
{
	timeTotal += timeSinceLastUpdate.asSeconds();
	// Update Model View Projection
	if (speed < MAX_SPEED)
	{
		speed += SPEED_INCREMENT;
	}
	for (auto & block : enemyBlocks)
	{
		checkReset(block);
		block = glm::translate(block, glm::vec3(0, 0, speed));
		if (checkCollision(model, block))
		{
			txtTimeEnd.setString("Time: " + string(to_string(static_cast<int>(timeTotal))));
			gameState = GameState::GameOver;
		}
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		model = glm::translate(model, glm::vec3(-0.3f, 0, 0));
		view = glm::translate(view, glm::vec3(0.3f, 0, 0));
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		model = glm::translate(model, glm::vec3(0.3f, 0, 0));
		view = glm::translate(view, glm::vec3(-0.3f, 0, 0));
	}
	checkX(model);
	checkX(view);
}

void Game::updateGameOver()
{
	if (sf::Keyboard::isKeyPressed(Keyboard::Return))
	{
		reInit();
		gameState = GameState::Playing;
	}
	else if (sf::Keyboard::isKeyPressed(Keyboard::Escape))
	{
		isRunning = false;
		window.close();
	}
}

void Game::renderGame()
{
	// Save current OpenGL render states
	// https://www.sfml-dev.org/documentation/2.0/classsf_1_1RenderTarget.php#a8d1998464ccc54e789aaf990242b47f7
	window.pushGLStates();

	// Find mouse position using sf::Mouse
	int x = Mouse::getPosition(window).x;
	int y = Mouse::getPosition(window).y;

	string hud = "Time: " + string(to_string(static_cast<int>(timeTotal)));

	Text text(hud, font);

	text.setColor(sf::Color(0, 255, 0, 170));
	text.setPosition(5.f, 0.f);
	text.setCharacterSize(30);

	window.draw(text);

	// Restore OpenGL render states
	// https://www.sfml-dev.org/documentation/2.0/classsf_1_1RenderTarget.php#a8d1998464ccc54e789aaf990242b47f7

	window.popGLStates();
	// Rebind Buffers and then set SubData
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vib);

	// Use Progam on GPU
	glUseProgram(progID);

	// Find variables in the shader
	// https://www.khronos.org/opengles/sdk/docs/man/xhtml/glGetAttribLocation.xml
	positionID = glGetAttribLocation(progID, "sv_position");
	if (positionID < 0) { DEBUG_MSG("positionID not found"); }
	colorID = glGetAttribLocation(progID, "sv_color");
	if (colorID < 0) { DEBUG_MSG("colorID not found"); }
	uvID = glGetAttribLocation(progID, "sv_uv");
	if (uvID < 0) { DEBUG_MSG("uvID not found"); }
	textureID = glGetUniformLocation(progID, "f_texture");
	if (textureID < 0) { DEBUG_MSG("textureID not found"); }
	mvpID = glGetUniformLocation(progID, "sv_mvp");
	if (mvpID < 0) { DEBUG_MSG("mvpID not found"); }
	drawIndividual(model);
	for (int i = 0; i < enemyBlocks.size(); i++)
	{
		drawIndividual(enemyBlocks.at(i));
	}
	drawIndividual(leftWall);
	drawIndividual(rightWall);


	window.display();

	// Disable Arrays
	glDisableVertexAttribArray(positionID);
	glDisableVertexAttribArray(colorID);
	glDisableVertexAttribArray(uvID);

	// Unbind Buffers with 0 (Resets OpenGL States...important step)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Reset the Shader Program to Use
	glUseProgram(0);

	// Check for OpenGL Error code
	error = glGetError();
	if (error != GL_NO_ERROR) {
		DEBUG_MSG(error);
	}
}

void Game::renderGameOver()
{
	// Save current OpenGL render states
	// https://www.sfml-dev.org/documentation/2.0/classsf_1_1RenderTarget.php#a8d1998464ccc54e789aaf990242b47f7
	window.pushGLStates();
	window.draw(txtGameOver);
	window.draw(txtGameOverInstructions);
	window.draw(txtTimeEnd);
	window.popGLStates();
	window.display();
}

void Game::reInit()
{
	timeTotal = 0;
	speed = BASE_SPEED;
	int pos = START_POS;
	for (int i = 0; i < NO_OF_ENEMIES; i++)
	{
		pos -= DISTANCE_BETWEEN_ENEMIES;
		enemyBlocks.at(i)[3][2] = pos;
		enemyBlocks.at(i)[3][0] = randomX();
	}
}

void Game::checkX(mat4& matIn)
{
	if (matIn[3][0] > 9.5f)
	{
		matIn[3][0] = 9.5f;
	}
	else if (matIn[3][0] < -9.5f)
	{
		matIn[3][0] = -9.5f;
	}
}