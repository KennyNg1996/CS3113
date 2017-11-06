/*


PRESS ENTER TO START GAME

MOVE AROUND WITH ARROW KEYS

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
#include <vector>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

float PI = 3.14159265358979323846;
#define FIXED_TIMESTEP 0.0166666f
int coinCount = 0;
const Uint8 *keys = SDL_GetKeyboardState(NULL);

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

enum EntityType {
	ENTITY_PLAYER, ENTITY_ENEMY,
	ENTITY_COIN, ENTITY_TILE
};

class Entity
{
public:
	float x;
	float y;
	float dirX;
	float dirY;
	float accX;
	float accY;
	SheetSprite sprite;
	bool isStatic;
	EntityType entityType;

	bool collidedTop = false;
	bool collidedBottom = false;
	bool collidedLeft = false;
	bool collidedRight = false;

	Entity(float posX, float posY, float dX, float dY, float aX, float aY, SheetSprite spr, bool stat, EntityType type) 
		: x(posX), y(posY), dirX(dX), dirY(dY), accX(aX), accY(aY), sprite(spr), isStatic(stat), entityType(type) {}

	float top()
	{
		return y + sprite.size / 2;
	}

	float bottom()
	{
		return y - sprite.size / 2;
	}

	float left()
	{
		return x - sprite.size * (sprite.width / sprite.height) / 2;
	}

	float right()
	{
		return x + sprite.size * (sprite.width / sprite.height) / 2;
	}

	void draw(Matrix& model, ShaderProgram& program)
	{
		model.identity();
		model.Translate(x, y, 0);
		program.setModelMatrix(model);
		sprite.Draw(&program);
	}
};

//display the main menu text
void renderMainMenu(Matrix& model, ShaderProgram& program, GLuint& texture)
{
	model.identity();
	model.Translate(-2.4f, 1.0f, 0.0f);
	program.setModelMatrix(model);
	DrawText(&program, texture, "Platformer Demo", 0.6f, -.25f);

	model.identity();
	model.Translate(-1.4, 0.0, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "Press ENTER to start the game", .2f, -.1f);
}

bool checkCollision(Entity& ent1, Entity& ent2)
{
	if (ent1.top() >= ent2.bottom() && ent1.bottom() <= ent2.top() && ent1.left() < ent2.right() && ent1.right() >= ent2.left())
		return true;

	return false;
}

enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER, STATE_WIN };
GameMode state;

//draw the players, tiles, and coins based on their x and y positions; follow the player as they move
void renderGameLevel(Matrix& model, Matrix& view, ShaderProgram& program, Entity& player, std::vector<Entity>& tiles, 
	std::vector<Entity>& coins, GLuint& texture)
{
	player.draw(model, program);

	for (Entity tile : tiles)
		tile.draw(model, program);

	for (Entity coin : coins)
	{
		//draw only if the coins is still dynamic; if it is static then the player has acquired it
		if (!coin.isStatic)
			coin.draw(model, program);
	}

	std::string strCoinCount = "" + coinCount;
	//draw the text that keeps track of how many coins the player has; draw to left corner of player
	//increment by player.x and player.y so that it is always on the player's screen
	model.identity();
	model.Translate(-3.35f + player.x, 1.9f + player.y, 0.0f);
	program.setModelMatrix(model);
	DrawText(&program, texture, "Coin Count: " + std::to_string(coinCount), 0.3f, -.15f);

	//enable scrolling
	view.identity();
	view.Translate(-player.x, -player.y, 0.0f);
	program.setViewMatrix(view);
}

//update positions of the entities and check for any collisions between entities
void update(Entity& player, std::vector<Entity>& tiles, std::vector<Entity>& coins, float elapsed)
{
	//reset collision variables before updating
	player.collidedTop = false;
	player.collidedBottom = false;
	player.collidedLeft = false;
	player.collidedRight = false;

	//update player's y position first, if they are in midair or colliding with the tiles/coins
	player.dirY += player.accY * elapsed;
	player.y += player.dirY * elapsed;

	for (Entity& tile : tiles)
	{
		if (checkCollision(player, tile))
		{
			//if they collided, calculate their penetration and move accordingly

			//if the player is above the tile, push the player up
			if (player.top() > tile.top())
			{
				player.y += fabs(tile.top() - player.bottom())  + 0.0001f;
				player.collidedBottom = true;
				player.dirY = 0.0f;
			}

			//if the player is below the tile push the player down
			else if (player.bottom() < tile.bottom())
			{
				player.y -= fabs(player.top() - tile.bottom()) + 0.0001f;
				player.collidedTop = true;
				player.dirY = 0.0f;
			}
		}
	}

	//update player's x position next 
	player.dirX += player.accX * elapsed;
	player.x += player.dirX * elapsed;

	for (Entity& tile : tiles)
	{
		if (checkCollision(player, tile))
		{
			//if they collided, calculate their penetration and move accordingly]

			//if the player collided with a tile to the left, push the player to the right
			if (player.left() > tile.left())
			{
				player.x += fabs(tile.right() - player.left()) + 0.0001f;
				player.collidedLeft = true;
				player.dirX = 0.0f;
			}

			//if the player collided with a tile to the right, push the player to the left
			else if (player.right() < tile.right())
			{
				player.x -= fabs(player.right() - tile.left()) + 0.0001f;
				player.collidedRight = true;
				player.dirX = 0.0f;
			}
		}
	}

	for (Entity& coin : coins)
	{
		//set the coin to static if player touches the coin aka removing it 
		if (checkCollision(player, coin) && !coin.isStatic)
		{
			coin.isStatic = true;
			coinCount++;
		}
	}
}

//move the player according to the arrow keys
void processInput(Entity& player, float elapsed)
{
	if (keys[SDL_SCANCODE_LEFT])
	{
		player.dirX = -1.5f;
	}
	else if (keys[SDL_SCANCODE_RIGHT])
	{
		player.dirX = 1.5f;
	}
	else
		player.dirX = 0.0f;

	//jump if the player is on the ground
	if (keys[SDL_SCANCODE_UP] && player.collidedBottom)
	{
		player.dirY = 1.5f;
	}
}

/*void renderGameOver(Matrix& model, ShaderProgram& program, GLuint& texture)
{
	model.identity();
	model.Translate(-1.2, 0.0, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "GAME OVER", 0.6, -.25);
}

void renderWin(Matrix& model, ShaderProgram& program, GLuint& texture)
{
	model.identity();
	model.Translate(-1.0, 0.0, 0);
	program.setModelMatrix(model);
	DrawText(&program, texture, "You win!", 0.6, -.25);
}*/

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("HW4 - Kenny Ng", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	//setting up before the loop
	glViewport(0, 0, 640, 360);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	//sprites for the game and the text
	GLuint playerSheetSprite = LoadTexture(RESOURCE_FOLDER"p2_front.png");
	GLuint tileSprite = LoadTexture(RESOURCE_FOLDER"grass.png");
	GLuint tile2Sprite = LoadTexture(RESOURCE_FOLDER"grassCenter.png");
	GLuint coinSprite = LoadTexture(RESOURCE_FOLDER"coinGold.png");
	GLuint textTexture = LoadTexture(RESOURCE_FOLDER"font1.png");


	//use the full sprite images for the entities
	SheetSprite playerSprite(playerSheetSprite, 0.0f, 0.0f, 1.0f, 1.0f, 0.3f);
	SheetSprite tile(tileSprite, 0.0f, 0.0f, 1.0f, 1.0f, 0.3f);
	SheetSprite tile2(tile2Sprite, 0.0f, 0.0f, 1.0f, 1.0f, 0.3f);
	SheetSprite coin(coinSprite, 0.0f, 0.0f, 1.0f, 1.0f, 0.2f);

	//constant y acceleration
	float gravity = -2.5f;

	Entity player(-2.0f, 0.0f, 0.0f, 0.0f, 0.0f, gravity, playerSprite, false, ENTITY_PLAYER);

	//create tiles for the level

	//draw the bottom tiles
	std::vector<Entity> tiles;
	std::vector<Entity> coins;

	for (int i = 0; i < 50; i++)
	{
		tiles.push_back(Entity(-5.5f + 0.3f * i, -0.3f, 0.0f, 0.0f, 0.0f, 0.0f, tile, true, ENTITY_TILE));
	}

	//draw the left most tiles
	for (int i = 0; i < 7; i++)
	{
		if (i < 6)
			tiles.push_back(Entity(-5.5f, -0.3f + 0.3f * i, 0.0f, 0.0f, 0.0f, 0.0f, tile2, true, ENTITY_TILE));
		else
			tiles.push_back(Entity(-5.5f, -0.3f + 0.3f * i, 0.0f, 0.0f, 0.0f, 0.0f, tile, true, ENTITY_TILE));
	}

	//draw the right most tiles
	for (int i = 0; i < 7; i++)
	{
		if (i < 6)
			tiles.push_back(Entity(-5.5f + 0.3f * 49, -0.3f + 0.3f * i, 0.0f, 0.0f, 0.0f, 0.0f, tile2, true, ENTITY_TILE));
		else
			tiles.push_back(Entity(-5.5f + 0.3f * 49, -0.3f + 0.3f * i, 0.0f, 0.0f, 0.0f, 0.0f, tile, true, ENTITY_TILE));
	}

	//draw the coins
	for (int i = 0; i < 9; i++)
	{
		coins.push_back(Entity(0.0f + 1.0f * i, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, coin, false, ENTITY_COIN));
	}

	float lastFrameTicks = 0.0f;
	Matrix projectionMatrix;
	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	Matrix modelMatrix;
	Matrix viewMatrix;

	state = STATE_MAIN_MENU;
	SDL_Event event;
	bool done = false;
	bool gameStart = false;
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

		//change states between main menu and game level
		switch (state)
		{
			case(STATE_MAIN_MENU) :
				renderMainMenu(modelMatrix, program, textTexture);
				if (keys[SDL_SCANCODE_RETURN])	{ state = STATE_GAME_LEVEL; }			//Enter key on the keyboard
				break;
			case(STATE_GAME_LEVEL) :
				renderGameLevel(modelMatrix, viewMatrix, program, player, tiles, coins, textTexture);
				update(player, tiles, coins, elapsed);
				processInput(player, elapsed);
				break;

			/*case(STATE_GAME_OVER) :
				renderGameOver(modelMatrix, program, textTexture);
				break;
			case(STATE_WIN) :
				renderWin(modelMatrix, program, textTexture);
				break;*/
		}

		SDL_GL_SwapWindow(displayWindow);
		
	}

	SDL_Quit();
	return 0;
}