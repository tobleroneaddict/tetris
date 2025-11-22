#include <stdio.h>
#define SDL_MAIN_HANDLED
//SDL 2.28.5-VC
#include  <SDL2/SDL.h>
#undef main
#include <stdbool.h>
#include <stdlib.h>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#define PATH "song.wav"
#include  <SDL2/SDL_ttf.h>
#include<time.h>


static const int width = 260*2;
static const int height = 480*2;
SDL_Renderer* renderer;
ma_result result;
ma_engine engine;
ma_sound sound;
ma_sound sound2;
ma_sound songLose;
TTF_Font* font;

char tutorialString[80] = "WASD, _, <-,->,ESC";

struct sfxStruct {
	ma_sound move;
	ma_sound rLeft;
	ma_sound rRight;
	ma_sound drop;
	ma_sound clear1;
	ma_sound clear2;
	ma_sound clear3;
	ma_sound gameOver;
	ma_sound fourclear;
};
#pragma warning(disable:4996)

struct sfxStruct sfx;

int bag[7] = { 1,1,1,1,1,1,1 }; //Bag of pieces just like the tetris standard

int tetrimino_L[3][3] = {
	{-1,-1,6},
	{6,6,6},
	{-1,-1,-1}
};
int tetrimino_T[3][3] = {
	{-1,2,-1},
	{2,2,2},
	{-1,-1,-1}
};
int tetrimino_S[3][3] = {
	{-1,3,3},
	{3,3,-1},
	{-1,-1,-1}
};
int tetrimino_Z[3][3] = {
	{5,5,-1},
	{-1,5,5},
	{-1,-1,-1}
};
int tetrimino_J[3][3] = {
	{4,-1,-1},
	{4,4,4},
	{-1,-1,-1}
};


//Why cant this be 0,0,0,0????????????????? whatever
int tetrimino_I[1][4] = {
	{0,1,1,0}
};

int tetrimino_O[2][2] = {
	{1,1},
	{1,1}
};
int tetrimino_O_offsetData[4][2] = { //Will account for the wobble
	{0,0},
	{0,-1},
	{-1,-1},
	{-1,0},
};
int speedFrames[21] = {
	53,49,45,41,37,33,28,22,17,11,10,9,8,7,6,6,5,5,4,4,3
};
struct Player {

	int x;
	int y;
	int rotation; // 0-3
	int color;
	int relativeData[5][5]; //The data of whatever shape of block is being dropped
	int score;
	int lines;
	int level;
	int gameOver;
	int nextColor;
};

//7 colors
//cyan I, yellow O, purple T, green S, blue J, red Z and orange L


//GAME VARIABLES
int field[10][24];
struct Player player = { 3,1,0,3 ,0,0,0, false, 0};

SDL_Surface* scoreText;
SDL_Texture* scoreText_texture;

SDL_Surface* lineText;
SDL_Texture* lineText_texture;

SDL_Surface* levelText;
SDL_Texture* levelText_texture;

SDL_Surface* statusText;
SDL_Texture* statusText_texture;

int timeout;
bool stickNow = false; 
bool game_pause = false;

void fillBag() { //Reset the bag for use
	for (int i = 0; i < 7; i++) {
		bag[i] = 1;
	}
}
int getPieceFromBag() { //Select a random piece from the bag and return int from 0-7 depending on the block
	//but first check if the bag is empty
	int sum = 0;
	for (int i = 0; i < 7; i++) {
		sum += bag[i];
	}
	if (sum == 0) { //Fill if so
		fillBag();
	}

	//Get a random piece

	int curr = rand() % 7;
	while (bag[curr] == 0) { //keep going until a valid piece is reached
		curr = rand() % 7;
	}

	bag[curr] = 0;
	return curr; //Return the piece after clearing it from the bag

}

//Clear a thing of data
void clearData(int data[5][5]) {
	for (int y = 0; y < 5; y++) {
		for (int x = 0; x < 5; x++) {
			data[x][y] = -1;
		}
	}
}
//Clear the entire board
void clearAll() {
	for (int y = 0; y < 24; y++) {
		for (int x = 0; x < 10; x++) {
			field[x][y] = -1;
		}
	}
}

