// Introduction lab that covers:
// * C++
// * SDL
// * 2D graphics
// * Plotting pixels
// * Video memory
// * Color representation
// * Linear interpolation
// * glm::vec3 and std::vector

#include "SDL.h"
#include <iostream>
#include "glm/glm.hpp"
#include <vector>
#include "SDLauxiliary.h"

using namespace std;
using glm::vec3;

// --------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Surface* screen;

// --------------------------------------------------------
// FUNCTION DECLARATIONS

void Draw();

void Interpolate(float a, float b, vector<float>& result);
void Interpolate(vec3 a, vec3 b , vector<vec3>& result);
// --------------------------------------------------------
// FUNCTION DEFINITIONS

int main( int argc, char* argv[] )
{
    //initialize screen
    screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
    //render loop

    while( NoQuitMessageSDL() )
    {
        Draw();
    }
    SDL_SaveBMP( screen, "screenshot.bmp" );
    return 0;
}

void Interpolate(vec3 a, vec3 b, vector<vec3>& result) {
    // Handle the special case when result.size is 1
    if (result.size() == 1) {
        result[0].x = (a.x + b.x) / 2;
        result[0].y = (a.y + b.y) / 2;
        result[0].z = (a.z + b.z) / 2;
    } else {
        // Calculate the vector for each step
        float steps = result.size() - 1;
        for(unsigned int i = 0; i < result.size(); i++){
            result[i].x = a.x + (i * ((b.x-a.x)/steps));
            result[i].y = a.y + (i * ((b.y-a.y)/steps));
            result[i].z = a.z + (i * ((b.z-a.z)/steps));
        }
    }
}

void Draw()
{
    // colors in the corners
    vec3 topLeft(1,0,0);     // red
    vec3 topRight(0,0,1);    // blue
    vec3 bottomRight(0,1,0); // green
    vec3 bottomLeft(1,1,0);  // yellow
    
    // Initialize the colors on the sides and interpolate all the pixels between the corners
    vector<vec3> leftSide(SCREEN_HEIGHT);
    vector<vec3> rightSide(SCREEN_HEIGHT);
    Interpolate(topLeft, bottomLeft, leftSide);
    Interpolate(topRight, bottomRight, rightSide);

    if( SDL_MUSTLOCK(screen) )
        SDL_UnlockSurface(screen);

    // Initialize the vector that will contain each row with pixels on the screen
    vector<vec3> final(SCREEN_WIDTH);


    // Loop over each row in the screen
    for( int y=0; y<SCREEN_HEIGHT; ++y )
    {
        // Interpolate from leftside to rightside for the current row
        Interpolate(leftSide[y], rightSide[y], final);

        // Update the color for each pixel in the row
        for( int x=0; x<SCREEN_WIDTH; ++x )
        {
            vec3 color(final[x].x,final[x].y, final[x].z);
            PutPixelSDL( screen, x, y, color );
        }
    }

    //update screen
    SDL_UpdateRect( screen, 0, 0, 0, 0 );
}


