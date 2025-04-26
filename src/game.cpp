#include "game.h"
#include "resource_manager.h"
#include "SpriteRenderer.h"
#include "ParticleGenerator.h"
#include "PostProcessor.h"
#include "Character.h"

SpriteRenderer  *Renderer = nullptr;

GameObject      *Player = nullptr;

BallObject     *Ball = nullptr;

ParticleGenerator   *Particles = nullptr;

PostProcessor   *Effects = nullptr;

GLfloat ShakeTime = 0.0f;

ma_result result;
ma_engine engine;
ma_sound bgmSound;
ma_sound solidBlockSound;
ma_sound softBlockSound;

float t_time = 3.0f;

Game::Game(GLuint width,GLuint height)
	: State(GAME_ACTIVE),Keys(),Width(width),Height(height)
{

}

Game::~Game()
{
	delete Renderer;
	delete Player;
	delete Ball;
	delete Particles;
	delete Effects;
	ma_sound_uninit(&bgmSound);
	ma_sound_uninit(&solidBlockSound);
	ma_sound_uninit(&softBlockSound);
	ma_engine_uninit(&engine);
}

void Game::Init()
{
	// 加载着色器
	ResourceManager::LoadShader("res/shader/SpriteVertex.vert","res/shader/SpriteFragment.frag","","sprite");
	ResourceManager::LoadShader("res/shader/CharacterVertex.vert","res/shader/CharacterFragment.frag","","textShader");
	// 配置着色器
	glm::mat4 projection = glm::ortho(0.0f,static_cast<GLfloat>(this->Width),
		static_cast<GLfloat>(this->Height),0.0f,-1.0f,1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("image",0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection",projection);

	ResourceManager::GetShader("textShader").Use().SetInteger("text",0);
	glm::mat4 textProjection = glm::ortho(0.0f,static_cast<GLfloat>(this->Width),0.0f,static_cast<GLfloat>(this->Height));
	ResourceManager::GetShader("textShader").Use().SetMatrix4("projection",textProjection);
	// 设置专用于渲染的控制
	Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));

	// 加载纹理
	ResourceManager::LoadTexture("res/textures/background.jpg",GL_FALSE,"background");
	ResourceManager::LoadTexture("res/textures/rock.jpeg",GL_TRUE,"block");
	ResourceManager::LoadTexture("res/textures/stranger1.jpg",GL_FALSE,"stranger1");
	ResourceManager::LoadTexture("res/textures/stranger2.jpg",GL_FALSE,"stranger2");
	ResourceManager::LoadTexture("res/textures/stranger3.jpg",GL_FALSE,"stranger3");
	ResourceManager::LoadTexture("res/textures/stranger4.jpg",GL_FALSE,"stranger4");
	ResourceManager::LoadTexture("res/textures/stranger5.jpg",GL_FALSE,"stranger5");

	// 加载关卡
	GameLevel one; one.Load("res/levels/one.lvl",this->Width,this->Height * 0.5);
	GameLevel two; two.Load("res/levels/two.lvl",this->Width,this->Height * 0.5);
	GameLevel three; three.Load("res/levels/three.lvl",this->Width,this->Height * 0.5);
	GameLevel four; four.Load("res/levels/four.lvl",this->Width,this->Height * 0.5);
	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Levels.push_back(three);
	this->Levels.push_back(four);
	this->Level = 0;


	// 加载玩家
	ResourceManager::LoadTexture("res/textures/paddle.png",true,"paddle");
	glm::vec2 playerPos = glm::vec2(
		this->Width / 2 - PLAYER_SIZE.x / 2,
		this->Height - PLAYER_SIZE.y
	);
	Player = new GameObject(playerPos,PLAYER_SIZE,ResourceManager::GetTexture("paddle"));

	// 加载球
	ResourceManager::LoadShader("res/shader/BallVertex.vert","res/shader/BallFragment.frag","","ball");
	ResourceManager::LoadTexture("res/textures/aoteman.png",GL_FALSE,"face");
	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS,-BALL_RADIUS * 2);
	Ball = new BallObject(ballPos,BALL_RADIUS,INITIAL_BALL_VELOCITY,
		ResourceManager::GetTexture("face"));
	// 加载粒子
	ResourceManager::LoadShader("res/shader/ParticlesVertex.vert","res/shader/ParticlesFragment.frag","","particle");
	ResourceManager::LoadTexture("res/textures/particle.png",GL_TRUE,"particle");
	ResourceManager::GetShader("particle").Use().SetInteger("sprite",0);
	ResourceManager::GetShader("particle").SetMatrix4("projection",projection);
	Particles = new ParticleGenerator(
	   ResourceManager::GetShader("particle"),
	   ResourceManager::GetTexture("particle"),
	   500,
	   glm::vec2(1*BALL_RADIUS,-BALL_RADIUS)
	);
	//glm::vec2(ballPos.x + 10 * BALL_RADIUS,ballPos.y)
	// 加载帧缓冲
	ResourceManager::LoadShader("res/shader/PostprocessingVertex.vert","res/shader/PostprocessingFragment.frag","","postprocessing");
	Effects = new PostProcessor(ResourceManager::GetShader("postprocessing"),this->Width,this->Height);


	// 加载道具
	ResourceManager::LoadTexture("res/textures/powerup_speed.png",GL_TRUE,"powerup_speed");
	ResourceManager::LoadTexture("res/textures/powerup_sticky.png",GL_TRUE,"powerup_sticky");
	ResourceManager::LoadTexture("res/textures/powerup_increase.png",GL_TRUE,"powerup_increase");
	ResourceManager::LoadTexture("res/textures/powerup_confuse.png",GL_TRUE,"powerup_confuse");
	ResourceManager::LoadTexture("res/textures/powerup_chaos.png",GL_TRUE,"powerup_chaos");
	ResourceManager::LoadTexture("res/textures/powerup_passthrough.png",GL_TRUE,"powerup_passthrough");

	// 字体渲染
	fontMagager.init("res/fonts/simsun.ttc");

	// 加载音效
	ma_sound_init_from_file(&engine,"res/music/bgm.mp3",0,NULL,NULL,&bgmSound);
	ma_sound_set_looping(&bgmSound,MA_TRUE);
	ma_sound_start(&bgmSound);

	ma_sound_init_from_file(&engine,"res/music/collisionBlock.mp3",0,NULL,NULL,&solidBlockSound);
	ma_sound_init_from_file(&engine,"res/music/collisionNormal.mp3",0,NULL,NULL,&softBlockSound);
}

