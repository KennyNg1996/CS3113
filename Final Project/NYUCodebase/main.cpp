/*

Animations: Title Screen Coming down, screen shaking when player or invader is hit

Sound Effects: Laser firing, any collisions, game over sound, player win sound

Music: Title screen and during game level

START GAME BY PRESSING 1, 2, OR 3 FOR DIFFICULTY

EACH BLOCK IS WORTH 10 POINTS AND EACH INVADER IS WORTH 100

PLAYER1: USE LEFT AND RIGHT ARROW KEYS TO MOVE, SPACEBAR TO SHOOT

PLAYER 2: USE A AND D KEYS TO MOVE, TAB TO SHOOT

PRESS Q TO QUIT (main menu and game over menu)

*/

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <stdio.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <assert.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <SDL_mixer.h>
#include <vector>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

float PI = 3.14159265358979323846;

SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}
	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(image);
	return retTexture;
}

void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	for (int i = 0; i < text.size(); i++) {
		int spriteIndex = (int)text[i];
		float texture_x = (float)(spriteIndex % 16) / 16.0f;
		float texture_y = (float)(spriteIndex / 16) / 16.0f;

		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}

	glBindTexture(GL_TEXTURE_2D, fontTexture);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6 * text.size());

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

class SheetSprite {
public:
	SheetSprite();
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
		size) : textureID(textureID), u(u), v(v), width(width), height(height), size(size) {}

	void Draw(ShaderProgram *program);

	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};

void SheetSprite::Draw(ShaderProgram *program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
	};
	float aspect = width / height;
	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, -0.5f * size };

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

class Entity
{
public:
	float x;
	float y;
	float dirX;
	float dirY;
	SheetSprite sprite;
	bool alive;

	Entity(float posX, float posY, float dX, float dY, SheetSprite spr, bool live)
		: x(posX), y(posY), dirX(dX), dirY(dY), sprite(spr), alive(live) {}

	void draw(Matrix& model, ShaderProgram& program)
	{
		model.identity();
		model.Translate(x, y, 0);
		program.setModelMatrix(model);
		sprite.Draw(&program);
	}
};

float lerp(float from, float to, float time) 
{
	return (1.0 - time)*from + time*to;
}

float start = 4.2f;
float end = 1.0f;

//display the main menu text
void renderMainMenu(Matrix& model, ShaderProgram& program, GLuint& texture, float elapsed)
{
	start -= 0.5f * elapsed;
	if (start <= end)
		start = end;
	model.identity();
	model.Translate(-2.1f, start, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "Space Invaders", 0.6, -.25);

	model.identity();
	model.Translate(-1.4, 0.0, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "Press 1, 2, or 3 to start the game", .2, -.1);

	model.identity();
	model.Translate(-1.1, -1.0, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "Press Q to exit the game", .2, -.1);
}

bool checkCollision(Entity& ent1, Entity& ent2)
{
	float ent1Left = ent1.x - ent1.sprite.size * (ent1.sprite.width / ent1.sprite.height) / 2;
	float ent1Right = ent1.x + ent1.sprite.size * (ent1.sprite.width / ent1.sprite.height) / 2;
	float ent2Left = ent2.x - ent2.sprite.size * (ent2.sprite.width / ent2.sprite.height) / 2;
	float ent2Right = ent2.x + ent2.sprite.size * (ent2.sprite.width / ent2.sprite.height) / 2;

	if ((ent1.y + ent1.sprite.size / 2) >= (ent2.y - ent2.sprite.size / 2)
		&& (ent1.y - ent1.sprite.size / 2) <= (ent2.y + ent2.sprite.size / 2) && ent1Left < ent2Right && ent1Right >= ent2Left)
	{
		return true;
	}

	return false;
}

int redLaserIndex = 0;
int invadersAlive = 30;
enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER, STATE_WIN };
const int EASY = 1;
const int NORMAL = 2;
const int HARD = 3;
int difficulty;
GameMode state;

float screenShake = 0;
float screenShakeValue = 0;
float screenShakeSpeed = 20;
float screenShakeIntensity = 0.01f;
int p1Score = 0;
int p2Score = 0;

