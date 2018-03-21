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
	float leg = 9.86f;
};

struct vectors
{
	float x;
	float z;
};

vectors scalar(float scalar, vectors v)
{
	return{ scalar * v.x, scalar * v.z };
}

vectors sum(vectors v1, vectors v2, vectors v3)
{
	return{ v1.x + v2.x + v3.x, v1.z + v2.z + v3.z };
}

vectors momentum{ 0.0f, 0.0f };
vectors thrust{ 0.0f, 0.0f };
vectors drag{ 0.0f, 0.0f };

enum currentState {Countdown, Go, FirstCheckpoint, Finish};


I3DEngine* myEngine = New3DEngine(kTLX);

float maxSpeed = 0.03f;

EKeyCode forwards = Key_W;
EKeyCode backwards = Key_S;
EKeyCode rightTurn = Key_D;
EKeyCode leftTurn = Key_A;

float rotateSpeed = 50.0f;

float dragFactor = -0.5f;
float thrustFactor = 0.02f;

float floatspeed = 0.0002f;
float currentfloat = 0.0f;
float floatLimit = 0.4f;

float carRadius = 6.0f;

float checkpointZSpawn[2] = { 0.0f, 100.0f };

float wallZSpawn = 46.0f;
float wallXSpawn[2] = { -10.5f, 9.5f };
float isleXSpawn[4] = { 10.0f, -10.0f, 10.0f, -10.0f };
float isleZSpawn[4] = { 40.0f, 40.0f, 53.0f, 53.0f  };

float checkpointNubX = 9.86f;

float counter = 4.0f;

float carMatrix[4][4];
//Deals with car movement
void carMovement(IModel* model)
{
	model->GetMatrix(&carMatrix[0][0]);
	vectors facingVector = { carMatrix[2][0], carMatrix[2][2] };
	
	if (myEngine->KeyHeld(leftTurn))
	{
		model->RotateLocalY(-rotateSpeed*timer);
	}
	if (myEngine->KeyHeld(rightTurn))
	{
		model->RotateLocalY(rotateSpeed*timer);
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

	model->Move(momentum.x, 0.0f, momentum.z);
	
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
	IModel* cardummy = dummyMesh->CreateModel(0,0,0);
	vector <IModel*> checkpointdummy;

	IMesh* carMesh = myEngine->LoadMesh("race2.x");
	IModel* car = carMesh->CreateModel(0.0f, 0.01f, -50.0f);

	IMesh* checkpointMesh = myEngine->LoadMesh("Checkpoint.x");
	IModel* checkpoint[2];

	IMesh* isleMesh = myEngine->LoadMesh("IsleStraight.x");
	IModel* isle[4];

	IMesh* wallMesh = myEngine->LoadMesh("Wall.x");
	IModel* wall[2];

	IFont* myFont = myEngine->LoadFont("Charlemagne std", 30);

	//Loading checkpoints & walls
	for (int i = 0; i < 2; i++)
	{
		checkpoint[i] = checkpointMesh->CreateModel(0, 0, checkpointZSpawn[i]);
		for (int j = 0; j < 2; j++)
		{
			//spawn dummies for collision??
		}
		wall[i] = wallMesh->CreateModel(wallXSpawn[i], 0, wallZSpawn);
	}
	//isles
	for (int j = 0; j < 4; j++)
	{
		isle[j] = isleMesh->CreateModel(isleXSpawn[j], 0, isleZSpawn[j]);
	}
	currentState gameState = Countdown;

	car->AttachToParent(cardummy);
	camera->AttachToParent(car);
	timer = myEngine->Timer();
	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();

		/**** Update your scene each frame here ****/
		timer = myEngine->Timer();
		
		//CAN'T MAKE IT WORK????
		/*if (gameState == Countdown)
		{
			if (counter <= 4.0f && counter > 3.0f)
			{
				myFont->Draw("3..", 150, 50, kRed);
				counter = counter - myEngine->Timer();
			}
			if (counter < 2.9f && counter > 2.0f)
			{
				myFont->Draw("2..", 150, 50, kRed );
				counter = counter - myEngine->Timer();
			}

			if (counter < 1.9f && counter > 1.0f)
			{
				myFont->Draw("1..", 150, 50, kRed);
				counter = counter - myEngine->Timer();
			}
			if (counter < 0.9f && counter > 0.0f)
			{
				myFont->Draw("Go!", 150, 50, kGreen);
				counter = counter - myEngine->Timer();
			}
		}*/
		/*if (gameState == Go)
		{*/
			carFloaty(car);
			carMovement(car);
		//}
		
	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}