void Game::Update(GLfloat dt)
{
	// 更新对象
	Ball->Move(dt,this->Width);
	// 检测碰撞
	this->DoCollisions();

	// 更新粒子
	Particles->Update(dt,*Ball,2,glm::vec2(Ball->Radius / 2));
	if(t_time>0){
		t_time -= dt;
	} else{
		t_time = -1;
	}
	// 更新道具
	this->UpdatePowerUps(dt);
	if(Ball->Position.y >= this->Height) // 球是否接触底部边界？
	{
		this->ResetLevel();
		this->ResetPlayer();
	}

	if(ShakeTime > 0.0f)
	{
		ShakeTime -= dt;
		if(ShakeTime <= 0.0f)
			Effects->Shake = false;
	}
}


void Game::ProcessInput(GLfloat dt)
{
	if(this->State == GAME_ACTIVE)
	{
		GLfloat velocity = PLAYER_VELOCITY * dt;
		// 移动挡板
		if(this->Keys[GLFW_KEY_A])
		{
			if(Player->Position.x >= 0){
				Player->Position.x -= velocity;
				if(Ball->Stuck)
					Ball->Position.x -= velocity;
			}

		}
		if(this->Keys[GLFW_KEY_D])
		{
			if(Player->Position.x <= this->Width - Player->Size.x){
				Player->Position.x += velocity;
				if(Ball->Stuck)
					Ball->Position.x += velocity;
			}
		}
		if(this->Keys[GLFW_KEY_SPACE])
			Ball->Stuck = false;
	}
}

