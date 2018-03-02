///////////////////////////////////
// Assignment 3 - Hover Racing	 //
// by Nathan Hendley			 //
///////////////////////////////////

#include <TL-Engine.h>;	// TL-Engine include file and namespace
using namespace tle;

float angle = 0.0f;
float radian = 0.0f;
float pi = 3.14159265359f;

float xDir = 0.0f;
float zDir = 0.0f;

bool floatingUp = true;
float Zero = 0.0f;
I3DEngine* myEngine = New3DEngine(kTLX);

EKeyCode forwards = Key_W;
EKeyCode backwards = Key_S;
EKeyCode rightTurn = Key_D;
EKeyCode leftTurn = Key_A;

float moveSpeed = 0.0000005f;
float currentSpeed = 0.0f;
float maxSpeed = 0.01f;
float maxBackSpeed = -0.001f;
float rotateSpeed = 0.004f;
float bankSpeed = 0.003f;

float floatspeed = 0.0002f;
float currentfloat = 0.0f;
float floatLimit = 0.4f;

float carRadius = 6.0f;

float checkpointZSpawn[2] = { 0.0f, 100.0f };

float wallZSpawn = 46.0f;
float wallXSpawn[2] = { -10.5f, 9.5f };
float isleXSpawn[4] = { 10.0f, -10.0f, 10.0f, -10.0f };
float isleZSpawn[4] = { 40.0f, 40.0f, 53.0f, 53.0f  };

//prototyping
void angleToRadian(float angle);
void radianToVector(float radian, float currentSpeed);

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

	IMesh* skyboxMesh = myEngine->LoadMesh("Skybox 07.x");
	IModel* skybox = skyboxMesh->CreateModel(0.0f, -960.0f, 0.0f);

	IMesh* floorMesh = myEngine->LoadMesh("ground.x");
	IModel* floor = floorMesh->CreateModel(0, 0, 0);

	IMesh* carMesh = myEngine->LoadMesh("race2.x");
	IModel* car = carMesh->CreateModel(0.0f, 0.01f, -50.0f);

	IMesh* checkpointMesh = myEngine->LoadMesh("Checkpoint.x");
	IModel* checkpoint[2];

	IMesh* isleMesh = myEngine->LoadMesh("IsleStraight.x");
	IModel* isle[4];

	IMesh* wallMesh = myEngine->LoadMesh("Wall.x");
	IModel* wall[2];

	//Loading checkpoints & walls
	for (int i = 0; i < 2; i++)
	{
		checkpoint[i] = checkpointMesh->CreateModel(0, 0, checkpointZSpawn[i]);
		wall[i] = wallMesh->CreateModel(wallXSpawn[i], 0, wallZSpawn);
	}
	//isles
	for (int j = 0; j < 4; j++)
	{
		isle[j] = isleMesh->CreateModel(isleXSpawn[j], 0, isleZSpawn[j]);
	}
	
	camera->AttachToParent(car);

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();

		/**** Update your scene each frame here ****/
		angleToRadian(angle);
		radianToVector(radian, currentSpeed);
		car->Move(xDir * moveSpeed, 0, zDir * moveSpeed);

		if (floatingUp == true)
		{
			car->MoveY(floatspeed);
			if (car->GetY() > floatLimit)
			{
				floatingUp = false;
			}
		}
		if (floatingUp == false)
		{
			car->MoveY(-floatspeed);
			if (car->GetY() < -floatLimit)
			{
				floatingUp = true;
			}
		}

		if (myEngine->KeyHeld(forwards))
		{
			if (currentSpeed < maxSpeed)
			{
				currentSpeed = currentSpeed + moveSpeed;
			}
			else
			{
				currentSpeed;
			}
		}
		if (myEngine->KeyHeld(backwards))
		{
			if (currentSpeed > maxBackSpeed)
			{
				currentSpeed = currentSpeed - moveSpeed;
			}
			else
			{
				currentSpeed;
			}
		}
		if (myEngine->KeyHeld(leftTurn))
		{
			angle = angle + rotateSpeed;
		}
		if (myEngine->KeyHeld(rightTurn))
		{
			angle = angle - rotateSpeed;
		}
	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}

void angleToRadian(float angle)
{
	radian = (angle * pi) / 180;
}
void radianToVector(float radian, float currentSpeed)
{
	xDir = sin(radian);
	zDir = cos(radian);
}