#pragma once
struct Input {
	bool upHeld = false;
	bool downHeld = false;
	bool wHeld = false;
	bool sHeld = false;
	bool respawnBallHeld = false;
	bool startPressed = false;
	int player1Score = 0;
	int player2Score = 0;
};

int collectInput(Input* inputThisFrame);