void Game::Render()
{
	if(this->State == GAME_ACTIVE)
	{
		Effects->BeginRender();
		// 绘制背景
		Renderer->DrawSprite(ResourceManager::GetTexture("background"),
			glm::vec2(0,0),glm::vec2(this->Width,this->Height),0.0f
		);

		// 绘制关卡
		this->Levels[this->Level].Draw(*Renderer);

		if(t_time>0.0f){
			renderCharacter(ResourceManager::GetShader("textShader"),"第"+std::to_string(this->Level+1)+"关",
				this->Width/2-150,this->Height/2,2.5f,glm::vec3(252.0f/255.0f,236.0f/255.0f,72.0f/255.0f));
		}
		// 绘制道具
		for(PowerUp &powerUp : this->PowerUps)
			if(!powerUp.Destroyed)
				powerUp.Draw(*Renderer);
		// 绘制玩家
		Player->Draw(*Renderer);
		// 绘制粒子   
		Particles->Draw();
		// 绘制球
		//Ball->Draw(*Renderer);
		Ball->ownDraw(ResourceManager::GetShader("ball"),static_cast<float>(this->Width),static_cast<float>(this->Height));

		Effects->EndRender();
		Effects->Render(glfwGetTime());
	}
}

GLboolean Game::CheckCollision(GameObject &one,GameObject &two) // AABB - AABB collision
{
	// x轴方向碰撞？
	bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
		two.Position.x + two.Size.x >= one.Position.x;
	// y轴方向碰撞？
	bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
		two.Position.y + two.Size.y >= one.Position.y;
	// 只有两个轴向都有碰撞时才碰撞
	return collisionX && collisionY;
}

Collision Game::CheckCollision(BallObject &one,GameObject &two)
{
	// 获取圆的中心
	glm::vec2 center(one.Position + one.Radius);
	// 计算AABB的信息（中心、半边长）
	glm::vec2 aabb_half_extents(two.Size.x / 2,two.Size.y / 2);
	glm::vec2 aabb_center(
		two.Position.x + aabb_half_extents.x,
		two.Position.y + aabb_half_extents.y
	);
	// 获取两个中心的差矢量
	glm::vec2 difference = center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference,-aabb_half_extents,aabb_half_extents);
	// AABB_center加上clamped这样就得到了碰撞箱上距离圆最近的点closest
	glm::vec2 closest = aabb_center + clamped;
	// 获得圆心center和最近点closest的矢量并判断是否 length <= radius
	difference = closest - center;

	if(glm::length(difference) <= one.Radius)
		return std::make_tuple(GL_TRUE,VectorDirection(difference),difference);
	else
		return std::make_tuple(GL_FALSE,UP,glm::vec2(0,0));
}

void ActivatePowerUp(PowerUp &powerUp);

void Game::DoCollisions()
{
	int num = 0;
	for(GameObject &box : this->Levels[this->Level].Bricks)
	{
		if(!box.Destroyed)
		{
			Collision collision = CheckCollision(*Ball,box);
			if(std::get<0>(collision))
			{
				if(!box.IsSolid){
					box.Destroyed = GL_TRUE;
					this->SpawnPowerUps(box);
					ma_sound_start(&softBlockSound);
					if(ma_sound_at_end(&softBlockSound)){
						ma_sound_seek_to_pcm_frame(&softBlockSound,0);
					}
				} else{
					ShakeTime = 0.05f;
					Effects->Shake = GL_TRUE;
					ma_sound_start(&solidBlockSound);
					if(ma_sound_at_end(&solidBlockSound)){
						ma_sound_seek_to_pcm_frame(&solidBlockSound,0);
					}
					//ma_engine_play_sound(&engine,"res/music/collisionBlock.mp3",NULL);
				}
				// 碰撞处理
				Direction dir = std::get<1>(collision);
				glm::vec2 diff_vector = std::get<2>(collision);
				if(!(Ball->PassThrough && !box.IsSolid)){
					if(dir == LEFT || dir == RIGHT) // 水平方向碰撞
					{
						Ball->Velocity.x = -Ball->Velocity.x; // 反转水平速度
						// 重定位
						GLfloat penetration = Ball->Radius - std::abs(diff_vector.x);
						if(dir == LEFT)
							Ball->Position.x += penetration; // 将球右移
						else
							Ball->Position.x -= penetration; // 将球左移
					} else // 垂直方向碰撞
					{
						Ball->Velocity.y = -Ball->Velocity.y; // 反转垂直速度
						// 重定位
						GLfloat penetration = Ball->Radius - std::abs(diff_vector.y);
						if(dir == UP)
							Ball->Position.y -= penetration; // 将球上移
						else
							Ball->Position.y += penetration; // 将球下移
					}
				}
			}
		} else{
			num++;
		}
	}

	if(num==this->Levels[this->Level].soft){
		// 通关了
		this->Level = (this->Level+1)%this->Levels.size();
		ResetLevel();
		ResetPlayer();
	}

	for(PowerUp &powerUp : this->PowerUps)
	{
		if(!powerUp.Destroyed)
		{
			// First check if powerup passed bottom edge, if so: keep as inactive and destroy
			if(powerUp.Position.y >= this->Height)
				powerUp.Destroyed = GL_TRUE;

			if(CheckCollision(*Player,powerUp))
			{	// Collided with player, now activate powerup
				ma_engine_play_sound(&engine,"res/music/powerup.wav",NULL);
				ActivatePowerUp(powerUp);
				powerUp.Destroyed = GL_TRUE;
				powerUp.Activated = GL_TRUE;
			}
		}
	}

	Collision result = CheckCollision(*Ball,*Player);
	if(!Ball->Stuck && std::get<0>(result))
	{
		// 检查碰到了挡板的哪个位置，并根据碰到哪个位置来改变速度
		GLfloat centerBoard = Player->Position.x + Player->Size.x / 2;
		GLfloat distance = (Ball->Position.x + Ball->Radius) - centerBoard;
		GLfloat percentage = distance / (Player->Size.x / 2);
		// 依据结果移动
		GLfloat strength = 2.0f;
		glm::vec2 oldVelocity = Ball->Velocity;
		Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
		//Ball->Velocity.y = -Ball->Velocity.y;
		Ball->Velocity.y = -1 * abs(Ball->Velocity.y);
		Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);

		Ball->Stuck = Ball->Sticky;
	}
}

