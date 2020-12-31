#pragma once

#define GEMSTONE_TAG 20U
#define GEMSTONE_POINTS_INTERVAL 1.f

enum class BehaviourType : uint8;

struct Behaviour
{
	GameObject *gameObject = nullptr;
	bool isServer = false;
	bool isLocalPlayer = false;

	virtual BehaviourType type() const = 0;

	virtual void start() { }

	virtual void onInput(const InputController &input) { }

	virtual void update() { }

	virtual void destroy() { }

	virtual void onCollisionTriggered(Collider &c1, Collider &c2) { }

	virtual void write(OutputMemoryStream &packet) { }

	virtual void read(const InputMemoryStream &packet) { }
};


enum class BehaviourType : uint8
{
	None,
	Spaceship,
	Laser,
	Gemstone,
	Pointer
};


struct Laser : public Behaviour
{
	float secondsSinceCreation = 0.0f;

	BehaviourType type() const override { return BehaviourType::Laser; }

	void start() override;

	void update() override;

	void write(OutputMemoryStream& packet) override;

	void read(const InputMemoryStream& packet) override;

	uint32 ownerTag = UINT32_MAX;
};


struct Spaceship : public Behaviour
{
	static const uint8 MAX_HIT_POINTS = 5;
	uint8 hitPoints = MAX_HIT_POINTS;
	bool enableInput = true;

	GameObject *lifebar = nullptr;

	BehaviourType type() const override { return BehaviourType::Spaceship; }

	void start() override;

	void onInput(const InputController &input) override;

	void update() override;

	void destroy() override;

	void onCollisionTriggered(Collider &c1, Collider &c2) override;

	void write(OutputMemoryStream &packet) override;

	void read(const InputMemoryStream &packet) override;
};

struct Gemstone : public Behaviour
{
	uint32 ownerTag = UINT32_MAX;

	float timePoints = 0.f;

	BehaviourType type() const override { return BehaviourType::Gemstone; }

	void start() override;

	void update() override;

	void onCollisionTriggered(Collider& c1, Collider& c2) override;

	void write(OutputMemoryStream& packet) override;

	void read(const InputMemoryStream& packet) override;
};

struct Pointer : public Behaviour
{
	BehaviourType type() const override { return BehaviourType::Pointer; }

	void update() override;

	uint32 ownerTag;
};