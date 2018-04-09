///////////////////////////////////
// Assignment 3 - Hover Racing	 //
// by Nathan Hendley			 //
///////////////////////////////////

#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <sstream>
#include <vector>

using namespace std;
using namespace tle;

I3DEngine* myEngine = New3DEngine(kTLX); //so functions and other stuff can be made outside main()

struct vectors
{
	float x;
	float z;
};

//scalar is thrustfactor..
vectors scalar(float scalar, vectors v)
{
	return{ scalar * v.x, scalar * v.z };
}

//v1 = current momentum, v2 = thrust, v3 = drag
vectors sum(vectors v1, vectors v2, vectors v3)
{
	return{ v1.x + v2.x + v3.x, v1.z + v2.z + v3.z };
}

//Used to work out movement
vectors momentum{ 0.0f, 0.0f };		//Actual movement of the car {X, Z}
vectors thrust{ 0.0f, 0.0f };		//thrust/drag used in calculations for momentum
vectors drag{ 0.0f, 0.0f };

////////////////////////////////////
// general car controls////////////
EKeyCode forwards = Key_W;
EKeyCode backwards = Key_S;
EKeyCode rightTurn = Key_D;
EKeyCode leftTurn = Key_A;
EKeyCode quit = Key_Escape;
EKeyCode SpaceBar = Key_Space;
///////////////////////////////////
//camera controls
EKeyCode cameraReset = Key_1;
EKeyCode cameraFP = Key_2;
EKeyCode cameraForward = Key_Up;
EKeyCode cameraBackward = Key_Down;
EKeyCode cameraRight = Key_Right;
EKeyCode cameraLeft = Key_Left;
////////////////////////////////////

float timer;						//controls the speed of the game, no matter which computer
const float cameraSpeed = 3.0f;		//moving the camera around *not rotating* -> arrow keys

const float carRadius = 6.0f;		//collision stuff 
float maxSpeed = 0.02f;				//max speed car can go
int carHealth = 100;
float thrustFactor = 0.02f;			//nyoom - modified in carMovement function when boosting

bool isOverheated = false;			//both used in boost area in carMovement
bool isBoosting = false;

float originalMax = 0.02f;			//also all used in boost area, had to be global 
float boostSpeed = 0.08f;			//because repeating the function just caused the countdown numbers to stay the same
float maxHeat = 4.0f;
float maxCooldown = 5.0f;
float heatTimer = 4.0f;
float cooldown = 5.0f;

const float checkpointInX = 9.5f;		//bounding area for the area inside the checkpoint
const float checkpointinZ = 2.0f;

//checkpoint stats!
float checkpointZSpawn[4] = { 0.0f, 150.0f, 300.0f, 250.0f };
float checkpointXSpawn[4] = { 0.0f, 100.0f, 75.0f, 150.0f };
const float checkpointRadius = 0.64f;
///////////////////////////////////////////////////////////////////////////////////////////
//spawn locations for isles/walls (collision function handles them as 1 object)
float islexSpawn[30] = { -15.0f, 15.0f, -15.0f, 15.0f, -15.0f, 15.0f,
						85.0f, 115.0f, 85.0f, 115.0f, 85.0f, 115.0f,
						85.0f, 115.0f, 85.0f, 115.0f, 85.0f, 115.0f,
						60.0f, 90.0f, 60.0f, 90.0f, 60.0f, 90.0f,
						165.0f, 135.0f, 165.0f, 135.0f, 165.0f, 135.0f };

float islezSpawn[30] = { 0.0f, 0.0f, 15.0f, 15.0f ,-15.0f, -15.0f,
						150.0f, 150.0f, 135.0f, 135.0f, 165.0f, 165.0f,
						185.0f, 185.0f, 200.0f, 200.0f, 215.0f, 215.0f,
						300.0f, 300.0f, 315.0f, 315.0f, 285.0f, 285.0f,
						265.0f, 265.0f, 250.0f, 250.0f, 235.0f, 235.0f };

float wallxSpawn[20] = { -15.0f, 15.0f, -15.0f, 15.0f,
						85.0f, 115.0f, 85.0f, 115.0f,
						85.0f, 115.0f, 85.0f, 115.0f,
						60.0f, 90.0f, 60.0f, 90.0f,
						165.0f, 165.0f, 135.0f, 135.0f };