Direction Game::VectorDirection(glm::vec2 target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f,1.0f),  // 上
		glm::vec2(1.0f,0.0f),  // 右
		glm::vec2(0.0f,-1.0f), // 下
		glm::vec2(-1.0f,0.0f)  // 左
	};
	GLfloat max = 0.0f;
	GLuint best_match = -1;
	for(GLuint i = 0; i < 4; i++)
	{
		GLfloat dot_product = glm::dot(glm::normalize(target),compass[i]);
		if(dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}
	return (Direction)best_match;
}

// Reset
void Game::ResetLevel()
{
	if(this->Level == 0)
		this->Levels[0].Load("res/levels/one.lvl",this->Width,this->Height * 0.5f);
	else if(this->Level == 1)
		this->Levels[1].Load("res/levels/two.lvl",this->Width,this->Height * 0.5f);
	else if(this->Level == 2)
		this->Levels[2].Load("res/levels/three.lvl",this->Width,this->Height * 0.5f);
	else if(this->Level == 3)
		this->Levels[3].Load("res/levels/four.lvl",this->Width,this->Height * 0.5f);

	t_time = 2.0f;
}
void Game::ResetPlayer()
{
	// Reset player/ball stats
	Player->Size = PLAYER_SIZE;
	Player->Position = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2,this->Height - PLAYER_SIZE.y);
	Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS,-(BALL_RADIUS * 2)),INITIAL_BALL_VELOCITY);

	// Also disable all active powerups
	Effects->Chaos = Effects->Confuse = GL_FALSE;
	Ball->PassThrough = Ball->Sticky = GL_FALSE;
	Player->Color = glm::vec3(1.0f);
	Ball->Color = glm::vec3(1.0f);
	PowerUps.clear();
}

GLboolean ShouldSpawn(GLuint chance)
{
	GLuint random = rand() % chance;
	return random == 0;
}

