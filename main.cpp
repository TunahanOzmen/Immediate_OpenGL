#include <iostream>
#include "Angel.h"
#define PI 3.14159265358979324
#define UNIT 25
#define WD_DIST 30
#define FPS 60
#define SPEEDCOEF 2

/* TO-DO
 * Pause anında olum animasyonu ve bombanin dusmeyi birakmasi lazim
 * README yaz, destruction animasyonu ekstra su an. onu yaz
 * oyun bitisi icin biseyler
 * resize yapildiginda sinirlar da genisletilsin. Hiz artsin/azalsin orana gore, buyuklukler artsin azalsin
 * s ile konumlari da yazilsin alienlarin
 * koselere sikisma sorununa bak

 * Skor algoritmasi eklencecek, ekrana yazilacak (ok)
 * Bomba sol buton ile birakilacak -- Bomba hakkı koy boylece array size'ı da limitlemis olursun (ok)
 * hem x hem y icin ayri ayri hizlar olsun (ok)
 * q oyundan cikar (ok)
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
const int scores[5] = {-4000, -2000, -1000, 1000, 2000};
GLsizei wh = 750, ww = 750; /* initial window size */
bool gameStatePause = false;
bool gameStateOver = false;
int score = 0;
static void endGameDisplay();


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
    int isTouched;
    //If alien moves to right then = 1, otherwise = 0. Same for moveUp
    int moveRight, moveUp;
    //If alien and bomb touched and 1 second passed
    int isDead;
};
struct Bomb{
    float color[3];
    float size;
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
        aliens[i].speedx  = (15 + rand() % 16)/20.0*SPEEDCOEF ; //[15, 30]
        aliens[i].speedy  = (15 + rand() % 16)/20.0*SPEEDCOEF ; //[15/20, 30/20]
        if(aliens[i].moveRight == 0)
            aliens[i].speedx *= -1;
        if(aliens[i].moveUp == 0)
            aliens[i].speedy *= -1;
    }
}

void resizeBomb(int bombIdx){
    bombs[bombIdx].size = r[bombs[bombIdx].level];
    bombs[bombIdx].points[0][0] = bombs[bombIdx].x + bombs[bombIdx].size/2 ; bombs[bombIdx].points[0][1] = bombs[bombIdx].y + bombs[bombIdx].size/2; //sag ust
    bombs[bombIdx].points[1][0] = bombs[bombIdx].x - bombs[bombIdx].size/2 ; bombs[bombIdx].points[1][1] = bombs[bombIdx].y + bombs[bombIdx].size/2; //sol ust
    bombs[bombIdx].points[2][0] = bombs[bombIdx].x - bombs[bombIdx].size/2 ; bombs[bombIdx].points[2][1] = bombs[bombIdx].y - bombs[bombIdx].size/2; //sol alt
    bombs[bombIdx].points[3][0] = bombs[bombIdx].x + bombs[bombIdx].size/2 ; bombs[bombIdx].points[3][1] = bombs[bombIdx].y - bombs[bombIdx].size/2; //sag alt

    bombs[bombIdx].color[0] = COLORS[bombs[bombIdx].level][0];
    bombs[bombIdx].color[1] = COLORS[bombs[bombIdx].level][1];
    bombs[bombIdx].color[2] = COLORS[bombs[bombIdx].level][2];
}
void createBomb(int x, int y){
    y = wh- y;
    bombs[numBombs].x = x;
    bombs[numBombs].y = y;
    bombs[numBombs].level = 0;
    bombs[numBombs].size = r[bombs[numBombs].level];
    resizeBomb(numBombs);
    numBombs++;
    if(numBombs >= 20){
        gameStateOver = true;
        glutDisplayFunc(endGameDisplay);
    }

}

void dropBomb(int id){
    for (int i = 0 ; i < numBombs ; i++){
        if(bombs[i].level < 5){
            bombs[i].level += 1;
            resizeBomb(i);
        }
    }
    glutTimerFunc(1000 , dropBomb, 0);
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

void bitMapString(float x, float y, std::string s){
    int i = 0;
    glRasterPos2f(x, y);
    while(s[i] != '\0'){
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, s[i++]);
    }
    i = 0;
    std::string remainingBombs = "Remaining Bombs: " + std::to_string(20-numBombs);
    glRasterPos2f(x, y-25);
    while(remainingBombs[i] != '\0'){
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, remainingBombs[i++]);
    }
}

