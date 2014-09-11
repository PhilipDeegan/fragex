/**

./main.cpp

Created on: 16 May 2013

Copyright (c) 2013, Philip Deegan

This file is part of fragex.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <irrlicht.h>
#include "driverChoice.h"

#include "irrlight.hpp"

#include <scene.hpp>

int main(int argc, char* argv[]) {	
	using namespace irrlight;
	std::shared_ptr<SceneGraph> sg(SceneGraph::INSTANCE());
	irr::IrrlichtDevice& device = *createDevice(video::EDT_BURNINGSVIDEO, core::dimension2d<u32>(512, 512), 32, false, false, false, sg.get());
	//irr::IrrlichtDevice * device  = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(512, 384), 32, false, false, false, SceneGraph::getInstance());

	sg->device(&device);
	sg->scene(new Scene(&device));

	try{
		while(device.run() && device.getVideoDriver())  sg->draw();		
	}
	catch(std::exception & e) {
		std::cout << e.what() << std::endl;
	}

	device.closeDevice();
	return 0;
}
