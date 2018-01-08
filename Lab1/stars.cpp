// Introduction lab that covers:
// * C++
// * SDL
// * 2D graphics
// * Plotting pixels
// * Video memory
// * Color representation
// * Linear interpolation
// * glm::vec3 and std::vector

#include "stars.hpp"
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
const float f = SCREEN_HEIGHT/2;
SDL_Surface* screen;
vector<vec3> stars( 1000 );
int t ;

// --------------------------------------------------------
// FUNCTION DECLARATIONS

void Draw();
void Update();

void Interpolate(float a, float b, vector<float>& result);
void Interpolate(vec3 a, vec3 b , vector<vec3>& result);
// --------------------------------------------------------
// FUNCTION DEFINITIONS

int main( int argc, char* argv[] )
{
    //initialize screen
    screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
    //render loop

    //(2) starting stars position
    for(unsigned int i = 0; i < stars.size(); i++){
        stars[i].x = 2 * (float(rand()) / float(RAND_MAX))-1;  // -1 <= x <= 1
        stars[i].y = 2 * (float(rand()) / float(RAND_MAX))-1;  // -1 <= y <= 1
        stars[i].z = (float)rand() / (float)RAND_MAX;          //  0 <= z <= 1
    }

    while( NoQuitMessageSDL() )
    {
        Update();
        Draw();
    }
    SDL_SaveBMP( screen, "screenshot.bmp" );
    return 0;
}

void Draw()
{
    // Fill the screen with black pixels
    SDL_FillRect(screen, 0, 0);

    if( SDL_MUSTLOCK(screen) )
        SDL_LockSurface(screen);

    for( size_t s=0; s<stars.size(); ++s ){
        // (3) Projection - Pinhole Camera
        float u = f * (stars[s].x / stars[s].z) + SCREEN_WIDTH /2 ;
        float v = f * (stars[s].y / stars[s].z) + SCREEN_HEIGHT /2 ;
        vec3 color = 0.3f * vec3(1,1,1) / (stars[s].z * stars[s].z); // brightness inversely proportional to the squared distance
        //vec3 color (1, 1, 1);
        PutPixelSDL(screen,u,v,color);
    }

    if( SDL_MUSTLOCK(screen) )
        SDL_UnlockSurface(screen);

    //update screen
    SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

void Update()
{
    int t2 = SDL_GetTicks();
    float dt = float(t2-t);
    t = t2;

    for( int s=0; s<stars.size(); ++s ){
        stars[s].z -= 0.0003*(float)dt;
        if( stars[s].z <= 0 )
            stars[s].z += 1;
        if( stars[s].z > 1 )
            stars[s].z -= 1;
    }
}