//This is for setUpBlock..pointer makes it break and im too lazy to fix it honestly
void putData(int data[5][5], int tetrimino[3][3]) { //Used for putting tetrimino data inside of any int [5][5] 
	for (int x_i = 0; x_i < 3; x_i++) {
		for (int y_i = 0; y_i < 3; y_i++) {
			//This will put a centered tetrimino inside the relativedata
			data[x_i + 1][y_i + 1] = tetrimino[x_i][y_i];
		}
	}
}
void fillData(int tetrimino[3][3]) {
	clearData(player.relativeData);
	putData(player.relativeData, tetrimino);
}

//Start the relative data off with some block data
void setUpBlock(int blockN) {
	player.color = blockN;
	//Assume a rotation of zero
	switch (blockN) {
	case 0:
		//I piece
		clearData(player.relativeData);
		for (int x_i = 0; x_i < 4; x_i++) {
			for (int y_i = 0; y_i < 1; y_i++) {
				//This will put a centered tetrimino inside the relativedata
				player.relativeData[x_i + 1][y_i + 2] = tetrimino_I[x_i][y_i]; //Has a weird color glitch
				if (player.relativeData[x_i + 1][y_i + 2] == 1) {
					player.relativeData[x_i + 1][y_i + 2] = 0;
				}
			}
		}
		
		break;
	case 1:
		//O piece
		clearData(player.relativeData);
		for (int x_i = 0; x_i < 2; x_i++) {
			for (int y_i = 0; y_i < 2; y_i++) {
				//This will put a centered tetrimino inside the relativedata
				player.relativeData[x_i + 1][y_i + 1] = tetrimino_O[x_i][y_i];
			}
		}
		break;
	case 2:
		//T
		fillData(tetrimino_T);
		break;
	case 3:
		//S
		fillData(tetrimino_S);
		break;
	case 4:
		//J
		fillData(tetrimino_J);
		break;
	case 5:
		//Z
		fillData(tetrimino_Z);
		break;
	case 6:
		//L
		fillData(tetrimino_L);
		break;
	default:
		printf("Somethings not quite right with setUpBlock...");
		break;
	}

}
void setUpBlockOffsets(int blockN,int offsetX,int offsetY) {
	//Assume a rotation of zero
	switch (blockN) {
	case 0:
		//I piece
		clearData(player.relativeData);
		for (int x_i = 0; x_i < 4; x_i++) {
			for (int y_i = 0; y_i < 1; y_i++) {
				//This will put a centered tetrimino inside the relativedata
				player.relativeData[x_i + 1 + offsetX][y_i + 2 + offsetY] = tetrimino_I[x_i][y_i]; //Has a weird color glitch
			}
		}
		break;
	case 1:
		//O piece
		clearData(player.relativeData);
		for (int x_i = 0; x_i < 2; x_i++) {
			for (int y_i = 0; y_i < 2; y_i++) {
				//This will put a centered tetrimino inside the relativedata
				player.relativeData[x_i + 1][y_i + 1] = tetrimino_O[x_i][y_i];
			}
		}
		break;
	case 2:
		//T
		fillData(tetrimino_T);
		break;
	case 3:
		//S
		fillData(tetrimino_S);
		break;
	case 4:
		//J
		fillData(tetrimino_J);
		break;
	case 5:
		//Z
		fillData(tetrimino_Z);
		break;
	case 6:
		//L
		fillData(tetrimino_L);
		break;
	default:
		printf("Somethings not quite right with setUpBlock...");
		break;
	}

}

void transposeData() {
	int temp[5][5];
	for (int x_i = 0; x_i < 5; x_i++) {
		for (int y_i = 0; y_i < 5; y_i++) {
			//This will put a centered tetrimino inside the relativedata
			temp[x_i][y_i] = player.relativeData[y_i][x_i];
		}
	}
	//Add back
	for (int x_i = 0; x_i < 5; x_i++) {
		for (int y_i = 0; y_i < 5; y_i++) {
			//This will put a centered tetrimino inside the relativedata
			player.relativeData[x_i][y_i] = temp[x_i][y_i];
		}
	}
}
void reverseData() {
	int temp[5][5];
	for (int x_i = 0; x_i < 5; x_i++) {
		for (int y_i = 0; y_i < 5; y_i++) {
			//This will put a centered tetrimino inside the relativedata
			temp[x_i][y_i] = player.relativeData[4-x_i][y_i];
		}
	}
	//Add back
	for (int x_i = 0; x_i < 5; x_i++) {
		for (int y_i = 0; y_i < 5; y_i++) {
			//This will put a centered tetrimino inside the relativedata
			player.relativeData[x_i][y_i] = temp[x_i][y_i];
		}
	}
}