//----------------------------------------------------------------------------
int checkCollision(Alien* alien){ //returns the bomb that collision happened with
    if(numBombs == 0)
        return -1;
    if(alien->isTouched == 1)
        return 1;
    for (int vertex = 0 ; vertex < NumVertices ; vertex++){//for every vertex
        for (int bomb=0 ; bomb < numBombs ; bomb++) { //check every bomb
            if(alien->level != bombs[bomb].level)
                continue;
            int alienx = alien->points[vertex][0]; int alieny = alien->points[vertex][1];
            int leftUpy = bombs[bomb].points[1][1];
            int leftDownx = bombs[bomb].points[2][0];
            int leftDowny = bombs[bomb].points[2][1];
            int rightDownx = bombs[bomb].points[3][0];
            //int leftUpx = bombs[bomb].points[1][0]; int rightDowny = bombs[bomb].points[3][1];
            if((alienx < rightDownx) && (alienx > leftDownx) && (alieny < leftUpy) && (alieny > leftDowny)){
                alien->isTouched = 1;
                alien->speedx = 0;
                alien->speedy = 0;
                score += scores[alien->level];
                return bomb;
            }
        }
    }
    return -1;
}

void killAlien(int alienIdx){
    aliens[alienIdx].isDead = 1;
}

void killingAnimation(int alienIdx){
    //
    if(aliens[alienIdx].isDead == 1)
        return;
    if((aliens[alienIdx].color[0] <= 0)&&(aliens[alienIdx].color[1] <= 0)&&(aliens[alienIdx].color[2] <= 0))
        killAlien(alienIdx);
    aliens[alienIdx].color[0] -= 0.01;
    aliens[alienIdx].color[1] -= 0.01;
    aliens[alienIdx].color[2] -= 0.01;
    glutTimerFunc(300, killingAnimation, alienIdx);
}

void display( void )
{
    glClear(GL_COLOR_BUFFER_BIT);
    for (int i = 19 ; i >= 0 ; i--){
        //bomba i'nin 5 moduna gore ayni leveldaysa cizdir.
        for (int j = 0 ; j < numBombs ; j++ ){
            if(bombs[j].level == i/4){ //gerekli sirada cizmesi icin.... 4?
                glColor3f(bombs[j].color[0],bombs[j].color[1],bombs[j].color[2]); //
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
        if(checkCollision(&aliens[i]) != -1){ //it means collision
            killingAnimation(i);
            glutTimerFunc(1000, killAlien, i); //after 1 second kill alien
        }
        glBegin(GL_POLYGON);
        glColor3f(aliens[i].color[0],aliens[i].color[1],aliens[i].color[2]);
            for (int j = 0; j < NumVertices ; j++){
                glVertex2f(aliens[i].points[j][0],aliens[i].points[j][1]);
            }
        glEnd();
    }
    std::string out = "Score: " + std::to_string(score);
    bitMapString(0,wh-20,out);
    glFlush();
}

void endGameDisplay( void ){
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.5, 0.2, 0.4, 5.0);

    std::string output = "GAME OVER \t YOUR FINAL SCORE:" + std::to_string(score)+" \n";
    int i = 0,x = ww/2.5, y=wh/2;
    std::string s = "        GAME OVER";
    glRasterPos2f(x, y);
    while(s[i] != '\0'){
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, s[i++]);
    }

    glRasterPos2f(x, y-20);
    i = 0;
    s = "YOUR FINAL SCORE:" + std::to_string(score);
    while(s[i] != '\0'){
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, s[i++]);
    }

    glFlush();
}
//----------------------------------------------------------------------------

void mouse( int button, int state, int x, int y )
{
    if ( state == GLUT_DOWN ) {
        switch( button ) {
            case GLUT_LEFT_BUTTON:
                if(numBombs <= 19)
                    createBomb(x, y);
                break;
        }
    }
}

//----------------------------------------------------------------------------
void checkDirection(){
    for (int i = 0 ; i < 20 ; i++){
        if(aliens[i].isDead != 0 || aliens[i].isTouched != 0)
            continue;
        for (int j = 0 ; j < NumVertices ; j++){
            if(aliens[i].points[j][0] < WD_DIST || aliens[i].points[j][0] > (ww-WD_DIST) ){
                aliens[i].speedx *= -1;
                if(aliens[i].moveRight == 0)
                    aliens[i].moveRight = 1;
                else
                    aliens[i].moveRight = 0;
                break;
            }
            if(aliens[i].points[j][1] < WD_DIST || aliens[i].points[j][1] > (wh-WD_DIST) ){
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
    glutTimerFunc(1000 , dropBomb, 0);

    glutMainLoop();


    return 0;
}
