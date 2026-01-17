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

void add_node_float(node_float * root, float val){
    if(root->size==0){
        root->size=1;
        root->val=val;
    }else{
        if(val!=root->val){
            if(val<root->val){
                if(root->chil[0]==NULL){
                    root->chil[0] = malloc(sizeof(node_float));
                    root->chil[0]->val = val;
                    root->chil[0]->size = 1;
                    root->chil[0]->chil[0] = root->chil[0]->chil[1] = NULL;
                }else{
                    add_node_float(root->chil[0],val);
                }
            }
            if(val>root->val){
                if(root->chil[1]==NULL){
                    root->chil[1] = malloc(sizeof(node_float));
                    root->chil[1]->val = val;
                    root->chil[1]->size = 1;
                    root->chil[1]->chil[0] = root->chil[1]->chil[1] = NULL;
                }else{
                    add_node_float(root->chil[1],val);
                }
            }
            root->size++;
            root->size=1;
            if(root->chil[0]!=NULL){
                root->size+=root->chil[0]->size;
            }
            if(root->chil[1]!=NULL){
                root->size+=root->chil[1]->size;
            }
        }
    }
}

size_t node_float_to_arr(node_float * root, float * arr, size_t ind){
    if (root == NULL) {
        return ind;
    }
    if(root->chil[0]!=NULL){
        ind = node_float_to_arr(root->chil[0], arr, ind);
    }
    arr[ind] = root->val;
    ind++;
    if(root->chil[1]!=NULL){
        ind = node_float_to_arr(root->chil[1], arr, ind);
    }
    return ind;
}

void free_node_float(node_float * root){
    if(root->chil[0]!=NULL){
        free_node_float(root->chil[0]);
    }
    if(root->chil[0]!=NULL){
        free_node_float(root->chil[1]);
    }
    free(root);
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

void globalize_node(cgltf_node * root){
    if(root->parent!=NULL){
        float mat[16];
        //matmul(root->parent->matrix,root->matrix,mat,4,4,4);
        matmul(root->matrix,root->parent->matrix,mat,4,4,4); //swapped
        memcpy(root->matrix, mat, 16*sizeof(float));
    }
    for(size_t i=0; i<root->children_count; i++){
        globalize_node(root->children[i]);
    }
}

#define scale_mat(scale){\
    scale[0],0,0,0,\
    0,scale[1],0,0,\
    0,0,scale[2],0,\
    0,0,0,1\
}

#define tran_mat(tran){\
    1,0,0,0,\
    0,1,0,0,\
    0,0,1,0,\
    tran[0],tran[1],tran[2],1\
}

