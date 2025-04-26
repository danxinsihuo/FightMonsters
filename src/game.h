#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "GameLevel.h"
#include "BallObject.h"
#include "PowerUp.h"

#include "miniaudio.h"

// Represents the current state of the game
enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};

enum Direction {
	UP,
	RIGHT,
	DOWN,
	LEFT
};

struct Text{
public:
	Shader t_shader;
	std::string t_text;
	glm::vec2 t_pos;
	glm::vec4 t_color;
	float t_scale;
	float t_time;

	Text(){}
	Text(Shader& shader,std::string text,glm::vec2 pos,glm::vec4 color,float scale,float time)
		:t_shader(shader),t_text(text),t_pos(pos),t_color(color),t_scale(scale),t_time(time)
	{}
};

typedef std::tuple<GLboolean,Direction,glm::vec2> Collision;

const float scale = 1.25;
// 初始化挡板的大小
const glm::vec2 PLAYER_SIZE(150 * scale,20 * scale);
// 初始化挡板的速率
const GLfloat PLAYER_VELOCITY(500.0f);

// 初始化球的速度
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f,-350.0f);
// 球的半径
const GLfloat BALL_RADIUS = 20.0f * scale;
// 粒子
const GLuint nr_particles = 500;

// 音效
extern ma_result result;
extern ma_engine engine;
extern ma_sound bgmSound;
extern ma_sound solidBlockSound;
extern ma_sound softBlockSound;

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game
{
public:
	// Game state
	GameState              State;
	GLboolean              Keys[1024];
	GLuint                 Width,Height;

	// Game level
	std::vector<GameLevel> Levels;
	GLuint                 Level;

	// Game powerup
	std::vector<PowerUp>  PowerUps;

	// Constructor/Destructor
	Game(GLuint width,GLuint height);
	~Game();
	// Initialize game state (load all shaders/textures/levels)
	void Init();
	// GameLoop
	void ProcessInput(GLfloat dt);
	void Update(GLfloat dt);
	void Render();
	void DoCollisions();
	GLboolean CheckCollision(GameObject &one,GameObject &two);
	Collision CheckCollision(BallObject &one,GameObject &two);
	Direction VectorDirection(glm::vec2 target);

	// reset
	void ResetLevel();
	void ResetPlayer();

	// powerup
	void SpawnPowerUps(GameObject &block);
	void UpdatePowerUps(GLfloat dt);
};