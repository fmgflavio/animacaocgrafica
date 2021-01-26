/*
  =========================================
  =	  CIC270 - COMPUTAÇÃO GRÁFICA         =
  =	    	  PROJETO FINAL               =
  =                                       =
  =    Flávio Mota Gomes - 2018005379     =
  =  Rafael Antunes Vieira - 2018000980   =	 
  =========================================
 */

// Bibliotecas necessarias para execução do programa
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "lib/utils.h"

/* Variaveis globais para usar na execução do programa*/

/* Variaveis que controla o tamanho da janela*/
int janela_largura = 1000;
int janela_altura = 550;

/** Angulo do X */
float px_angle = 0.0f;

/** Variavel de ingremento do angulo X */
float px_inc = 0.3f;

/** Angulo do Y */
float py_angle = 0.0f;

/** Variavel de ingremento do angulo Y */
float py_inc = 0.6f;

/* Variavel do programa */
int program;

/* Variavel controla o zoom na imagem */
float ZoomZ = 0.5;

/* Faz o controle da animação da iluminação */
float posicao_camera_X = 5.0;

/* Faz o controle do sentido da animação da iluminação */
int sentido = -1;

/* Contador que controla a tempo em que a animação da iluminação executa*/
int contador = 0;

/* Vetor do objeto. */
unsigned int CUBO;

/* Buffer do objeto. */
unsigned int CUBO_BUFFER;

/** Vertex shader. */
const char *vertex_code = "\n"
                          "#version 330 core\n"
                          "layout (location = 0) in vec3 position;\n"
                          "layout (location = 1) in vec3 normal;\n"
                          "\n"
                          "uniform mat4 M;\n"
                          "uniform mat4 view;\n"
                          "uniform mat4 projection;\n"
                          "\n"
                          "out vec3 vNormal;\n"
                          "out vec3 fragPosition;\n"
                          "\n"
                          "void main()\n"
                          "{\n"
                          "    gl_Position = projection * view * M * vec4(position, 1.0);\n"
                          "    vNormal = normal;\n"
                          "    fragPosition = vec3(M * vec4(position, 1.0));\n"
                          "}\0";

/** Fragment shader. */
const char *fragment_code = "\n"
                            "#version 330 core\n"
                            "\n"
                            "in vec3 vNormal;\n"
                            "in vec3 fragPosition;\n"
                            "\n"
                            "out vec4 fragColor;\n"
                            "\n"
                            "uniform vec3 objectColor;\n"
                            "uniform vec3 lightColor;\n"
                            "uniform vec3 lightPosition;\n"
                            "uniform vec3 cameraPosition;\n"
                            "\n"
                            "void main()\n"
                            "{\n"
                            "    float ka = 0.5;\n"
                            "    vec3 ambient = ka * lightColor;\n"
                            "\n"
                            "    float kd = 0.8;\n"
                            "    vec3 n = normalize(vNormal);\n"
                            "    vec3 l = normalize(lightPosition - fragPosition);\n"
                            "\n"
                            "    float diff = max(dot(n,l), 0.0);\n"
                            "    vec3 diffuse = kd * diff * lightColor;\n"
                            "\n"
                            "    float ks = 1.0;\n"
                            "    vec3 v = normalize(cameraPosition - fragPosition);\n"
                            "    vec3 r = reflect(-l, n);\n"
                            "\n"
                            "    float spec = pow(max(dot(v, r), 0.0), 3.0);\n"
                            "    vec3 specular = ks * spec * lightColor;\n"
                            "\n"
                            "    vec3 light = (ambient + diffuse + specular) * objectColor;\n"
                            "    fragColor = vec4(light, 1.0);\n"
                            "}\0";

/* Funções do programa */
void display(void);
void reshape(int, int);
void keyboard(unsigned char, int, int);
void Rotacao(void);
void initData(void);
void initShaders(void);
void rotacao(void);
void animacao_da_luz(void);
void coordenadas_do_cubo(void);
void cria_cubos(float x, float y, float z, int selec_rotacao);

