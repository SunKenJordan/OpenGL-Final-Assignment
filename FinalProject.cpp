/*
 Title: COMP390_Final_Project
 Description: A scene looking down a road with snow, fog, lighting and several objects
 Date: April 28, 2026
 Author: Jordan Vangel
 Version: 1.0
*/

/*
 DOCUMENTATION

 Program Purpose:
    Final project for COMP390

 Compile and Execution:
    Using Visual Studio 2022, ensure that freeglut is installed and the freeglut.lib is linked
    Ensure that front, back, up, down, left, right.bmp is in the same folder as this program
    Build using visual studio or by pressing f5

 Notes:

 Classes:
    loadBMP() - Loads a .bmp file and converts it into an OpenGL texture
    initSnow() - Randomly generates starting coordinates for every snowflake
    drawSnow() - Renders points and updates their falling animation
    init() - Sets up background color, depth testing, lighting, and fog
    timer() - Forces a screen redraw every 20ms for the snow animation
    drawPath() - Draws the black road, yellow center line, and sidewalks
    drawStreetLamp() - Renders a lamp post with a base, pole, and emissive bulb
    drawBuilding() - Creates a building with a door, balcony, and grid of windows
    drawBench() - Constructs a park bench with a seat, backrest, and legs
    drawRoadSign() - Draws a diamond shaped warning sign on a metal pole
    drawHydrant() - Creates a fire hydrant using cylinders and spheres
    keyboard() - Handles camera movement for W, A, S, and D keys
    lights() - Sets up the global moonlight and six local street lights
    drawSkybox() - Renders the 6 sided textured background surrounding the scene
    display() - The main rendering loop that draws all objects and environment
    reshape() - Adjusts the viewport and perspective when the window is resized
    menu() - Creates a right-click menu to toggle fog, snow, or exit

 Functions:

*/

/*
 TEST PLAN

 Normal case:
    Draws a scene involving a road, yellow line, houses, lightpoles, snow, fog, benches, a fire hyrdant, and road signs 

 Bad Data case 1
 n/a

 Discussion:
    // Skybox - Some Space Skyboxes! Why not? by Jettelly:  https://jettelly.com/blog/some-space-skyboxes-why-not
    Converted skybox png's to bmp's using paint
*/


#include <windows.h>
#include <GL/freeglut.h>
#include <iostream>
#include <string>

using namespace std;

// Camera position 
float camZ = 5.0f;

// Camera controls
float camX = 0.0f; // Side-to-side position
float lookZ = 0.0f; // Looking point
float speed = 0.5f; // Movement speed

// Fog bool
bool fogEnabled = true; 
// Snow bool
bool snowEnabled = true;

// Sets the max number of snowflakes
const int MAX_SNOW = 500;
// Stores snowflake coordinates
float snowX[MAX_SNOW], snowY[MAX_SNOW], snowZ[MAX_SNOW];


// Texture data
GLubyte* l_texture;
BITMAPFILEHEADER fileheader;
BITMAPINFOHEADER infoheader;
RGBTRIPLE rgb;

// Holds the 6 pictures of the sky box
GLuint skyboxTextures[6];

// Loads a .bmp photo into the program
GLuint loadBMP(string fn) {
    FILE* l_file;
    const char* filename = fn.c_str();

    // Open the bitmap file
    fopen_s(&l_file, filename, "rb");
    if (l_file == NULL) return 0;

    // Read the file header
    fread(&fileheader, sizeof(fileheader), 1, l_file);
    // Seek to the start of the info header
    fseek(l_file, sizeof(fileheader), SEEK_SET);
    // Read image data
    fread(&infoheader, sizeof(infoheader), 1, l_file);

    // Calculate total memory needed
    size_t imageSize = (size_t)infoheader.biWidth * infoheader.biHeight * 4;
    l_texture = (GLubyte*)malloc(imageSize);

    // Loop through every pixel to convert 24-bit BGR to 32-bit RGBA
    int j = 0;
    for (int i = 0; i < infoheader.biWidth * infoheader.biHeight; i++) {
        fread(&rgb, sizeof(rgb), 1, l_file);
        l_texture[j + 0] = (GLubyte)rgb.rgbtRed;
        l_texture[j + 1] = (GLubyte)rgb.rgbtGreen;
        l_texture[j + 2] = (GLubyte)rgb.rgbtBlue;
        l_texture[j + 3] = (GLubyte)255;
        j += 4;
    }
    // Close the file
    fclose(l_file);

    // Texture generation
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, infoheader.biWidth, infoheader.biHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, l_texture);

    // Simple filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Clean up memory
    free(l_texture);

    // Returns texture ID
    return texID;
}

