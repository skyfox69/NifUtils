#pragma once
#ifndef _HAVOK_UTILITIES_
#define _HAVOK_UTILITIES_

// Math and base include
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/System/Error/hkDefaultError.h>
#include <Common/Base/Memory/System/Util/hkMemoryInitUtil.h>
#include <Common/Base/Monitor/hkMonitorStream.h>
#include <Common/Base/Memory/System/hkMemorySystem.h>
#include <Common/Base/Memory/Allocator/Malloc/hkMallocAllocator.h>

/////////////////////////////////////////////////////////////////////////////////////////////// Dynamics includes
#include <Physics/Collide/hkpCollide.h>	
#include <Physics/Collide/Agent/ConvexAgent/SphereBox/hkpSphereBoxAgent.h>	
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>					
#include <Physics/Collide/Shape/Convex/Sphere/hkpSphereShape.h>				
#include <Physics/Collide/Dispatch/hkpAgentRegisterUtil.h>					

#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastInput.h>			
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastOutput.h>			

#include <Physics/Dynamics/World/hkpWorld.h>								
#include <Physics/Dynamics/Entity/hkpRigidBody.h>							
#include <Physics/Utilities/Dynamics/Inertia/hkpInertiaTensorComputer.h>	

#include <Common/Base/Thread/Job/ThreadPool/Cpu/hkCpuJobThreadPool.h>
#include <Common/Base/Thread/Job/ThreadPool/Spu/hkSpuJobThreadPool.h>
#include <Common/Base/Thread/JobQueue/hkJobQueue.h>

/////////////////////////////////////////////////////////////////////////////////////////////// Visual Debugger includes
#include <Common/Visualize/hkVisualDebugger.h>
#include <Physics/Utilities/VisualDebugger/hkpPhysicsContext.h>				

/////////////////////////////////////////////////////////////////////////////////////////////// Keycode

#include <stdio.h>

static void HK_CALL errorReport(const char* msg, void* userArgGivenToInit)
{
	printf("%s", msg);
}
/*
Class HavokUtilities
Pre-Author: Piotr Pluta (The creator of this project for Havok SDK 6.50) - http://piotrpluta.opol.pl - June 2009
Author : Recep Doga Siyli (Updating the previous project to Havok SDK 7.10)
(I just changed the way of handling the memory system accordingly and also the include files)
(I tried to update the comments --> not much changes to the pre-written comments but also additions
from the ConsoleExampleMt comments)

Class used to encapsulate Havok Physics data and initialize Havok engine. 
Only one instance is needed (refactor to singleton if desired)

Interface overview:
    
- HavokUtilities() sets default timestep and calls private initHavok() method (data initialization).
- ~HavokUtilities() calls deinitHavok(). This way, if HavokUtilities object is destroyed, all resources assigned by it will be freed.
- registerVisualDebugger() sets up Visual Debugger. If you want to use VD, you have to call this after creating HavokUtilities object
- stepSimulation(float dt) and stepVisualDebugger(float dt) methods have to be called inside some loop. They tell the simulation and VD to step forward by delta time dt.
- getWorld() returns pointer to hkpWorld (Havok world instance)
- getVisualDebugger() returns pointer to hkVisualDebugger (VD instance)

Usage:

//Create class instance. Doing this initializes Havok automatically.
HavokUtilities* havokUtilities = new HavokUtilities();

//Register Havok Visual Debugger(if needed)
havokUtilities->registerVisualDebugger()

//Step simulation and visual debugger in a loop
havokUtilities->stepSimulation(timestep);
havokUtilities->stepVisualDebugger(timestep);

//delete HavokUtilities. All resources aquired during initialization will be released
delete havokUtilities;
*/

class HavokUtilities
{
public:
	HavokUtilities(bool visualDebuggerActive);
	virtual ~HavokUtilities(void);
	
	void registerVisualDebugger();

	void stepSimulation(float deltaTime);	
	void stepVisualDebugger(float deltaTime);

	hkpWorld* getWorld();
	hkVisualDebugger* getVisualDebugger();
	
private:
	void initHavok();
	void deinitHavok();

	//Variables for memory needs of HAVOK
		hkMemoryRouter* m_pMemoryRouter;
		hkHardwareInfo m_hardwareInfo;
		int m_iTotalNumThreadsUsed;
		hkCpuJobThreadPoolCinfo m_threadPoolCinfo;
		hkJobThreadPool* m_pThreadPool;
		hkJobQueueCinfo m_jobQueuInfo;
		hkJobQueue* m_pJobQueue;

	//World control variables
		hkpWorldCinfo m_worldInfo;
		hkpWorld* m_pPhysicsWorld;

	//Visual debugger
		bool m_bVisualDebuggerActive;
		hkArray<hkProcessContext*> m_arrayPhysicsContext;
		hkVisualDebugger* m_pVisualDebugger;
		hkpPhysicsContext* m_pPhysicsContext;
public :
	//you can add more functionality just by adding new function here
	void addFixedSurface(const hkVector4& position, const hkVector4& dimensions);
	void addMovingBoxes(const hkVector4& position, const hkVector4& dimensions);
};

#endif