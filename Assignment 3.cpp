///////////////////////////////////
// Assignment 3 - Hover Racing	 //
// by Nathan Hendley			 //
///////////////////////////////////

#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <sstream>
#include <vector>
using namespace std;
using namespace tle;
bool floatingUp = true;
float timer;

struct checkpoints
{
	float x;
	float z;
	
};

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

enum currentState {Waiting, Countdown, Go, FirstCheckpoint, Finish};

I3DEngine* myEngine = New3DEngine(kTLX);

float maxSpeed = 0.02f;

EKeyCode forwards = Key_W;
EKeyCode backwards = Key_S;
EKeyCode rightTurn = Key_D;
EKeyCode leftTurn = Key_A;
EKeyCode quit = Key_Escape;

EKeyCode cameraForward = Key_Up;
EKeyCode cameraBackward = Key_Down;
EKeyCode cameraRight = Key_Right;
EKeyCode cameraLeft = Key_Left;
EKeyCode SpaceBar = Key_Space;

float mouseMoveX;
float mouseMoveY;

float carOldX;
float carOldZ;

float maxRotation = 0.1;

float rotateSpeed = 50.0f;

float cameraSpeed = 3.0f;

float dragFactor = -0.5f;
float thrustFactor = 0.02f;
float bumpFactor = 0.6f;

float floatspeed = 0.0002f;
float currentfloat = 0.0f;
float floatLimit = 0.4f;

float carRadius = 6.0f;
float wallWidth = 2.0f;
float wallLength = 10.0f;
float checkpointInX = 8.0f;
float checkpointinZ = 2.0f;

float checkpointZSpawn[2] = { 0.0f, 100.0f };
float checkpointXSpawn[2] = { 0.0f, 0.0f };
float checkpointRadius = 0.64f;

float checkpointMinX, checkpointMaxX, checkpointMinZ, checkpointMaxZ;

float wallZSpawn = 46.0f;
float wallXSpawn[2] = { -10.5f, 9.5f };
float isleXSpawn[4] = { 10.0f, -10.0f, 10.0f, -10.0f };
float isleZSpawn[4] = { 40.0f, 40.0f, 53.0f, 53.0f  };

float legPos = 9.86f;

float counter = 4.0f;		//used to countdown

float carMatrix[4][4];