// Initialize random snow flake positions
void initSnow() {
    for (int i = 0; i < MAX_SNOW; i++) {
        // X Between -10 and 10, Y start at various heights , Z long the whole road
        snowX[i] = (rand() % 20) - 10.0f;
        snowY[i] = (rand() % 10);
        snowZ[i] = (rand() % 110) - 100.0f;
    }
}

// Draw snowflakes
void drawSnow() {
    // If snow is disabled, do nothing
    if (!snowEnabled) return;

    // No shading
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(2.0f);

    // Draw snowflakes
    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_SNOW; i++) {
        // Draw snowflake
        glVertex3f(snowX[i], snowY[i], snowZ[i]);

        // Falling animation
        snowY[i] -= 0.02f; // Falling speed

        // Reset if it hits the ground
        if (snowY[i] < 0.0f) {
            snowY[i] = 10.0f;
        }
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

// Initialization
void init() {
    // Dark grey background
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    // Depth
    glEnable(GL_DEPTH_TEST);
    // Lights
    glEnable(GL_LIGHTING);
    // Allows glColor3f to still work for basic colors
    glEnable(GL_COLOR_MATERIAL);
    // Normalize the lights
    glEnable(GL_NORMALIZE);
    // Smooths out the light across surfaces
    glShadeModel(GL_SMOOTH);
    // Turn the fog engine on
    glEnable(GL_FOG);

    // Fog color, dark grey
    float fogColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    glFogfv(GL_FOG_COLOR, fogColor);

    // Use Exponential mode for a smoother look
    glFogi(GL_FOG_MODE, GL_EXP);
    // Adjusts how think the fog is
    glFogf(GL_FOG_DENSITY, 0.05f);
    // Ask OpenGL to use highest quality
    glHint(GL_FOG_HINT, GL_NICEST);
}

// Timer for snowflakes
void timer(int v) {
    // Force a redraw
    glutPostRedisplay();
    // Calls again in 20ms
    glutTimerFunc(20, timer, 0);
}

// Draws the street, sidewalks and yellow line
void drawPath() {
    // Road dimensions
    float roadStart = 10.0f;
    float roadEnd = -100.0f;
    float step = 2.0f;

    // Draws the road in parts, to allow proper lighting on smaller sections
    for (float z = roadStart; z > roadEnd; z -= step) {

        // Dark charcoal road
        glColor3f(0.1f, 0.1f, 0.1f); 
        glBegin(GL_QUADS);
        glVertex3f(-3.0f, 0.01f, 10.0f);
        glVertex3f(3.0f, 0.01f, 10.0f);
        glVertex3f(3.0f, 0.01f, -100.0f);
        glVertex3f(-3.0f, 0.01f, -100.0f);
        glEnd();

        // Yellow line
        glColor3f(1.0f, 0.8f, 0.0f);
        glBegin(GL_QUADS);
        glVertex3f(-0.1f, 0.02f, 10.0f);
        glVertex3f(0.1f, 0.02f, 10.0f);
        glVertex3f(0.1f, 0.02f, -100.0f);
        glVertex3f(-0.1f, 0.02f, -100.0f);
        glEnd();

        // Left sidewalk
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_QUADS);
        // Top surface
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-3.0f, 0.05f, z);
        glVertex3f(-5.0f, 0.05f, z);
        glVertex3f(-5.0f, 0.05f, z - step);
        glVertex3f(-3.0f, 0.05f, z - step);
        // Curb face
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-3.0f, 0.05f, z);
        glVertex3f(-3.0f, 0.00f, z);
        glVertex3f(-3.0f, 0.00f, z - step);
        glVertex3f(-3.0f, 0.05f, z - step);
        glEnd();

        // Right sidewalk
        glBegin(GL_QUADS);
        // Top surface
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(3.0f, 0.05f, z);
        glVertex3f(5.0f, 0.05f, z);
        glVertex3f(5.0f, 0.05f, z - step);
        glVertex3f(3.0f, 0.05f, z - step);
        // Curb face
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(3.0f, 0.05f, z);
        glVertex3f(3.0f, 0.00f, z);
        glVertex3f(3.0f, 0.00f, z - step);
        glVertex3f(3.0f, 0.05f, z - step);
        glEnd();
    }
}

