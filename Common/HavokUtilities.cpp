#pragma warning (disable: 4305)
//you need this. it creates pointer to characters which Havok Engine uses
//if you put this line into HavokUtilities.h you will get re-definition errors
//because all cpp files which includes HavokUtilities.h will try to create these character pointers in their .obj copies
//and this will create Linking redefinition problems
#include <Common/Base/keycode.cxx>

//You do not want to include the animation headers at this point
//if you comment this out it will give unresolved symbol errors for animation libraries
#ifdef HK_FEATURE_PRODUCT_ANIMATION
#undef HK_FEATURE_PRODUCT_ANIMATION
#endif
//I need to do that because of some memory linkings at compile time
//if you comment this out it will give unresolved symbol errors
#ifndef HK_EXCLUDE_LIBRARY_hkgpConvexDecomposition
#define HK_EXCLUDE_LIBRARY_hkgpConvexDecomposition
#endif

//this is for linking hkBase.lib
//if you comment this out it will give unresolved symbol errors for hkBase.lib
#include <Common/Base/Config/hkProductFeatures.cxx> 

//you want to include this for being able to define the following struct and function
#include <Physics/Collide/hkpCollide.h>	
//these lines are needed for getting rid of linker problems for hkpCollide.lib
//I could not find a better solution
//source for why I do this --> http://software.intel.com/en-us/forums/showthread.php?t=73886
//struct hkTestEntry* hkUnitTestDatabase = HK_NULL; 
hkBool HK_CALL hkTestReport(hkBool32 cond, const char* desc, const char* file, int line) {return false;}
#include "HavokUtilities.hpp"

HavokUtilities::HavokUtilities(bool visualDebuggerActive)
{
	m_bVisualDebuggerActive = visualDebuggerActive;
	if (visualDebuggerActive==true)
	{//RDS Begin
		#define HAVOK_VISUAL_DEBUGGER_ENABLED 1
	}//RDS End
	initHavok();
}
//////////////////////////////////////////////////////////////////HAVOK BASIC NEEDED FUNCTION --> START,END,REGISTER etc.
HavokUtilities::~HavokUtilities(void)
{
	deinitHavok();
}

