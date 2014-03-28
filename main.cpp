//============================================================================
// Name        : main.cpp
// Author      : 
// Version     :
// Copyright   : 
// Description : 
//============================================================================

#include <irrlicht.h>
#include "driverChoice.h"

#include "kul/cgi/scene.hpp"
#include "kul/cgi/scene/graph.hpp"
#include "kul/cgi/irr/2D/isometric.hpp"


class Scene : public AScene {
	public:
		Scene() {}
		~Scene();
		void setUp 		(IrrlichtDevice *device){}
		void tearDown   (IrrlichtDevice *device){}
		void draw 		(IrrlichtDevice *device){}
		bool keyDown	(IrrlichtDevice *device, irr::EKEY_CODE keyCode)	{ return false;}
		bool keyUp 		(IrrlichtDevice *device, irr::EKEY_CODE keyCode)	{ return false;}
		bool OnEvent	(IrrlichtDevice *device, const irr::SEvent& event)	{ return false;}
};



int main(int argc, char* argv[]) {	
	using namespace kul::cgi;
	std::shared_ptr<SceneGraph> sg(SceneGraph::getInstance());
	irr::IrrlichtDevice& device = *createDevice(video::EDT_BURNINGSVIDEO, core::dimension2d<u32>(800, 600), 32, false, false, false, sg.get());
	//irr::IrrlichtDevice * device  = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(512, 384), 32, false, false, false, SceneGraph::getInstance());

	sg->setDevice(&device);
	sg->setScene(new Scene());

	try{
		while(device.run() && device.getVideoDriver())  sg->draw();		
	}
	catch(std::exception & e) {
		std::cout << e.what() << std::endl;
	}

	device.closeDevice();
	return 0;
}
