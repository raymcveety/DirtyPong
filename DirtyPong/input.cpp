
#include "Input.h"
#include <iostream>
#include <SDL.h>

int collectInput(Input* inputThisFrame)
{
	// Quit flag
	bool quit = false;

	//Event data structure
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0)
	{
		//User requests quit
		if (e.type == SDL_QUIT)
		{
			quit = true;
			return 0;
		}

		// Handle keypress
		if (e.type == SDL_KEYDOWN)
		{
			switch (e.key.keysym.sym)
			{
				// left player input
			case SDLK_w:
				inputThisFrame->wHeld = true;
				break;
			case SDLK_s:
				inputThisFrame->sHeld = true;
				break;

				// right player input
			case SDLK_UP:
				inputThisFrame->upHeld = true;
				break;
			case SDLK_DOWN:
				inputThisFrame->downHeld = true;
				break;

				// general input
			case SDLK_r:
				inputThisFrame->respawnBallHeld = true;
				break;

			case SDLK_p:
				inputThisFrame->startPressed = true;
			default:
				break;
			}
		}
		else if (e.type == SDL_KEYUP)
		{
			switch (e.key.keysym.sym)
			{
				// left player input
			case SDLK_w:
				inputThisFrame->wHeld = false;
				break;
			case SDLK_s:
				inputThisFrame->sHeld = false;
				break;

				// right player input
			case SDLK_UP:
				inputThisFrame->upHeld = false;
				break;
			case SDLK_DOWN:
				inputThisFrame->downHeld = false;
				break;

				// general input
			case SDLK_r:
				inputThisFrame->respawnBallHeld = false;
				break;

			case SDLK_p:
				inputThisFrame->startPressed = false;
			default:
				break;
			}
		}
	}
	//std::cout << "In collectInput in input.cpp \n";
}