float wallzSpawn[20] = { 7.5f, 7.5f, -7.5f, -7.5f,
						142.5f, 142.5f, 157.5f, 157.f,
						207.5f, 207.5f, 192.5f, 192.5f,
						307.5f, 307.5f, 292.5f, 292.5f,
						257.5f, 242.5f, 257.5f, 242.5f };
//////////////////////////////////////////////////////////////////////////////////////////////

//locations for tanks///////////
float tankxSpawn[11] = { -5.0f, 50.0f, 45.0f, 50.0f, 100.0f, 80.0f, 90.0f, 100.0f, 70.0f, 110.0f, 120.0f };
float tankzSpawn[11] = { 40.0f, 100.0f, 20.0f, 220.0f, 260.0f, 350.0f, 350.0f, 350.0f, 350.0f, 350.0f, 350.0f };
////////////////////////////////

float checkpointMinX, checkpointMaxX, checkpointMinZ, checkpointMaxZ; //used for checkpoint collision

const float legPos = 9.86f;		//local position of the checkpoint legs. used to place dummies

float counter = 4.0f;		//used to countdown

float carMatrix[4][4];		//for getting facing vector!

///////////////////////////
//		FUNCTIONS! :D	// ---------------------------------------------------------------------------------------------
/////////////////////////

//Deals with car movement (Scalar multiplaction to calculate thrust using drag, and momentum.
void carMovement(IModel* car, IFont* font)
{

	float dragFactor = -0.5f;
	float rotateSpeed = 50.0f;		//car turining speed!

	car->GetMatrix(&carMatrix[0][0]);
	vectors facingVector = { carMatrix[2][0], carMatrix[2][2] };

	if (myEngine->KeyHeld(leftTurn))
	{
		car->RotateLocalY(-rotateSpeed * timer);
	}
	if (myEngine->KeyHeld(rightTurn))
	{
		car->RotateLocalY(rotateSpeed*timer);
	}

	//stops the main forward motion interfering while boosting, 
	//and makes sure it's being used when overheated, even if the user is being an arse
	if (isBoosting == false || (isBoosting == true && isOverheated == true))
	{
		if (myEngine->KeyHeld(forwards))
		{
			//Makes sure the car just can't endlessly accelerate
			if (momentum.z > maxSpeed || momentum.x > maxSpeed || momentum.z < -maxSpeed || momentum.x < -maxSpeed)
			{
				thrust = { 0.0f, 0.0f };
			}
			//otherwise, gotta go fast!
			else
			{
				thrust = scalar(thrustFactor * timer, facingVector);
			}
		}
		//stops going fast if you let go of W.
		else
		{
			thrust = { 0.0f, 0.0f };
		}
		//Beep... Beep... Beep..
		if (myEngine->KeyHeld(backwards))
		{
			thrust = scalar(-thrustFactor * timer, facingVector);
		}
	}
	//////////////////////////////////////////////////////////////////////////////////
	//boost time!
	stringstream boostStatus; //for current timers like countdowns and stuff

	//these are basically just a toggle, it's easier than checking for the 
	//player holding spacebar on every single condition
	if (myEngine->KeyHeld(SpaceBar))
	{
		isBoosting = true;
	}
	else
	{
		isBoosting = false;
	}

	//Nyoom
	if (isBoosting == true && isOverheated == false)
	{
		thrustFactor = boostSpeed;
		heatTimer -= timer;

		font->Draw("BOOSTING!", 500, 645, kRed);
		boostStatus << "Time Left: " << heatTimer;
		font->Draw(boostStatus.str(), 500, 670, kRed);
		boostStatus.str("");

		//BOOM. Overheat. 
		if (heatTimer < 0.0f)
		{
			momentum = { momentum.x / 3, momentum.z / 3 }; //Sudden breaks. Can't just boost and coast it
			thrustFactor = originalMax; //Slow the heck down 
			cooldown = 0.0f; //starts the countup
			isOverheated = true;
		}
	}
	//if it's not overheated, the heat gauge cools off.
	else if (isBoosting == false && isOverheated == false)
	{
		thrustFactor = originalMax;
		if (heatTimer < maxHeat)
		{
			heatTimer += timer;
		}
	}
	else //It's overheated and trying to boost ain't gonna do nuffin either. 
	{
		stringstream cooldownTimer;
		thrustFactor = originalMax;
		cooldown += timer;
		cooldownTimer << int(cooldown);
		font->Draw("OVERHEAT!", 500, 595, kRed);
		font->Draw("Cooling down..", 500, 620, kRed);
		font->Draw(cooldownTimer.str(), 500, 645, kRed);
		if (cooldown >= maxCooldown) //When the cooldown finishes, resets both heatgauge and allows boosting
		{
			heatTimer = maxHeat;
			isOverheated = false;
		}
	}

	//nyoom
	drag = scalar(dragFactor * timer, momentum);
	momentum = sum(momentum, thrust, drag);

	car->Move(momentum.x, 0.0f, momentum.z);

}

