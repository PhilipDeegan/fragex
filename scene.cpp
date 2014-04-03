/*
 * Scene.cpp
 *
 *  Created on: 13 Mar 2012
 *	  Author: dekken
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
