#define __STDC_WANT_LIB_EXT1__ 1
#define SDL_MAIN_HANDLED
#define GLEW_STATIC
#include <SDL.h>
#include "GL/glew.h"
#include <SDL_opengl.h>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/matrix_transform.hpp"
// #include <gl\glu.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <stddef.h>
#include <functional>
#include <assert.h>
#include <vector>
#include <memory.h>
//#include <windows.h>

#include "unix_stub.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#ifndef __WIN32__
#define sprintf_s sprintf
#endif

#ifdef __WIN32__
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "../../../GLOV/3rdparty/libs/SDL2Win32Release.lib")
#pragma comment(lib, "../../../GLOV/3rdparty/libs/glewWin32.lib")
#endif

SDL_Window *screen;
Uint32 frame_timestamp;

static bool glCheckError(const char *file, int line)
{
	int ret = glGetError();
	if (ret) {
		printf("GL Error in file %s on line %d: %d (", file, line, ret);
		if (ret == GL_INVALID_ENUM) {
			printf("GL_INVALID_ENUM ");
		}
		if (ret == GL_INVALID_VALUE) {
			printf("GL_INVALID_VALUE ");
		}
		if (ret == GL_INVALID_OPERATION) {
			printf("GL_INVALID_OPERATION ");
		}
		if (ret == GL_STACK_OVERFLOW) {
			printf("GL_STACK_OVERFLOW ");
		}
		if (ret == GL_STACK_UNDERFLOW) {
			printf("GL_STACK_UNDERFLOW ");
		}
		if (ret == GL_OUT_OF_MEMORY) {
			printf("GL_OUT_OF_MEMORY ");
		}
		printf(")\n");

		return false;
	}
	return true;
}

#define checkError() glCheckError(__FILE__, __LINE__)


char *fload(const char *filename)
{
	FILE *fl;
	if (fopen_s(&fl, filename, "rb"))
		return NULL;
	fseek(fl, 0, SEEK_END);
	int sz = ftell(fl);
	fseek(fl, 0, SEEK_SET);
	char *ret = (char*)malloc(sz + 1);
	int bytes_read = fread(ret, sizeof(char), sz, fl);
	ret[bytes_read] = 0;
	fclose(fl);
	return ret;
}

void printProgramLog(GLuint program)
{
	if (!glIsProgram(program)) {
		printf("Name %d is not a program\n", program);
		return;
	}
	int buf_size = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &buf_size);

	int msg_len = 0;
	char* buf = new char[buf_size];
	glGetProgramInfoLog(program, buf_size, &msg_len, buf);
	if (msg_len > 0)
		printf("%s\n", buf);
	delete[] buf;
}

void printShaderLog(GLuint shader)
{
	if (!glIsShader(shader)) {
		printf("Name %d is not a shader\n", shader);
		return;
	}

	int buf_size = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &buf_size);
	char* buf = new char[buf_size];
	int msg_len = 0;
	glGetShaderInfoLog(shader, buf_size, &msg_len, buf);
	if (msg_len > 0)
		printf("%s\n", buf);
	delete[] buf;
}

bool compileShader(int handle, const char *source)
{
	//Set vertex source
	const GLchar* source_array[] = { source };
	glShaderSource(handle, 1, source_array, NULL);
	checkError();

	//Compile vertex source
	glCompileShader(handle);
	checkError();

	//Check vertex shader for errors
	GLint res = GL_FALSE;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &res);
	if (res != GL_TRUE)
	{
		printf("Unable to compile shader %d!\n", handle);
		printShaderLog(handle);
		return false;
	}
	checkError();
	return true;
}

GLuint loadProgram(const char *vert_path, const char *frag_path, std::function<void(GLuint prog_id)> binding)
{
	checkError();
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	if (!compileShader(vertex_shader, fload(vert_path)))
		return 0;

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	if (!compileShader(fragment_shader, fload(frag_path)))
		return false;

	GLuint prog_id = glCreateProgram();

	binding(prog_id);

	glAttachShader(prog_id, vertex_shader);
	glAttachShader(prog_id, fragment_shader);
	glLinkProgram(prog_id);

	//Check for errors
	GLint res = GL_TRUE;
	glGetProgramiv(prog_id, GL_LINK_STATUS, &res);
	if (res != GL_TRUE)
	{
		printf("Error linking program %d!\n", prog_id);
		printProgramLog(prog_id);
		return 0;
	}
	checkError();
	return prog_id;
}

