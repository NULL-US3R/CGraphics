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
	glGenVertexArrays(1,&m->vao);
	glBindVertexArray(m->vao);
	GLuint vbo;
	glGenBuffers(1,&vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, m->mesh_length*sizeof(float), m->mesh, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_model(model * m){
	glBindVertexArray(m->vao);
	glUseProgram(m->prog);
	glDrawArrays(GL_TRIANGLES, 0, m->mesh_length);
	glBindVertexArray(0);
}

int main(){
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window * w = SDL_CreateWindow("fractal", 800,800, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);

    SDL_GLContext ctx = SDL_GL_CreateContext(w);
    glewInit();

    float points[18] = {
        -1,-1,-1,
        1,-1,-1,
        1,1,-1,

        -1,-1,-1,
        -1,1,-1,
        1,1,-1,
    };

    GLuint p1 = makeprog("./shaders/1.vert", "./shaders/1.frag");;

    model flat1 = {0};
    flat1.mesh = points;
    flat1.mesh_length = 18;
    flat1.prog = p1;
    load_model_to_gpu(&flat1);


    // GLuint vbo;
    // glGenBuffers(1, &vbo);
    // glBindBuffer(GL_ARRAY_BUFFER,vbo);
    // glBufferData(GL_ARRAY_BUFFER,sizeof(points),points,GL_STATIC_DRAW);
    // glVertexAttribPointer(0,3,GL_FLOAT,0,0,0);
    // glEnableVertexAttribArray(0);

    // GLuint prog = makeprog("./shaders/1.vert", "./shaders/1.frag");

    glUseProgram(p1);

    {
        int w_w,w_h;
        SDL_GetWindowSize(w, &w_w, &w_h);
        glUniform2i(0,w_w,w_h);
        glViewport(0, 0, w_w,w_h);
    }

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
        glUniform1f(1,(float)SDL_GetTicks()/1000.);


        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);
        // glDrawArrays(GL_TRIANGLES, 0, 6);
        draw_model(&flat1);
        SDL_GL_SwapWindow(w);
    }
    SDL_DestroyWindow(w);
    SDL_Quit();
    return 0;
}