bool collisionRotation(int dir) {
	int temp[5][5];
	for (int x_i = 0; x_i < 5; x_i++) {
		for (int y_i = 0; y_i < 5; y_i++) {
			//This will put a centered tetrimino inside the relativedata
			temp[x_i][y_i] = player.relativeData[x_i][y_i];
		}
	}
	if (dir == 1) {
		transposeData();
		reverseData();
	}
	else {
		transposeData();
		reverseData();
		transposeData();
		reverseData();
		transposeData();
		reverseData();
	}
	bool canMove = true;
	for (int y = 0; y < 5; y++) {
		for (int x = 0; x < 5; x++) {
			//printf("%d", player.relativeData[x][y]);
			if (player.relativeData[x][y] != -1) {

				// Transform block to world space
				int wX = x + player.x;
				int wY = y + player.y;


				if (field[wX][wY] != -1) { //If itwould be inside another block
					canMove = false;
				}
			}

			player.relativeData[x][y] = temp[x][y];
		}
	}
	
	if (canMove) {
		return false;
	}
	return true;
}
//Rotate the block by 1 or -1
void rotateBlock(int rotationDirection) {
	
	//Time to rotate
	if (rotationDirection == 1) {
		

		if (player.color != 1) {//No need to rotate the cube
			if (!collisionRotation(1)) { 
				player.rotation++;
				ma_sound_stop(&sfx.rRight);
				ma_sound_start(&sfx.rRight);
				transposeData();
				reverseData();
			}
		}
	}
	else {
		

		if (player.color != 1) {
			if (!collisionRotation(-1)) {
				player.rotation--;
				ma_sound_stop(&sfx.rLeft);
				ma_sound_start(&sfx.rLeft);
				transposeData();
				reverseData();
				transposeData();
				reverseData();
				transposeData();
				reverseData();
			}
		}
	}
	if (player.rotation == 4)  { player.rotation = 0; }
	if (player.rotation == -1) { player.rotation = 3; }
}

void drawBlock(int x, int y,int color,int a) {
	int r = 255;
	int g = 0;
	int b = 255;

	switch (color) {
	case 0:
		r = 0;
		g = 255;
		b = 255;
		break;
	case 1:
		r = 255;
		g = 255;
		b = 0;
		break;
	case 2:
		r = 255;
		g = 0;
		b = 255;
		break;
	case 3:
		r = 0;
		g = 255;
		b = 0;
		break;
	case 4:
		r = 0;
		g = 0;
		b = 255;
		break;
	case 5:
		r = 255;
		g = 0;
		b = 0;
		break;
	case 6:
		r = 255;
		g = 127;
		b = 0;
		break;
	default:
		printf("\nUnknown Block type of %d", color);
		r = 255;
		g = 255;
		b = 255;
		break;
	}



	SDL_Rect rect = { x,y,32,32 };
	SDL_Rect rect2 = { x + 1,y + 1,30,30 };

	//Outer
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, a);
	SDL_RenderFillRect(renderer, &rect);
	//Inner
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderFillRect(renderer, &rect2);
}

void drawPlayer(bool color) {
	//draw directly onto field
	for (int y = 0; y < 5; y++) {
		for (int x = 0; x < 5; x++) {
			//printf("%d", player.relativeData[x][y]);
			if (player.relativeData[x][y] != -1) { 

				if (color) { //In color
					drawBlock(100 + ((x + player.x) * 32), 100 + ((y + player.y) * 32), player.relativeData[x][y],255);
				}
				else { //Transparent
					drawBlock(100 + ((x + player.x) * 32), 100 + ((y + player.y) * 32), player.relativeData[x][y],100);
				}

			}
		}
	}
}
//Check for collision with the floor and blocks, if detected, return true. otherwise false. (willreturn false most  of the time)
bool checkCollision() {
	//Algorithm is very similar to horizontalmove.
	bool canMove = true;
	for (int y = 0; y < 5; y++) {
		for (int x = 0; x < 5; x++) {
			//printf("%d", player.relativeData[x][y]);
			if (player.relativeData[x][y] != -1) {

				// Transform block to world space
				int wX = x + player.x;
				int wY = y + player.y;

				if (wY + 1 >= 24) { //If it would be below the floor
					canMove = false;
				}

				if (field[wX][wY + 1] != -1) { //If itwould be inside another block
					canMove = false;
				}
			}
		}
	}

	if (canMove) {
		return false;
	}
	return true;
}


