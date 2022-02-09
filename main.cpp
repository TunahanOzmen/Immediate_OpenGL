#include <iostream>
#include <stdio.h>
#include "Angel.h"
#define PI 3.14159265358979324
#define UNIT 25
#define WD_DIST 30
#define FPS 120
/* TO-DO
 * reshape yapildiginda sinirlar da genisletilsin. Hiz artsin/azalsin orana gore, buyuklukler artsin azalsin
 * Skor algoritmasi eklencecek, ekrana yazilacak
 * Bomba sol buton ile birakilacak -- Bomba hakkı koy boylece array size'ı da limitlemis olursun
 * * hem x hem y icin ayri ayri hizlar olsun (ok)
 * * q oyundan cikar (ok)
 * p oyunu durdurur veya devam ettirir (ok)
 * s, oyun pause edildiyse tek bir basamak ileri gider  (ok)
 */
const int NumVertices = 30; //Each circle composed of 30 vertices
const float r[5] = {UNIT+8,UNIT+6,UNIT+4,UNIT+2,UNIT}; //alienRadius, levela gore belirlenebilir aslinda
const float COLORS[5][3] = {
        {1.0, 1.0, 0.0},              //LIGHT YELLOW
        {1.000, 0.843, 0.000},        //YELLOW
        {0.722, 0.525, 0.043},        //DARK YELLOW
        {1, 0.000, 1},                //PURPLE
        {0.5, 0.0, 0.5},              //DARK PURPLE
};

GLsizei wh = 750, ww = 750; /* initial window size */
bool gameStatePause = false;
struct Alien {
    float diameter;
    //Vertices to draw a circle
    float points[NumVertices][2];
    //Each alien has a fixed level, changeable location (x, y)
    int level, x, y;
    //Each alien has a color according to fixed level
    float color[3];
    //speed of the alien (in the x-y axes)
    float speedx, speedy;
    //If alien and bomb touched then = 1, otherwise(default) = 0
    int isTouched=0;
    //If alien moves to right then = 1, otherwise = 0. Same for moveUp
    int moveRight, moveUp;
    //If alien and bomb touched and 1 second passed
    int isDead;
};
struct Bomb{
    int size;
    int level, x, y;
    float points[4][2]; //4 vertex 2 axes
};
Alien aliens[20]; //5 levels, 4 aliens each
Bomb bombs[20];
int numBombs = 0;
void createAliens(){
    srand(time(NULL));
    for (int i = 0 ; i < 20 ; i++){
        //level ver moduna gore, sonrasında da renk ata
        aliens[i].level = i / 4;
        aliens[i].diameter = r[aliens[i].level];
        for(int j = 0; j < NumVertices; ++j)
        {
            aliens[i].color[0] = COLORS[i/4][0];
            aliens[i].color[1] = COLORS[i/4][1];
            aliens[i].color[2] = COLORS[i/4][2];
        }
        //Select random x and y in the window
        aliens[i].x = 50 + rand() % (ww-100); aliens[i].y = 50 + rand() % (wh-100);
        //A circle around the x and y
        float t = 0; // Angle parameter.
        for(int j = 0; j < NumVertices; ++j)
        {
            aliens[i].points[j][0] = (float)(aliens[i].x+aliens[i].diameter*cos(t));
            aliens[i].points[j][1] = (float)(aliens[i].y+aliens[i].diameter*sin(t));
            t += 2 * PI / NumVertices;
        }

        //Default Attributes
        aliens[i].isTouched  = 0;
        aliens[i].isDead = 0;
        aliens[i].moveRight  = rand() % 2;
        aliens[i].moveUp  = rand() % 2;
        aliens[i].speedx  = (15 + rand() % 16)/20.0 ; //[15, 30]
        aliens[i].speedy  = (15 + rand() % 16)/20.0 ; //[15/20, 30/20]
        if(aliens[i].moveRight == 0)
            aliens[i].speedx *= -1;
        if(aliens[i].moveUp == 0)
            aliens[i].speedy *= -1;
    }
}

void resizeBomb(int bombIdx){
    bombs[bombIdx].size = r[bombs[bombIdx].level]/2;
    bombs[bombIdx].points[0][0] = bombs[bombIdx].x + bombs[bombIdx].size/2 ; bombs[bombIdx].points[0][1] = bombs[bombIdx].y + bombs[bombIdx].size/2; //sag ust
    bombs[bombIdx].points[1][0] = bombs[bombIdx].x - bombs[bombIdx].size/2 ; bombs[bombIdx].points[1][1] = bombs[bombIdx].y + bombs[bombIdx].size/2; //sol ust
    bombs[bombIdx].points[2][0] = bombs[bombIdx].x - bombs[bombIdx].size/2 ; bombs[bombIdx].points[2][1] = bombs[bombIdx].y - bombs[bombIdx].size/2; //sol alt
    bombs[bombIdx].points[3][0] = bombs[bombIdx].x + bombs[bombIdx].size/2 ; bombs[bombIdx].points[3][1] = bombs[bombIdx].y - bombs[bombIdx].size/2; //sag alt
}
void createBomb(int x, int y){
    y = wh- y;
    bombs[numBombs].x = x;
    bombs[numBombs].y = y;
    bombs[numBombs].level = 0;
    bombs[numBombs].size = r[bombs[numBombs].level]/2;
    resizeBomb(numBombs);
    numBombs++;
}
/* reshaping routine called whenever window is resized
or moved */

