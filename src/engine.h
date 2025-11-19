#include <GL/glew.h>

typedef struct model{
	GLuint vao, prog; //если не знаешь что это - не трогай
	size_t mesh_length;
	float * mesh;
	float rotation[3]; //вокруг осей xyz по порядку
	float position[3]; //ну тут думаю понятно
}model;

typedef struct entity{
	model * model;
	// дописывайте что угодно по желанию
	// также когда будешь делать логику, подскажи куда А.Ролексу воткнуть коллизию.
}entity;
