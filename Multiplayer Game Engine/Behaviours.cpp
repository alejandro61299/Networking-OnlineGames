#include "Networks.h"
#include "Behaviours.h"



void Laser::start()
{
	gameObject->networkInterpolationEnabled = false;
	App->modSound->playAudioClip(App->modResources->audioClipLaser);
}

void Laser::update()
{
	secondsSinceCreation += Time.deltaTime;

	const float pixelsPerSecond = 1000.0f;
	gameObject->position += vec2FromDegrees(gameObject->angle) * pixelsPerSecond * Time.deltaTime;

	if (isServer)
	{
		const float neutralTimeSeconds = 0.1f;
		if (secondsSinceCreation > neutralTimeSeconds && gameObject->collider == nullptr) {
			gameObject->collider = App->modCollision->addCollider(ColliderType::Laser, gameObject);
		}

		const float lifetimeSeconds = 2.0f;
		if (secondsSinceCreation >= lifetimeSeconds) {
			NetworkDestroy(gameObject);
		}
	}
}

void Laser::write(OutputMemoryStream& packet)
{
	packet << ownerTag;
}

void Laser::read(const InputMemoryStream& packet)
{
	packet >> ownerTag;
}

void Spaceship::start()
{
	lifebar = Instantiate();
	lifebar->sprite = App->modRender->addSprite(lifebar);
	lifebar->sprite->pivot = vec2{ 0.0f, 0.5f };
	lifebar->sprite->order = 5;

	if (!isServer && gameObject->tag == App->modNetClient->GetClientId())
	{
		App->modNetClient->setPlayerGameObjectNetId(gameObject->networkId);
	}
} 

void Spaceship::onInput(const InputController &input)
{
	if (enableInput == false) return;

	if (input.horizontalAxis != 0.0f)
	{
		const float rotateSpeed = 180.0f;
		gameObject->angle += input.horizontalAxis * rotateSpeed * Time.deltaTime;

		if (isServer)
		{
			NetworkUpdate(gameObject);
		}
	}

	if (input.actionDown == ButtonState::Pressed)
	{
		const float advanceSpeed = 200.0f;
		
		gameObject->position += vec2FromDegrees(gameObject->angle) * advanceSpeed * Time.deltaTime;

		if (isServer)
		{
			NetworkUpdate(gameObject);
		}
	}

	if (input.actionLeft == ButtonState::Press)
	{
		if (isServer)
		{
			GameObject *laser = NetworkInstantiate();

			laser->position = gameObject->position;
			laser->angle = gameObject->angle;
			laser->size = { 20, 60 };

			laser->sprite = App->modRender->addSprite(laser);
			laser->sprite->order = 3;
			laser->sprite->texture = App->modResources->laser;

			Laser *laserBehaviour = App->modBehaviour->addLaser(laser);
			laserBehaviour->isServer = isServer;
			laserBehaviour->ownerTag = gameObject->tag;
			laserBehaviour->isServer = isServer;
		}
	}
}

void Spaceship::update()
{
	static const vec4 colorAlive = vec4{ 0.2f, 1.0f, 0.1f, 0.5f };
	static const vec4 colorDead = vec4{ 1.0f, 0.2f, 0.1f, 0.5f };
	const float lifeRatio = max(0.01f, (float)(hitPoints) / (MAX_HIT_POINTS));
	lifebar->position = gameObject->position + vec2{ -50.0f, -50.0f };
	lifebar->size = vec2{ lifeRatio * 80.0f, 5.0f };
	lifebar->sprite->color = lerp(colorDead, colorAlive, lifeRatio);

	if (gameObject->interpolationTime < 0.1f)
	{
		//Interpolation
		float ratio = gameObject->interpolationTime / 0.1f;
		gameObject->position = lerp(gameObject->initialPosition, gameObject->finalPosition, ratio);


		gameObject->angle = lerp(gameObject->initialAngle, gameObject->finalAngle, ratio);

		

	}
	else
	{
		//LOG("%f", gameObject->interpolationTime);
	}
	gameObject->interpolationTime += Time.deltaTime;

}