// Draws the street lamps
void drawStreetLamp() {
    // Dark grey
    glColor3f(0.05f, 0.05f, 0.05f);

    // Base 
    glPushMatrix();
    glTranslatef(0.0f, 0.1f, 0.0f);
    glScalef(0.4f, 0.2f, 0.4f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Pole
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 0.0f);
    glRotatef(-90, 1, 0, 0);
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, 0.05, 0.05, 2.0, 12, 1); 
    glPopMatrix();

    // Joint
    glPushMatrix();
    glTranslatef(0.0f, 2.2f, 0.0f);
    glutSolidSphere(0.1, 10, 10);
    glPopMatrix();

    // Arm
    glPushMatrix();
    glTranslatef(0.0f, 2.2f, 0.0f);
    glRotatef(90, 0, 1, 0);
    gluCylinder(quad, 0.03, 0.03, 0.6, 10, 1);
    glPopMatrix();

    // Shade
    glPushMatrix();
    glTranslatef(0.6f, 2.15f, 0.0f);
    glRotatef(-90, 1, 0, 0);
    glutSolidCone(0.3, 0.15, 15, 15);
    glPopMatrix();

    // Bulb
    glPushMatrix();
    glTranslatef(0.6f, 2.05f, 0.0f);

    // Create an emissive yellow
    float emissiveColor[] = { 1.0f, 1.0f, 0.5f, 1.0f };
    // Apply it to the bulb
    glMaterialfv(GL_FRONT, GL_EMISSION, emissiveColor);

    glColor3f(1.0f, 1.0f, 0.6f);
    glutSolidSphere(0.18, 15, 15);

    // Reset emission
    float noEmission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
    glPopMatrix();

    // Clean up memory
    gluDeleteQuadric(quad); 
}