void rot_mat(float * out, float rot[4]){
    float qi = rot[0];
    float qj = rot[1];
    float qk = rot[2];
    float qr = rot[3];
    float qr2 = qr*qr;
    float qi2 = qi*qi;
    float qj2 = qj*qj;
    float qk2 = qk*qk;

    float s = fsqrt(qr2+qi2+qj2+qk2);
    s = 1./(s*s);
    float mat_out[16] = {
        1-2*s*(qj2+qk2),2*s*(qi*qj-qk*qr),2*s*(qi*qk-qj*qr),0,
        2*s*(qi*qj-qk*qr),1-2*s*(qi2+qk2),2*s*(qj*qk-qi*qr),0,
        2*s*(qi*qk-qj*qr),2*s*(qj*qk-qi*qr),1-2*s*(qi2+qj2),0,
        0,0,0,1
    };
    memcpy(out, mat_out, 16*sizeof(float));
}

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
        if(p->attributes[i].type == cgltf_attribute_type_joints){
            cgltf_accessor * acc = p->attributes[i].data;
            m->bone_id_length = acc->count*4;
            m->bone_ids = malloc(m->bone_id_length * sizeof(int));

            for(size_t j = 0; j < acc->count; j++) {
                cgltf_uint tmp[4];
                cgltf_accessor_read_uint(acc, j, tmp, 4); // Correct way to read VEC4 joints
                m->bone_ids[j * 4 + 0] = (int)tmp[0];
                m->bone_ids[j * 4 + 1] = (int)tmp[1];
                m->bone_ids[j * 4 + 2] = (int)tmp[2];
                m->bone_ids[j * 4 + 3] = (int)tmp[3];
            }
        }
        if(p->attributes[i].type == cgltf_attribute_type_weights){
            cgltf_accessor * acc = p->attributes[i].data;
            m->bone_weight_length = acc->count*4;
            m->bone_weights = malloc(m->bone_weight_length*sizeof(float));
            cgltf_accessor_unpack_floats(acc, m->bone_weights, m->bone_weight_length);
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

    //walk through joints and compute matrices for animations
    if(data->animations_count>0){
        //get all timestamps
        m->num_bones = data->skins[0].joints_count;
        m->bones = malloc(m->num_bones*16*sizeof(float));
        memset(m->bones, 0, m->num_bones*16*sizeof(float));
        for(size_t i=0; i<m->num_bones; i++){
            m->bones[i * 16 + 0] = 1.0f;
            m->bones[i * 16 + 5] = 1.0f;
            m->bones[i * 16 + 10] = 1.0f;
            m->bones[i * 16 + 15] = 1.0f;
        }
        m->anim_count = data->animations_count;
        m->anims = malloc(m->anim_count*sizeof(animation));
        memset(m->anims,0,m->anim_count*sizeof(animation));
        for(size_t i=0; i<data->animations_count; i++){
            cgltf_animation * anim = &data->animations[i];
            node_float * set = malloc(sizeof(node_float));
            memset(set,0,sizeof(node_float));
            for(size_t j=0; j<anim->channels_count; j++){
                cgltf_animation_channel * chan = &anim->channels[j];
                size_t numt = chan->sampler->input->count;
                float times[numt];

                cgltf_accessor_unpack_floats(chan->sampler->input, times, numt);
                for(size_t k=0; k<numt; k++){
                    add_node_float(set, times[k]);
                }
            }
            m->anims[i].num_frames = set->size;
            m->anims[i].timestamps = malloc(set->size*sizeof(float));
            node_float_to_arr(set, m->anims[i].timestamps, 0);
            free_node_float(set);
            m->anims[i].frames = malloc(m->anims[i].num_frames*m->num_bones*16*sizeof(float));
            for(size_t j=0; j<m->anims[i].num_frames; j++){
                //reset
                float mat_one[16] = {
                    1,0,0,0,
                    0,1,0,0,
                    0,0,1,0,
                    0,0,0,1
                };
                for(size_t k=0; k<data->skins[0].joints_count; k++){
                    memcpy(data->skins[0].joints[k]->matrix, mat_one, 16*sizeof(float));
                }
                //scale
                for(size_t k=0; k<anim->channels_count; k++){
                    cgltf_animation_channel * chan = &anim->channels[k];
                    if(chan->target_path == cgltf_animation_path_type_scale){
                        float stamps[chan->sampler->input->count];
                        cgltf_accessor_unpack_floats(chan->sampler->input, stamps, chan->sampler->input->count);
                        for(size_t i1=0; i1<chan->sampler->input->count; i1++){
                            if(stamps[i1]==m->anims[i].timestamps[j]){
                                float scale[3];
                                cgltf_accessor_read_float(chan->sampler->output, i1, scale, 3);
                                float mat[16] = scale_mat(scale);
                                float out_mat[16] = {0};
                                //matmul(mat,chan->target_node->matrix,out_mat,4,4,4);
                                matmul(chan->target_node->matrix,mat,out_mat,4,4,4); //swapped
                                memcpy(chan->target_node->matrix,out_mat,16*sizeof(float));
                            }
                        }
                    }
                }
                //rotate
                for(size_t k=0; k<anim->channels_count; k++){
                    cgltf_animation_channel * chan = &anim->channels[k];
                    if(chan->target_path == cgltf_animation_path_type_rotation){
                        float stamps[chan->sampler->input->count];
                        cgltf_accessor_unpack_floats(chan->sampler->input, stamps, chan->sampler->input->count);
                        for(size_t i1=0; i1<chan->sampler->input->count; i1++){
                            if(stamps[i1]==m->anims[i].timestamps[j]){
                                float rot[4];
                                cgltf_accessor_read_float(chan->sampler->output, i1, rot, 4);
                                float mat[16];
                                rot_mat(mat, rot);
                                float out_mat[16] = {0};
                                //matmul(mat,chan->target_node->matrix,out_mat,4,4,4);
                                matmul(chan->target_node->matrix,mat,out_mat,4,4,4); //swapped
                                memcpy(chan->target_node->matrix,out_mat,16*sizeof(float));
                            }
                        }
                    }
                }
                //translate
                for(size_t k=0; k<anim->channels_count; k++){
                    cgltf_animation_channel * chan = &anim->channels[k];
                    if(chan->target_path == cgltf_animation_path_type_translation){
                        float stamps[chan->sampler->input->count];
                        cgltf_accessor_unpack_floats(chan->sampler->input, stamps, chan->sampler->input->count);
                        for(size_t i1=0; i1<chan->sampler->input->count; i1++){
                            if(stamps[i1]==m->anims[i].timestamps[j]){
                                float scale[3];
                                cgltf_accessor_read_float(chan->sampler->output, i1, scale, 3);
                                float mat[16] = tran_mat(scale);
                                float out_mat[16] = {0};
                                //matmul(mat,chan->target_node->matrix,out_mat,4,4,4);
                                matmul(chan->target_node->matrix,mat,out_mat,4,4,4); //swapped
                                memcpy(chan->target_node->matrix,out_mat,16*sizeof(float));
                            }
                        }
                    }
                }
                //make em global
                globalize_node(data->skins[0].joints[0]);
                //make em full
                for(size_t k = 0; k<data->skins[0].joints_count; k++){
                    float mat[16];
                    float inv_mat[16];
                    cgltf_accessor_read_float(data->skins[0].inverse_bind_matrices, k, inv_mat, 16);
                    //matmul(data->skins[0].joints[k]->matrix,inv_mat,mat,4,4,4);
                    matmul(inv_mat,data->skins[0].joints[k]->matrix,mat,4,4,4); //swapped
                    memcpy(data->skins[0].joints[k]->matrix, mat, 16*sizeof(float));
                }
                //copy to array;
                for(size_t k = 0; k<data->skins[0].joints_count; k++){
                    memcpy(&m->anims[i].frames[j*m->num_bones+k], data->skins[0].joints[k]->matrix, 16*sizeof(float));
                }
            }
        }
    }

    cgltf_free(data);
    return m;
}

void load_frame(model * m, size_t id_anim, size_t id_frame){
    memcpy(m->bones, m->anims[id_anim].frames[id_frame*m->num_bones*16], m->num_bones*16*sizeof(float));
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
	e2->model = load_model("./models/2.glb");
	add_to_group(&all, e2);
	e2->model->position[0]=10;

	entity * e1 = malloc(sizeof(entity));
	e1->model = load_model("./models/3.glb");
	add_to_group(&all, e1);
	e1->model->rotation[0]=1;
	e1->model->position[0]=-10;
	load_frame(e1->model, 0, 0);
}
