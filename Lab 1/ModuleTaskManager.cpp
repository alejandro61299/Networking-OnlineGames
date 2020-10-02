#include "ModuleTaskManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <list>
#include <condition_variable>

std::mutex mtx;

void ModuleTaskManager::threadMain()
{
	while (true)
	{
		// TODO 3:
		// - Wait for new tasks to arrive
		// - Retrieve a task from scheduledTasks
		// - Execute it
		// - Insert it into finishedTasks

		{
			std::unique_lock<std::mutex> lock(mtx);
			while (scheduledTasks.empty()) {
				event.wait(lock);
				if (exitFlag)
					return;
			}
		}

		{
			std::unique_lock<std::mutex> lock(mtx);  // <--  Block the memory accces from other threads

			if (!scheduledTasks.empty()) {

				Task* task = scheduledTasks.front();
				scheduledTasks.pop();
				task->execute();
				finishedTasks.push(task);
			}
		}

	}
}

bool ModuleTaskManager::init()
{
	// TODO 1: Create threads (they have to execute threadMain())

	for (int i = 0; i < MAX_THREADS; ++i)
	{
		threads[i] = std::thread(&ModuleTaskManager::threadMain, this);
	}

	return true;
}

bool ModuleTaskManager::update()
{
	// TODO 4: Dispatch all finished tasks to their owner module (use Module::onTaskFinished() callback)

	{
		std::unique_lock<std::mutex> lock(mtx);
		while (!finishedTasks.empty())
		{
			Task* task = finishedTasks.front();
			finishedTasks.pop();
			task->owner->onTaskFinished(task);
		}

	}

	return true;
}

bool ModuleTaskManager::cleanUp()
{
	// TODO 5: Notify all threads to finish and join them
	{
		std::unique_lock<std::mutex> lock(mtx);
		exitFlag = true;
		event.notify_all();
	}

	for (int i = 0; i < MAX_THREADS; ++i)
	{
		threads[i].join();
	}


	return true;
}

void ModuleTaskManager::scheduleTask(Task *task, Module *owner)
{
	// TODO 2: Insert the task into scheduledTasks so it is executed by some thread
	
	std::unique_lock<std::mutex> lock(mtx);
	task->owner = owner;
	scheduledTasks.push(task);
	event.notify_one();

}