GLuint test_prog_id;
int loc_mvp;
int loc_mv_inv_trans;
GLuint sprite_prog_id;
int sprite_screen_transform;
int sprite_tex0;
bool loadShaders()
{
	checkError();
	test_prog_id = loadProgram("data/test.vert", "data/test.frag", [](GLuint prog_id){
		glBindAttribLocation(prog_id, 0, "vpos");
		glBindAttribLocation(prog_id, 1, "vuvs");
		glBindAttribLocation(prog_id, 2, "vcolor");
		glBindAttribLocation(prog_id, 3, "vnormal");
	});
	if (!test_prog_id)
		return false;
	loc_mvp = glGetUniformLocation(test_prog_id, "mvp");
	loc_mv_inv_trans = glGetUniformLocation(test_prog_id, "mv_inv_trans");
	glUseProgram(test_prog_id);
	glUniform1i(glGetUniformLocation(test_prog_id, "tex0"), 0);
	glUseProgram(0);

	sprite_prog_id = loadProgram("data/sprite.vert", "data/sprite.frag", [](GLuint prog_id){
		glBindAttribLocation(prog_id, 0, "vpos");
		glBindAttribLocation(prog_id, 1, "vuvs");
		glBindAttribLocation(prog_id, 2, "vcolor");
	});
	if (!sprite_prog_id)
		return false;
	sprite_screen_transform = glGetUniformLocation(sprite_prog_id, "screen_transform");
	glUseProgram(sprite_prog_id);
	glUniform1i(glGetUniformLocation(sprite_prog_id, "tex0"), 0);
	glUseProgram(0);

	return true;
}

unsigned int vertex_array_object_ids[1];
bool loadModel()
{
	static const GLfloat cube[] = {
		// Model space is LH and requires Counter-clockwise winding
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,

		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f
	};

	static const GLfloat cube_uvs[] = {
		0, 0,
		0, 1,
		1, 1,
		0, 0,
		1, 1,
		1, 0,

		1, 1,
		0, 0,
		1, 0,
		0, 0,
		1, 1,
		0, 1,

		1, 1,
		0, 1,
		0, 0,
		1, 1,
		0, 0,
		1, 0,

		1, 1,
		1, 0,
		0, 0,
		1, 1,
		0, 0,
		0, 1,

		1, 1,
		0, 0,
		0, 1,
		1, 1,
		1, 0,
		0, 0,

		0, 1,
		0, 0,
		1, 0,
		1, 1,
		0, 1,
		1, 0, 
	};
	static const GLfloat cube_normals[] = {
		-1, 0, 0,
		-1, 0, 0,
		-1, 0, 0,
		-1, 0, 0,
		-1, 0, 0,
		-1, 0, 0,

		1, 0, 0,
		1, 0, 0,
		1, 0, 0,
		1, 0, 0,
		1, 0, 0,
		1, 0, 0,

		0, -1, 0,
		0, -1, 0,
		0, -1, 0,
		0, -1, 0,
		0, -1, 0,
		0, -1, 0,

		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,

		0, 0, -1,
		0, 0, -1,
		0, 0, -1,
		0, 0, -1,
		0, 0, -1,
		0, 0, -1,

		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
	};


	// Allocate Vertex Array Objects
	assert(glGenVertexArrays);
	glGenVertexArrays(1, &vertex_array_object_ids[0]);
	// Setup first Vertex Array Object
	assert(glBindVertexArray);
	glBindVertexArray(vertex_array_object_ids[0]);
	unsigned int vertex_buffer_object_ids[4];
	assert(glGenBuffers);
	glGenBuffers(ARRAY_SIZE(vertex_buffer_object_ids), vertex_buffer_object_ids);

	// VBO for vertex data
	int idx = 0;
	assert(glBindBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_ids[idx]);
	assert(glBufferData);
	glBufferData(GL_ARRAY_BUFFER, ARRAY_SIZE(cube) * sizeof(GLfloat), cube, GL_STATIC_DRAW);
	assert(glVertexAttribPointer);
	glVertexAttribPointer(idx, 3, GL_FLOAT, GL_FALSE, 0, 0);
	assert(glEnableVertexAttribArray);
	glEnableVertexAttribArray(idx);

	// VBO for texture coordinates
	++idx;
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_ids[idx]);
	glBufferData(GL_ARRAY_BUFFER, ARRAY_SIZE(cube_uvs) * sizeof(GLfloat), cube_uvs, GL_STATIC_DRAW);
	glVertexAttribPointer(idx, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(idx);

	// Color
	++idx;

	// VBO for normal data
	++idx;
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_ids[idx]);
	glBufferData(GL_ARRAY_BUFFER, ARRAY_SIZE(cube_normals) * sizeof(GLfloat), cube_normals, GL_STATIC_DRAW);
	glVertexAttribPointer(idx, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(idx);

	glBindVertexArray(0);
	checkError();
	return true;
}