/*  Função que cria o cubo na coordenada correta 
    Recebe o valor de X,Y,Z , e um valor para selecionar se quer ou não a rotação no cubo.
*/
void cria_cubos(float x, float y, float z, int selec_rotacao)
{

    // Essa escala é comum para todos os objetos
    glm::mat4 Escala = glm::scale(glm::mat4(1.0f), glm::vec3(0.5, 0.5, ZoomZ));
    // Rotação para os objetos que se desejam ser rotacionados
    // Eixo Y
    glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), glm::radians(py_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    // Eixo X
    glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), glm::radians(px_angle), glm::vec3(1.0f, 0.0f, 0.0f));

    // Utiliza o vetor com os dados do cubo
    glBindVertexArray(CUBO);

    // Definindo a posição que cada cubo vai ficar para formar a palavra " UNIFEI " no final. (Como se fosse um LEGO)
    glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));

    // Criando a variavel que vai guardar o calculo da matriz
    glm::mat4 M;

    // Faz a verificação se precisa aplicar a rotação no cubo que sera criado
    if (selec_rotacao == 1)
    {
        // Calculos da matriz com rotação
        M = T * Escala * Rx * Ry;
    }
    else
    {
        // Calculos da matriz sem rotação
        M = T * Escala;
    }

    // Escreve na tela após os calculos da matriz
    unsigned int loc = glGetUniformLocation(program, "M");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(M));
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

// Função responsavel por animar a iluminação
// Essa função alterna a iluminação entre -5.0 e +5.0 (começo e fim da palavra "UNIFEI")
void animacao_da_luz(void)
{
    contador++;
    // Verifica o sentido que a iluminação vai percorrer
    // 1 ele avança positivamente no eixo X
    //-1 ele avança negativamente no eixo X
    if (sentido == 1)
    {
        posicao_camera_X = posicao_camera_X + 0.1;
    }
    else
    {
        posicao_camera_X = posicao_camera_X - 0.1;
    }
    if (contador == 100 && sentido == 1)
    {
        contador = 0;
        sentido = -1;
    }
    else if (contador == 100 && sentido == -1)
    {
        contador = 0;
        sentido = 1;
    }
}

// Função responsavel por calcular o angulo da rotação dos objetos na tela
void rotacao()
{
    // Angulo de rotação para o eixo Y
    py_angle = ((py_angle + py_inc) < 360.0f) ? py_angle + py_inc : 360.0 - py_angle + py_inc;

    // Angulo de rotação para o eixo X
    px_angle = ((px_angle + px_inc) < 360.0f) ? px_angle + px_inc : 360.0 - px_angle + px_inc;

    glutPostRedisplay();
}

// Responsavel por exibir os objetos que aparece na tela
void display()
{
    // Define a cor do fundo
    glClearColor(0.63529, 0.66666, 0.67843, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    // Define a view
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
    unsigned int loc = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));

    // Define a projeção da animação
    glm::mat4 projection = glm::perspective(glm::radians(70.0f), (janela_largura / (float)janela_altura), 0.1f, 100.0f);
    loc = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(projection));

    // Define a cor do objeto para a iluminação.
    loc = glGetUniformLocation(program, "objectColor");
    glUniform3f(loc, 0.0, 0.24745, 0.43921);

    // Define a cor da luz sobre o objeto.
    loc = glGetUniformLocation(program, "lightColor");
    glUniform3f(loc, 1.0, 1.0, 1.0);

    // Define a posição da luz.
    loc = glGetUniformLocation(program, "lightPosition");
    glUniform3f(loc, 0.0, 3.0, 2.0);

    // Função que faz a animação da iluminação do programa
    animacao_da_luz();

    // Define a posição da camera.
    loc = glGetUniformLocation(program, "cameraPosition");
    glUniform3f(loc, posicao_camera_X, 0.0, 0.0);

    // Chamando a função que contem os coordenadas onde os cubos serão posicionados
    coordenadas_do_cubo();

    glutSwapBuffers();
}

// Cria janela com as larguras e alturas da janela definidas
void reshape(int width, int height)
{
    janela_largura = width;
    janela_altura = height;
    glViewport(0, 0, width, height);
    glutPostRedisplay();
}

// Compila os shaders criados pelo programa
void initShaders()
{
    // Solicite um programa e slots de shader da GPU
    program = createShaderProgram(vertex_code, fragment_code);
}

