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

//cam.xy

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
	printf("x:%f, y:%f, z:%f\n",cam1.position[0],cam1.position[1],cam1.position[2]);
}

void update(entity * e){ // дописать аргументы по необходимости или использовать external переменные
	// сюда пихать логику сущностей
}

void init(){
	// сюда инициализацию группы all т.е. всех начальных сущностей
}
