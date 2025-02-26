#include "TextureDebugShader.h"
#include "World.h"

const GLfloat TextureDebugShader::g_quad_vertex_buffer_data[] = {
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
};

TextureDebugShader::TextureDebugShader() : Shader("TextureDebug.vertexshader", "TextureDebug.fragmentshader")
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

TextureDebugShader::~TextureDebugShader()
{
}

void TextureDebugShader::draw(glm::mat4 & view_mat, GameEntity* entity)
{	
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, entity->mesh->getMeshes().at(0)->textures.at(0).id);
	// Set our "renderedTexture" sampler to user Texture Unit 0
	glUniform1i(getUniformHandle("texture"), 0);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(getAttributeHandle("vertexPosition_modelspace"));
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glVertexAttribPointer(
		getAttributeHandle("vertexPosition_modelspace"), // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// Draw the triangles !
	// You have to disable GL_COMPARE_R_TO_TEXTURE above in order to see anything !
	glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
	glDisableVertexAttribArray(getAttributeHandle("vertexPosition_modelspace"));
}

/*
Draws a specified texture starting at the initial location and expanding the width to the right and height upwards.
*/
void TextureDebugShader::drawTextureCentered(GLuint textureId, int xloc, int yloc, int width, int height, float percent, const glm::vec4 &color)
{
	glUseProgram(getProgramID());

	glViewport(xloc - width / 2, yloc - height / 2, width, height);

	if (DEBUG_MODE)
		check_gl_error("TextureDebugShader error part");

	glBindVertexArray(VAO);

	if (DEBUG_MODE)
		check_gl_error("TextureDebugShader error part 2");

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	if (DEBUG_MODE)
		check_gl_error("TextureDebugShader error part 2.5555");

	glBindTexture(GL_TEXTURE_2D, textureId);

	if (DEBUG_MODE)
		check_gl_error("TextureDebugShader error part 3");

	// Set our "renderedTexture" sampler to user Texture Unit 0
	glUniform1i(getUniformHandle("uTexture"), 0);
	glUniform1f(getUniformHandle("percent"), percent);
	//glUniform4fv(getUniformHandle("uColor"), 0, glm::value_ptr(color));

	if (DEBUG_MODE)
		check_gl_error("TextureDebugShader error before");

	// Draw the triangles !
	// You have to disable GL_COMPARE_R_TO_TEXTURE above in order to see anything !
	glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
	glViewport(0, 0, screen_width, screen_height);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	if (DEBUG_MODE)
		check_gl_error("TextureDebugShader error after");
}

/*
	Draws a specified texture starting at the initial location and expanding the width to the right and height upwards.
 */
void TextureDebugShader::drawTexture(GLuint textureId, int xloc, int yloc, int width, int height, float percent, const glm::vec4 &color)
{
	glUseProgram(getProgramID());
	
	glViewport(xloc, yloc, width, height);

	if (DEBUG_MODE)
		check_gl_error("TextureDebugShader error part");

	glBindVertexArray(VAO);

	if (DEBUG_MODE)
		check_gl_error("TextureDebugShader error part 2");

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	if (DEBUG_MODE)
		check_gl_error("TextureDebugShader error part 2.5555");

	glBindTexture(GL_TEXTURE_2D, textureId);

	if (DEBUG_MODE)
		check_gl_error("TextureDebugShader error part 3");

	// Set our "renderedTexture" sampler to user Texture Unit 0
	glUniform1i(getUniformHandle("uTexture"), 0);
	glUniform1f(getUniformHandle("percent"), percent);
	//glUniform4fv(getUniformHandle("uColor"), GL_FALSE, glm::value_ptr(color));

	if (DEBUG_MODE)
		check_gl_error("TextureDebugShader error before");

	// Draw the triangles !
	// You have to disable GL_COMPARE_R_TO_TEXTURE above in order to see anything !
	glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
	glViewport(0, 0, screen_width, screen_height);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	if (DEBUG_MODE)
		check_gl_error("TextureDebugShader error after");
}

void TextureDebugShader::drawTexture(GLuint textureId, int xloc, int yloc, int width, int height, const glm::vec4 &color)
{
	drawTexture(textureId, xloc, yloc, width, height, 1.0f, color);
}

/*
	Draws a specified texture at 0, 0 with the specified width and height.
*/
void TextureDebugShader::drawTexture(GLuint textureId, int width, int height, const glm::vec4 &color)
{
	drawTexture(textureId, 0, 0, width, height, color);
}
