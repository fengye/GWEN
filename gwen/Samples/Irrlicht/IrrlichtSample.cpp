/* 
 * An Irrlicht GWen sample
 *
 * Author: Ye Feng
 *
 */

#include <irrlicht.h>

#include "Gwen/Gwen.h"
#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/UnitTest/UnitTest.h"
#include "Gwen/Input/Irrlicht.h"
#include "Gwen/Renderers/Irrlicht.h"

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

class MyEventReceiver : public IEventReceiver
{
public:
	MyEventReceiver(Gwen::Controls::Canvas* pCanvas) : input(pCanvas)
	{ 
	}

	virtual bool OnEvent(const SEvent& event)
	{
		return input.HandleEvents(event);
	}

private:
	Gwen::Input::Irrlicht input;
};

/*
This is the main method. We can now use main() on every platform.
*/
int main()
{
	/*
	The most important function of the engine is the createDevice()
	function. The IrrlichtDevice is created by it, which is the root
	object for doing anything with the engine. createDevice() has 7
	parameters:

	- deviceType: Type of the device. This can currently be the Null-device,
	   one of the two software renderers, D3D8, D3D9, or OpenGL. 

	- windowSize: Size of the Window or screen in FullScreenMode to be
	   created. In this example we use 640x480.

	- bits: Amount of color bits per pixel. This should be 16 or 32. The
	   parameter is often ignored when running in windowed mode.

	- fullscreen: Specifies if we want the device to run in fullscreen mode
	   or not.

	- stencilbuffer: Specifies if we want to use the stencil buffer (for
	   drawing shadows).

	- vsync: Specifies if we want to have vsync enabled, this is only useful
	   in fullscreen mode.

	- eventReceiver: An object to receive events.

	Always check the return value to cope with unsupported drivers,
	dimensions, etc.
	*/
	IrrlichtDevice *device =
		createDevice(video::EDT_OPENGL, dimension2d<u32>(1004, 650), 32,
			false, false, false, NULL);

	if (!device)
		return 1;

	/*
	Set the caption of the window to some nice text. Note that there is an
	'L' in front of the string. The Irrlicht Engine uses wide character
	strings when displaying text.
	*/
	device->setWindowCaption(L"GWEN - Irrlicht Sample");

	/*
	Get a pointer to the VideoDriver, the SceneManager and the graphical
	user interface environment, so that we do not always have to write
	device->getVideoDriver(), device->getSceneManager(), or
	device->getGUIEnvironment().
	*/
	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager* smgr = device->getSceneManager();
	IGUIEnvironment* guienv = device->getGUIEnvironment();

	//
	// Create a GWEN Irrlicht Renderer
	//
	Gwen::Renderer::Irrlicht* pRenderer = new Gwen::Renderer::Irrlicht(device);
	//
	// Create a GWEN skin
	//
	Gwen::Skin::TexturedBase* pSkin = new Gwen::Skin::TexturedBase(pRenderer);
	pSkin->Init("DefaultSkin.png");
	// The fonts work differently in Allegro - it can't use
	// system fonts. So force the skin to use a local one.
	pSkin->SetDefaultFont(L"OpenSans.ttf", 11);
	//
	// Create a Canvas (it's root, on which all other GWEN panels are created)
	//
	Gwen::Controls::Canvas* pCanvas = new Gwen::Controls::Canvas(pSkin);
	pCanvas->SetSize(998, 650 - 24);
	pCanvas->SetDrawBackground(true);
	pCanvas->SetBackgroundColor(Gwen::Color(150, 170, 170, 255));
	//
	// Create our unittest control (which is a Window with controls in it)
	//
	UnitTest* pUnit = new UnitTest(pCanvas);
	pUnit->SetPos(10, 10);

	MyEventReceiver* eventReceiver = new MyEventReceiver(pCanvas);
	device->setEventReceiver(eventReceiver);

	while(device->run())
	{
		driver->beginScene(true, true, SColor(255,100,101,140));

		smgr->drawAll();
		guienv->drawAll();

		pCanvas->RenderCanvas();

		driver->endScene();
	}

	delete eventReceiver;
	delete pCanvas;
	delete pSkin;
	delete pRenderer;
	/*
	After we are done with the render loop, we have to delete the Irrlicht
	Device created before with createDevice(). In the Irrlicht Engine, you
	have to delete all objects you created with a method or function which
	starts with 'create'. The object is simply deleted by calling ->drop().
	See the documentation at irr::IReferenceCounted::drop() for more
	information.
	*/
	device->drop();

	return 0;
}
