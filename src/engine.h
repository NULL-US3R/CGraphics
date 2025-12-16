#include <GL/glew.h>

typedef struct p6image {
	size_t w,h;
	uint8_t * data;
} p6image;

typedef struct model{
	p6image * texture_src;
	GLuint vao, prog; //если не знаешь что это - не трогай
	size_t verts_length, faces_length, tex_length;
	float * verts, * tex;
	unsigned int * faces, texture;
	float rotation[3]; //вокруг осей xyz по порядку
	float position[3]; //ну тут думаю понятно
}model;

typedef struct entity{
	model * model;
	// дописывайте что угодно по желанию
	// также когда будешь делать логику, подскажи куда А.Ролексу воткнуть коллизию.
}entity;

void update(entity * e);

typedef struct entity_group{
	size_t length, cap;
	entity ** arr;
}entity_group;

void add_to_group(entity_group * group, entity * e);

void update_group(entity_group * group);

void init();