//Floaty!
bool floatingUp = true;
void carFloaty(IModel* model)
{
	float floatspeed = 0.0002f;
	float currentfloat = 0.0f;
	float floatLimit = 0.4f;

	if (floatingUp == true)
	{
		model->MoveY(floatspeed);
		if (model->GetY() > floatLimit)
		{
			floatingUp = false;
		}
	}
	if (floatingUp == false)
	{
		model->MoveY(-floatspeed);
		if (model->GetY() < -floatLimit)
		{
			floatingUp = true;
		}
	}
}

//Uses mouse to look around, 1 resets, 2 puts it into First Person. Arrow keys move it around
float mouseMoveX;
float mouseMoveY;
void cameraControl(float mouseX, float mouseY, I3DEngine* myEngine, ICamera* camera, IModel* car)
{
	mouseMoveX = myEngine->GetMouseMovementY();
	mouseMoveY = myEngine->GetMouseMovementX();

	camera->RotateLocalX(mouseMoveX);
	camera->RotateY(mouseMoveY);

	if (myEngine->KeyHeld(cameraForward))
	{
		camera->MoveZ(cameraSpeed * timer);
	}
	if (myEngine->KeyHeld(cameraBackward))
	{
		camera->MoveZ(-cameraSpeed * timer);
	}
	if (myEngine->KeyHeld(cameraLeft))
	{
		camera->MoveX(-cameraSpeed * timer);
	}
	if (myEngine->KeyHeld(cameraRight))
	{
		camera->MoveX(cameraSpeed * timer);
	}
	if (myEngine->KeyHit(cameraReset))
	{
		camera->SetLocalPosition(0.0f, 10.0f, -20.0f);
		camera->ResetOrientation();
		camera->RotateLocalX(20.0f);
	}
	if (myEngine->KeyHit(cameraFP))
	{
		camera->SetLocalPosition(0.0f, 5.3f, 0.0f);
		camera->ResetOrientation();
	}
}

//possibly merge into one function with walls. Currently just handles collision with checkpoint legs
void legCollision(IModel* car, IModel* dummy[])
{
	//checkpoint detection (legs)
	for (int i = 0; i < 8; i++)
	{
		float x = car->GetX() - dummy[i]->GetX();
		float z = car->GetZ() - dummy[i]->GetZ();

		float distance = sqrt(x * x + z * z);

		if (distance < carRadius + checkpointRadius) //Sphere to sphere detection for checkpoint legs.
		{
			momentum = { -momentum.x, -momentum.z };
			thrust = { -thrust.x, thrust.z };
			carHealth -= 1;
		}
	}
}