//draw the players, invaders, and lasers based on their x and y positions and if they're alive
//only draw blocks if difficulty is normal or higher
void renderGameLevel(Matrix& model, ShaderProgram& program, Entity& player, Entity& player2, std::vector<Entity>& invaders,
	Entity& greenLaser, Entity& blueLaser, std::vector<Entity>& redLasers, std::vector<Entity>& blocks, std::vector<Entity>& moreBlocks,
	int difficulty, GLuint& texture)
{
	//easy difficulty or higher
	if (player.alive)
		player.draw(model, program);
	if (player2.alive)
		player2.draw(model, program);
	for (Entity invader : invaders)
	{
		if (invader.alive)
			invader.draw(model, program);
	}

	if (greenLaser.alive)
		greenLaser.draw(model, program);
	if (blueLaser.alive)
		blueLaser.draw(model, program);
	for (Entity redLaser : redLasers)
	{
		if (redLaser.alive)
			redLaser.draw(model, program);
	}

	if (difficulty == EASY)
	{
		for (Entity& block : blocks)
		{
			block.alive = false;
		}
	}

	//normal difficulty or higher
	if (difficulty <= NORMAL)
	{
		for (Entity& block : moreBlocks)
		{
			block.alive = false;
		}
	}

	if (difficulty >= NORMAL)
	{
		for (Entity block : blocks)
		{
			if (block.alive)
				block.draw(model, program);
		}
	}

	//hard difficulty
	if (difficulty == HARD)
	{
		for (Entity block : moreBlocks)
		{
			if (block.alive)
				block.draw(model, program);
		}
	}

	model.identity();
	model.Translate(-3.4, 2.8 + sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "P1 Score: " + std::to_string(p1Score), 0.3, -0.15);

	model.identity();
	model.Translate(1.4, 2.8 + sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "P2 Score: " + std::to_string(p2Score), 0.3, -0.15);
}

Mix_Music *menuMusic;
Mix_Music *music;
Mix_Chunk *fireSound;
Mix_Chunk *collisionSound;
Mix_Chunk *gameOverSound;
Mix_Chunk *winSound;