static unsigned int textures[2];
bool loadTextures()
{
	glGenTextures(ARRAY_SIZE(textures), textures);
	unsigned char *pixel_data = new unsigned char[512 * 512 * 4];

	// 16x16 random
	int idx = 0;
	for (int ii = 0; ii < 16; ii++) {
		for (int jj = 0; jj < 16 ; jj++) {
			pixel_data[idx++] = (unsigned char)(rand() * 256LL / RAND_MAX);
			pixel_data[idx++] = (unsigned char)(rand() * 256LL / RAND_MAX);
			pixel_data[idx++] = (unsigned char)(rand() * 256LL / RAND_MAX);
			pixel_data[idx++] = 255;
		}
	}
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// 512x512 high frequency, mipmaps, clamping
	idx = 0;
	int res = 256;
	for (int ii = 0; ii < res; ii++) {
		float ypos = ii / (float)(res - 1);
		for (int jj = 0; jj < res; jj++) {
			float xpos = jj / (float)(res - 1);
			float d = sqrtf((xpos - 0.5f) * (xpos - 0.5f) + (ypos - 0.5f) * (ypos - 0.5f));
			d *= d * d;
			Uint8 v = (Uint8)(127 + sin(d * 500) * 127);
			pixel_data[idx++] = (xpos < 0.5) ? 255 - v : 0;
			pixel_data[idx++] = (xpos < 0.5) ? 255 - v : v;
			pixel_data[idx++] = (xpos < 0.5) ? 255 - v : v;
			pixel_data[idx++] = 255;
		}
	}
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res, res, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	checkError();
	glGenerateMipmap(GL_TEXTURE_2D);
	checkError();

	glBindTexture(GL_TEXTURE_2D, 0);
	checkError();
	delete []pixel_data;
	return true;
}

static int screen_w = 1024, screen_h = 768;

unsigned int colorSwizzle(unsigned int c)
{
	// done in spriteListQueue() currently in Glov
	return (c << 24) | ((c & 0xFF00) << 8) | ((c & 0xFF0000) >> 8) | (c >> 24);
}

// Stub out Glov stuff
#define AUTOTIMER_START_FUNC()
#define AUTOTIMER_START(foo)
#define g_sprite_draw 1
#define glovGLCheckError checkError
#define gl31DrawModeSet(foo)
#define gl31TextureSetList(texture_id) glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texture_id);
#define gl31ShaderSet(foo) glUseProgram(spe->state.shader); glUniform4f(sprite_screen_transform, 2.f / screen_w, 2.f / screen_h, 0, 0)
#define setPixelShaderParameter(foo, bar)

typedef struct SpritePoint {
	float x, y;
	float u, v;
	unsigned int color;
} SpritePoint;

typedef float Vec4[4];

typedef enum GeoMode {
	SPRITEGEOMODE_QUADS,
	SPRITEGEOMODE_TRIANGLES,
} GeoMode;

typedef struct SpriteParameters
{
	float alphaRef;
	Vec4 params[4];
	//DrawMode draw_mode;
	GeoMode geo_mode;
} SpriteParameters;

typedef struct SpriteListEntryState
{
	int shader;
	int textures;
	SpriteParameters params;
} SpriteListEntryState;

