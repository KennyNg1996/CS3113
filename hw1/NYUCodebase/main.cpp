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

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	//setting up before the loop
	glViewport(0, 0, 640, 360);
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint fireTexture = LoadTexture(RESOURCE_FOLDER"fire2.png");
	GLuint ringTexture = LoadTexture(RESOURCE_FOLDER"ring.png");
	GLuint sunTexture = LoadTexture(RESOURCE_FOLDER"sun.png");

    Matrix projectionMatrix;
	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	Matrix modelMatrix;
	Matrix viewMatrix;

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;

			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
			glUseProgram(program.programID);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			glBindTexture(GL_TEXTURE_2D, fireTexture);

			program.setModelMatrix(modelMatrix);

			float fireVertices[] = { 2.0f, 2.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 2.0f, 2.0f };
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, fireVertices);
			glEnableVertexAttribArray(program.positionAttribute);

			float fireTexCoords[] = { 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f };
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, fireTexCoords);
			glEnableVertexAttribArray(program.texCoordAttribute);

			glBindTexture(GL_TEXTURE_2D, fireTexture);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(program.positionAttribute);
			glDisableVertexAttribArray(program.texCoordAttribute);

			program.setModelMatrix(modelMatrix);

			float ringVertices[] = { -1.0f, -1.0f, -3.0f, -1.0f, -3.0f, -3.0f, -3.0f, -3.0f, -1.0f, -3.0f, -1.0f, -1.0f };
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ringVertices);
			glEnableVertexAttribArray(program.positionAttribute);

			float ringTexVertices[] = { 0.75f, 0.75f, -0.75f, 0.75f, -0.75f, -0.75f, -0.75f, -0.75f, 0.75f, -0.75f, 0.75f, 0.75f };
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ringTexVertices);
			glEnableVertexAttribArray(program.texCoordAttribute);
			
			glBindTexture(GL_TEXTURE_2D, ringTexture);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(program.positionAttribute);
			glDisableVertexAttribArray(program.texCoordAttribute);

			program.setModelMatrix(modelMatrix);

			float sunVertices[] = { 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f };
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, sunVertices);
			glEnableVertexAttribArray(program.positionAttribute);

			float sunTexVertices[] = { 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f };
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, sunTexVertices);
			glEnableVertexAttribArray(program.texCoordAttribute);

			glBindTexture(GL_TEXTURE_2D, sunTexture);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(program.positionAttribute);
			glDisableVertexAttribArray(program.texCoordAttribute);
		
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
