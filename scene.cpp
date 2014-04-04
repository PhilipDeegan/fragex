/**

./scene.hpp

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
#include "scene.hpp"

void Scene::setUp   (IrrlichtDevice *device){}

void Scene::tearDown   (IrrlichtDevice *device){}

bool Scene::keyDown(IrrlichtDevice *device, irr::EKEY_CODE keyCode) {	
	return SceneKeyHandler::keyDown	(device, *this, keyCode); 
}
bool Scene::keyUp  (IrrlichtDevice *device, irr::EKEY_CODE keyCode) {	
	return SceneKeyHandler::keyUp	(device, *this, keyCode); 
}
bool Scene::OnEvent(IrrlichtDevice *device, const SEvent& event){
	if(event.EventType == EET_MOUSE_INPUT_EVENT && event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN){}
	return false;
}

void Scene::draw(IrrlichtDevice *device){
	if (device->isWindowActive()){
		irr::video::IVideoDriver * driver	= device->getVideoDriver();
		driver->beginScene(true, true, video::SColor(0, 255, 255, 255));		
		
		uint fps = driver->getFPS();
		if(lFps != fps){
			core::stringw c(L"Regex Fractals in irrlicht - [");
			c += driver->getName() ;
			c += L"] FPS: ";
			c += fps;
			device->setWindowCaption(c.c_str());
		}
		if(console.isShown()) {
			console.draw(device);
			device->getGUIEnvironment()->drawAll();
		}
		else{
			for(const Quadrant& q : pixels)
				driver->drawPixel(q.x, q.y, irr::video::SColor(255, q.r, q.g, q.b));
		}
		driver->endScene();
	}else{
		device->yield();
	}
}