//same as above but for walls(inc isles) & tanks
void wallCollision(IModel* wall[], IModel* car, IModel* tank[], IModel* tank2)
{
	float wallWidth = 2.0f;			//includes isle for simplicity
	float wallLength = 10.0f;		//includes isle for simplicity
	float tankradius = 4.5f;

	//Sphere to box collision
	for (int i = 0; i < 20; i++)
	{
		float wallMinX, wallMaxX, wallMinZ, wallMaxZ;
		wallMinX = wall[i]->GetX() - wallWidth;
		wallMaxX = wall[i]->GetX() + wallWidth;
		wallMinZ = wall[i]->GetZ() - wallLength;
		wallMaxZ = wall[i]->GetZ() + wallLength;

		if (car->GetX() > wallMinX && car->GetX() < wallMaxX
			&& car->GetZ() > wallMinZ && car->GetZ() < wallMaxZ)
		{
			momentum = { -momentum.x , -momentum.z };	//boing!
			thrust = { 0.0f, 0.0f };					//stops the car keeping on going forward.
			carHealth -= 1;
		}
	}
	//tank collision (sphere-to-sphere)
	for (int i = 0; i < 11; i++)
	{
		float x = car->GetX() - tank[i]->GetX();
		float z = car->GetZ() - tank[i]->GetZ();

		float distance = sqrt(x * x + z * z);

		if (distance < carRadius + checkpointRadius) //Sphere to sphere detection for tanks.
		{
			momentum = { -momentum.x, -momentum.z };
			thrust = { -thrust.x, thrust.z };
			carHealth -= 1;
		}
	}
	for (int i = 0; i < 1; i++)
	{
		float x = car->GetX() - tank2->GetX();
		float z = car->GetZ() - tank2->GetZ();

		float distance = sqrt(x * x + z * z);

		if (distance < carRadius + checkpointRadius) //Sphere to sphere detection for tanks.
		{
			momentum = { -momentum.x, -momentum.z };
			thrust = { -thrust.x, thrust.z };
			carHealth -= 1;
		}
	}
}

//gets the checkpoint inside area for stage changing
void getCheckpoint(IModel* checkpoint)
{
	checkpointMinX = checkpoint->GetX() - checkpointInX;
	checkpointMaxX = checkpoint->GetX() + checkpointInX;
	checkpointMinZ = checkpoint->GetZ() - checkpointinZ;
	checkpointMaxZ = checkpoint->GetZ() + checkpointinZ;
}

//text output for nyooms.
void speedOutput(IFont* font)
{
	stringstream speedText; //outputs speed in whole units (Have to multiply by 1k because speed is 0.002 or thereabouts)
	stringstream carStatus;

	if (momentum.z < 0.0f && momentum.x < 0.0f)
	{
		speedText << int((-momentum.z + -momentum.x) * 1000);
		carStatus << int(carHealth) << "%";
	}
	else if (momentum.z > 0.0f && momentum.x > 0.0f)
	{
		speedText << int((momentum.z + momentum.x) * 1000); //basically all these ensure that it's only in whole,
		carStatus << int(carHealth) << "%";					//positive numbers, despite direction of travel
	}
	else if (momentum.z < 0.0f && momentum.x > 0.0f)
	{
		speedText << int((-momentum.z + momentum.x) * 1000);
		carStatus << int(carHealth) << "%";
	}
	else if (momentum.z > 0.0f && momentum.x < 0.0f)
	{
		speedText << int((momentum.z + -momentum.x) * 1000);
		carStatus << int(carHealth) << "%";
	}
	else
	{
		speedText << int((momentum.z + momentum.x) * 1000);
		carStatus << int(carHealth) << "%";
	}
	font->Draw(carStatus.str(), 850, 625, kRed);
	font->Draw(speedText.str(), 328, 617, kBlack);
	speedText.str("");
}
//----------------------------------------------------------------------------------------------------------------------------