//Sends a block from relative to world space, then paints it onto the background.
void SendBlockToField() {
	for (int y = 0; y < 5; y++) {
		for (int x = 0; x < 5; x++) {
			//printf("%d", player.relativeData[x][y]);
			if (player.relativeData[x][y] != -1) {

				// Transform block to world space
				int wX = x + player.x;
				int wY = y + player.y;
				field[wX][wY] = player.relativeData[x][y]; //Set field position to that blocks data
				player.relativeData[x][y] = -1; //Delete that blocks data
			}
		}
	}
}


void renderScores() {
	//Text

	// Set color to black
	SDL_Color color = { 255, 255, 255 };


	SDL_DestroyTexture(scoreText_texture);
	SDL_DestroyTexture(lineText_texture);
	SDL_DestroyTexture(levelText_texture);
	SDL_UnlockSurface(scoreText);
	SDL_UnlockSurface(lineText);
	SDL_UnlockSurface(levelText);





	//Converted from windows itoa
	
	char lineString[80];
	snprintf(lineString, sizeof(lineString), "Lines: %d", player.lines);
	lineText = TTF_RenderText_Solid(font, lineString, color);
	lineText_texture = SDL_CreateTextureFromSurface(renderer, lineText);

	char levelString[80];
	snprintf(levelString, sizeof(levelString), "Level: %d", player.level);
	levelText = TTF_RenderText_Solid(font, levelString, color);
	levelText_texture = SDL_CreateTextureFromSurface(renderer, levelText);

	char scoreString[80];
	snprintf(scoreString, sizeof(scoreString), "Score: %d", player.score);
	scoreText = TTF_RenderText_Solid(font, scoreString, color);
	scoreText_texture = SDL_CreateTextureFromSurface(renderer, scoreText);



}

//Give the player a new block after sending to the field
void newBlock() {
	SendBlockToField();

	//Check for gameover

	for (int x = 0; x < 10; x++) {
		if (field[x][0] != -1) { //If blocks detected at top, trigger gameover sequence
			player.gameOver = true; ma_sound_stop(&sound);
			ma_sound_start(&sfx.gameOver); ma_sound_start(&songLose);
			SDL_Color color = { 255,255,255 };
			TTF_SetFontSize(font, 30);
			statusText = TTF_RenderText_Solid(font, "Game Over! Press space to restart.", color);
			statusText_texture = SDL_CreateTextureFromSurface(renderer, statusText);
			TTF_SetFontSize(font, 24);
			renderScores();
			
		}
	}


	setUpBlock(player.nextColor);
	
	player.nextColor = getPieceFromBag();

	player.x = 2;
	player.y = -1;
}

//Drops the piece by one unit
void drop(int calledFromKey) {
	if (checkCollision() == false) {
		player.y++;
		if (calledFromKey == 1) {
			ma_sound_stop(&sfx.move);
			ma_sound_start(&sfx.move);
		}
	}
	//printf("\n%d", player.y);a
}


void drawNext(int offsetX, int offsetY) { //Draws the next block
	int tempData[5][5];
	clearData(tempData);
	//Fill with the right block
	switch (player.nextColor) {
	case 0:
		//I piece
		for (int x_i = 0; x_i < 4; x_i++) {
			for (int y_i = 0; y_i < 1; y_i++) {
				//This will put a centered tetrimino inside the relativedata
				tempData[x_i + 1][y_i + 2] = tetrimino_I[x_i][y_i]; //Has a weird color glitch
				if (tempData[x_i + 1][y_i + 2] == 1) {
					tempData[x_i + 1][y_i + 2] = 0;
				}
			}
		}

		break;
	case 1:
		//O piece
		for (int x_i = 0; x_i < 2; x_i++) {
			for (int y_i = 0; y_i < 2; y_i++) {
				//This will put a centered tetrimino inside the relativedata
				tempData[x_i + 1][y_i + 1] = tetrimino_O[x_i][y_i];
			}
		}
		break;
	case 2:
		//T
		putData(tempData,tetrimino_T);
		break;
	case 3:
		//S
		putData(tempData,tetrimino_S);
		break;
	case 4:
		//J
		putData(tempData,tetrimino_J);
		break;
	case 5:
		//Z
		putData(tempData,tetrimino_Z);
		break;
	case 6:
		//L
		putData(tempData,tetrimino_L);
		break;
	default:
		printf("Somethings not quite right with drawNext...");
		break;
	}
	//Draw
	for (int y = 0; y < 5; y++) {
		for (int x = 0; x < 5; x++) {
			//printf("%d", player.relativeData[x][y]);
			if (tempData[x][y] != -1) {
				drawBlock(100 + ((x + 7) * 32), 100 + ((y - 4 ) * 32), tempData[x][y], 255);

			}
		}
	}

}
void fastDrop() { //Fast drop block
	bool hit = false;
	while (!hit) {
		drop(0);
		hit = checkCollision();
	}
	newBlock();
	ma_sound_stop(&sfx.drop);
	ma_sound_start(&sfx.drop);
}

