#include "Networks.h"
#include <string>
#include "ModuleResources.h"

#if defined(USE_TASK_MANAGER)

void ModuleResources::TaskLoadTexture::execute()
{
	(*texture) = App->modTextures->loadTexture(filename);
	(*texture)->id = id;
}

#endif

Texture* ModuleResources::getTextureNumber(int num)
{
	if (num == 0) return  number0;
	else if (num == 1) return  number1;
	else if (num == 2) return  number2;
	else if (num == 3) return  number3;
	else if (num == 4) return  number4;
	else if (num == 5) return  number5;
	else if (num == 6) return  number6;
	else if (num == 7) return  number7;
	else if (num == 8) return  number8;
	else if (num == 9) return  number9;
	else return nullptr;
}

bool ModuleResources::init()
{
	background = App->modTextures->loadTexture("background.jpg");

#if !defined(USE_TASK_MANAGER)
	space = App->modTextures->loadTexture("space_background.jpg");
	asteroid1 = App->modTextures->loadTexture("asteroid1.png");
	asteroid2 = App->modTextures->loadTexture("asteroid2.png");
	spacecraft1 = App->modTextures->loadTexture("spacecraft1.png");
	spacecraft2 = App->modTextures->loadTexture("spacecraft2.png");
	spacecraft3 = App->modTextures->loadTexture("spacecraft3.png");
	loadingFinished = true;
	completionRatio = 1.0f;
#else
	loadTextureAsync("space_background.jpg",	&space);
	loadTextureAsync("asteroid1.png",			&asteroid1);
	loadTextureAsync("asteroid2.png",			&asteroid2);
	loadTextureAsync("spacecraft1.png",			&spacecraft1);
	loadTextureAsync("spacecraft2.png",			&spacecraft2);
	loadTextureAsync("spacecraft3.png",			&spacecraft3);
	loadTextureAsync("laser.png",				&laser);
	loadTextureAsync("explosion1.png",			&explosion1);
	loadTextureAsync("gemstone.png",			&gemstone);
	loadTextureAsync("arrow.png",				&arrow);
	loadTextureAsync("waiting_text.png",		&waitingText);
	loadTextureAsync("victory_text.png",		&victoryText);
	loadTextureAsync("defeat_text.png",			&defeatText);
	loadTextureAsync("0.png",					&number0);
	loadTextureAsync("1.png",					&number1);
	loadTextureAsync("2.png",					&number2);
	loadTextureAsync("3.png",					&number3);
	loadTextureAsync("4.png",					&number4);
	loadTextureAsync("5.png",					&number5);
	loadTextureAsync("6.png",					&number6);
	loadTextureAsync("7.png",					&number7);
	loadTextureAsync("8.png",					&number8);
	loadTextureAsync("9.png",					&number9);

	//for (int i = 0; i < 10; ++i)
	//{
	//	numbers[i] = nullptr;
	//	std::string file = std::to_string(i) + std::string(".png");
	//	loadTextureAsync(file.data(), &numbers[i]);
	//}

#endif

	audioClipLaser = App->modSound->loadAudioClip("laser.wav");
	audioClipExplosion = App->modSound->loadAudioClip("explosion.wav");
	//App->modSound->playAudioClip(audioClipExplosion);

	return true;
}

#if defined(USE_TASK_MANAGER)

void ModuleResources::loadTextureAsync(const char * filename, Texture **texturePtrAddress)
{
	ASSERT(taskCount < MAX_RESOURCES);
	
	TaskLoadTexture *task = &tasks[taskCount];
	task->owner = this;
	task->filename = filename;
	task->texture = texturePtrAddress;
	task->id = taskCount++; // Hardcode texture ID problem

	App->modTaskManager->scheduleTask(task, this);
}

void ModuleResources::onTaskFinished(Task * task)
{
	ASSERT(task != nullptr);

	TaskLoadTexture *taskLoadTexture = dynamic_cast<TaskLoadTexture*>(task);

	for (uint32 i = 0; i < taskCount; ++i)
	{
		if (task == &tasks[i])
		{
			finishedTaskCount++;
			task = nullptr;
			break;
		}
	}

	ASSERT(task == nullptr);

	if (finishedTaskCount == taskCount)
	{
		finishedLoading = true;

		// Create the explosion animation clip
		explosionClip = App->modRender->addAnimationClip();
		explosionClip->frameTime = 0.1f;
		explosionClip->loop = false;
		for (int i = 0; i < 16; ++i)
		{
			float x = (i % 4) / 4.0f;
			float y = (i / 4) / 4.0f;
			float w = 1.0f / 4.0f;
			float h = 1.0f / 4.0f;
			explosionClip->addFrameRect(vec4{ x, y, w, h });
		}
	}
}

#endif