enum currentState { Waiting, Countdown, Go, FirstCheckpoint, SecondCheckpoint, ThirdCheckpoint, Finish };

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder(".\\media");

	/**** Set up your scene here ****/
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//The usual models 'n' stuff																								//
	ICamera* camera = myEngine->CreateCamera(kManual, 0.0f, 10.0f, -20.0f);														//
	camera->RotateLocalX(20.0f);																								//
																																//
	IFont* myFont = myEngine->LoadFont("Elephant", 30);																			//
	ISprite* uiElement = myEngine->CreateSprite("testUI.png", 250, 560);		//UI at bottom of screen						//
																																//
	IMesh* skyboxMesh = myEngine->LoadMesh("Skybox 07.x");																		//
	IModel* skybox = skyboxMesh->CreateModel(0.0f, -960.0f, 0.0f);																//
																																//
	IMesh* floorMesh = myEngine->LoadMesh("ground.x");																			//
	IModel* floor = floorMesh->CreateModel(0, 0, 0);																			//
																																//
	IMesh* dummyMesh = myEngine->LoadMesh("dummy.x");				//various dummies around the game for collision junk		//
																																//
	IMesh* carMesh = myEngine->LoadMesh("race2.x");																				//
	IModel* car = carMesh->CreateModel(0.0f, 0.01f, -50.0f);																	//
																																//
	IMesh* checkpointMesh = myEngine->LoadMesh("Checkpoint.x");																	//
	IModel* checkpoint[4];																										//
	IModel* legdummy[8];																										//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Checkpoint leg dummies... I'm sure there's a more efficient way.. :( 
	//(have considered using for loops, but then numbers were confusing me)
	{
		//makes actual checkpoints first
		for (int i = 0; i < 4; i++)
		{
			checkpoint[i] = checkpointMesh->CreateModel(checkpointXSpawn[i], 0, checkpointZSpawn[i]);
		}

		//then attaches a dummy to the left leg, then right leg of each checkpoint. 
		legdummy[0] = dummyMesh->CreateModel(legPos, 0, 0);
		legdummy[0]->AttachToParent(checkpoint[0]);

		legdummy[1] = dummyMesh->CreateModel(-legPos, 0, 0);
		legdummy[1]->AttachToParent(checkpoint[0]);

		legdummy[2] = dummyMesh->CreateModel(legPos, 0, 0);
		legdummy[2]->AttachToParent(checkpoint[1]);

		legdummy[3] = dummyMesh->CreateModel(-legPos, 0, 0);
		legdummy[3]->AttachToParent(checkpoint[1]);

		legdummy[4] = dummyMesh->CreateModel(legPos, 0, 0);
		legdummy[4]->AttachToParent(checkpoint[2]);

		legdummy[5] = dummyMesh->CreateModel(-legPos, 0, 0);
		legdummy[5]->AttachToParent(checkpoint[2]);

		legdummy[6] = dummyMesh->CreateModel(legPos, 0, 0);
		legdummy[6]->AttachToParent(checkpoint[3]);

		legdummy[7] = dummyMesh->CreateModel(-legPos, 0, 0);
		legdummy[7]->AttachToParent(checkpoint[3]);
	}
	////////////////////////////////////////////////////////////////////////////
	//Loading walls-----------------------------------------------------------
	IMesh* isleMesh = myEngine->LoadMesh("IsleStraight.x");
	IModel* isle[30];
	for (int i = 0; i < 30; i++)
	{
		isle[i] = isleMesh->CreateModel(islexSpawn[i], 0, islezSpawn[i]);
	}

	IMesh* wallMesh = myEngine->LoadMesh("Wall.x");
	IModel* wall[20];
	for (int i = 0; i < 20; i++)
	{
		wall[i] = wallMesh->CreateModel(wallxSpawn[i], 0, wallzSpawn[i]);
	}
	//------------------------------------------------------------------------
	////////////////////////////////////////////////////////////////////////////
	//tank tiem---------------------------------------------------------------
	//regular ol' tanks
	IMesh* tankMesh = myEngine->LoadMesh("TankSmall2.x");
	IModel* tank[11];
	for (int i = 0; i < 11; i++)
	{
		tank[i] = tankMesh->CreateModel(tankxSpawn[i], 0, tankzSpawn[i]);
	}

	//tank that's in the floor and in the way
	IMesh* floorTankMesh = myEngine->LoadMesh("TankSmall1.x");
	IModel* floorTank = tankMesh->CreateModel(80, -5, 80);
	floorTank->RotateZ(20.0f);
	//------------------------------------------------------------------------
	//////////////////////////////////////////////////////////////////////////////
	camera->AttachToParent(car);
	timer = myEngine->Timer();

	currentState gameState = Waiting; //initial state
									  // The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();

		/**** Update your scene each frame here ****/
		timer = myEngine->Timer();

		//Game first boots up in this state, waits for player to press space
		if (gameState == Waiting)
		{
			myFont->Draw("Press Space to Start!", 500, 625, kRed);
			if (myEngine->KeyHit(SpaceBar))
			{
				gameState = Countdown;
			}
		}
		//counts from 3 to 0 after starting
		if (gameState == Countdown)
		{
			if (counter <= 4.0f && counter > 3.1f)
			{
				myFont->Draw("3..", 850, 625, kRed);
			}
			if (counter < 3.0f && counter > 2.0f)
			{
				myFont->Draw("2..", 850, 625, kRed);
			}
			if (counter < 2.0f && counter > 1.0f)
			{
				myFont->Draw("1..", 850, 625, kRed);
			}
			if (counter < 1.0f && counter > 0.0f)
			{
				myFont->Draw("Go!", 850, 625, kBlack);
			}
			if (counter < 0.0f)
			{
				myFont->Draw("Go!", 850, 625, kBlack);
				gameState = Go;
			}

			counter = counter - timer;
		}

		//The following states until "Finish" are basically the main race. 
		//going through the first checkpoint makes you go into FirstCheckpoint state
		//only the appropriate checkpoints trigger the appropriate state, and only in order
		if (gameState == Go)
		{
			carMovement(car, myFont);
			speedOutput(myFont);
			carFloaty(car);
			cameraControl(mouseMoveX, mouseMoveY, myEngine, camera, car);
			legCollision(car, legdummy);
			wallCollision(wall, car, tank, floorTank);

			getCheckpoint(checkpoint[0]); //checks for a specific checkpoint, then checks to see if a collision with the middle of it happens
			if (car->GetX() > checkpointMinX + carRadius && car->GetX() < checkpointMaxX + carRadius
				&& car->GetZ() > checkpointMinZ + carRadius && car->GetZ() < checkpointMaxZ + carRadius)
			{
				gameState = FirstCheckpoint;
			}
		}
		if (gameState == FirstCheckpoint)
		{
			speedOutput(myFont);
			myFont->Draw("Stage 1 Complete", 500, 625, kBlack);
			carFloaty(car);
			carMovement(car, myFont);
			cameraControl(mouseMoveX, mouseMoveY, myEngine, camera, car);
			legCollision(car, legdummy);
			wallCollision(wall, car, tank, floorTank);

			getCheckpoint(checkpoint[1]);//checks for a specific checkpoint, then checks to see if a collision with the middle of it happens
			if (car->GetX() > checkpointMinX + carRadius && car->GetX() < checkpointMaxX + carRadius
				&& car->GetZ() > checkpointMinZ + carRadius && car->GetZ() < checkpointMaxZ + carRadius)
			{
				gameState = SecondCheckpoint;
			}
		}
		if (gameState == SecondCheckpoint)
		{
			speedOutput(myFont);
			myFont->Draw("Stage 2 Complete", 500, 625, kBlack);
			carFloaty(car);
			carMovement(car, myFont);
			cameraControl(mouseMoveX, mouseMoveY, myEngine, camera, car);
			legCollision(car, legdummy);
			wallCollision(wall, car, tank, floorTank);
			getCheckpoint(checkpoint[2]);//checks for a specific checkpoint, then checks to see if a collision with the middle of it happens
			if (car->GetX() > checkpointMinX + carRadius && car->GetX() < checkpointMaxX + carRadius
				&& car->GetZ() > checkpointMinZ + carRadius && car->GetZ() < checkpointMaxZ + carRadius)
			{
				gameState = ThirdCheckpoint;
			}
		}
		if (gameState == ThirdCheckpoint)
		{
			carMovement(car, myFont);
			speedOutput(myFont);
			myFont->Draw("Stage 3 Complete", 500, 625, kBlack);
			myFont->Draw(" ", 850, 625, kRed);
			carFloaty(car);
			cameraControl(mouseMoveX, mouseMoveY, myEngine, camera, car);
			legCollision(car, legdummy);
			wallCollision(wall, car, tank, floorTank);
			getCheckpoint(checkpoint[3]);//checks for a specific checkpoint, then checks to see if a collision with the middle of it happens
			if (car->GetX() > checkpointMinX + carRadius && car->GetX() < checkpointMaxX + carRadius
				&& car->GetZ() > checkpointMinZ + carRadius && car->GetZ() < checkpointMaxZ + carRadius)
			{
				gameState = Finish;
			}
		}

		//Winner Winner Chicken Dinner
		if (gameState == Finish)
		{
			myFont->Draw("Race Complete!", 500, 625, kGreen);
			carMovement(car, myFont);
			carFloaty(car);
			legCollision(car, legdummy);
			wallCollision(wall, car, tank, floorTank);
			cameraControl(mouseMoveX, mouseMoveY, myEngine, camera, car);
		}
		if (myEngine->KeyHit(quit))
		{
			myEngine->Stop();
		}

	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}