void HavokUtilities::initHavok()
{	
	{//initialize Havok Memory
		// Allocate 0.5MB of physics solver buffer.
		//hkMemoryRouter* m_pMemoryRouter;
		m_pMemoryRouter = hkMemoryInitUtil::initDefault(hkMallocAllocator::m_defaultMallocAllocator, hkMemorySystem::FrameInfo(500000));
		hkBaseSystem::init( m_pMemoryRouter, errorReport );
	}
	{// Initialize the multi-threading classes, hkJobQueue, and hkJobThreadPool
		// Most of the comments are copied and pasted from ConsoleExampleMt.cpp of HavokDemos folder (2010 version)
		// They can be used for all Havok multithreading tasks. In this exmaple we only show how to use
		// them for physics, but you can reference other multithreading demos in the demo framework
		// to see how to multithread other products. The model of usage is the same as for physics.
		// The hkThreadpool has a specified number of threads that can run Havok jobs.  These can work
		// alongside the main thread to perform any Havok multi-threadable computations.
		// The model for running Havok tasks in Spus and in auxilary threads is identical.  It is encapsulated in the
		// class hkJobThreadPool.  On PlayStation(R)3 we initialize the SPU version of this class, which is simply a SPURS taskset.
		// On other multi-threaded platforms we initialize the CPU version of this class, hkCpuJobThreadPool, which creates a pool of threads
		// that run in exactly the same way.  On the PlayStation(R)3 we could also create a hkCpuJobThreadPool.  However, it is only
		// necessary (and advisable) to use one Havok PPU thread for maximum efficiency. In this case we simply use this main thread
		// for this purpose, and so do not create a hkCpuJobThreadPool.
		// Get the number of physical threads available on the system
		//hkHardwareInfo m_hardwareInfo;
		hkGetHardwareInfo(m_hardwareInfo);
		m_iTotalNumThreadsUsed = m_hardwareInfo.m_numThreads;

		// We use one less than this for our thread pool, because we must also use this thread for our simulation
		//hkCpuJobThreadPoolCinfo m_threadPoolCinfo;
		m_threadPoolCinfo.m_numThreads = m_iTotalNumThreadsUsed - 1;

		//RDS_PREVDEFINITIONS this line is from previous HavokWrapper
		//m_threadPoolCinfo.m_allocateRuntimeMemoryBlocks = true;
		// This line enables timers collection, by allocating 200 Kb per thread.  If you leave this at its default (0),
		// timer collection will not be enabled.
		m_threadPoolCinfo.m_timerBufferPerThreadAllocation = 200000;
		m_pThreadPool = new hkCpuJobThreadPool( m_threadPoolCinfo );

		// We also need to create a Job queue. This job queue will be used by all Havok modules to run multithreaded work.
		// Here we only use it for physics.
		m_jobQueuInfo.m_jobQueueHwSetup.m_numCpuThreads = m_iTotalNumThreadsUsed;
		m_pJobQueue = new hkJobQueue(m_jobQueuInfo);

		//
		// Enable monitors for this thread.
		//

		// Monitors have been enabled for thread pool threads already (see above comment).
		hkMonitorStream::getInstance().resize(200000);
	}

	{// <PHYSICS-ONLY>: Create the physics world.			
		// At this point you would initialize any other Havok modules you are using.
		// The world cinfo contains global simulation parameters, including gravity, solver settings etc.

		// Set the simulation type of the world to multi-threaded.
		m_worldInfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_MULTITHREADED;

		// Flag objects that fall "out of the world" to be automatically removed - just necessary for this physics scene
		// In other words, objects that fall "out of the world" will be automatically removed
		m_worldInfo.m_broadPhaseBorderBehaviour = hkpWorldCinfo::BROADPHASE_BORDER_REMOVE_ENTITY;
	
		//RDS_HARDCODED values here --> just for me to see if I can make this part better
		//standard gravity settings, collision tolerance and world size 
		m_worldInfo.m_gravity.set(0,-9.8f,0);
		//I do not know what the next line does. For this demo it doesnt change anything if/if not having this line enabled
		//m_worldInfo.setupSolverInfo(hkpWorldCinfo::SOLVER_TYPE_8ITERS_MEDIUM);
		m_worldInfo.m_collisionTolerance = 0.1f;
		//This will effect the removal of objects when they fall out of the world.
		//If you have BROADPHASE_BORDER_REMOVE_ENTITY then your entities will be removed from Havok according to this number you set
		m_worldInfo.setBroadPhaseWorldSize(5000.0f);

		//initialize world with created info
		m_pPhysicsWorld = new hkpWorld(m_worldInfo);

		// RDS_PREVDEFINITIONS this line is from another HavokWrapper. Helps when using VisualDebugger
		// Disable deactivation, so that you can view timers in the VDB. This should not be done in your game.
		m_pPhysicsWorld->m_wantDeactivation = false;


		// When the simulation type is SIMULATION_TYPE_MULTITHREADED, in the debug build, the sdk performs checks
		// to make sure only one thread is modifying the world at once to prevent multithreaded bugs. Each thread
		// must call markForRead / markForWrite before it modifies the world to enable these checks.
		m_pPhysicsWorld->markForWrite();

		// Register all collision agents, even though only box - box will be used in this particular example.
		// It's important to register collision agents before adding any entities to the world.
		hkpAgentRegisterUtil::registerAllAgents( m_pPhysicsWorld->getCollisionDispatcher() );

		// We need to register all modules we will be running multi-threaded with the job queue
		m_pPhysicsWorld->registerWithJobQueue( m_pJobQueue );

		// Now we have finished modifying the world, release our write marker.
		m_pPhysicsWorld->unmarkForWrite();
	}

	{//RDS Begin --> you can use such a way to enable VisualDebugger
	 //so that you  do not need commenting out some lines all the time you change it
		#ifdef HAVOK_VISUAL_DEBUGGER_ENABLED
			registerVisualDebugger();
		#endif	
	}//RDS End
}

void HavokUtilities::deinitHavok()
{
	//clean up physics world
	m_pPhysicsWorld->markForWrite();
	m_pPhysicsWorld->removeReference();

	//clean up visual debugger(if used)
	#ifdef HAVOK_VISUAL_DEBUGGER_ENABLED
		m_pVisualDebugger->removeReference();
		m_pPhysicsContext->removeReference();
	#endif	

	//delete job queue, thread poll, deallocate memory
	delete m_pJobQueue;
	m_pThreadPool->removeReference();

	//quit base system, this also deallocates the buffer which is used for Havok
	hkBaseSystem::quit();
}

void HavokUtilities::registerVisualDebugger()
{// Initialize the VDB
	// <PHYSICS-ONLY>: Register physics specific visual debugger processes
	// By default the VDB will show debug points and lines, however some products such as physics and cloth 
	// have additional viewers that can show geometries etc and can be enabled and disabled by the VDB app.
	{
		// The visual debugger so we can connect remotely to the simulation
		// The context must exist beyond the use of the VDB instance, and you can make
		// whatever contexts you like for your own viewer types.
	
		m_pPhysicsContext = new hkpPhysicsContext();// Create context for Visual Debugger
		hkpPhysicsContext::registerAllPhysicsProcesses(); //Reigster all the physics viewers
		
		m_pPhysicsWorld->markForWrite();//RDS for the following line to be able to have effect on m_pPhysicsWorld
		m_pPhysicsContext->addWorld(m_pPhysicsWorld); // add the physics world so the viewers can see it
		m_arrayPhysicsContext.pushBack(m_pPhysicsContext);

		// Now we have finished modifying the world, release our write marker.
		m_pPhysicsWorld->unmarkForWrite();
	}

	//Create VDB instance
	m_pVisualDebugger = new hkVisualDebugger(m_arrayPhysicsContext);
	m_pVisualDebugger->serve();
}

