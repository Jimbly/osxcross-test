#define GLEW_STATIC
#include "GL/glew.h"

int main(int argc, char *argv[])
{
	GLenum glew_err = glewInit();
	return 0;
}