// Responsavel pelos comandos no teclado
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        exit(0);
        // Sai do programa
    case 'q':
    case 'Q':
        exit(0);
    case '1':
        // Interliga os pontos do cubo
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    case '2':
        // Preenche as faces do cubo
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    case 'W':
    case 'w':
        // Aproxima do objeto
        ZoomZ = ZoomZ + 0.2;
        break;
        // Afasta do objeto
    case 'S':
    case 's':
        ZoomZ = ZoomZ - 0.2;
        break;
    }

    glutPostRedisplay();
}

// Contem as coordenadas do vertice do objeto
// desenhado na tela para serem usado no OpenGL

void initData()
{
    // Defina os vértices do Cubo.
    float dados_CUBO[] = {
        //Primeiro triângulo da face frontal
        // coordenada      // cor
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        //Segundo triângulo da face frontal
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        //Primeiro triângulo da face direita.
        0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        //Segundo triângulo da face direita.
        0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        //Primeiro triângulo da face posterior.
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
        //Segundo triângulo da face posterior.
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
        //Primeiro triângulo da face esquerda.
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        //Segundo triângulo da face esquerda.
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        //Primeiro triângulo da face superior.
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
        //Segundo triângulo da face superior.
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
        //Primeiro triângulo da face inferior.
        -0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f,
        //Segundo triângulo da face inferior.
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f};

    // Matriz de vértices.
    glGenVertexArrays(1, &CUBO);
    glBindVertexArray(CUBO);

    // Buffer de vértice
    glGenBuffers(1, &CUBO_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, CUBO_BUFFER);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dados_CUBO), dados_CUBO, GL_STATIC_DRAW);

    // Defina os atributos.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Desvincular objeto de matriz de vértices.
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