typedef void(*VoidCallback)(void *userdata);

typedef struct SpriteListEntry
{
	VoidCallback callback;
	//float z;
	//float sort_y;
	//U32 sort_hint;
	void *userdata;
	int pass;
	SpriteListEntryState state;
	int num_points;
	SpritePoint points[1];
} SpriteListEntry;

std::vector<SpriteListEntry*> sprites;
struct {
	std::vector<SpriteListEntry*> &sprites;
} glov_state = { sprites };

//////////////////////////////////////////////////////////////////////////
// Copy to Glov start - gl31sprites

void bindSpriteIndexBuffer(int num_quads, int *num_indexed_points)
{
	*num_indexed_points = num_quads * 6;
#define INIT_SPRITE_INDEX_SIZE 200
	static unsigned int ibo;
	static int max_len = 0;
	if (num_quads > max_len && ibo) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind so it can be deleted
		glDeleteBuffers(1, &ibo);
		ibo = 0;
	}
	if (!ibo) {
		max_len = MAX(INIT_SPRITE_INDEX_SIZE, MAX(num_quads, (int)(max_len * 1.5)));
		assert(max_len * 4 <= 65536); // must fit in USHORT!
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		int byte_size = max_len * sizeof(short) * 6;
		unsigned short *buf = (unsigned short*)malloc(byte_size);
		for (int ii = 0; ii < max_len; ii++) {
			buf[ii * 6 + 0] = ii * 4 + 0;
			buf[ii * 6 + 1] = ii * 4 + 3;
			buf[ii * 6 + 2] = ii * 4 + 1;
			buf[ii * 6 + 3] = ii * 4 + 1;
			buf[ii * 6 + 4] = ii * 4 + 3;
			buf[ii * 6 + 5] = ii * 4 + 2;
		}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, byte_size, buf, GL_STATIC_DRAW);
		free(buf);
	} else {
		// PERF: Could maybe avoid this binding if it's already bound, however it's bound to the VAO, not globally,
		//   so the state tracking is slightly finicky.
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	}
	glovGLCheckError();
}

static GLuint vao_buffer[100];
static GLuint vbo_buffer[ARRAY_SIZE(vao_buffer)];
static bool mid_draw;
static int sprite_buffer_index;
static SpritePoint *sprite_points = NULL;
static int sprite_points_count;
static int sprite_points_size = 0;
static SpriteListEntry *last_spe;

static void drawFinish() {
	if (mid_draw) {
		AUTOTIMER_START(drawFinish);
		// TODO: Try other methods here, compare perf (reused single buffer, disposable buffers)
		/* circular use of buffers */
		sprite_buffer_index = (sprite_buffer_index + 1) % ARRAY_SIZE(vao_buffer);

		glBindVertexArray(vao_buffer[sprite_buffer_index]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_buffer[sprite_buffer_index]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(SpritePoint) * sprite_points_count, sprite_points, GL_STREAM_DRAW);
		// Need to re-bind the buffer to the vertex array each frame
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SpritePoint), (GLvoid*)offsetof(SpritePoint, x));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpritePoint), (GLvoid*)offsetof(SpritePoint, u));
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(SpritePoint), (GLvoid*)offsetof(SpritePoint, color));

		if (last_spe->state.params.geo_mode == SPRITEGEOMODE_QUADS) {
			// bind index buffer to this array
			int num_indexed_points;
			bindSpriteIndexBuffer(sprite_points_count / 4, &num_indexed_points);
			glovGLCheckError();

			glDrawElements(GL_TRIANGLES, num_indexed_points, GL_UNSIGNED_SHORT, NULL);
		} else {
			glDrawArrays(GL_TRIANGLES, 0, sprite_points_count);
		}
		sprite_points_count = 0;
		glovGLCheckError();
		mid_draw = false;
		last_spe = NULL;
	}
}


