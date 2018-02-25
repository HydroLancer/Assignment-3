// Assignment 3.cpp: A program using the TL-Engine

#include <TL-Engine.h>	// TL-Engine include file and namespace
using namespace tle;

struct SCamera
{
	ICamera* camera;
};

struct SPlayerCar
{

};

struct SWalls
{
	IModel* 
};

struct SCheckpoint
{

};

SCamera camera;
SPlayerCar playerCar;
SWalls wall;
SCheckpoint checkpoint;

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine( kTLX );
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder( "E:\\Uni Stuff!\\CO1301 - Games Concepts\\Assignments\\Assignment 3\\media" );

	/**** Set up your scene here ****/
	camera = myEngine->CreateCamera(kManual, 0.0f, -20.0f, 20.0f);

	IMesh* skyboxMesh = myEngine->LoadMesh("Skybox 07.x");
	IModel* skybox = skyboxMesh->CreateModel(0.0f, -960.0f, 0.0f);

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();

		/**** Update your scene each frame here ****/

	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}
