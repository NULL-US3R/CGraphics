#include "./engine.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

entity_group all = {0};
camera cam1 = {0};

uint64_t current_actions = 0;

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

void update_cam(){
	float speed = .1;
	// if(current_actions&CAM_FORWARD){
	// 	cam1.position[2]+=speed;
	// }
	// if(current_actions&CAM_BACKWARD){
	// 	cam1.position[2]-=speed;
	// }
	// if(current_actions&CAM_RIGHT){
	// 	cam1.position[0]+=speed;
	// }
	// if(current_actions&CAM_LEFT){
	// 	cam1.position[0]-=speed;
	// }

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
	// cam1.rot_mat[0]=cos(cam1.rotation[1]);
	// cam1.rot_mat[2]=sin(cam1.rotation[1]);
	// cam1.rot_mat[4]=1;
	// cam1.rot_mat[6]=-cam1.rot_mat[2];
	// cam1.rot_mat[8]=cam1.rot_mat[0];
	//cam1.rot_mat[15]=1;

	//printf("x:%f, y:%f, z:%f\n",cam1.position[0],cam1.position[1],cam1.position[2]);
}

void update(entity * e){ // дописать аргументы по необходимости или использовать external переменные
	// сюда пихать логику сущностей
}

void init(){
	// сюда инициализацию группы all т.е. всех начальных сущностей
	for(int i=0; i<3; i++){
		cam1.rot_mat[3*i+i]=1.;
	}

}
