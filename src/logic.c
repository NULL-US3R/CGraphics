#include "./engine.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#define CGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "./cgltf/cgltf.h"
#include "./stb/stb_image.h"

entity_group all = {0};
camera cam1 = {0};

uint64_t current_actions = 0;

model * load_model(char * filename){
    //printf("loading %s:\n",filename);
    cgltf_options opt = {0};
    cgltf_data* data = NULL;
    cgltf_result res = cgltf_parse_file(&opt, filename, &data);

    if(res != cgltf_result_success){
        printf("unable to load file\n");
    }
    cgltf_load_buffers(&opt, data, filename);
    cgltf_primitive * p = &data->meshes[0].primitives[0];

    model * m = malloc(sizeof(model));
    memset(m,0,sizeof(model));

    //printf("parsing attributes\n");

    for(size_t i=0; i<p->attributes_count; i++){
        if(p->attributes[i].type == cgltf_attribute_type_position){
            cgltf_accessor * acc = p->attributes[i].data;
            m->verts_length = acc->count*3;
            m->verts = malloc(m->verts_length*sizeof(float));
            cgltf_accessor_unpack_floats(acc, m->verts, m->verts_length);
        }
        if(p->attributes[i].type == cgltf_attribute_type_texcoord){
            cgltf_accessor * acc = p->attributes[i].data;
            m->tex_length = acc->count*2;
            m->tex = malloc(m->tex_length*sizeof(float));
            cgltf_accessor_unpack_floats(acc, m->tex, m->tex_length);
        }
    }

    //printf("parsing faces\n");

    m->faces_length = p->indices->count;
    m->faces = malloc(sizeof(unsigned int) * m->faces_length);
    cgltf_accessor_unpack_indices(p->indices, m->faces, sizeof(unsigned int), m->faces_length);

    //printf("done, cleaning up\n");
    memset(m->rotation, 0, sizeof(float)*3);
    memset(m->position, 0, sizeof(float)*3);


    //printf("%ld\n",data->images_count);
    if(data->textures_count>0){
        uint8_t * img_dat = data->textures[0].image->buffer_view->buffer->data + data->textures[0].image->buffer_view->offset;
        int x,y,n;
        p6image * img = malloc(sizeof(p6image));
        img->data = stbi_load_from_memory(img_dat, data->textures[0].image->buffer_view->buffer->size-data->textures[0].image->buffer_view->offset, &x, &y, &n, 3);
        img->w = x;
        img->h = y;
        m->texture_src = img;
    }

    if(data->animations_count>0){
        printf("model %s has %lu animations\n",filename,data->animations_count);
    }

    cgltf_free(data);
    return m;
}

void add_to_group(entity_group * group, entity * e){
	if(group->length==group->cap){
		if(group->cap==0){
			group->cap=2;
		}else{
			group->cap*=2;
		}
		group->arr = realloc(group->arr, group->cap*sizeof(entity*));
	}
	group->arr[group->length] = e;
	group->length++;
}

void update_group(entity_group * group){
	for(size_t i=0; i<group->length; i++){
		update(group->arr[i]);
	}
}

void matmul(float * a, float * b, float * o, size_t sr, size_t sm, size_t sc){
	for(size_t r=0; r<sr; r++){
		for(size_t c=0; c<sc; c++){
			o[sc*r+c] = 0;
			for(size_t m=0; m<sm; m++){
				o[sc*r+c] += a[sm*r+m]*b[sc*m+c];
			}
		}
	}
}

void update_cam_rot(){
	float cosy=cos(cam1.rotation[1]),siny=sin(cam1.rotation[1]);
	float ymat[9] = {
		cosy,0,siny,
		0,   1,   0,
		-siny,0,cosy
	};
	float cosx=cos(cam1.rotation[0]),sinx=sin(cam1.rotation[0]);
	float xmat[9] = {
		1,0,0,
		0,cosx,sinx,
		0,-sinx,cosx,
	};
	matmul(xmat, ymat, cam1.rot_mat, 3, 3, 3);
}

void update_cam_pos(){
	float speed = .1;
	float d[2] = {0};
	if(current_actions&CAM_FORWARD){
		d[1]=1;
	}
	if(current_actions&CAM_BACKWARD){
		d[1]=-1;
	}
	if(current_actions&CAM_RIGHT){
		d[0]=1;
	}
	if(current_actions&CAM_LEFT){
		d[0]=-1;
	}
	float nd[2] = {0};
	nd[0] = d[0] * cos(cam1.rotation[1]) - d[1] * sin(cam1.rotation[1]);
	nd[1] = d[0] * sin(cam1.rotation[1]) + d[1] * cos(cam1.rotation[1]);

	cam1.position[2]+=speed*nd[1];
	cam1.position[0]+=speed*nd[0];
}

void update(entity * e){ // дописать аргументы по необходимости или использовать external переменные
	// сюда пихать логику сущностей
}

void init(){
	// сюда инициализацию группы all т.е. всех начальных сущностей
	for(int i=0; i<3; i++){
		cam1.rot_mat[3*i+i]=1.;
	}

	entity * e2 = malloc(sizeof(entity));
	e2->model = load_model("/home/main/Desktop/graphics/CGraphics/models/2.glb");
	add_to_group(&all, e2);
	e2->model->position[0]=10;

	entity * e1 = malloc(sizeof(entity));
	e1->model = load_model("/home/main/Desktop/graphics/CGraphics/models/3.glb");
	add_to_group(&all, e1);
	e1->model->rotation[0]=1;
	e1->model->position[0]=-10;
}