// Draws the buildings, adds variance so all the houses don't look the same
void drawBuilding(float heightScale, float widthScale, bool hasBalcony, float rotation) {
    glPushMatrix();
    glScalef(widthScale, heightScale, widthScale);
    glRotatef(rotation, 0, 1, 0);

    // Body
    glColor3f(0.4f, 0.4f, 0.45f);
    glPushMatrix();
    glTranslatef(0.0f, 0.5f, 0.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Roof
    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f);
    glScalef(1.05f, 0.05f, 1.05f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Door
    glColor3f(0.1f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 0.15f, 0.51f);
    glScalef(0.2f, 0.3f, 0.02f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Windows
    glColor3f(1.0f, 1.0f, 0.7f);
    // Determine the height limit for windows based on balcony
    float maxHeight = hasBalcony ? 0.65f : 0.9f; 

    // Draws windows on building
    for (float y = 0.4f; y < maxHeight; y += 0.25f) {
        for (float x = -0.3f; x <= 0.3f; x += 0.6f) {
            glPushMatrix();
            glTranslatef(x, y, 0.51f);
            glScalef(0.2f, 0.15f, 0.02f);
            glutSolidCube(1.0);
            glPopMatrix();
        }
    }

    // Balcony
    if (hasBalcony) {
        glColor3f(0.3f, 0.3f, 0.3f);
        // Floor
        glPushMatrix();
        glTranslatef(0.0f, 0.65f, 0.65f);
        glScalef(0.8f, 0.05f, 0.3f); 
        glutSolidCube(1.0);
        glPopMatrix();
        
        // Railing
        glPushMatrix();
        glTranslatef(0.0f, 0.75f, 0.79f);
        glScalef(0.8f, 0.2f, 0.02f);
        glutSolidCube(1.0);
        glPopMatrix();
    }

    glPopMatrix();
}

// Draws a bench
void drawBench() {
    // Seat
    glColor3f(0.4f, 0.2f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 0.4f, 0.0f);
    glScalef(1.5f, 0.1f, 0.6f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Back
    glPushMatrix();
    glTranslatef(0.0f, 0.8f, -0.25f);
    glScalef(1.5f, 0.6f, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Legs
    glColor3f(0.1f, 0.1f, 0.1f);
    float legX[4] = { -0.6f, 0.6f, -0.6f, 0.6f };
    float legZ[4] = { -0.2f, -0.2f, 0.2f, 0.2f };
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(legX[i], 0.2f, legZ[i]);
        glScalef(0.1f, 0.4f, 0.1f);
        glutSolidCube(1.0);
        glPopMatrix();
    }
}

// Draws a road sign
void drawRoadSign() {
    GLUquadric* q = gluNewQuadric();

    // Pole
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(q, 0.05, 0.05, 2.0, 10, 10);
    glPopMatrix();

    // Sign
    glColor3f(1.0f, 0.8f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 1.6f, 0.06f);
    glRotatef(45, 0, 0, 1);
    glScalef(0.6f, 0.6f, 0.02f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Back brackets
    glColor3f(0.2f, 0.2f, 0.2f);
    float boltY[3] = { 1.4f, 1.6f, 1.8f };
    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glTranslatef(0.0f, boltY[i], 0.0f);
        glScalef(0.08f, 0.04f, 0.12f);
        glutSolidCube(1.0);
        glPopMatrix();
    }
    gluDeleteQuadric(q);
}

// Draws a fire hydrant
void drawHydrant() {
    GLUquadric* q = gluNewQuadric();

    // Base
    glColor3f(0.6f, 0.1f, 0.1f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(q, 0.25, 0.25, 0.1, 15, 5);
    glPopMatrix();

    // Body
    glPushMatrix();
    glTranslatef(0.0f, 0.1f, 0.0f);
    glRotatef(-90, 1, 0, 0);
    gluCylinder(q, 0.2, 0.2, 0.6, 15, 5);
    glPopMatrix();

    // Cap
    glPushMatrix();
    glTranslatef(0.0f, 0.7f, 0.0f);
    glutSolidSphere(0.2, 15, 15);
    glPopMatrix();

    // Sides
    glColor3f(0.5f, 0.5f, 0.5f); 
    for (float xPos : {-0.15f, 0.15f}) {
        glPushMatrix();
        glTranslatef(xPos, 0.45f, 0.0f);
        glRotatef(90, 0, 1, 0);
        gluCylinder(q, 0.08, 0.08, 0.1, 10, 2);
        glPopMatrix();
    }

    // Front
    glPushMatrix();
    glTranslatef(0.0f, 0.45f, 0.15f);
    glutSolidSphere(0.07, 10, 10);
    glPopMatrix();

    gluDeleteQuadric(q);
}

// Used to move the camera
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'w': // Forward
        camZ -= speed;
        lookZ -= speed;
        break;
    case 's': // Backward
        camZ += speed;
        lookZ += speed;
        break;
    case 'a': // Left
        camX -= speed;
        break;
    case 'd': // Right
        camX += speed;
        break;
    case 27: // Exit with escape key
        exit(0);
        break;
    }
    glutPostRedisplay();
}

// Draws the lights
void lights() {
    // Global moonlight, greyblue from above
    float moonColor[] = { 0.15f, 0.15f, 0.3f, 1.0f };
    float moonPos[] = { 0.0f, 10.0f, 10.0f, 0.0f };

    // Turn on moonlight
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, moonColor);
    glLightfv(GL_LIGHT0, GL_POSITION, moonPos);

    // Yellow street light lights
    float yellowLight[] = { 1.0f, 1.0f, 0.6f, 1.0f };
    float whiteSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    // 6 lights, 3 on each side of the road
    for (int i = 0; i < 3; i++) {
        float zPos = -i * 15.0f;

        // Left lights
        int leftID = GL_LIGHT1 + i;
        float leftPos[] = { -2.8f, 2.0f, zPos, 1.0f };

        glEnable(leftID);
        glLightfv(leftID, GL_DIFFUSE, yellowLight);
        glLightfv(leftID, GL_SPECULAR, whiteSpecular);
        glLightfv(leftID, GL_POSITION, leftPos);

        // Makes the light fade naturally over distance
        glLightf(leftID, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(leftID, GL_LINEAR_ATTENUATION, 0.1f);
        glLightf(leftID, GL_QUADRATIC_ATTENUATION, 0.05f);

        // Right lights
        int rightID = GL_LIGHT4 + i;
        float rightPos[] = { 2.8f, 2.0f, zPos, 1.0f }; // Positioned under right lanterns

        glEnable(rightID);
        glLightfv(rightID, GL_DIFFUSE, yellowLight);
        glLightfv(rightID, GL_SPECULAR, whiteSpecular);
        glLightfv(rightID, GL_POSITION, rightPos);

        // Makes the light fade naturally over distance
        glLightf(rightID, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(rightID, GL_LINEAR_ATTENUATION, 0.1f);
        glLightf(rightID, GL_QUADRATIC_ATTENUATION, 0.05f);
    }
}

// Draws the skybox
void drawSkybox() {
    // No shading, fog or depth
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDepthMask(GL_FALSE);
    glEnable(GL_TEXTURE_2D);
    glColor3f(0.5f, 0.5f, 0.6f);

    glPushMatrix();

    // Center the skybox on the camera
    glTranslatef(camX, 2.0f, camZ);
    // Make it large enough to hold the whole scene
    glScalef(80.0f, 80.0f, 80.0f); 

    // Front
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
    glTexCoord2f(1, 0); glVertex3f(1, -1, -1);
    glTexCoord2f(1, 1); glVertex3f(1, 1, -1);
    glTexCoord2f(0, 1); glVertex3f(-1, 1, -1);
    glEnd();

    // Back
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[1]);
    glBegin(GL_QUADS);
    glTexCoord2f(1, 0); glVertex3f(-1, -1, 1);
    glTexCoord2f(0, 0); glVertex3f(1, -1, 1);
    glTexCoord2f(0, 1); glVertex3f(1, 1, 1);
    glTexCoord2f(1, 1); glVertex3f(-1, 1, 1);
    glEnd();

    // Left
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[2]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-1, -1, 1);
    glTexCoord2f(1, 0); glVertex3f(-1, -1, -1);
    glTexCoord2f(1, 1); glVertex3f(-1, 1, -1);
    glTexCoord2f(0, 1); glVertex3f(-1, 1, 1);
    glEnd();

    // Right
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[3]);
    glBegin(GL_QUADS);
    glTexCoord2f(1, 0); glVertex3f(1, -1, 1);
    glTexCoord2f(0, 0); glVertex3f(1, -1, -1);
    glTexCoord2f(0, 1); glVertex3f(1, 1, -1);
    glTexCoord2f(1, 1); glVertex3f(1, 1, 1);
    glEnd();

    // Top
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[4]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(-1, 1, -1);
    glTexCoord2f(1, 1); glVertex3f(1, 1, -1);
    glTexCoord2f(1, 0); glVertex3f(1, 1, 1);
    glTexCoord2f(0, 0); glVertex3f(-1, 1, 1);
    glEnd();

    // Bottom
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[5]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
    glTexCoord2f(1, 0); glVertex3f(1, -1, -1);
    glTexCoord2f(1, 1); glVertex3f(1, -1, 1);
    glTexCoord2f(0, 1); glVertex3f(-1, -1, 1);
    glEnd();

    glPopMatrix();

    // Reset states so the rest of the world draws correctly
    glDepthMask(GL_TRUE);
    if (fogEnabled) glEnable(GL_FOG);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Camera
    gluLookAt(camX, 2.0, camZ, camX, 2.0, lookZ, 0.0, 1.0, 0.0);
    
    // Skybox
    drawSkybox();
    
    // Lights
    lights();
    
    // Buildings
    for (int i = 0; i < 15; i++) {
        // Space apart
        float zPos = 10.0f - (i * 8.0f);
        // Vary heights between 4 and 5 units
        float h = (i % 3 == 0) ? 5.0f : 4.0f;
        // Width
        float w = 4.0f;
        // Every second building gets a balcony
        bool balcony = (i % 2 == 0);

        // Left side
        glPushMatrix();
        glTranslatef(-6.0f - (w * 0.5f * 0.5f), 0.0f, zPos);
        drawBuilding(h, w, balcony, 90.0f);
        glPopMatrix();

        // Right side
        glPushMatrix();
        glTranslatef(6.0f + (w * 0.5f * 0.5f), 0.0f, zPos);
        drawBuilding(h, w, balcony, -90.0f);
        glPopMatrix();
    }
    
    // Lamps
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < 5; i++) {
        // Space a apart
        float zOffset = -i * 15.0f;

        // Left
        glPushMatrix();
        glTranslatef(-3.4f, 0.05f, zOffset);
        drawStreetLamp();
        glPopMatrix();

        // Right
        glPushMatrix();
        glTranslatef(3.4f, 0.05f, zOffset);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        drawStreetLamp();
        glPopMatrix();
    }
    
    // Benches
    for (int i = 0; i < 4; i++) {
        // Space apart
        float zOffset = -7.5f - (i * 15.0f); 

        if (i % 2 == 0) {
            // Left
            glPushMatrix();
            glTranslatef(-4.2f, 0.05f, zOffset); // On the left sidewalk
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);   // Face toward the road
            drawBench();
            glPopMatrix();
        }
        else {
            // Right
            glPushMatrix();
            glTranslatef(4.2f, 0.05f, zOffset);  // On the right sidewalk
            glRotatef(270.0f, 0.0f, 1.0f, 0.0f);  // Face toward the road
            drawBench();
            glPopMatrix();
        }
    }

    // Sign 
    glPushMatrix();
    glTranslatef(3.3f, 0.0f, -4.4f); 
    drawRoadSign();
    glPopMatrix();

    // Sign 2
    glPushMatrix();
    glTranslatef(-3.3f, 0.0f, -4.4f); 
    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    drawRoadSign();
    glPopMatrix();

    // Fire Hydrant
    glPushMatrix();
    glTranslatef(3.3f, 0.05f, -10.0f);
    drawHydrant();
    glPopMatrix();

    // Road, sidewalk and yellow line
    drawPath();

    // Snow
    drawSnow(); 

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (float)h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// Menu items, turn off fog and snow
void menu(int value) {
    switch (value) {
    case 1: // Toggle Fog
        fogEnabled = !fogEnabled;
        if (fogEnabled) glEnable(GL_FOG);
        else glDisable(GL_FOG);
        break;
    case 2: // Toggle Snow
        snowEnabled = !snowEnabled;
        break;
    case 3: // Exit
        exit(0);
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    // Initialization
    glutInit(&argc, argv);
    initSnow(); 
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("COMP390 Final Project");
    init();
    glutKeyboardFunc(keyboard);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    // Menu
    glutCreateMenu(menu);
    glutAddMenuEntry("Toggle Fog", 1);
    glutAddMenuEntry("Toggle Snow", 2); 
    glutAddMenuEntry("Exit", 3);        

    // Attach the menu to the right mouse button
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    // Loads the skybox textures
    skyboxTextures[0] = loadBMP("front.bmp");
    skyboxTextures[1] = loadBMP("back.bmp");
    skyboxTextures[2] = loadBMP("left.bmp");
    skyboxTextures[3] = loadBMP("right.bmp");
    skyboxTextures[4] = loadBMP("up.bmp");
    skyboxTextures[5] = loadBMP("down.bmp");

    // Snowflake timer
    timer(20);

    glutMainLoop();
    return 0;
}