//update positions of the invaders and check for any collisions between entities
void update(Matrix& viewMatrix, ShaderProgram& program, Entity& player, Entity& player2, std::vector<Entity>& invaders, Entity& greenLaser, Entity& blueLaser,
	std::vector<Entity>& redLasers, std::vector<Entity>& blocks, std::vector<Entity>& moreBlocks, float elapsed, int difficulty)
{
	//move the green laser up if fired
	if (greenLaser.alive)
		greenLaser.y += 3.0f * elapsed;
	if (greenLaser.y > 3.0f)
		greenLaser.alive = false;			//don't update the laser if its off screen

	//move the blue laser up if fired
	if (blueLaser.alive)
		blueLaser.y += 3.0f * elapsed;
	if (blueLaser.y > 3.0f)
		blueLaser.alive = false;			//don't update the laser if its off screen

	//increase the laser speed depending on difficul4ty
	int redLaserSpeed;
	if (difficulty == EASY)
		redLaserSpeed = 2.0f;
	else if (difficulty == NORMAL)
		redLaserSpeed = 2.5f;
	else
		redLaserSpeed = 3.0f;

	for (Entity& redLaser : redLasers)
	{
		if (redLaser.alive)
			redLaser.y -= redLaserSpeed * elapsed;
		if (redLaser.y <= -3.0f)
			redLaser.alive = false;
		//check for collision between all red lasers and players; end game if both players are not alive
		if (redLaser.alive && player.alive && checkCollision(player, redLaser))
		{
			Mix_PlayChannel(-1, collisionSound, 0);
			redLaser.alive = false;
			player.alive = false;
			screenShake = 45;
		}
		if (redLaser.alive && player2.alive && checkCollision(player2, redLaser))
		{
			Mix_PlayChannel(-1, collisionSound, 0);
			redLaser.alive = false;
			player2.alive = false;
			screenShake = 45;
		}
		if (!player.alive && !player2.alive)
		{
			state = STATE_GAME_OVER;
			Mix_FadeOutMusic(4000);
			Mix_PlayChannel(-1, gameOverSound, 0);
		}
	}

	//check for block collision
	for (Entity& block : blocks)
	{
		if (block.alive && greenLaser.alive && checkCollision(greenLaser, block))
		{
			Mix_PlayChannel(-1, collisionSound, 0);
			block.alive = false;
			greenLaser.alive = false;
			p1Score += 10;
		}

		if (block.alive && blueLaser.alive && checkCollision(blueLaser, block))
		{
			Mix_PlayChannel(-1, collisionSound, 0);
			block.alive = false;
			blueLaser.alive = false;
			p2Score += 10;
		}
	}

	for (Entity& block : moreBlocks)
	{
		if (block.alive && greenLaser.alive && checkCollision(greenLaser, block))
		{
			Mix_PlayChannel(-1, collisionSound, 0);
			block.alive = false;
			greenLaser.alive = false;
			p1Score += 10;
		}

		if (block.alive && blueLaser.alive && checkCollision(blueLaser, block))
		{
			Mix_PlayChannel(-1, collisionSound, 0);
			block.alive = false;
			blueLaser.alive = false;
			p2Score += 10;
		}
	}

	if (screenShake > 0)
	{
		screenShakeValue += elapsed;
		viewMatrix.Translate(0.0f, sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity,
			0.0f);
		screenShake -= 0.3;
	}
	else
	{
		viewMatrix.identity();
		program.setViewMatrix(viewMatrix);
	}

	//check if the invaders hit the edges
	bool turnAround = false;
	for (Entity& invader : invaders)
	{
		//check if green laser collided with any invaders
		//screen will shake when laser hits an invader
		if (invader.alive && greenLaser.alive && checkCollision(greenLaser, invader))
		{
			Mix_PlayChannel(-1, collisionSound, 0);
			invader.alive = false;
			greenLaser.alive = false;
			invadersAlive--;
			p1Score += 100;
			if (invadersAlive == 0)
			{
				state = STATE_WIN;
				Mix_HaltMusic();
				Mix_PlayChannel(-1, winSound, 0);
			}
			screenShake = 15;
		}

		//check if blue laser collided with any invaders
		if (invader.alive && blueLaser.alive && checkCollision(blueLaser, invader))
		{
			Mix_PlayChannel(-1, collisionSound, 0);
			invader.alive = false;
			blueLaser.alive = false;
			invadersAlive--;
			p2Score += 100;
			if (invadersAlive == 0)
			{
				state = STATE_WIN;
				Mix_HaltMusic();
				Mix_PlayChannel(-1, winSound, 0);
			}
			screenShake = 15;
		}

		else if (invader.alive)					//if the invader is alive generate a bullet at a chance to fire down
		{
			if ((std::rand() % 100 + 1) <= 1)
			{
				//only 3 invader bullets will be on screen at once
				//invaders can shoot multiple bullets to increase difficulty at end
				if (!redLasers[redLaserIndex].alive)
				{
					Mix_PlayChannel(-1, fireSound, 0);
					redLasers[redLaserIndex].alive = true;
					redLasers[redLaserIndex].x = invader.x;
					redLasers[redLaserIndex].y = invader.y - invader.sprite.height / 2;
					redLaserIndex++;

					if (redLaserIndex > 2)
						redLaserIndex = 0;
				}
			}

			if (checkCollision(invader, player))
				player.alive = false;

			if (checkCollision(invader, player2))
				player2.alive = false;
			if (!player.alive && !player2.alive)
			{
				state = STATE_GAME_OVER;
				Mix_FadeOutMusic(4000);
				Mix_PlayChannel(-1, gameOverSound, 0);
			}
		}


		//move the invaders based on time
		invader.x += 0.3f * invader.dirX * elapsed;

		//if they reached the right or left edges change the direction
		if (invader.x >= 3.3f)
		{
			turnAround = true;
		}
		else if (invader.x <= -3.3f)
		{
			turnAround = true;
		}
	}

	//reverse directions if an edge is reached
	if (turnAround)
	{
		for (Entity& invader : invaders)
		{
			if (invaders[29].x >= 3.3f && invaders[29].dirX > 0)
				invader.dirX *= -1;
			else if (invaders[0].x <= -3.3f && invaders[0].dirX < 0)
				break;
		}

		//fixes the problem of invaders being stuck sometimes; will make code less messy at a later time
		if (invaders[0].x <= -3.3f && invaders[0].dirX < 0)
		{
			for (Entity& invader : invaders)
			{
				invader.dirX *= -1;
			}
		}
	}
}

const Uint8 *keys = SDL_GetKeyboardState(NULL);

//move player1 if left or right keys are pressed and fire if spacebar is pressed
//move player2 if A or D keys are pressed and fire if TAB key is pressed
void processInput(Entity& player, Entity& player2, Entity& greenLaser, Entity& blueLaser, float elapsed)
{
	if (keys[SDL_SCANCODE_LEFT] && player.x >= -3.3f)
	{
		player.x -= 1.2f * elapsed;
	}

	if (keys[SDL_SCANCODE_RIGHT] && player.x <= 3.3f)
	{
		player.x += 1.2f * elapsed;
	}

	if (keys[SDL_SCANCODE_A] && player2.x >= -3.3f)
	{
		player2.x -= 1.2f * elapsed;
	}

	if (keys[SDL_SCANCODE_D] && player2.x <= 3.3f)
	{
		player2.x += 1.2f * elapsed;
	}

	if (keys[SDL_SCANCODE_SPACE])
	{
		if (player.alive && !greenLaser.alive)
		{
			//set the laser to be alive aka on screen and position it on top of the player
			greenLaser.alive = true;
			greenLaser.x = player.x;
			greenLaser.y = player.y + player.sprite.height / 2;
			Mix_PlayChannel(-1, fireSound, 0);
		}
	}

	if (keys[SDL_SCANCODE_TAB])
	{
		if (player2.alive && !blueLaser.alive)
		{
			//set the laser to be alive aka on screen and position it on top of the player
			blueLaser.alive = true;
			blueLaser.x = player2.x;
			blueLaser.y = player2.y + player2.sprite.height / 2;
			Mix_PlayChannel(-1, fireSound, 0);
		}
	}
}