//Deals with car movement (Using functions above. Scalar multiplaction to calculate thrust using drag, and momentum.
void carMovement(IModel* car)
{
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

	if (myEngine->KeyHeld(forwards))
	{
		//Makes sure the car just can't endlessly accelerate
		if (momentum.z > maxSpeed || momentum.z > maxSpeed || momentum.z < -maxSpeed || momentum.x < -maxSpeed)
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

	//nyoom
	drag = scalar(dragFactor * timer, momentum);
	momentum = sum(momentum, thrust, drag);

	car->Move(momentum.x, 0.0f, momentum.z);

}

//Floaty!
void carFloaty(IModel* model)
{
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

void cameraControl(float mouseX, float mouseY, I3DEngine* myEngine, ICamera* camera)
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
}

void legCollision(IModel* car, IModel* dummy[])
{
	//checkpoint detection (legs)
	for (int i = 0; i < 4; i++)
	{
		float x = car->GetX() - dummy[i]->GetX();
		float z = car->GetZ() - dummy[i]->GetZ();

		float distance = sqrt(x * x + z * z);
		
		if (distance < carRadius + checkpointRadius) //Sphere to sphere detection for checkpoint legs.
		{
			momentum = { -momentum.x, -momentum.z };
			thrust = { -thrust.x, thrust.z };
		}
	}
}

void wallCollision(IModel* wall[], IModel* car)
{
	for (int i = 0; i < 2; i++)
	{
		float wallMinX,	wallMaxX, wallMinZ, wallMaxZ;
		wallMinX = wall[i]->GetX() - wallWidth;
		wallMaxX = wall[i]->GetX() + wallWidth;
		wallMinZ = wall[i]->GetZ() - wallLength;
		wallMaxZ = wall[i]->GetZ() + wallLength;

		if (car->GetX() > wallMinX && car->GetX() < wallMaxX
			&& car->GetZ() > wallMinZ && car->GetZ() < wallMaxZ)
		{
			momentum = { -momentum.x , -momentum.z};
			thrust = { 0.0f, 0.0f };

		}
	}
}

void getCheckpoint(IModel* checkpoint)
{
	checkpointMinX = checkpoint->GetX() - checkpointInX;
	checkpointMaxX = checkpoint->GetX() + checkpointInX;
	checkpointMinZ = checkpoint->GetZ() - checkpointinZ;
	checkpointMaxZ = checkpoint->GetZ() + checkpointinZ;
}

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder( "E:\\Uni Stuff!\\CO1301 - Games Concepts\\Assignments\\Assignment 3\\media" );

	/**** Set up your scene here ****/

	//The usual models 'n' stuff
	ICamera* camera = myEngine->CreateCamera(kManual, 0.0f, 10.0f, -20.0f);
	camera->RotateLocalX(20.0f);

	ISprite* uiElement = myEngine->CreateSprite("testUI.png", 40, 560);

	IMesh* skyboxMesh = myEngine->LoadMesh("Skybox 07.x");
	IModel* skybox = skyboxMesh->CreateModel(0.0f, -960.0f, 0.0f);

	IMesh* floorMesh = myEngine->LoadMesh("ground.x");
	IModel* floor = floorMesh->CreateModel(0, 0, 0);

	IMesh* dummyMesh = myEngine->LoadMesh("dummy.x");

	IMesh* carMesh = myEngine->LoadMesh("race2.x");
	IModel* car = carMesh->CreateModel(0.0f, 0.01f, -50.0f);
	IMesh* checkpointMesh = myEngine->LoadMesh("Checkpoint.x");
	IModel* checkpoint[2];
	IModel* legdummy[4];
	

	for (int i = 0; i < 2; i++)
	{
		checkpoint[i] = checkpointMesh->CreateModel(checkpointXSpawn[i], 0, checkpointZSpawn[i]);
	}
	
	legdummy[0] = dummyMesh->CreateModel(legPos, 0, 0);
	legdummy[0]->AttachToParent(checkpoint[0]);

	legdummy[1] = dummyMesh->CreateModel(-legPos, 0, 0);
	legdummy[1]->AttachToParent(checkpoint[0]);

	legdummy[2] = dummyMesh->CreateModel(legPos, 0, 0);
	legdummy[2]->AttachToParent(checkpoint[1]);

	legdummy[3] = dummyMesh->CreateModel(-legPos, 0, 0);
	legdummy[3]->AttachToParent(checkpoint[1]);
	

	IMesh* isleMesh = myEngine->LoadMesh("IsleStraight.x");
	IModel* isle[4];

	IMesh* wallMesh = myEngine->LoadMesh("Wall.x");
	IModel* wall[2];

	IFont* myFont = myEngine->LoadFont("Charlemagne std", 30);

	//Loading walls
	for (int i = 0; i < 2; i++)
	{
		wall[i] = wallMesh->CreateModel(wallXSpawn[i], 0, wallZSpawn);
	}
	//isles
	for (int j = 0; j < 4; j++)
	{
		isle[j] = isleMesh->CreateModel(isleXSpawn[j], 0, isleZSpawn[j]);
	}
	currentState gameState = Waiting;

	camera->AttachToParent(car);
	timer = myEngine->Timer();

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();

		/**** Update your scene each frame here ****/
		timer = myEngine->Timer();
		
		if (gameState == Waiting)
		{
			myFont->Draw("Press Space to Start!", 150, 50, kRed);
			if (myEngine->KeyHit(SpaceBar))
			{
				gameState = Countdown;
			}
		}
		if (gameState == Countdown)
		{
			if (counter <= 4.0f && counter > 3.1f)
			{
				myFont->Draw("3..", 150, 50, kRed);
			}
			if (counter < 3.0f && counter > 2.0f)
			{
				myFont->Draw("2..", 150, 50, kRed );
			}
			if (counter < 2.0f && counter > 1.0f)
			{
				myFont->Draw("1..", 150, 50, kRed);
			}
			if (counter < 1.0f && counter > 0.0f)
			{
				myFont->Draw("Go!", 150, 50, kGreen);
			}
			if (counter < 0.0f)
			{
				myFont->Draw("Go!", 150, 50, kGreen);
				gameState = Go;
			}

			counter = counter - timer;
		}
		if (gameState == Go)
		{
			myFont->Draw(" ", 80, 600, kBlue);
			carFloaty(car);
			carMovement(car);
			//cameraControl(mouseMoveX, mouseMoveY, myEngine, camera);
			legCollision(car, legdummy);
			wallCollision(wall, car);

			getCheckpoint(checkpoint[0]);
			if (car->GetX() > checkpointMinX + carRadius && car->GetX() < checkpointMaxX + carRadius 
				&& car->GetZ() > checkpointMinZ + carRadius && car->GetZ() < checkpointMaxZ + carRadius)
			{
				gameState = FirstCheckpoint;
			}
		}
		if (gameState == FirstCheckpoint)
		{
			myFont->Draw("Stage 1 Complete", 80, 600, kRed);
			carFloaty(car);
			carMovement(car);
			//cameraControl(mouseMoveX, mouseMoveY, myEngine, camera);
			legCollision(car, legdummy);
			wallCollision(wall, car);

			getCheckpoint(checkpoint[1]);
			if (car->GetX() > checkpointMinX + carRadius && car->GetX() < checkpointMaxX + carRadius 
				&& car->GetZ() > checkpointMinZ + carRadius && car->GetZ() < checkpointMaxZ + carRadius)
			{
				gameState = Finish;
			}
		}
		if (gameState == Finish)
		{
			myFont->Draw("Race Complete!", 80, 600, kGreen);
			carFloaty(car);
			cameraControl(mouseMoveX, mouseMoveY, myEngine, camera);
			legCollision(car, legdummy);
			wallCollision(wall, car);
		}
		if (myEngine->KeyHit(quit))
		{
			myEngine->Stop();
		}
		
	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}