void drawGhost() { //Ghost to aid in fast dropping
	bool hit = false;
	int oY = player.y;
	while (!hit) {
		drop(0);
		hit = checkCollision();
	}

	drawPlayer(false);

	player.y = oY;
}

//Check and clear the lines
void checkScreen() {
	int rowsCleared = 0;
	for (int y = 0; y < 24; y++) {
		int cleared = 0;
		for (int x = 0; x < 10; x++) {
			if (field[x][y] != -1) { cleared++; }
		}
		//If all 10 columns filled
		if (cleared == 10) {
			rowsCleared++; // Can be from 1 to 4
			//Clear that line
			for (int x = 0; x < 10; x++) {
				field[x][y] = -1;
			}

			//THEN drop all lines down that are above the cleared line
			for (int y2 = y; y2 > 1;y2--) {
				for (int x = 0; x < 10; x++) {
					field[x][y2] = field[x][y2 - 1];
				}
				
			}
		}
	}
	if (rowsCleared != 0) {
		
		int r = rand() % 3;
		switch (r) {
		case 0:
			ma_sound_start(&sfx.clear1);
			break;
		case 1:
			ma_sound_start(&sfx.clear2);
			break;
		case 2:
			ma_sound_start(&sfx.clear3);
			break;
		}

		if (rowsCleared == 4) {
			ma_sound_start(&sfx.fourclear);
		}
		if (rowsCleared >= 5) { //Impossible
			printf("hacker");
		}

		//Award de points		
		switch (rowsCleared) {
		case 1:
			player.score += 40 * (player.level + 1);
			break;
		case 2:
			player.score += 100 * (player.level + 1);
			break;
		case 3:
			player.score += 300 * (player.level + 1);
			break;
		case 4:
			player.score += 1200 * (player.level + 1);
			break;

		}

		//Award lines and calculate level
		player.lines += rowsCleared;
		player.level = (player.lines - (player.lines % 10)) / 10;

		//Render scores
		
		renderScores();
	}
}

//Game loop will run forever...until you close it
//Put all the game code, render, etc. but NOT keyboard! keyboard is in the main loop ive decided.