void gl31SpritesDrawRange(int i, int count, int pass)
{
	AUTOTIMER_START_FUNC();

	if (!g_sprite_draw)
		return;

	sprite_points_count = 0;

	mid_draw = false;
	last_spe = NULL;

	if (!vao_buffer[0]) {
		// Generate Buffers and VertexArrays
		glGenBuffers(ARRAY_SIZE(vbo_buffer), vbo_buffer);
		glGenVertexArrays(ARRAY_SIZE(vao_buffer), vao_buffer);
		// Set up initial state on all VAOs
		for (int ii = 0; ii < ARRAY_SIZE(vao_buffer); ii++) {
			glBindVertexArray(vao_buffer[ii]);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
		}
	}



	sprite_points_count = 0;

#define fit(size) \
	if ((size) > sprite_points_size) { \
		sprite_points_size = (size)*3/2; \
		sprite_points = (SpritePoint*)realloc(sprite_points, sprite_points_size * sizeof(sprite_points[0])); \
					}

	fit(count * 4);

	for (int j = 0; j < count; j++) {
		SpriteListEntry *spe = glov_state.sprites[i + j];
		if (spe->pass != pass)
			continue;
		if (spe->callback)
		{
			drawFinish();
			AUTOTIMER_START(callback);
			gl31DrawModeSet(DM_REGULAR);

			// Unneeded, but just to make sure no one stomps on them
			glBindVertexArray(0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			spe->callback(spe->userdata);

			//glovGraphicsInitBeforeSprites(); // Reset matrices in case they've been messed with?
		} else {
			if (!mid_draw || memcmp(&spe->state, &last_spe->state, sizeof(spe->state)) != 0) {
				drawFinish();
				AUTOTIMER_START(set_textures_shaders);

				gl31TextureSetList(spe->state.textures);
				glovGLCheckError();

				assert(spe->state.shader);

				setPixelShaderParameter(0, spe->state.params.params[0]);
				setPixelShaderParameter(1, spe->state.params.params[1]);
				setPixelShaderParameter(2, spe->state.params.params[2]);
				setPixelShaderParameter(3, spe->state.params.params[3]);
				gl31ShaderSet(spe->state.shader->render_shader);
				gl31DrawModeSet(spe->state.params.draw_mode);

				glovGLCheckError();

				mid_draw = true;
				last_spe = spe;
			}

			{
				fit(sprite_points_count + spe->num_points);
				memcpy(&sprite_points[sprite_points_count], spe->points, sizeof(SpritePoint)*spe->num_points);
				sprite_points_count += spe->num_points;
			}
		}
	}
	drawFinish();
}

void gl31SpritesDrawDone()
{
	gl31DrawModeSet(DM_REGULAR);
	sprite_buffer_index = 0;
	// Unneeded, but just to make sure no one stomps on them
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Copy to Glov End - gl31sprites
//////////////////////////////////////////////////////////////////////////

void queueGeo(GeoMode geo_mode, SpritePoint *points, int num_points, int texture)
{
	SpriteListEntry *spe = (SpriteListEntry *)calloc(sizeof(SpriteListEntry) + sizeof(SpritePoint) * (num_points - 1), 1);
	spe->num_points = num_points;
	spe->pass = 0;
	spe->state.shader = sprite_prog_id;
	spe->state.textures = texture;
	spe->state.params.geo_mode = geo_mode;
	memcpy(&spe->points[0], points, num_points * sizeof(SpritePoint));
	sprites.push_back(spe);
}


void drawSprites2(SpritePoint *points, int sprite_count, int texture_id)
{
	static unsigned int vao;
	static unsigned int vbos[1];

	if (!vao) {
		glGenVertexArrays(1, &vao);
		glGenBuffers(ARRAY_SIZE(vbos), vbos);

		glBindVertexArray(vao);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
	} else {
		glBindVertexArray(vao);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sprite_count * 4 * sizeof(SpritePoint), points, GL_STREAM_DRAW);
	// Need to re-bind the buffer to the vertex array each frame
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SpritePoint), (void*)offsetof(SpritePoint, x));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpritePoint), (void*)offsetof(SpritePoint, u));
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(SpritePoint), (void*)offsetof(SpritePoint, color));

	// bind index buffer to this array
	// Perf TODO: it probably already is!  Unless we regenerated it
	int num_indexed_points;
	bindSpriteIndexBuffer(sprite_count, &num_indexed_points);
	checkError();

	// Bind textures to texture units, since the Shader references texture by texture *unit* not texture name!
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	checkError();

	glUseProgram(sprite_prog_id);
	glUniform4f(sprite_screen_transform, 2.f / screen_w, 2.f / screen_h, 0, 0);
	glDrawElements(GL_TRIANGLES, num_indexed_points, GL_UNSIGNED_SHORT, NULL);
	glBindVertexArray(0);
	checkError();
}

