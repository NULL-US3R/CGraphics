#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

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
    free(src);
    return sh;
}

GLuint makeprog(){
    GLuint prog = glCreateProgram();
    GLuint vsh = makeshader("./1.vert", GL_VERTEX_SHADER);
    GLuint fsh = makeshader("./1.frag", GL_FRAGMENT_SHADER);
    glAttachShader(prog,vsh);
    glAttachShader(prog,fsh);
    glLinkProgram(prog);
    glDeleteShader(vsh);
    glDeleteShader(fsh);
    return prog;
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

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(points),points,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,0,0,0);
    glEnableVertexAttribArray(0);

    GLuint prog = makeprog();

    glUseProgram(prog);

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
        glDrawArrays(GL_TRIANGLES, 0, 6);
        SDL_GL_SwapWindow(w);
    }
    SDL_DestroyWindow(w);
    SDL_Quit();
    return 0;
}