void reshape(GLsizei w, GLsizei h)
{
    /* adjust clipping box */

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble)w, 0.0, (GLdouble)h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* adjust viewport and clear */

    glViewport(0, 0, w, h);

    /* set global size for use by drawing routine */

    ww = w;
    wh = h;
}

void init(){
    glViewport(0, 0, ww, wh);

    /* Pick 2D clipping window to match size of screen window
    This choice avoids having to scale object coordinates
    each time window is resized */

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble)ww, 0.0, (GLdouble)wh);

    /* set clear color to black and clear window */

    //glClearColor(1.0, 1.0, 1.0, 1.0);
    glClearColor(0,0,0,0);
    //glClear(GL_COLOR_BUFFER_BIT);
    createAliens();
    glFlush();
}

void bitMapString(float x, float y, char s[]){
    int i = 0;
    glRasterPos2f(x, y);
    while(s[i] != '\0'){
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, s[i++]);
    }
}

void display( void )
{
    glClear(GL_COLOR_BUFFER_BIT);
    for (int i = 19 ; i >= 0 ; i--){
        //bomba i'nin 5 moduna gore ayni leveldaysa cizdir.
        for (int j = 0 ; j < numBombs ; j++ ){
            if(bombs[j].level == i%5){ //gerekli sirada cizmesi icin
                glColor3f(1,1,1); //
                glBegin(GL_POLYGON);
                for (int vertex = 0 ; vertex < 4 ; vertex++){
                    glVertex2f(bombs[j].points[vertex][0],bombs[j].points[vertex][1]);
                }
                glEnd();
            }
        }
        if(aliens[i].isDead == 1){
            continue;
        }
        glBegin(GL_POLYGON);
        glColor3f(aliens[i].color[0],aliens[i].color[1],aliens[i].color[2]);
            for (int j = 0; j < NumVertices ; j++){
                glVertex2f(aliens[i].points[j][0],aliens[i].points[j][1]);
            }
        glEnd();
    }
    bitMapString(0,wh-20,"Deneme"); //deneme yerinde string donen bir skor hesaplama fonksiyonuna callback olmali..
                                            // ya da vurus yaptikca bir degisken guncellenirse daha verimli olabilir.
    glFlush();
}

//----------------------------------------------------------------------------

void mouse( int button, int state, int x, int y )
{
    if ( state == GLUT_DOWN ) {
        switch( button ) {
            case GLUT_LEFT_BUTTON:
                if(numBombs <= 20)
                    createBomb(x, y);
                break;
            //case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;  break;
            //case GLUT_RIGHT_BUTTON:   Axis = Zaxis;  break;
        }
    }
}

//----------------------------------------------------------------------------
void checkDirection(){
    for (int i = 0 ; i < 20 ; i++){
        for (int j = 0 ; j < NumVertices ; j++){
            if(aliens[i].points[j][0] < WD_DIST || aliens[i].points[j][0] > (wh-WD_DIST) ){
                aliens[i].speedx *= -1;
                if(aliens[i].moveRight == 0)
                    aliens[i].moveRight = 1;
                else
                    aliens[i].moveRight = 0;
                break;
            }
            if(aliens[i].points[j][1] < WD_DIST || aliens[i].points[j][1] > (ww-WD_DIST) ){
                aliens[i].speedy *= -1;
                if(aliens[i].moveUp == 0)
                    aliens[i].moveUp = 1;
                else
                    aliens[i].moveUp = 0;
                break;
            }
        }
    }
}

void idle( int id )
{
    if(gameStatePause)
        return;
    checkDirection();
    for (int i = 0 ; i < 20 ; i++){
        for (int j = 0 ; j < NumVertices ; j++){
            aliens[i].points[j][0] += aliens[i].speedx;
            aliens[i].points[j][1] += aliens[i].speedy;
            aliens[i].x += aliens[i].speedx;
            aliens[i].y += aliens[i].speedy;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(1000 / FPS, idle, 0);
}

//----------------------------------------------------------------------------

void keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
        case 033: // Escape Key
        case 'q': case 'Q':
            exit( EXIT_SUCCESS );
            break;
        case 'p': case 'P':
            gameStatePause = !gameStatePause;
            idle(1);
            break;
        case 's': case 'S':
            if(gameStatePause){
                gameStatePause = !gameStatePause;
                idle(2);
                gameStatePause = !gameStatePause;
            }
            break;
    }
}
//----------------------------------------------------------------------------
int main( int argc, char **argv ) {
    //std::cout << "Hello, World!" << std::endl;

    glutInit( &argc, argv );
    glutInitContextVersion(3,2); // initialize the OpenGL Context to 3.2.
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode( GLUT_RGBA | GLUT_SINGLE); // GLUT_DOUBLE | GLUT_DEPTH
    glutInitWindowSize( ww, wh );

    //create graphics window
    glutCreateWindow( "The Game" );

    //include the following statement due to an error in GLEW library
    //glewExperimental = GL_TRUE;

    glewInit();

    init();

    glutReshapeFunc(reshape);
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
    glutMouseFunc( mouse );
    //glutIdleFunc( idle );
    glutTimerFunc(1000 / FPS, idle, 0);

    glutMainLoop();


    return 0;
}