bool shift_down = false;
bool control_down = false;

void drawSprites() {
	bool queue_mode = !shift_down;
	float pos0 = 10;
	float pos1 = 100;
	float pos3 = 200;
	float pos4 = 220;
	float xoffs = 50 + 50 * sin(frame_timestamp * 0.004f);
	float yoffs = 50 + 50 * cos(frame_timestamp * 0.004f);
	SpritePoint quads[] = {
		{ pos0 + xoffs, pos0 + yoffs, 0, 0, colorSwizzle(0xFFFFFFff) },
		{ pos1 + xoffs, pos0 + yoffs, 1, 0, colorSwizzle(0xFF0000ff) },
		{ pos1 + xoffs, pos1 + yoffs, 1, 1, colorSwizzle(0x00FF00ff) },
		{ pos0 + xoffs, pos1 + yoffs, 0, 1, colorSwizzle(0x0000FFff) },

		{ pos3, pos0, 0, 0, colorSwizzle(0xFFFFFFff) },
		{ pos4, pos0, 1, 0, colorSwizzle(0xFF0000ff) },
		{ pos4, pos1, 1, 1, colorSwizzle(0x00FF00ff) },
		{ pos3, pos1, 0, 1, colorSwizzle(0x0000FF00) },
	};

	if (queue_mode) {
		queueGeo(SPRITEGEOMODE_QUADS, quads, ARRAY_SIZE(quads), textures[0]);
	} else {
		drawSprites2(quads, ARRAY_SIZE(quads) / 4, textures[0]);
	}

	pos3 = 300;
	pos4 = 330;
	SpritePoint tris[] = {
		{ pos3, pos0, 0, 0, colorSwizzle(0xFFFFFFff) },
		{ pos3, pos1, 1, 1, colorSwizzle(0x00FF00ff) },
		{ pos4, pos0, 1, 0, colorSwizzle(0xFF0000ff) },

		{ pos3, pos0, 0, 0, colorSwizzle(0xFFFFFFff) },
		{ pos4, pos1, 1, 1, colorSwizzle(0x00FF00ff) },
		{ pos4, pos0, 1, 0, colorSwizzle(0xFF0000ff) },
	};

	if (queue_mode)
		queueGeo(SPRITEGEOMODE_TRIANGLES, tris, ARRAY_SIZE(tris), textures[0]);

	static float v = 0.1f;
	if (control_down) {
		int mx, my;
		SDL_GetMouseState(&mx, &my);
		v = (mx - screen_w / 2) / (float)screen_w;
	}
	pos0 = screen_w / 2 - v * screen_w;
	pos1 = screen_w / 2 + v * screen_w;
	SpritePoint quads2[] = {
		{ pos0, pos0, 0, 0, colorSwizzle(0xFFFFFFff) },
		{ pos1, pos0, 1, 0, colorSwizzle(0xFFFFFFff) },
		{ pos1, pos1, 1, 1, colorSwizzle(0xFFFFFFff) },
		{ pos0, pos1, 0, 1, colorSwizzle(0xFFFFFFff) },
	};
	if (queue_mode)
		queueGeo(SPRITEGEOMODE_QUADS, quads2, ARRAY_SIZE(quads2), textures[1]);
	else
		drawSprites2(quads2, ARRAY_SIZE(quads2) / 4, textures[1]);

	gl31SpritesDrawRange(0, sprites.size(), 0);
	gl31SpritesDrawDone();

	for (unsigned int ii = 0; ii < sprites.size(); ii++) {
		free(sprites[ii]);
	}
	sprites.clear();
}