void Game::SpawnPowerUps(GameObject &block)
{
	if(ShouldSpawn(50)) // 1 in 75 chance
		this->PowerUps.push_back(PowerUp("speed",glm::vec3(0.5f,0.5f,1.0f),0.0f,block.Position,ResourceManager::GetTexture("powerup_speed")));
	if(ShouldSpawn(50))
		this->PowerUps.push_back(PowerUp("sticky",glm::vec3(1.0f,0.5f,1.0f),20.0f,block.Position,ResourceManager::GetTexture("powerup_sticky")));
	if(ShouldSpawn(50))
		this->PowerUps.push_back(PowerUp("pass-through",glm::vec3(0.5f,1.0f,0.5f),10.0f,block.Position,ResourceManager::GetTexture("powerup_passthrough")));
	if(ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("pad-size-increase",glm::vec3(1.0f,0.6f,0.4),0.0f,block.Position,ResourceManager::GetTexture("powerup_increase")));
	if(ShouldSpawn(75)) // Negative powerups should spawn more often
		this->PowerUps.push_back(PowerUp("confuse",glm::vec3(1.0f,0.3f,0.3f),15.0f,block.Position,ResourceManager::GetTexture("powerup_confuse")));
	if(ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("chaos",glm::vec3(0.9f,0.25f,0.25f),15.0f,block.Position,ResourceManager::GetTexture("powerup_chaos")));

}
// PowerUps
GLboolean IsOtherPowerUpActive(std::vector<PowerUp> &powerUps,std::string type);

void Game::UpdatePowerUps(GLfloat dt)
{
	for(PowerUp &powerUp : this->PowerUps)
	{
		powerUp.Position += powerUp.Velocity * dt;
		if(powerUp.Activated)
		{
			powerUp.Duration -= dt;

			if(powerUp.Duration <= 0.0f)
			{
				// Remove powerup from list (will later be removed)
				powerUp.Activated = GL_FALSE;
				// Deactivate effects
				if(powerUp.Type == "sticky")
				{
					if(!IsOtherPowerUpActive(this->PowerUps,"sticky"))
					{	// Only reset if no other PowerUp of type sticky is active
						Ball->Sticky = GL_FALSE;
						Player->Color = glm::vec3(1.0f);
					}
				} else if(powerUp.Type == "pass-through")
				{
					if(!IsOtherPowerUpActive(this->PowerUps,"pass-through"))
					{	// Only reset if no other PowerUp of type pass-through is active
						Ball->PassThrough = GL_FALSE;
						Ball->Color = glm::vec3(1.0f);
					}
				} else if(powerUp.Type == "confuse")
				{
					if(!IsOtherPowerUpActive(this->PowerUps,"confuse"))
					{	// Only reset if no other PowerUp of type confuse is active
						Effects->Confuse = GL_FALSE;
					}
				} else if(powerUp.Type == "chaos")
				{
					if(!IsOtherPowerUpActive(this->PowerUps,"chaos"))
					{	// Only reset if no other PowerUp of type chaos is active
						Effects->Chaos = GL_FALSE;
					}
				}
			}
		}
	}
	// Remove all PowerUps from vector that are destroyed AND !activated (thus either off the map or finished)
	// Note we use a lambda expression to remove each PowerUp which is destroyed and not activated
	this->PowerUps.erase(std::remove_if(this->PowerUps.begin(),this->PowerUps.end(),
		[](const PowerUp &powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
	),this->PowerUps.end());
}

void ActivatePowerUp(PowerUp &powerUp)
{
	// Initiate a powerup based type of powerup
	if(powerUp.Type == "speed")
	{
		Ball->Velocity *= 1.2;
	} else if(powerUp.Type == "sticky")
	{
		Ball->Sticky = GL_TRUE;
		Player->Color = glm::vec3(1.0f,0.5f,1.0f);
	} else if(powerUp.Type == "pass-through")
	{
		Ball->PassThrough = GL_TRUE;
		Ball->Color = glm::vec3(1.0f,0.5f,0.5f);
	} else if(powerUp.Type == "pad-size-increase")
	{
		Player->Size.x += 50;
	} else if(powerUp.Type == "confuse")
	{
		if(!Effects->Chaos)
			Effects->Confuse = GL_TRUE; // Only activate if chaos wasn't already active
	} else if(powerUp.Type == "chaos")
	{
		if(!Effects->Confuse)
			Effects->Chaos = GL_TRUE;
	}
}

GLboolean IsOtherPowerUpActive(std::vector<PowerUp> &powerUps,std::string type)
{
	// Check if another PowerUp of the same type is still active
	// in which case we don't disable its effect (yet)
	for(const PowerUp &powerUp : powerUps)
	{
		if(powerUp.Activated)
			if(powerUp.Type == type)
				return GL_TRUE;
	}
	return GL_FALSE;
}