void renderGameOver(Matrix& model, ShaderProgram& program, GLuint& texture)
{
	model.identity();
	model.Translate(-1.4, 0.5, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "GAME OVER", 0.6, -.25);

	model.identity();
	model.Translate(-1.2, -0.5, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "Press Q to exit the game", .2, -.1);

	model.identity();
	model.Translate(-3.0, -1.5, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "P1 Score: " + std::to_string(p1Score), 0.3, -0.15);

	model.identity();
	model.Translate(0.5, -1.5, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "P2 Score: " + std::to_string(p2Score), 0.3, -0.15);
}

void renderWin(Matrix& model, ShaderProgram& program, GLuint& texture)
{
	model.identity();
	model.Translate(-1.2, 0.0, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "You win!", 0.6, -.25);

	model.identity();
	model.Translate(-1.2, -0.5, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "Press Q to exit the game", .2, -.1);

	model.identity();
	model.Translate(-3.0, -1.5, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "P1 Score: " + std::to_string(p1Score), 0.3, -0.15);

	model.identity();
	model.Translate(0.5, -1.5, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "P2 Score: " + std::to_string(p2Score), 0.3, -0.15);
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Final Project - Kenny Ng", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 540, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	menuMusic = Mix_LoadMUS("gameMusic.mp3");
	music = Mix_LoadMUS("levelMusic.mp3");
	fireSound = Mix_LoadWAV("laser.wav");
	collisionSound = Mix_LoadWAV("collision.wav");
	gameOverSound = Mix_LoadWAV("gameOver.wav");
	winSound = Mix_LoadWAV("win2.wav");

	//setting up before the loop
	glViewport(0, 0, 640, 540);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	//sprite sheet for the game sprites and for the text
	GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
	GLuint textTexture = LoadTexture(RESOURCE_FOLDER"font1.png");


	//use sprite sheet coords from XML file to load in player1_green, player1_blue, blocks, and enemyRed1 sprites, as well as lasers
	//block sprite only takes a square porion of the sprite so it has no borders
	SheetSprite playerShip(spriteSheetTexture, 237.0f / 1024.0f, 377.0f / 1024.0f, 99.0 / 1024.0f, 75.0f / 1024.0f, 0.3f);
	SheetSprite player2Ship(spriteSheetTexture, 112.0f / 1024.0f, 791.0f / 1024.0f, 112.0 / 1024.0f, 75.0f / 1024.0f, 0.3f);
	SheetSprite enemy(spriteSheetTexture, 425.0f / 1024.0f, 384.0f / 1024.0f, 93.0 / 1024.0f, 84.0f / 1024.0f, 0.3f);
	SheetSprite greenLaser(spriteSheetTexture, 843.0f / 1024.0f, 116.0f / 1024.0f, 13.0 / 1024.0f, 57.0f / 1024.0f, 0.2f);
	SheetSprite blueLaser(spriteSheetTexture, 841.0f / 1024.0f, 647.0f / 1024.0f, 13.0 / 1024.0f, 37.0f / 1024.0f, 0.2f);
	SheetSprite redLaser(spriteSheetTexture, 843.0f / 1024.0f, 977.0f / 1024.0f, 13.0 / 1024.0f, 37.0f / 1024.0f, 0.2f);
	SheetSprite block(spriteSheetTexture, 780.0f / 1024.0f, 770.0f / 1024.0f, 20.0 / 1024.0f, 20.0f / 1024.0f, 0.2f);

	Entity player(0.0f, -2.8f, 0.0f, 0.0f, playerShip, true);
	Entity player2(0.0f, -2.5f, 0.0f, 0.0f, player2Ship, true);
	std::vector<Entity> invaders;
	//make a loop to generate 30 invader entities, 6 per row
	for (float i = -1.85f; i <= 2.0f; i += 0.75f)
	{
		for (float j = 2.2f; j >= 0.4f; j -= 0.4f)
			invaders.push_back(Entity(i, j, 1.5f, 0.0f, enemy, true));
	}

	//generate 2 rows of blocks; blocks will only appear in normal and hard modes as obstacles
	//ENEMY LASERS ARE ALLOWED TO BYPASS BLOCKS TO INCREASE DIFFICULTY
	std::vector<Entity> blocks;
	for (float i = -3.4f; i < 3.5f; i+= 0.2f)
	{
		blocks.push_back(Entity(i, 0.0f, 1.0f, 0.0f, block, true));
	}

	//layer 2 of blocks only in hard mode
	std::vector<Entity> moreBlocks;
	for (float i = -3.4f; i < 3.5f; i += 0.2f)
	{
		moreBlocks.push_back(Entity(i, -1.0f, 1.0f, 0.0f, block, true));
	}

	//player only gets to have one alive bullet at a time
	//make bullet vectors for the invader lasers to be stored off screen set as not alive
	Entity laser = Entity(100.0f, 100.0f, 0.0f, 1.0f, greenLaser, false);
	Entity laser2 = Entity(100.0f, 100.0f, 0.0f, 1.0f, blueLaser, false);
	std::vector<Entity> redLasers;

	for (int i = 0; i < 3; i++)
	{
		redLasers.push_back(Entity(-100.0f - i, -100.0f - i, 0.0f, 1.0f, redLaser, false));
	}

	float lastFrameTicks = 0.0f;
	Matrix projectionMatrix;
	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -3.0f, 3.0f, -1.0f, 1.0f);
	Matrix modelMatrix;
	Matrix viewMatrix;

	state = STATE_MAIN_MENU;
	SDL_Event event;
	bool done = false;
	bool gameStart = false;

	Mix_PlayMusic(menuMusic, -1);
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(program.programID);

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		//change states between main menu and game level, display Game Over or Win if conditions are met
		switch (state)
		{
		case(STATE_MAIN_MENU) :
			renderMainMenu(modelMatrix, program, textTexture, elapsed);
			if (keys[SDL_SCANCODE_1])										//Enter 1 on the keyboard for easy mode
			{
				state = STATE_GAME_LEVEL;
				Mix_PlayMusic(music, -1);
				difficulty = EASY;

				for (Entity& invader : invaders)							//easy mode has no blocks and invaders
				{															//move with twice the speed
					invader.dirX *= 2;
				}
			}

			if (keys[SDL_SCANCODE_2])										//Enter 2 on the keyboard for normal mode
			{
				state = STATE_GAME_LEVEL;
				Mix_PlayMusic(music, -1);
				difficulty = NORMAL;
				for (Entity& invader : invaders)							//normal mode has one row of blocks and invaders
				{															//move with 3x the speed
					invader.dirX *= 3;
				}
			}

			if (keys[SDL_SCANCODE_3])										//Enter 3 on the keyboard for hard mode
			{
				state = STATE_GAME_LEVEL;
				Mix_PlayMusic(music, -1);
				difficulty = HARD;
				for (Entity& invader : invaders)							//hard mode has two row of blocks and invaders
				{															//move with 4x the speed
					invader.dirX *= 4;
				}
			}

			if (keys[SDL_SCANCODE_Q])
				done = true;											//hit Q to exit
			break;
		case(STATE_GAME_LEVEL) :
			renderGameLevel(modelMatrix, program, player, player2, invaders, laser, laser2, redLasers, blocks, moreBlocks, difficulty, textTexture);
			update(viewMatrix, program, player, player2, invaders, laser, laser2, redLasers, blocks, moreBlocks, elapsed, difficulty);
			processInput(player, player2, laser, laser2, elapsed);
			break;
		case(STATE_GAME_OVER) :
			renderGameOver(modelMatrix, program, textTexture);
			if (keys[SDL_SCANCODE_Q])										//hit Q to quit
				done = true;
			break;
		case(STATE_WIN) :
			renderWin(modelMatrix, program, textTexture);
			if (keys[SDL_SCANCODE_Q])										//hit Q to quit
				done = true;
			break;
		}

		SDL_GL_SwapWindow(displayWindow);

	}

	Mix_FreeMusic(menuMusic);
	Mix_FreeMusic(music);
	Mix_FreeChunk(fireSound);
	Mix_FreeChunk(collisionSound);
	Mix_FreeChunk(gameOverSound);
	Mix_FreeChunk(winSound);

	SDL_Quit();
	return 0;
}