Vec4 lighting_ambient;
Vec4 lighting_diffuse;
Vec4 lighting_position;
#define setVec4(dst, x, y, z, w) (((dst)[0]=(x)),((dst)[1]=(y)),((dst)[2]=(z)),((dst)[3]=(w)))
void draw3D()
{
	glovGLCheckError();
	setVec4(lighting_ambient, 0.25f, 0.25f, 0.25f, 1.f);
	setVec4(lighting_diffuse, 1.0f, 1.0f, 1.0f, 1.0f);
	setVec4(lighting_position, -100.0f, 80.0f, 40.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glovGLCheckError();

	glUseProgram(test_prog_id);

	// Projection matrix : 45° Field of View
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), screen_w / (float)screen_h, 0.7f, 10000.f);

	// Or, for an ortho camera :
	//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

	static float v1 = 0;
	static float v2 = 0;
	if (!control_down) {
		int mx, my;
		SDL_GetMouseState(&mx, &my);
		v1 = 2 * (mx - screen_w / 2) / (float)screen_w;
		v2 = 2 * (my - screen_h / 2) / (float)screen_h;
	}

	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(v1 * 10, v2 * 10, 3), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 0, 1)  // Head is up (set to 0,-1,0 to look upside-down)
		);

	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 mv = View * Model;
		// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 mvp = Projection * mv; // Remember, matrix multiplication is the other way around
	glm::mat3 mv_inv_trans = glm::mat3(glm::transpose(glm::inverse(mv)));

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
	glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix3fv(loc_mv_inv_trans, 1, GL_FALSE, &mv_inv_trans[0][0]);
	glovGLCheckError();

#undef setPixelShaderParameter
#define setPixelShaderParameter(idx, vec4, label) glUniform4fv(glGetUniformLocation(test_prog_id, label), 1, vec4);
	glovGLCheckError();
	setPixelShaderParameter(0, lighting_ambient, "ambient");
	glovGLCheckError();
	setPixelShaderParameter(1, lighting_diffuse, "light_diffuse");
	glovGLCheckError();
	// calc direction in viewspace
	glm::vec3 lightdir_ws = glm::normalize(glm::vec3(0 - lighting_position[0], 0 - lighting_position[1], 0 - lighting_position[2]));
	glm::vec3 lightdir_vs3 = lightdir_ws * glm::mat3(glm::transpose(View));
	glm::vec4 lightdir_vs = glm::vec4(lightdir_vs3[0], lightdir_vs3[1], lightdir_vs3[2], 0);

	setPixelShaderParameter(2, &lightdir_vs[0], "lightdir_vs");
	glovGLCheckError();
	static const Vec4 unit_tex_mod = { 1, 1, 0, 0 };
	setPixelShaderParameter(3, unit_tex_mod, "texmod0");
	glovGLCheckError();
	setPixelShaderParameter(4, unit_tex_mod, "texmod1");
	glovGLCheckError();

	gl31TextureSetList(textures[0]);

	glBindVertexArray(vertex_array_object_ids[0]);	// First VAO
	glVertexAttrib3f(2, 1.0, 1.0, 1.0); // set constant color attribute
	glovGLCheckError();
	glDrawArrays(GL_TRIANGLES, 0, 3 * 6 * 2);
	glovGLCheckError();
	glBindVertexArray(0);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glovGLCheckError();
}

void updateFPS()
{
	static Uint32 last_frame_timestamp;
	if (!last_frame_timestamp) {
		last_frame_timestamp = frame_timestamp;
		return;
	}
	Uint32 dt = frame_timestamp - last_frame_timestamp;
	last_frame_timestamp = frame_timestamp;
	static int frames = 0;
	static int time = 0;
	static Uint32 minframe;
	static Uint32 maxframe;
	if (dt < minframe || !frames)
		minframe = dt;
	if (dt > maxframe || !frames)
		maxframe = dt;
	++frames;
	time += dt;
	if (time > 1000) {
		float mspf = time / (float)frames;
		float fps = 1000.f / mspf;
		time = 0;
		frames = 0;
		char buf[64];
		sprintf_s(buf, "FPS: %1.2f    ms/f: %1.1f (%d-%d)", fps, mspf, minframe, maxframe);
		SDL_SetWindowTitle(screen, buf);
	}
}

void renderFrame()
{
	frame_timestamp = SDL_GetTicks();
	
	updateFPS();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw3D();

	drawSprites();

	checkError();
}