void gameLoop() {
	if (!game_pause) { //If not paused
	//If not gameover
		if (!player.gameOver) {
			//Drop block timer
			int currTime = SDL_GetTicks64();
			if (currTime > timeout) {

				//Block speed is dependant on the players level run through a LUT
				//Speed is the same as gameboy edition
				int adjustedLevel = player.level; 
				if (adjustedLevel > 20) { adjustedLevel = 20; } //Prevent access violations
				timeout = currTime + (int)((float)1000*((float)speedFrames[adjustedLevel]/ (float)60));
				drop(0); //Not called from key input so zero

				//Helps add a little bit of delay before it actually lands
				if (stickNow == true) { //To B...
					if (checkCollision()) { //Check for vertical collisions 
						//To C!
						newBlock(); //Send to field and Get the new block
						ma_sound_start(&sfx.drop);

						checkScreen(); //Then check for any line clears
					}
					stickNow = false;

				}
				//Goes from A...
				if (checkCollision()) { //Check for vertical collisions
					stickNow = true;
				}
			}

			

			//Bounds
			SDL_Rect bigRect = { 100,100,10 * 32,24 * 32 };
			SDL_SetRenderDrawColor(renderer, 50, 0, 100, 255);
			SDL_RenderFillRect(renderer, &bigRect);
			SDL_SetRenderDrawColor(renderer, 250, 250, 250, 255);
			SDL_RenderDrawRect(renderer, &bigRect);

			for (int y = 0; y < 24; y++) {
				for (int x = 0; x < 10; x++) {
					if (field[x][y] != -1) { drawBlock(100 + (x * 32), 100 + (y * 32), field[x][y],255); }
				}
			}

			drawGhost(); 
			drawPlayer(true);

			drawNext(true,5);
			
			//Draw texts


			SDL_Rect dest = { 200, 10, scoreText->w, scoreText->h };
			SDL_RenderCopy(renderer, scoreText_texture, NULL, &dest);

			SDL_Rect dest2 = { 200, 35, lineText->w, lineText->h };
			SDL_RenderCopy(renderer, lineText_texture, NULL, &dest2);
			SDL_Rect dest3 = { 200, 60, levelText->w, levelText->h };

			SDL_RenderCopy(renderer, levelText_texture, NULL, &dest3);

			SDL_Rect dest4 = { 20, 920, statusText->w, statusText->h };

			SDL_RenderCopy(renderer, statusText_texture, NULL, &dest4);
		}
		else { //Gameover screen

			SDL_Rect dest = { 200, 10, scoreText->w, scoreText->h };
			SDL_RenderCopy(renderer, scoreText_texture, NULL, &dest);

			SDL_Rect dest2 = { 200, 35, lineText->w, lineText->h };
			SDL_RenderCopy(renderer, lineText_texture, NULL, &dest2);
			SDL_Rect dest3 = { 200, 60, levelText->w, levelText->h };

			SDL_RenderCopy(renderer, levelText_texture, NULL, &dest3);


			SDL_Rect dest4 = { 20, 920, statusText->w, statusText->h };

			SDL_RenderCopy(renderer, statusText_texture, NULL, &dest4);
		}
	}
	 else { //Paused
		SDL_Rect dest = { 200, 10, scoreText->w, scoreText->h };
		SDL_RenderCopy(renderer, scoreText_texture, NULL, &dest);

		SDL_Rect dest2 = { 200, 35, lineText->w, lineText->h };
		SDL_RenderCopy(renderer, lineText_texture, NULL, &dest2);
		SDL_Rect dest3 = { 200, 60, levelText->w, levelText->h };

		SDL_RenderCopy(renderer, levelText_texture, NULL, &dest3);

		SDL_Rect dest4 = { 20, 920, statusText->w, statusText->h };

		SDL_RenderCopy(renderer, statusText_texture, NULL, &dest4);

	}
}

//moves the block in a certain direction after checking if its clear to do so.
void horizontalMove(int direction) {
	//Check if it can move freely
	bool canMove = true; //If this is EVER set to false, it cannot move.

	//Algorithm: Iterate through each block on the relativeData. if all blocks on the field
	//Either left or right of the current block are free, canMove will never be set to false.
	//If it happens to encounter an occupied space to the right or left of each block, canMove is set to false and no
	//movment happens.
	//Go thru each block
	for (int y = 0; y < 5; y++) {
		for (int x = 0; x < 5; x++) {
			//printf("%d", player.relativeData[x][y]);
			if (player.relativeData[x][y] != -1) {

				// Transform block to world space
				int wX = x + player.x;
				int wY = y + player.y;
				if (field[wX + direction][wY] != -1) {
					canMove = false;
				}
			}
		}
	}

	if (canMove) {
		player.x += direction;
		ma_sound_stop(&sfx.move);
		ma_sound_start(&sfx.move);
	}
}


//For the audio
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
	ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
	if (pDecoder == NULL) {
		return;
	}

	ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

	(void)pInput;
}


