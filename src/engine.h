#include <GL/glew.h>

#define CAM_FORWARD 0x1
#define CAM_BACKWARD 0x2
#define CAM_RIGHT 0x4
#define CAM_LEFT 0x8


typedef struct p6image {
	size_t w,h;
	uint8_t * data;
} p6image;

typedef struct model{
	p6image * texture_src;
	GLuint vao, prog; //если не знаешь что это - не трогай
	size_t verts_length, faces_length, tex_length;
	float * verts, * tex;
	int * bone_ids;
	float * bone_weights;
	size_t num_bones;
	float * bones[4][4];
	size_t num_frames;
	float ** frames[4][4]; //кости во времени
	unsigned int * faces, texture;
	float rotation[3]; //вокруг осей xyz по порядку
	float rot_mat[9];
	float position[3]; //ну тут думаю понятно
}model;

model * load_model(char * filename);

typedef struct camera{
	float rotation[3];
	float position[3];
	float tarrget[3];
	float rot_mat[16];
	float clip_mat[16];
}camera;

typedef struct entity{
	model * model;
	// дописывайте что угодно по желанию
	// также когда будешь делать логику, подскажи куда А.Ролексу воткнуть коллизию.
}entity;

void matmul(float * a, float * b, float * o, size_t sr, size_t sm, size_t sc);

void update(entity * e);

typedef struct entity_group{
	size_t length, cap;
	entity ** arr;
}entity_group;

//добавить имбицила к группе
void add_to_group(entity_group * group, entity * e);

//обновить все сучности сразу в группу
void update_group(entity_group * group);

//обновить бандикам
void update_cam_pos();
void update_cam_rot();

//инициировать секс
void init();
