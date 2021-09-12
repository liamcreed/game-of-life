#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef struct
{
    GLFWwindow *glfw_window;
    int closed;
} Window;

#define SCR_SIZE 800
void WindowInit(Window *window)
{
    if (!glfwInit())
        printf("[ERROR]: Failed to init GLFW!\n");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window->glfw_window = glfwCreateWindow(SCR_SIZE, SCR_SIZE, "Game of Life", NULL, NULL);

    if (!window->glfw_window)
    {
        glfwTerminate();
        printf("[ERROR]: Failed to create window!\n");
    }
    glfwMakeContextCurrent(window->glfw_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("[ERROR]: Failed to init glad!\n");
    }

    window->closed = 0;
    glfwSwapInterval(0);
}
void WindowUpdate(Window *window)
{
    if (glfwWindowShouldClose(window->glfw_window))
        window->closed = 1;
    else
        window->closed = 0;

    glfwSwapBuffers(window->glfw_window);
    glfwPollEvents();
}
void WindowDestroy(Window *window)
{
    glfwDestroyWindow(window->glfw_window);
    glfwTerminate();
}


unsigned int CreateShader(const char* v_src, const char* f_src)
{
    unsigned int v_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v_shader, 1, &v_src, NULL);
    glCompileShader(v_shader);

    int success;
    char infoLog[512];
    glGetShaderiv(v_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(v_shader, 512, NULL, infoLog);
        printf("%s %s\n", "Failed to compile VERTEX shader!:", infoLog);
    }

    unsigned int f_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f_shader, 1, &f_src, NULL);
    glCompileShader(f_shader);

    glGetShaderiv(f_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(f_shader, 512, NULL, infoLog);
        printf("%s %s\n", "Failed to compile FRAGMENT shader!:", infoLog);
    }

    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, v_shader);
    glAttachShader(shader_program, f_shader);
    glLinkProgram(shader_program);

    glDeleteShader(v_shader);
    glDeleteShader(f_shader);

    return shader_program;
}


#define ALIVE 1
#define DEAD  0
#define GRID_SIZE 1024

typedef struct Cell
{
    int state;
    float r, g, b;
}Cell;


int main(void)
{
    
    Window window;
    WindowInit(&window);

    float vertices[] = 
    {
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f
    };

    unsigned int indices[] =
    {
        0, 1, 3,
        1, 2, 3
    };

    const char *v_shader_src = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 v_pos;\n"
    "layout (location = 1) in vec2 v_uv;\n"
    "out vec2 o_uv;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(v_pos, 1.0);\n"
    "   o_uv = vec2(v_uv.x, 1.0 - v_uv.y);\n;"
    "}\0";

    const char *f_shader_src = 
    "#version 330 core\n"
    "in vec2 o_uv;\n"
    "out vec4 f_color;\n"
    "uniform sampler2D tex;\n"
    "void main()\n"
    "{\n"
    "f_color = texture(tex, o_uv);\n"
    //"f_color = vec4(o_uv, 0.0f,  1.0f);\n"
    "}\0";

    unsigned int shader = CreateShader(v_shader_src, f_shader_src);
    glUseProgram(shader);

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,  sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    static Cell cells_state[GRID_SIZE][GRID_SIZE];
    static Cell cells_output[GRID_SIZE][GRID_SIZE];
    
    for (int x = 0; x < GRID_SIZE; x++)
    {
        for (int y = 0; y < GRID_SIZE; y++)
        {
            int random  =  (rand() % 2);
            Cell cell;
            //if(random > 8)
             //   cell.state = 1;
           // else
              //  cell.state = 0;

            cell.r = 255;
            cell.g = 255;
            cell.b = 255;
            cell.state = random;
            cells_state[x][y] =  cell;
            cells_output[x][y] = cell;

        }
    }
    
    unsigned char texture_data[GRID_SIZE * GRID_SIZE * 4];
    for (int i = 0; i < GRID_SIZE * GRID_SIZE * 4; i+=4)
    {
        texture_data[i + 0] = 255 ;
        texture_data[i + 1] = 255 ;
        texture_data[i + 2] = 255 ;
        texture_data[i + 3] = 255;
    }

    unsigned int texture;
    glGenTextures(1, &texture);  
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture); 


    float update_speed = 1;
    int update = 0;

    while (!window.closed)
    {
        WindowUpdate(&window);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        if(update < 1 / update_speed)
        {
            update+=1;
        }
        else if(update == 1 / update_speed)
        {
            update = 0;
            for (int x = 0; x < GRID_SIZE; x++)
            {
                for (int y = 0; y < GRID_SIZE; y++)
                {
                    int live_neighbours = cells_state[x - 1][y + 1].state + cells_state[x][y + 1].state + cells_state[x + 1][y + 1].state +
                                          cells_state[x - 1][y + 0].state +                               cells_state[x + 1][y + 0].state +
                                          cells_state[x - 1][y - 1].state + cells_state[x][y - 1].state + cells_state[x + 1][y - 1].state;

                    if(cells_state[x][y].state == ALIVE)
                    {
                        if(live_neighbours > 3 || live_neighbours < 2)
                            cells_output[x][y].state = DEAD;
                        else
                            cells_output[x][y].state = ALIVE;
                    }
                    if(cells_state[x][y].state == DEAD && live_neighbours == 3)
                    {
                        cells_output[x][y].state = ALIVE;
                    }
                    
                }
            } 
            for (int x = 0; x < GRID_SIZE; x++)
            {
                for (int y = 0; y < GRID_SIZE; y++)
                {
                    cells_state[x][y].state = cells_output[x][y].state;
                    texture_data[(x * 4) + (y * GRID_SIZE * 4) + 0] = cells_output[x][y].state * cells_output[x][y].r;
                    texture_data[(x * 4) + (y * GRID_SIZE * 4) + 1] = cells_output[x][y].state * cells_output[x][y].g;
                    texture_data[(x * 4) + (y * GRID_SIZE * 4) + 2] = cells_output[x][y].state * cells_output[x][y].b;
                    texture_data[(x * 4) + (y * GRID_SIZE * 4) + 3] = 255;
                }
            } 
        } 
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GRID_SIZE, GRID_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
        glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(unsigned int), GL_UNSIGNED_INT, 0);
    }

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteProgram(shader);
    glDeleteVertexArrays(1, &vao);

    WindowDestroy(&window); 
}