//============================================================================
// Name        : main.cpp
// Author      : 
// Version     :
// Copyright   : 
// Description : 
//============================================================================

#include <irrlicht.h>
#include "driverChoice.h"

#include "irrlight/scene.hpp"
#include "irrlight/scene/graph.hpp"

#include <scene.hpp>

int main(int argc, char* argv[]) {	
	using namespace irrlight;
	std::shared_ptr<SceneGraph> sg(SceneGraph::getInstance());
	irr::IrrlichtDevice& device = *createDevice(video::EDT_BURNINGSVIDEO, core::dimension2d<u32>(512, 512), 32, false, false, false, sg.get());
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
