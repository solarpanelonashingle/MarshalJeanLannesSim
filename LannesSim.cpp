#include <SDL.h>
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <vector>
#include <iostream>
#include "Hitbox.h"

//Screen dimension
const int SCREEN_WIDTH = 1440;
const int SCREEN_HEIGHT = 900;

//global declarations (yikes)
bool init();
bool loadMedia( std::string path );
void close();
SDL_Surface* loadSurface( std::string path );

SDL_Window* window = NULL;
    
SDL_Surface* screenSurface = NULL;

SDL_Surface* backgroundSurface = NULL;
SDL_Surface* imageSurface = NULL;
SDL_Surface* ballSurface = NULL;

bool init()
{
    bool success = true;
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL machine broken.. SDL Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Create window
        window = SDL_CreateWindow( "LannesSimulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( window == NULL )
        {
            printf( "The window could not be drawn SDL Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //Get window's surface
            screenSurface = SDL_GetWindowSurface( window );
        }
    }
    return success;
}

bool loadMedia( std::string path, SDL_Surface* surface )
{
    bool success = true;

    surface = loadSurface( path );
    if( surface == NULL )
    {
        printf( "Failed to load stretching image!\n" );
        success = false;
    }

    return success;
}

void close()
{
    //Free loaded image
    SDL_FreeSurface( backgroundSurface );
    backgroundSurface = NULL;
    SDL_FreeSurface( imageSurface );
    imageSurface = NULL;
    SDL_FreeSurface( ballSurface );
    ballSurface = NULL;

    //Destroy window
    SDL_DestroyWindow( window );
    window = NULL;

    //Quits SDL
    SDL_Quit();
}

SDL_Surface* loadSurface( std::string path )
{
    SDL_Surface* returnSurface = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = SDL_LoadBMP( path.c_str() );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
    }
    else
    {
        returnSurface = SDL_ConvertSurface( loadedSurface, screenSurface->format, NULL );
        if( returnSurface == NULL )
        {
            printf( "Unable to optimize image %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }
        SDL_FreeSurface( loadedSurface );
    }
    return returnSurface;
}

int main( int argc, char* args[] )
{
    if( !init() )
    {
        close();
        return 0;
    }
    //Load background
    backgroundSurface = loadSurface( "background.bmp" );

    std::vector<SDL_Surface*> lannesVect;
    //player model
    Hitbox lannes{ 0, 772 };
    imageSurface = loadSurface( "lannesstep1.bmp" );

    lannesVect.push_back( imageSurface );

    imageSurface = loadSurface( "lannesstep2.bmp" );

    lannesVect.push_back( imageSurface );

    imageSurface = loadSurface( "lannesjump.bmp" );
    lannesVect.push_back( imageSurface );

    //cannonball texture
    ballSurface = loadSurface("cannonball.bmp");
    SDL_SetColorKey(ballSurface, SDL_TRUE, SDL_MapRGB(ballSurface->format, 255, 0, 0));

    //load the player model
    SDL_Rect lannesPos;
    lannesPos.x = lannes.getXCoord();
    lannesPos.y = lannes.getYCoord();
    SDL_BlitSurface( imageSurface, NULL, screenSurface, &lannesPos );

    //Apply the background stretched to the appropriate resolution
    SDL_Rect stretchRect;
    stretchRect.x = 0;
    stretchRect.y = 0;
    stretchRect.w = SCREEN_WIDTH;
    stretchRect.h = SCREEN_HEIGHT;
    SDL_BlitScaled( backgroundSurface, NULL, screenSurface, &stretchRect );
    SDL_UpdateWindowSurface( window );

    bool quit = false;
    bool lannesJumping = false;
    int lannesWalkTick = 0;
    std::vector<Cannonball> cannonballList; 

    Cannonball cannonball{ lannes };

    SDL_Event e;

    while( !quit )
    {
        //resets the window
        SDL_BlitScaled( backgroundSurface, NULL, screenSurface, &stretchRect );
        while( SDL_PollEvent( &e ) != 0 )
        {
            //User requests quit
            if( e.type == SDL_QUIT )
            {
                quit = true;
            }
            else if( e.type == SDL_KEYDOWN )
            {
                //Select surfaces based on key press
                switch( e.key.keysym.sym )
                {
                    case SDLK_UP:
                    lannesJumping = true;
                    break;

                    case SDLK_DOWN:
                    break;

                    case SDLK_LEFT:
                    lannes.moveHitboxHoriz( false );
                    lannesWalkTick++;
                    break;

                    case SDLK_RIGHT:
                    lannes.moveHitboxHoriz( true );
                    lannesWalkTick++;
                    break;

                    default:
                    break;
                }
            }
        }

        //redraws lannes every tick
        lannesPos.x = lannes.getXCoord();
        lannesPos.y = lannes.getYCoord();

        //walking animation
        if(!lannesJumping)
        {
            if( lannesWalkTick < 3 )
            {
                imageSurface = lannesVect.at(0);
            }
            else if( lannesWalkTick <= 6 )
            {
                if( lannesWalkTick == 6 ){ lannesWalkTick = 0; }
                imageSurface = lannesVect.at(1);
            }
        }

        //lannes jumping loop
        if( lannesJumping )
        {
            lannesJumping = lannes.moveHitboxVert();
            imageSurface = lannesVect.at(2);
            lannesWalkTick = 0;
        }

        //safety, so that there is always a sprite drawn
        if(!imageSurface)
        {
            imageSurface = lannesVect.at(0);
        }
        SDL_SetColorKey(imageSurface, SDL_TRUE, SDL_MapRGB(imageSurface->format, 255, 0, 0));
        SDL_BlitSurface( imageSurface, NULL, screenSurface, &lannesPos );
  
        //cannonball spawns
        //int random = rand() % 200 + 1;     // v2 in the range 1 to 200

        cannonball.calculateTrajectory();
        SDL_Rect ballPos;
        ballPos.x = cannonball.xCoord;
        ballPos.y = cannonball.yCoord;
        SDL_BlitSurface( ballSurface, NULL, screenSurface, &ballPos );

        /*if( random = 100 )
        {
            Cannonball cannonball{ lannes };
            cannonballList.push_back( cannonball );
        }

        if( !cannonballList.empty() )
        {
            std::vector<Cannonball>::iterator ballIt = cannonballList.begin();
            std::vector<Cannonball>::iterator ballItEnd = cannonballList.end();
            for( ; ballIt != ballItEnd; ++ballIt )
            {
                Cannonball ball = *ballIt;
                ball.calculateTrajectory();

                ball.detectHit();

                SDL_Rect ballPos;
                ballPos.x = ball.xCoord;
                ballPos.y = ball.yCoord;
                SDL_BlitSurface( ballSurface, NULL, screenSurface, &ballPos );

                std::cout<<ball.xCoord<<" "<< ball.yCoord << " ";


            }
        }*/

        SDL_UpdateWindowSurface( window );
    }

    close();
    return 0;
}