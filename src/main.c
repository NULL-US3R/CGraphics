#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include "./engine.h"

char * fromfile(char * name){
    FILE * f = fopen(name, "r");
    fseek(f, 0, SEEK_END);
    size_t l = ftell(f);
    fseek(f, 0, SEEK_SET);
    char * out = malloc(l+1);
    fread(out,1,l,f);
    fclose(f);
    out[l] = '\0';
    return out;
}

GLuint makeshader(char * name, GLuint type){
    GLuint sh = glCreateShader(type);
    char * src = fromfile(name);
    glShaderSource(sh,1,(const GLchar * const*)&src,0);
    glCompileShader(sh);
    char log[512] = {0};
    int suc;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &suc);
    if(!suc){
   		glGetShaderInfoLog(sh,512,NULL,log);
     	printf("Shader compilation failed:\n%s\n",log);
    }
    free(src);
    return sh;
}

GLuint makeprog(char * vname, char * fname){
    GLuint prog = glCreateProgram();
    GLuint vsh = makeshader(vname, GL_VERTEX_SHADER);
    GLuint fsh = makeshader(fname, GL_FRAGMENT_SHADER);
    glAttachShader(prog,vsh);
    glAttachShader(prog,fsh);
    glLinkProgram(prog);
    char log[512] = {0};
    int suc;
    glGetProgramiv(prog,GL_LINK_STATUS,&suc);
    if(!suc){
   		glGetProgramInfoLog(prog,512,NULL,log);
     	printf("Program linking failed:\n%s\n",log);
    }
    glDeleteShader(vsh);
    glDeleteShader(fsh);
    return prog;
}

void load_model_to_gpu(model * m){
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m->texture);
	glBindTexture(GL_TEXTURE_2D, m->texture);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0, GL_RGB, m->texture_src->w, m->texture_src->h, 0, GL_RGB, GL_UNSIGNED_BYTE, m->texture_src->data);
	glBindTexture(GL_TEXTURE_2D,0);

	glGenVertexArrays(1,&m->vao);
	glBindVertexArray(m->vao);
	GLuint vbo, ebo, tex;
	glGenBuffers(1,&vbo);
	glGenBuffers(1,&ebo);
	glGenBuffers(1,&tex);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, m->verts_length*sizeof(float), m->verts, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->faces_length*sizeof(unsigned int), m->faces, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, tex);
	glBufferData(GL_ARRAY_BUFFER, m->tex_length*sizeof(float), m->tex, GL_STATIC_DRAW);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void draw_model(model * m){
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m->texture);
	glBindVertexArray(m->vao);

	glUseProgram(m->prog);
	glUniform1i(glGetUniformLocation(m->prog,"tex1"),0);
	glUniform3f(3,m->rotation[0],m->rotation[1],m->rotation[2]);
	glUniform3f(4,m->position[0],m->position[1],m->position[2]);

	glDrawElements(GL_TRIANGLES, m->faces_length, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void draw_entity(entity * e){
	draw_model(e->model);
}

void draw_group(entity_group * gr){
	for(size_t i=0; i<gr->length; i++){
		draw_entity(gr->arr[i]);
	}
}

p6image * loadp6(char * name){
	p6image * out = malloc(sizeof(p6image));
	FILE * f = fopen(name, "r");
	char buf[80];
	fgets(buf,80,f);
	if(buf[0]=='P'&&buf[1]=='6'){
		fgets(buf,80,f);
		out->w = atoi(buf);
		size_t i=0;
		while(buf[i]!=' '){
			i++;
		}
		i++;
		out->h = atoi(buf+i);
		//printf("w:%lu h:%lu\n",out->w, out->h);
		out->data = malloc(out->w*out->h*3);
		fread(out->data, 1, out->w*out->h*3, f);
	}
	fclose(f);
	return out;
}

int main(){
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window * w = SDL_CreateWindow("fractal", 800,800, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);

    SDL_GLContext ctx = SDL_GL_CreateContext(w);
    glewInit();

    float points[24] = {
        -1,-1,1,
        1,-1,1,
        -1,1,1,
        1,1,1,
        -1,-1,-1,
        1,-1,-1,
        -1,1,-1,
        1,1,-1
    };

    unsigned int flats[36] = {
    	0,1,2,
        1,2,3,

        0,1,4,
        1,4,5,

        0,2,4,
        2,4,6,

        1,3,5,
        3,5,7,

        2,3,6,
        3,6,7,

        4,5,6,
        5,6,7
    };

    float texes[16] = {
	    0,0,
	    1,0,
	    0,1,
	    1,1,

		1,1,
		0,1,
		1,0,
		0,0,
    };

    p6image * im1 = loadp6("./textures/tex1.ppm");

    GLuint p1 = makeprog("./shaders/1.vert", "./shaders/1.frag");;

    model flat1 = {0};
    flat1.texture_src=im1;
    flat1.verts = points;
    flat1.verts_length = 24;
    flat1.tex = texes;
    flat1.tex_length = 16;
    flat1.faces = flats;
    flat1.faces_length = 36;
    flat1.prog = p1;
    flat1.position[2]=5.;
    load_model_to_gpu(&flat1);

    glUseProgram(p1);

    {
        int w_w,w_h;
        SDL_GetWindowSize(w, &w_w, &w_h);
        glUniform2i(0,w_w,w_h);
        glViewport(0, 0, w_w,w_h);
    }

    extern entity_group all;
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);
    init();

    SDL_Event e;
    uint8_t run=1;
    while(run){
        while(SDL_PollEvent(&e)){
            if(e.type == SDL_EVENT_QUIT){
                run=0;
            }
            if(e.type == SDL_EVENT_KEY_DOWN){
                if(e.key.key == SDLK_ESCAPE){
                    run=0;
                }
            }
            if(e.type == SDL_EVENT_WINDOW_RESIZED){
                int w_w,w_h;
                SDL_GetWindowSize(w, &w_w, &w_h);
                glUniform2i(0,w_w,w_h);
                glViewport(0, 0, w_w,w_h);
            }
        }
        flat1.rotation[0] += 1e-2;
        flat1.rotation[2] += 1e-2;
        flat1.rotation[1] += 1e-2;
        // flat1.position[2] += 1e-2;
        update_group(&all);
        glUniform1f(1,(float)SDL_GetTicks()/1000.);
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        draw_model(&flat1);
        draw_group(&all);
        SDL_GL_SwapWindow(w);
    }
    SDL_DestroyWindow(w);
    SDL_Quit();
    return 0;
}