int main(int argc, char** argv) {

	srand(time(NULL)); //Make random more random (and have a different seed each time)
	//https://miniaud.io/docs/manual/index.html#:~:text=5.-,Engine,-The%20ma_engine%20API
	
	clearAll();	//Clear the screen
	clearData(player.relativeData); //Clear the players data

	result = ma_engine_init(NULL, &engine);
	if (result != MA_SUCCESS) {
		printf("Failed to initialize audio engine.");
		return -1;
	}
	
	ma_result result = ma_sound_init_from_file(&engine, "sounds/song.mp3", 0, NULL, NULL, &sound);

	ma_sound_init_from_file(&engine, "sounds/song2.mp3", 0, NULL, NULL, &sound2);
	ma_sound_init_from_file(&engine, "sounds/songLose.mp3", 0, NULL, NULL, &songLose);

	ma_sound_init_from_file(&engine, "sounds/clear1.wav", 0, NULL, NULL, &sfx.clear1);
	ma_sound_init_from_file(&engine, "sounds/clear2.wav", 0, NULL, NULL, &sfx.clear2);
	ma_sound_init_from_file(&engine, "sounds/clear3.wav", 0, NULL, NULL, &sfx.clear3);
	ma_sound_init_from_file(&engine, "sounds/drop.wav", 0, NULL, NULL, &sfx.drop);
	ma_sound_init_from_file(&engine, "sounds/gameOver.wav", 0, NULL, NULL, &sfx.gameOver);
	ma_sound_init_from_file(&engine, "sounds/rotateLeft.wav", 0, NULL, NULL, &sfx.rLeft);
	ma_sound_init_from_file(&engine, "sounds/rotateRight.wav", 0, NULL, NULL, &sfx.rRight);
	ma_sound_init_from_file(&engine, "sounds/move.wav", 0, NULL, NULL, &sfx.move);
	ma_sound_init_from_file(&engine, "sounds/4clear.wav", 0, NULL, NULL, &sfx.fourclear);

	ma_sound_set_volume(&sfx.move, 0.5f);
	ma_sound_set_looping(&sound, true);
	ma_sound_set_looping(&sound2, true);

	ma_sound_start(&sound2);


	SDL_SetMainReady();



	//Init SDL

	SDL_Init(SDL_INIT_VIDEO);

	//Init TTF
	if (TTF_Init() < 0) {
		printf("Could not initialize SDL_TTF.");
	}
	

	font = TTF_OpenFont("TerminusTTF-4.49.3.ttf", 24);
	if (!font) {
		printf("Could not load font 'TerminusTTF-4.49.3.ttf'");
	}
	//Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	//Mix_Music* backgroundMusic = Mix_LoadMUS("tetrisspectrumholobyte.mp3.mp3");

	SDL_Window* window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);

	//Make a renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	bool running = true;
	bool runningTitle = true;
	SDL_Event event;
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	//SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	SDL_SetWindowResizable(window, true); //Resizable window
	SDL_RenderSetLogicalSize(renderer, width, height); //Support scaling

	newBlock(); //For the title screen
	player.y = 10;
	player.x++;
	int scroll = 0; //Scrolling Title
	int scroll2 = 0;
	int timeout2 = 0; //Just for scrolling

	SDL_Color color = { 255, 255, 255 };
	TTF_SetFontSize(font, 24);
	scoreText = TTF_RenderText_Solid(font, "Press Space Press Space Press Space Press Space Press Space Press Space Press Space", color);
	TTF_SetFontSize(font, 50);
	levelText = TTF_RenderText_Solid(font, "TETRIS TETRIS TETRIS TETRIS TETRIS", color);
	TTF_SetFontSize(font, 24);
	lineText = TTF_RenderText_Solid(font, "Placeholder", color);
	TTF_SetFontSize(font, 30);
	statusText = TTF_RenderText_Solid(font, tutorialString, color);
	statusText_texture = SDL_CreateTextureFromSurface(renderer, statusText);
	TTF_SetFontSize(font, 24);
	//Prerender textures (If need to update, call SDL_UpdateTexture())
	
	levelText_texture = SDL_CreateTextureFromSurface(renderer, levelText);
	scoreText_texture = SDL_CreateTextureFromSurface(renderer, scoreText);
	lineText_texture = SDL_CreateTextureFromSurface(renderer, lineText);


	while (runningTitle) { //TITLE SCREEN
		
		int currTime = SDL_GetTicks64();
		if (currTime > timeout) {
			if (player.color == 6) { player.color = -1; }
			//printf("CHANGE");
			setUpBlock(player.color + 1);
			timeout = currTime + 403;
		}
		if (currTime > timeout2) {
			scroll++;
			scroll2++;
			timeout2 = currTime + 5;
			if (scroll >= 175) { scroll = 0; }
			if (scroll2 >= 144) { scroll2 = 0; }
		}

		SDL_SetRenderDrawColor(renderer, 0, 87, 184, 255);
		SDL_RenderClear(renderer);
		SDL_Rect dest = { -250+scroll, 60, levelText->w, levelText->h };
		//SDL_Rect dest = { 180, 60, levelText->w, levelText->h };
		SDL_RenderCopy(renderer, levelText_texture, NULL, &dest);
		
		
		SDL_Rect dest2 = { 0 - scroll2, 800, scoreText->w, scoreText->h };
		SDL_RenderCopy(renderer, scoreText_texture, NULL, &dest2);
		
		drawPlayer(true);

		//Keyboard
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
				runningTitle = false;
			}
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_SPACE: //Play game
					runningTitle = false;
				}
			}
		}
		

		SDL_RenderPresent(renderer);

		
	}




	TTF_SetFontSize(font, 24);

	ma_sound_stop(&sound2);
	
	

	ma_sound_start(&sound);

	clearAll();
	clearData(player.relativeData);

	player.nextColor = rand() % 7;
	newBlock(); //For the game
	
	timeout = 0;
	//setUpBlock(0);


	TTF_SetFontSize(font, 24);

	scoreText = TTF_RenderText_Solid(font, "Score: X", color);

	levelText = TTF_RenderText_Solid(font, "Level: X", color);

	lineText = TTF_RenderText_Solid(font, "Lines: X", color);

	//Prerender textures (If need to update, call SDL_UpdateTexture())

	levelText_texture = SDL_CreateTextureFromSurface(renderer, levelText);
	scoreText_texture = SDL_CreateTextureFromSurface(renderer, scoreText);
	lineText_texture = SDL_CreateTextureFromSurface(renderer, lineText);

	renderScores();
	//Code loop
	while (running) {
		//Keyboard
		while (SDL_PollEvent(&event)) {
			//Quit game
			if (event.type == SDL_QUIT) {
				running = false;
			}
			//Pause game
			if (!player.gameOver && event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					if (game_pause) {
						TTF_SetFontSize(font, 30);
						statusText = TTF_RenderText_Solid(font,tutorialString, color);
						statusText_texture = SDL_CreateTextureFromSurface(renderer, statusText);
						TTF_SetFontSize(font, 24);
						game_pause = false;
						ma_sound_start(&sound);
					}
					else { game_pause = true; ma_sound_stop(&sound);
					SDL_Color color = { 255,255,255 };
					TTF_SetFontSize(font, 30);
					statusText = TTF_RenderText_Solid(font, "Paused! Press escape to continue.", color);
					TTF_SetFontSize(font, 24);
					statusText_texture = SDL_CreateTextureFromSurface(renderer, statusText);
					}
				}
			}
			//Other keys
			if (!game_pause && !player.gameOver && event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_a: //Go Left
					horizontalMove(-1);
					break;
				case SDLK_d: //Go Right
					horizontalMove(1);
					break;
				case SDLK_RIGHT: //Rotate CW
					rotateBlock(1);
					break;
				case SDLK_SPACE: //Right arrow alternative
					rotateBlock(1);
					break;
				case SDLK_LEFT: //Rotate CCW
					rotateBlock(-1);
					break;
				case SDLK_s: //Drop block
					drop(1);
					stickNow = true;
					break;
				case SDLK_w: //Go Left
					fastDrop();
					checkScreen();
					stickNow = true;
					break;
				
				//case SDLK_UP:
				//	player.level++;
					//break;
				}
			}
			else {
				if (player.gameOver && event.type == SDL_KEYDOWN) { //Gameover screen restart key
					switch (event.key.keysym.sym) { //Maybe add more cases
					case SDLK_SPACE: //Go Left
						player.gameOver = false;
						
						player.lines = 0;
						player.level = 0;
						player.score = 0;

						ma_sound_stop(&songLose);
						ma_sound_start(&sound);

						clearAll();
						clearData(player.relativeData);
						TTF_SetFontSize(font, 30);
						statusText = TTF_RenderText_Solid(font,tutorialString, color);
						statusText_texture = SDL_CreateTextureFromSurface(renderer, statusText);
						TTF_SetFontSize(font, 24);
						fillBag();
						newBlock();
						renderScores();
						break;
					}
				}
			}
		}


		//Clear screen, draw, show
		SDL_SetRenderDrawColor(renderer, 0, 87, 184, 255);
		SDL_RenderClear(renderer);

		gameLoop();

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyTexture(scoreText_texture);
	SDL_FreeSurface(scoreText);
	SDL_DestroyTexture(lineText_texture);
	SDL_FreeSurface(lineText);
	SDL_DestroyTexture(levelText_texture);
	SDL_FreeSurface(levelText);

	ma_engine_uninit(&engine);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	//Release
	//Mix_FreeMusic(backgroundMusic);
	//Mix_CloseAudio();
	SDL_Quit();
	
	return 0;
}