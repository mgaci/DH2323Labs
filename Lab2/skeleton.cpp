#include <iostream>
#include "glm/glm.hpp"
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include <math.h>
#include <algorithm>

using namespace std;
using glm::vec3;
using glm::mat3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 100;
const int SCREEN_HEIGHT = 100;
SDL_Surface* screen;
int t;
vector<Triangle> triangles;
vec3 cameraPos(0, 0, -2);
float focalLength = SCREEN_WIDTH / 2;
mat3 R;
float yaw = 0.0f;
vec3 lightPos( 0, -0.5, -0.7 );
vec3 lightColor = 10.f * vec3( 1, 1, 1 );
vec3 indirectLight = 0.5f*vec3( 1, 1, 1 );
float threshold = 0.001f;

//---------------------------------------------------------------------------
// Structures
struct Intersection
{
    vec3 position;
    float distance;
    int triangleIndex;
};

// ----------------------------------------------------------------------------
// FUNCTIONS
void Update();
void Draw();
bool ClosestIntersection(
                         vec3 start,
                         vec3 dir,
                         const vector<Triangle>& triangles,
                         Intersection& closestIntersection
                         );
void RotateCamera();
vec3 DirectLight( const Intersection& i );


int main( int argc, char* argv[] )
{
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
    focalLength = SCREEN_HEIGHT / 2;
    LoadTestModel( triangles);

	t = SDL_GetTicks();	// Set start value for timer.

	while( NoQuitMessageSDL() )
	{
		Update();
		Draw();
	}

	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Update() {
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2-t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;
    
    Uint8* keystate = SDL_GetKeyState(0);
    
    vec3 right( R[0][0], R[0][1], R[0][2] );
    vec3 down( R[1][0], R[1][1], R[1][2] );
    vec3 forward( R[2][0], R[2][1], R[2][2] );
    
    if ( keystate[SDLK_UP] )
    {
        // Move camera forward
        cameraPos += forward;
    }
    if( keystate[SDLK_DOWN] )
    {
        // Move camera backward
        cameraPos -= forward;
    }
    if( keystate[SDLK_LEFT] )
    {
        // Rotate camera to the left
        yaw += 0.2f;
        RotateCamera();
    }
    if( keystate[SDLK_RIGHT] )
    {
        // Rotate camera to the right
        yaw -= 0.2f;
        RotateCamera();
    }
    if( keystate[SDLK_w] ) {
        lightPos -= forward;
    }
    if( keystate[SDLK_s] ) {
        lightPos += forward;
    }
    if( keystate[SDLK_a] ) {
        lightPos -= right;
    }
    if( keystate[SDLK_d] ) {
        lightPos += right;
    }
    if( keystate[SDLK_q] ) {
        lightPos -= down;
    }
    if( keystate[SDLK_e] ) {
        lightPos += down;
    }
}

void RotateCamera () {
    // https://en.wikipedia.org/wiki/Rotation_matrix
    R = mat3( cos(yaw),  0,  sin(yaw),
                     0,  1,         0,
             -sin(yaw),  0,  cos(yaw));
}

void Draw() {
	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);
    
    Intersection closest;

	for( int y=0; y<SCREEN_HEIGHT; y++ )
	{
		for( int x=0; x<SCREEN_WIDTH; x++ )
		{
            // (24): d = (x-W/2, y-H/2, f)
            vec3 d( x - (SCREEN_WIDTH / 2), y - (SCREEN_HEIGHT / 2), focalLength );
            d = glm::normalize(d);
            d = R * d;
            
            bool intersectionFound = ClosestIntersection(cameraPos, d, triangles, closest);
            if (intersectionFound)
            {
                // (30): R = ρ * T = ρ * (D+N)
                vec3 ro = triangles[closest.triangleIndex].color;
                vec3 DN = DirectLight(closest) + indirectLight;
                PutPixelSDL(screen,x,y, ro * DN);
            }
            else {
                PutPixelSDL(screen, x, y, vec3(0,0,0));
            }
            SDL_UpdateRect(screen, 0, 0, 0, 0);
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

bool ClosestIntersection(vec3 start,
                         vec3 dir,
                         const vector<Triangle>& triangles,
                         Intersection& closestIntersection) {
    int index = 0;
    float m = std::numeric_limits<float>::max();
    
    for (int i = 0; i < triangles.size(); i++)        {
        Triangle triangle = triangles[i];
        
        vec3 v0 = triangle.v0;
        vec3 v1 = triangle.v1;
        vec3 v2 = triangle.v2;
        vec3 e1 = v1 - v0;
        vec3 e2 = v2 - v0;
        vec3 b = start - v0;
        mat3 A(-dir, e1, e2);
        vec3 x = glm::inverse(A) * b;
        
        // x = (t u v)T
        float t = x.x;
        float u = x.y;
        float v = x.z;
        
        // (7, 8, 9, 11)
        if (0 < u && 0 <= v && u+v <= 1 && 0 <= t) {
            if (t < m) {
                m = t;
                index = i;
            }
        }
    }
    
    if (index >= 0){
        closestIntersection.triangleIndex = index;
        // 5.1 Direct Shadow
        closestIntersection.position = start + (m * dir);
        closestIntersection.distance = m;
        return true;
    }
    
    return false;
}

vec3 DirectLight( const Intersection& i ) {
    float r = glm::length(i.position - lightPos);
    
    // A = 4πr2
    float A = 4 * M_PI * pow (r, 2);
    
    // 5.1 Direct Shadow
    Intersection shadow;
    vec3 dir = glm::normalize(i.position - lightPos);
    bool intersectionFound = ClosestIntersection(lightPos, dir, triangles, shadow);
    if (intersectionFound) {
        if (shadow.distance < r - threshold)
            return vec3(0,0,0);
    }
    
    vec3 rVector = glm::normalize(lightPos - i.position);
    vec3 nVector = triangles[i.triangleIndex].normal;

    // Dot Production
    float rn = glm::dot(rVector, nVector);
    
    // (27): D = B max(r̂ . n̂, 0) = (P max (r̂ . n̂, 0))/4πr2
    vec3 D = lightColor * max(rn, 0.0f) / A;
    return D;
}
