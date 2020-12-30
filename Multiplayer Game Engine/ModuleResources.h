#pragma once

#define USE_TASK_MANAGER

struct Texture;

class ModuleResources : public Module
{
public:

	Texture *background = nullptr;
	Texture *space = nullptr;
	Texture *asteroid1 = nullptr;
	Texture *asteroid2 = nullptr;
	Texture *spacecraft1 = nullptr;
	Texture *spacecraft2 = nullptr;
	Texture *spacecraft3 = nullptr;
	Texture *laser = nullptr;
	Texture *explosion1 = nullptr;
	Texture *gemstone = nullptr;
	Texture *arrow = nullptr;


	Texture *waitingText = nullptr;
	Texture *readyText = nullptr;
	Texture *letsrockText = nullptr;
	Texture *respawnText = nullptr;
	Texture *victoryText = nullptr;
	Texture *defeatText = nullptr;

	Texture* number0 = nullptr;
	Texture* number1 = nullptr;
	Texture* number2 = nullptr;
	Texture* number3 = nullptr;
	Texture* number4 = nullptr;
	Texture* number5 = nullptr;
	Texture* number6 = nullptr;
	Texture* number7 = nullptr;
	Texture* number8 = nullptr;
	Texture* number9 = nullptr;

	//Texture* numbers[10];

	AnimationClip *explosionClip = nullptr;
	AudioClip *audioClipLaser = nullptr;
	AudioClip *audioClipExplosion = nullptr;

	bool finishedLoading = false;

	Texture* getTextureNumber(int num);

private:

	bool init() override;

#if defined(USE_TASK_MANAGER)
	
	class TaskLoadTexture : public Task
	{
	public:
		int id = 0;
		const char *filename = nullptr;
		Texture **texture = nullptr;
		void execute() override;
	};

	static const int MAX_RESOURCES = 30;
	TaskLoadTexture tasks[MAX_RESOURCES] = {};
	uint32 taskCount = 0;
	uint32 finishedTaskCount = 0;

	void onTaskFinished(Task *task) override;

	void loadTextureAsync(const char *filename, Texture **texturePtrAddress);

#endif

};

