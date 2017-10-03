/*


START GAME BY PRESSING SPACEBAR

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

//create an Entity for Ball
class Ball
{
public:
	float x;						//position on the x axis
	float y;						//position on the y axis
	float dirX;						//direction its facing on the x axis
	float dirY;						//direction its facing on the y axis

	//constructor for ball
	Ball(float positionX, float positionY, float dX, float dY)
		: x(positionX), y(positionY), dirX(dX), dirY(dY) {}

	//moves the ball according to how much time has elapsed
	void moveBall(float timeElapsed)
	{
		x += cos(dirX) * 0.8f * timeElapsed;
		y += sin(dirY) * 0.8f * timeElapsed;
	}
};

//create an Entity for the Paddles
class Paddle
{
public:
	//establish the 4 points of the Paddle rectangle
	float top;
	float bottom;
	float left;
	float right;

	Paddle(float t, float b, float l, float r) : top(t), bottom(b), left(l), right(r) {}
};

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("HW2 - Kenny Ng", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	//setting up before the loop
	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint pongTexture = LoadTexture(RESOURCE_FOLDER"pong.png");
	GLuint leftTexture = LoadTexture(RESOURCE_FOLDER"leftwins.png");
	GLuint rightTexture = LoadTexture(RESOURCE_FOLDER"rightwins.png");

	float lastFrameTicks = 0.0f;
	Matrix projectionMatrix;
	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	Matrix modelMatrix;
	Matrix viewMatrix;

	Paddle leftPaddle(0.7f, -0.7f, -3.4f, -3.3f);
	Paddle rightPaddle(0.7f, -0.7f, 3.3f, 3.4f);
	Ball ball(0.0f, 0.0f, PI/6, PI/3); 

	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	SDL_Event event;
	bool done = false;
	bool gameStart = false;
	bool leftWin = false;
	bool rightWin = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

			float ticks = (float)SDL_GetTicks() / 1000.0f;
			float elapsed = ticks - lastFrameTicks;
			lastFrameTicks = ticks;

			//check if key is pressed
			//left paddle uses W and S keys to move
			if (keys[SDL_SCANCODE_W])
			{
				//only allow it to move up if it isn't already at the top
				if (leftPaddle.top < 2.0f)
				{
					leftPaddle.top += 0.5f * elapsed;
					leftPaddle.bottom += 0.5f * elapsed;
				}
			}
			if (keys[SDL_SCANCODE_S])
			{
				//only allow it to move down if it isn't already at the bottom
				if (leftPaddle.bottom > -2.0f)
				{
					leftPaddle.top -= 0.5f * elapsed;
					leftPaddle.bottom -= 0.5f * elapsed;
				}
			}

			//right paddle uses up and down keys to move
			if (keys[SDL_SCANCODE_UP])
			{
				//only allow it to move up if it isn't already at the top
				if (rightPaddle.top < 2.0f)
				{
					rightPaddle.top += 0.5f * elapsed;
					rightPaddle.bottom += 0.5f * elapsed;
				}
			}
			if (keys[SDL_SCANCODE_DOWN])
			{
				//only allow it to move down if it isn't already at the bottom
				if (rightPaddle.bottom > -2.0)
				{
					rightPaddle.top -= 0.5f * elapsed;
					rightPaddle.bottom -= 0.5f * elapsed;
				}
			}

			//start the game if space is pressed
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE)
			{
				if (!gameStart)
					gameStart = true;
			}

				//start game if space is pressed
				if (gameStart)
				{
					//check if left won by comparing the right paddle's right side to the ball's x position
					if (ball.x - 0.08f > rightPaddle.right)
					{
						//reset game
						ball.x = 0.0f;
						ball.y = 0.0f;
						ball.dirX = PI/6;
						ball.dirY = PI/3;
						leftWin = true;
						gameStart = false;
					}

					//check if right won by comparing the left paddle's left side to the ball's x position
					else if (ball.x + 0.08f < leftPaddle.left)
					{
						//reset game
						ball.x = 0.0f;
						ball.y = 0.0f;
						ball.dirX = PI / 6;
						ball.dirY = PI / 3;
						rightWin = true;
						gameStart = false;
					}

					//check for left or right paddle collision
					else if ((ball.x - 0.08f <= leftPaddle.right && ball.y - 0.08f <= leftPaddle.top && ball.y + 0.08f >= leftPaddle.bottom)
						|| (ball.x + 0.08f >= rightPaddle.left && ball.y - 0.08f <= rightPaddle.top && ball.y + 0.08f >= rightPaddle.bottom))
					{
						ball.dirX = ball.dirX * + PI;
						ball.moveBall(elapsed);
					}

					//check if ball hit the top or bottom wall and reverse if it did
					else if (ball.y + 0.08f >= 2.0f || ball.y - 0.08f <= -2.0f)
					{
						ball.dirY = ball.dirY * -1;
						ball.moveBall(elapsed);
					}

					//if none of the other ifs have been reached, ball is not colliding with anything
					else
					{
						ball.moveBall(elapsed);
					}
				}

				glClear(GL_COLOR_BUFFER_BIT);
				glUseProgram(program.programID);

				//left is considered Player 1
				if (leftWin)
				{
					modelMatrix.identity();
					program.setModelMatrix(modelMatrix);

					float leftWinVertices[] = { -2.0, -2.0, 2.0, -2.0, 2.0, 2.0, -2.0, -2.0, 2.0, 2.0, -2.0, 2.0 };
					glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, leftWinVertices);
					glEnableVertexAttribArray(program.positionAttribute);
					float leftWinCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
					glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, leftWinCoords);
					glEnableVertexAttribArray(program.texCoordAttribute);
					glBindTexture(GL_TEXTURE_2D, leftTexture);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					glDisableVertexAttribArray(program.positionAttribute);
					glDisableVertexAttribArray(program.texCoordAttribute);
				}

				//right is considered Player 2
				else if (rightWin)
				{
					modelMatrix.identity();
					program.setModelMatrix(modelMatrix);

					float rightWinVertices[] = { -1.0, -1.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0 };
					glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, rightWinVertices);
					glEnableVertexAttribArray(program.positionAttribute);
					float rightWinCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
					glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, rightWinCoords);
					glEnableVertexAttribArray(program.texCoordAttribute);
					glBindTexture(GL_TEXTURE_2D, rightTexture);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					glDisableVertexAttribArray(program.positionAttribute);
					glDisableVertexAttribArray(program.texCoordAttribute);
				}
				else
				{
					//start drawing paddles and ball
					modelMatrix.identity();
					modelMatrix.Translate(leftPaddle.left, (leftPaddle.top + leftPaddle.bottom) / 2, 0.0f);
					program.setModelMatrix(modelMatrix);
					program.setProjectionMatrix(projectionMatrix);
					program.setViewMatrix(viewMatrix);

					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

					float texCoords[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f };

					float leftPaddleVertices[] = { -0.05f, -0.7f, 0.05f, -0.7f, 0.05f, 0.7f, -0.05f, -0.7f, 0.05f, 0.7f, -0.05f, 0.7f };
					glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, leftPaddleVertices);
					glEnableVertexAttribArray(program.positionAttribute);
					glDrawArrays(GL_TRIANGLES, 0, 6);

					glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
					glEnableVertexAttribArray(program.texCoordAttribute);
					glBindTexture(GL_TEXTURE_2D, pongTexture);

					glDisableVertexAttribArray(program.positionAttribute);
					glDisableVertexAttribArray(program.texCoordAttribute);

					modelMatrix.identity();
					modelMatrix.Translate(rightPaddle.right, (rightPaddle.top + rightPaddle.bottom) / 2, 0.0f);
					program.setModelMatrix(modelMatrix);

					float rightPaddleVertices[] = { -0.05f, -0.7f, 0.05f, -0.7f, 0.05f, 0.7f, -0.05f, -0.7f, 0.05f, 0.7f, -0.05f, 0.7f };
					glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, rightPaddleVertices);
					glEnableVertexAttribArray(program.positionAttribute);
					glDrawArrays(GL_TRIANGLES, 0, 6);

					glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
					glEnableVertexAttribArray(program.texCoordAttribute);
					glBindTexture(GL_TEXTURE_2D, pongTexture);

					glDisableVertexAttribArray(program.positionAttribute);
					glDisableVertexAttribArray(program.texCoordAttribute);

					modelMatrix.identity();
					modelMatrix.Translate(ball.x, ball.y, 0.0f);
					program.setModelMatrix(modelMatrix);
					float ballVertices[] = { -0.08f, -0.08f, 0.08f, -0.08f, 0.08f, 0.08f, 0.08f, 0.08f, -0.08f, 0.08f, -0.08f, -0.08f };
					glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballVertices);
					glEnableVertexAttribArray(program.positionAttribute);
					glDrawArrays(GL_TRIANGLES, 0, 6);

					glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
					glEnableVertexAttribArray(program.texCoordAttribute);
					glBindTexture(GL_TEXTURE_2D, pongTexture);

					glDisableVertexAttribArray(program.positionAttribute);
					glDisableVertexAttribArray(program.texCoordAttribute);
				}

				SDL_GL_SwapWindow(displayWindow);
		
	}

	SDL_Quit();
	return 0;
}