void windowResize(int w, int h)
{
	screen_w = w;
	screen_h = h;
	glViewport(0, 0, w, h);
}

bool is_fullscreen = false;
void toggleFullscreen()
{
	static int last_size_x, last_size_y;
	is_fullscreen = !is_fullscreen;
	if (is_fullscreen) {
		last_size_x = screen_w;
		last_size_y = screen_h;
		SDL_SetWindowFullscreen(screen, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_RESIZABLE);
	} else {
		SDL_SetWindowFullscreen(screen, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
		SDL_SetWindowSize(screen, last_size_x, last_size_y);
		SDL_SetWindowPosition(screen, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}
}

int main(int argc, char *argv[])
{
	SDL_SetHint(SDL_HINT_VIDEO_X11_XVIDMODE, "0"); // Need this on Linux to get full screen toggling working
	SDL_Init(SDL_INIT_VIDEO);

	screen = SDL_CreateWindow("OpenGL 3.1 Test",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h,
		SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
	//Use OpenGL 3.1 core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// Can't do this, breaks Steam!
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GLContext context = SDL_GL_CreateContext(screen);
	const char *err = SDL_GetError();
	if (err && err[0]) {
		printf("OpenGL context creation error! SDL Error: %s\n", err);
		if (context) {
			// we probably failed to create a GL3.1 context, got a legacy context
		} else {
			// Try creating a legacy context directly (GL3.0 path hits here, can test by setting MAJOR_VERSION to 99 above)
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
			context = SDL_GL_CreateContext(screen);
		}
	}

	if (context == NULL) {
		printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
		return -1;
	}

	checkError();

	printf("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
	printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
	printf("GL_VERSION: %s\n", glGetString(GL_VERSION));

	checkError();

	GLenum glew_err = glewInit();
	if (glew_err != GLEW_OK) {
		printf("Error initializing GLEW! %s\n", glewGetErrorString(glew_err));
		return -1;
	}
	checkError();

	if (!GLEW_VERSION_3_1) {
		// We DO have a OpenGL 1.x context available here, though!
		printf("OpenGL context does not support 3.1, not valid for this test app!\n");
		return -1;
	}
	checkError();

	//Use Vsync
	if (SDL_GL_SetSwapInterval(1) < 0) {
		printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
	}
	checkError();

	if (!loadShaders())
		return -1;
	if (!loadModel())
		return -1;
	if (!loadTextures())
		return -1;

	checkError();
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LESS);
	checkError();

	bool quit = false;
	while (!quit) {
		SDL_PumpEvents();
		SDL_Event evt;
		while (SDL_PollEvent(&evt))
		{
			static bool just_got_alt_enter = false;
			if (evt.type == SDL_QUIT || (evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_CLOSE))  {
				quit = true;
			} else if (evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				windowResize(evt.window.data1, evt.window.data2);
			} else if (evt.type == SDL_KEYDOWN) {
				if ((evt.key.keysym.sym == SDLK_RETURN || evt.key.keysym.sym == SDLK_KP_ENTER) && SDL_GetModState() & KMOD_ALT) {
					// we're going to eat the up, don't let app handle the down
					just_got_alt_enter = true;
				}
				if (evt.key.keysym.sym == SDLK_LSHIFT) {
					shift_down = true;
				}
				if (evt.key.keysym.sym == SDLK_LCTRL) {
					control_down = true;
				}
			} else if (evt.type == SDL_KEYUP) {
				if ((evt.key.keysym.sym == SDLK_RETURN || evt.key.keysym.sym == SDLK_KP_ENTER) && (SDL_GetModState() & KMOD_ALT || just_got_alt_enter)) {
					toggleFullscreen();
					just_got_alt_enter = false;
				} else if (evt.key.keysym.sym == SDLK_ESCAPE) {
					quit = true;
				}
				if (evt.key.keysym.sym == SDLK_LSHIFT) {
					shift_down = false;
				}
				if (evt.key.keysym.sym == SDLK_LCTRL) {
					control_down = false;
				}
			}
		}
		renderFrame();
		SDL_GL_SwapWindow(screen);
	}

	SDL_Quit();

	return 0;
}