void Spaceship::destroy()
{
	Destroy(lifebar);
}

void Spaceship::onCollisionTriggered(Collider &c1, Collider &c2)
{
	if (c2.type == ColliderType::Laser)
	{
		Laser* laser = (Laser*)c2.gameObject->behaviour;
		if (laser->ownerTag == gameObject->tag) return;

		if (isServer)
		{
			// Destroy the laser
			NetworkDestroy(c2.gameObject);
		
			if (hitPoints > 0)
			{
				hitPoints--;
				NetworkUpdate(gameObject);
			}

			// Destroy ship
			float size;
			vec2  position;

			if (hitPoints <= 0)
			{
				// Big Random explosion
				GameManager::spawnExplosion(true, gameObject->position, 0.f);
				// Remove gameObject on client proxy data
				App->modNetServer->gameManager.despawnPlayer(gameObject->tag);
			}
			else
			{
				// Little Random explosion
				GameManager::spawnExplosion(false, gameObject->position, 0.f);
			}

			App->modSound->playAudioClip(App->modResources->audioClipExplosion);
			// NOTE(jesus): Only played in the server right now...
			// You need to somehow make this happen in clients
		}
	}
}

void Spaceship::write(OutputMemoryStream & packet)
{
	packet << hitPoints;
	packet << enableInput;
}

void Spaceship::read(const InputMemoryStream & packet)
{
	packet >> hitPoints;
	packet >> enableInput;
}

void Gemstone::start()
{
	gameObject->networkInterpolationEnabled = false;
}

void Gemstone::update()
{
	if (ownerTag != UINT32_MAX)
	{
		GameObject* playerSpaceship = App->modGameObject->FindGameObjectByTag(ownerTag);

		if (playerSpaceship == nullptr)
		{
			ownerTag = UINT32_MAX;
			return;
		}

		float speed = 2.f;
		vec2 offset = gameObject->position - playerSpaceship->position;
		float distance = length(offset);

		if (distance > 90.f)
			gameObject->position = lerp(gameObject->position, playerSpaceship->position, speed * Time.deltaTime);
	}
}

void Gemstone::onCollisionTriggered(Collider& c1, Collider& c2)
{
	if (c2.type == ColliderType::Player)
	{
		if (isServer)
		{
			if (ownerTag != UINT32_MAX) return;
			Spaceship* spaceship = (Spaceship*)c2.gameObject->behaviour;
			ownerTag = spaceship->gameObject->tag;
			NetworkUpdate(gameObject);
		}
	}
}

void Gemstone::write(OutputMemoryStream& packet)
{
	packet << ownerTag;
}

void Gemstone::read(const InputMemoryStream& packet)
{
	packet >> ownerTag;
}

void Pointer::update()
{
	GameObject* playerSpaceship = App->modGameObject->FindGameObjectByTag(ownerTag);
	GameObject* gemstoneGo = App->modGameObject->FindGameObjectByTag(GEMSTONE_TAG);
		
	if (playerSpaceship == nullptr || gemstoneGo == nullptr )
	{
		gameObject->sprite->order = -5;
		return;
	}
	else
		gameObject->sprite->order = 6;

	vec2 offset = gemstoneGo->position - playerSpaceship->position;
	float distance = length(offset);

	if (distance <= 400.f && distance >= 150.f)
		gameObject->sprite->color.a = lerp(0.f, 1.f, (distance - 150.f) / 250.f );
	else if (distance < 150.f)
		gameObject->sprite->order = -5;
	else
		gameObject->sprite->color.a = 1.f;

	offset = normalize(offset);
	offset = 100.f * offset;

	gameObject->position =  playerSpaceship->position + offset;
	gameObject->angle = 90.f + atan2(offset.y, offset.x) * RAD2DEG;
}