void HavokUtilities::stepSimulation(float deltaTime)
{
	//step multithreaded simulation using this thread and all threads in the thread pool
	m_pPhysicsWorld->stepMultithreaded(m_pJobQueue, m_pThreadPool, deltaTime);
	stepVisualDebugger(deltaTime);
	hkMonitorStream::getInstance().reset();
	m_pThreadPool->clearTimerData();		
}

void HavokUtilities::stepVisualDebugger(float deltaTime)
{
	#ifdef HAVOK_VISUAL_DEBUGGER_ENABLED
		//synchronize the timer data and step Visual Debugger
		m_pPhysicsContext->syncTimers( m_pThreadPool );
		m_pVisualDebugger->step();
	#endif	
}

hkpWorld* HavokUtilities::getWorld()
{
	return m_pPhysicsWorld;
}
hkVisualDebugger* HavokUtilities::getVisualDebugger()
{
	return m_pVisualDebugger;
}
///////////////////////////////////////////////////////////////////HAVOK PHYSICS WRAPPER FUNCTIONS
void HavokUtilities::addFixedSurface(const hkVector4& position, 
					 const hkVector4& dimensions)
{
	//addFixedSurface function
	//creates fixed surface with specified position and dimensions

	//create box shape using given dimensions
	hkReal m_fhkConvexShapeRadius=0.05;
	hkpShape* fixedSurfaceShape = new hkpBoxShape(dimensions,m_fhkConvexShapeRadius);

	//create rigid body information structure 
	hkpRigidBodyCinfo m_rigidBodyInfo;
	
	//MOTION_FIXED means static element in game scene
	m_rigidBodyInfo.m_mass = 0.0;
	m_rigidBodyInfo.m_shape = fixedSurfaceShape;
	m_rigidBodyInfo.m_motionType = hkpMotion::MOTION_FIXED;
	m_rigidBodyInfo.m_position = position;
	hkVector4 m_fAxis = hkVector4(0.0,0.0,1.0);
	hkReal m_fAngle=-0.1;
	m_rigidBodyInfo.m_rotation = hkQuaternion(m_fAxis,m_fAngle);

	//create new rigid body with supplied info
	hkpRigidBody* m_pRigidBody = new hkpRigidBody(m_rigidBodyInfo);

	//add rigid body to physics world
	m_pPhysicsWorld->lock();
	m_pPhysicsWorld->addEntity(m_pRigidBody);

	//decerase reference counter for rigid body and shape
	m_pRigidBody->removeReference();
	fixedSurfaceShape->removeReference();

	m_pPhysicsWorld->unlock();
}

void HavokUtilities::addMovingBoxes(const hkVector4& position, 
					 const hkVector4& dimensions)
{
	//addMovingBoxes function
	//creates moving boxes with specified position and dimensions

	//create box shape using given dimensions
	hkReal m_fhkConvexShapeRadius=0.05;
	hkpShape* movingBodyShape = new hkpBoxShape(dimensions,m_fhkConvexShapeRadius);

	// Compute the inertia tensor from the shape
	hkpMassProperties m_massProperties;
	hkReal m_massOfBox = 5.0;
	hkpInertiaTensorComputer::computeShapeVolumeMassProperties(movingBodyShape, m_massOfBox, m_massProperties);

	//create rigid body information structure 
	hkpRigidBodyCinfo m_rigidBodyInfo;
	
	// Assign the rigid body properties
	m_rigidBodyInfo.m_position = position;
	m_rigidBodyInfo.m_mass = m_massProperties.m_mass;
	m_rigidBodyInfo.m_centerOfMass = m_massProperties.m_centerOfMass;
	m_rigidBodyInfo.m_inertiaTensor = m_massProperties.m_inertiaTensor;
	m_rigidBodyInfo.m_shape = movingBodyShape;
	m_rigidBodyInfo.m_motionType = hkpMotion::MOTION_BOX_INERTIA;

	//create new rigid body with supplied info
	hkpRigidBody* m_pRigidBody = new hkpRigidBody(m_rigidBodyInfo);

	//add rigid body to physics world
	m_pPhysicsWorld->lock();
	m_pPhysicsWorld->addEntity(m_pRigidBody);

	//decerase reference counter for rigid body and shape
	m_pRigidBody->removeReference();
	movingBodyShape->removeReference();

	m_pPhysicsWorld->unlock();
}