// Função responsavel por passar as coordenadas para a criação dos cubos na tela
// Esta função chama a função que cria o cubo na tela, passando as coordenadas corretas
void coordenadas_do_cubo(void)
{
    //------------------------------------------------------------------------------
    /* Informando as coordenadas dos cubos que formam a palavra "UNIFEI" */

    // Cubo 1 --------------------------------

    cria_cubos(-5.0, 0.0, -0.5, 0);

    // Cubo 2 ---------------------------------

    cria_cubos(-4.5, 0.0, -0.5, 0);

    // Cubo 3 ---------------------------------

    cria_cubos(-4.0, 0.0, -0.5, 0);

    // Cubo 4 ---------------------------------

    cria_cubos(-3.5, 0.0, -0.5, 0);

    // Cubo 5 ---------------------------------

    cria_cubos(-5.0, 0.5, -0.5, 0);

    // Cubo 6 ---------------------------------

    cria_cubos(-5.0, 1.0, -0.5, 0);

    // Cubo 7 ---------------------------------

    cria_cubos(-5.0, 1.5, -0.5, 0);

    // Cubo 8 ---------------------------------

    cria_cubos(-5.0, 2.0, -0.5, 0);

    // Cubo 9 ---------------------------------

    cria_cubos(-3.5, 0.5, -0.5, 0);

    // Cubo 10 ---------------------------------

    cria_cubos(-3.5, 1.0, -0.5, 0);

    // Cubo 11 ---------------------------------

    cria_cubos(-3.5, 1.5, -0.5, 0);

    // Cubo 12 ---------------------------------

    cria_cubos(-3.5, 2.0, -0.5, 0);

    /* Cubos de 13 a 25 formão a letra N  */

    // Cubo 13 ---------------------------------

    cria_cubos(-2.5, 2.0, -0.5, 0);

    // Cubo 14 ---------------------------------

    cria_cubos(-2.5, 1.5, -0.5, 0);

    // Cubo 15 ---------------------------------

    cria_cubos(-2.5, 1.0, -0.5, 0);

    // Cubo 16 ---------------------------------

    cria_cubos(-2.5, 0.5f, -0.5, 0);

    // Cubo 17 ---------------------------------

    cria_cubos(-2.5, 0.0, -0.5, 0);

    // Cubo 18 ---------------------------------

    cria_cubos(-2.0, 1.5, -0.5, 0);

    // Cubo 19 ---------------------------------

    cria_cubos(-1.5, 1.0, -0.5, 0);

    // Cubo 20 ---------------------------------

    cria_cubos(-1.0, 0.5, -0.5, 0);

    // Cubo 21 ---------------------------------

    cria_cubos(-1.0, 0.0, -0.5, 0);

    // Cubo 22 ---------------------------------

    cria_cubos(-1.0, 1.0, -0.5, 0);

    // Cubo 23 ---------------------------------

    cria_cubos(-1.0, 1.5, -0.5, 0);

    // Cubo 24 ---------------------------------

    cria_cubos(-1.0, 2.0, -0.5, 0);

    /* Cubos de 25 a 31 formão a letra I  */

    // Cubo 25 ---------------------------------

    cria_cubos(0.0, 0.0, -0.5, 0);

    // Cubo 26 ---------------------------------

    cria_cubos(0.0, 0.5, -0.5, 0);

    // Cubo 27 ---------------------------------

    cria_cubos(0.0, 1.0, -0.5, 0);

    // Cubo 28 ---------------------------------

    cria_cubos(0.0, 1.5, -0.5, 0);

    // Cubo 29 ---------------------------------

    cria_cubos(0.0, 2.0, -0.5, 0);

    // Cubo 31 ---------------------------------

    cria_cubos(0.0, 3.0, -0.5, 1);

    /* Cubos de 32 a 40 formão a letra F  */

    // Cubo 32 ---------------------------------

    cria_cubos(1.0, 0.0, -0.5, 0);

    // Cubo 33 ---------------------------------

    cria_cubos(1.0, 0.5, -0.5, 0);

    // Cubo 34 ---------------------------------

    cria_cubos(1.0, 1.0, -0.5, 0);

    // Cubo 35 ---------------------------------

    cria_cubos(1.5, 1.0, -0.5, 0);

    // Cubo 36 ---------------------------------

    cria_cubos(2.0, 1.0, -0.5, 0);

    // Cubo 37 ---------------------------------

    cria_cubos(1.0, 1.5, -0.5, 0);

    // Cubo 38 ---------------------------------

    cria_cubos(1.0, 2.0, -0.5, 0);

    // Cubo 39 ---------------------------------

    cria_cubos(1.5, 2.0, -0.5, 0);

    // Cubo 40 ---------------------------------

    cria_cubos(2.0, 2.0, -0.5, 0);

    /* Cubos de 41 a 51 forma a letra E  */

    // Cubo 41 ---------------------------------

    cria_cubos(3.0, 0.0, -0.5, 0);

    // Cubo 42 ---------------------------------

    cria_cubos(3.5, 0.0, -0.5, 0);

    // Cubo 43 ---------------------------------

    cria_cubos(4.0, 0.0, -0.5, 0);

    // Cubo 44 ---------------------------------

    cria_cubos(3.0, 0.5, -0.5, 0);

    // Cubo 45 ---------------------------------

    cria_cubos(3.0, 1.0, -0.5, 0);

    // Cubo 46 ---------------------------------

    cria_cubos(3.5, 1.0, -0.5, 0);

    // Cubo 47 ---------------------------------

    cria_cubos(4.0, 1.0, -0.5, 0);

    // Cubo 48 ---------------------------------

    cria_cubos(3.0, 1.5, -0.5, 0);

    // Cubo 49 ---------------------------------

    cria_cubos(3.0, 2.0, -0.5, 0);

    // Cubo 50 ---------------------------------

    cria_cubos(3.5, 2.0, -0.5, 0);

    // Cubo 51 ---------------------------------

    cria_cubos(4.0, 2.0, -0.5, 0);

    /* Cubos de 52 a 57 formão a letra I  */

    // Cubo 52 ---------------------------------

    cria_cubos(5.0, 0.0, -0.5, 0);

    // Cubo 53 ---------------------------------

    cria_cubos(5.0, 0.5, -0.5, 0);

    // Cubo 54 ---------------------------------

    cria_cubos(5.0, 1.0, -0.5, 0);

    // Cubo 55 ---------------------------------

    cria_cubos(5.0, 1.5, -0.5, 0);

    // Cubo 56 ---------------------------------

    cria_cubos(5.0, 2.0, -0.5, 0);

    // Cubo 57 ---------------------------------

    cria_cubos(5.0, 3.0, -0.5, 1);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(janela_largura, janela_altura);
    glutCreateWindow("PROJETO FINAL");
    glewExperimental = GL_TRUE;
    glewInit();
    initData();
    initShaders();
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(rotacao);
    glutMainLoop();
}
