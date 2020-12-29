#include "Networks.h"
#include "ModuleGameObject.h"

void GameObject::write(OutputMemoryStream& packet, const bool useFlags)
{
	if (useFlags)
	{
		packet << updateFlags;
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::POSITION)))
	{
		packet << position.x;
		packet << position.y;
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::ROTATION)))
	{
		packet << angle;
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::SIZE)))
	{
		packet << size.x;
		packet << size.y;
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::TAG)))
	{
		packet << tag;
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::SPRITE)))
	{
		if (sprite != nullptr && sprite->texture != nullptr) {
			packet << sprite->texture->id;
			packet << sprite->order;
			packet << sprite->color.r;
			packet << sprite->color.g;
			packet << sprite->color.b;
			packet << sprite->color.a;
			packet << sprite->pivot.x;
			packet << sprite->pivot.y;
		}
		else {
			packet << -1;
		}
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::ANIMATION)))
	{
		if (animation != nullptr && animation->clip != nullptr) {
			packet << true;
			packet << animation->clip->id;
			packet << animation->currentFrame;
			packet << animation->elapsedTime;
		}
		else {
			packet << false;
		}
	}	
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::COLLIDER)))
	{
		if (collider != nullptr)
		{
			packet << true;
			packet << collider->type;
			packet << collider->isTrigger;
		}
		else
		{
			packet << false;
		}
	}		

	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::BEHAVIOUR)))
	{
		if (behaviour != nullptr)
		{
			packet << true;
			packet << behaviour->type();
			behaviour->write(packet);
		}
		else {
			packet << false;
		}
	}	
}

void GameObject::read(const InputMemoryStream& packet, const bool useFlags)
{
	if (useFlags)
	{
		packet >> updateFlags;
	}

	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::POSITION)))
	{
		packet >> position.x;
		packet >> position.y;
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::ROTATION)))
	{
		packet >> angle;
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::SIZE)))
	{
		packet >> size.x;
		packet >> size.y;
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::TAG)))
	{
		packet >> tag;
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::SPRITE)))
	{
		int textureID = -1;
		packet >> textureID;

		if (textureID != -1) {
			if (sprite == nullptr) 
			{
				sprite = App->modRender->addSprite(this);
			}

			sprite->texture = App->modTextures->getTextureById(textureID);
			packet >> sprite->order;
			packet >> sprite->color.r;
			packet >> sprite->color.g;
			packet >> sprite->color.b;
			packet >> sprite->color.a;
			packet >> sprite->pivot.x;
			packet >> sprite->pivot.y;
		}
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::ANIMATION)))
	{
		bool hasAnimation = false;
		packet >> hasAnimation;

		if (hasAnimation) 
		{
			if (animation == nullptr)
			{
				animation = App->modRender->addAnimation(this);
			}

			uint16 clipId = 0;
			packet >> clipId;
			animation->clip = App->modRender->getAnimationClip(clipId);
			packet >> animation->currentFrame;
			packet >> animation->elapsedTime;
		}
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::COLLIDER)))
	{
		bool hasCollider = false;
		packet >> hasCollider;

		if (hasCollider)
		{
			ColliderType colliderType = ColliderType::None;
			packet >> colliderType;

			if (collider == nullptr)
			{
				collider = App->modCollision->addCollider(colliderType, this);
			}

			packet >> collider->isTrigger;
		}
	}

	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::BEHAVIOUR)))
	{
		bool hasBehaviour = false;
		packet >> hasBehaviour;

		if (hasBehaviour)
		{ 
			BehaviourType behaviourType = BehaviourType::None;
			packet >> behaviourType;

			if (behaviour == nullptr)
			{
				behaviour = App->modBehaviour->addBehaviour(behaviourType, this);
			}

			behaviour->read(packet);
		}
	}
}

void GameObject::readDummy(const InputMemoryStream& packet, const bool useFlags)
{
	if (useFlags)
	{
		packet >> updateFlags;
	}

	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::POSITION)))
	{
		packet >> position.x;
		packet >> position.y;
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::ROTATION)))
	{
		packet >> angle;
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::SIZE)))
	{
		packet >> size.x;
		packet >> size.y;
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::TAG)))
	{
		packet >> tag;
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::SPRITE)))
	{
		int textureID = -1;
		packet >> textureID;

		if (textureID != -1) {
			int spriteDummyInt;
			float spriteDummy[6];
			packet >> spriteDummyInt;
			packet >> spriteDummy[0];
			packet >> spriteDummy[1];
			packet >> spriteDummy[2];
			packet >> spriteDummy[3];
			packet >> spriteDummy[4];
			packet >> spriteDummy[5];
		}
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::ANIMATION)))
	{
		bool hasAnimation = false;
		packet >> hasAnimation;

		if (hasAnimation)
		{

			uint16 clipId = 0;
			packet >> clipId;

			uint8 currentFrameDummy = 0;
			packet >> currentFrameDummy;

			float elapsedTimeDummy = 0.f;
			packet >> elapsedTimeDummy;
		}
	}
	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::COLLIDER)))
	{
		bool hasCollider = false;
		packet >> hasCollider;

		if (hasCollider)
		{
			ColliderType colliderType = ColliderType::None;
			packet >> colliderType;

			bool colliderIsTriggerDummy;
			packet >> colliderIsTriggerDummy;
		}
	}

	if (!useFlags || (useFlags && HasUpdateFlag(UpdateFlags::BEHAVIOUR)))
	{
		bool hasBehaviour = false;
		packet >> hasBehaviour;

		if (hasBehaviour)
		{
			BehaviourType behaviourType = BehaviourType::None;
			packet >> behaviourType;


			Spaceship ship;
			Laser laser;
			Gemstone gemstone;

			switch (behaviourType)
			{
			case BehaviourType::Spaceship:
				
				ship.read(packet);
				break;
			case BehaviourType::Laser:
				
				laser.read(packet);
				break;
			case BehaviourType::Gemstone:
				
				gemstone.read(packet);
				break;
			default:
				break;
			}
		}
	}
}

bool ModuleGameObject::init()
{
	return true;
}

bool ModuleGameObject::preUpdate()
{
	BEGIN_TIMED_BLOCK(GOPreUpdate);

	static const GameObject::State gNextState[] = {
		GameObject::NON_EXISTING, // After NON_EXISTING
		GameObject::STARTING,     // After INSTANTIATE
		GameObject::UPDATING,     // After STARTING
		GameObject::UPDATING,     // After UPDATING
		GameObject::DESTROYING,   // After DESTROY
		GameObject::NON_EXISTING  // After DESTROYING
	};

	for (GameObject &gameObject : gameObjects)
	{
		gameObject.state = gNextState[gameObject.state];
	}

	END_TIMED_BLOCK(GOPreUpdate);

	return true;
}

bool ModuleGameObject::update()
{
	// Delayed destructions
	for (DelayedDestroyEntry &destroyEntry : gameObjectsWithDelayedDestruction)
	{
		if (destroyEntry.object != nullptr)
		{
			destroyEntry.delaySeconds -= Time.deltaTime;
			if (destroyEntry.delaySeconds <= 0.0f)
			{
				Destroy(destroyEntry.object);
				destroyEntry.object = nullptr;
			}
		}
	}

	return true;
}

bool ModuleGameObject::postUpdate()
{
	return true;
}

bool ModuleGameObject::cleanUp()
{
	return true;
}

GameObject * ModuleGameObject::Instantiate()
{
	for (uint32 i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		GameObject &gameObject = App->modGameObject->gameObjects[i];

		if (gameObject.state == GameObject::NON_EXISTING)
		{
			gameObject = GameObject();
			gameObject.id = i;
			gameObject.state = GameObject::INSTANTIATE;
			return &gameObject;
		}
	}

	ASSERT(0); // NOTE(jesus): You need to increase MAX_GAME_OBJECTS in case this assert crashes
	return nullptr;
}

void ModuleGameObject::Destroy(GameObject * gameObject)
{
	ASSERT(gameObject->networkId == 0); // NOTE(jesus): If it has a network identity, it must be destroyed by the Networking module first

	static const GameObject::State gNextState[] = {
		GameObject::NON_EXISTING, // After NON_EXISTING
		GameObject::DESTROY,      // After INSTANTIATE
		GameObject::DESTROY,      // After STARTING
		GameObject::DESTROY,      // After UPDATING
		GameObject::DESTROY,      // After DESTROY
		GameObject::DESTROYING    // After DESTROYING
	};

	ASSERT(gameObject->state < GameObject::STATE_COUNT);
	gameObject->tag = UINT32_MAX;
	gameObject->state = gNextState[gameObject->state];
}

void ModuleGameObject::Destroy(GameObject * gameObject, float delaySeconds)
{
	for (uint32 i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		if (App->modGameObject->gameObjectsWithDelayedDestruction[i].object == nullptr)
		{
			App->modGameObject->gameObjectsWithDelayedDestruction[i].object = gameObject;
			App->modGameObject->gameObjectsWithDelayedDestruction[i].delaySeconds = delaySeconds;
			break;
		}
	}
}

GameObject* ModuleGameObject::FindGameObjectByTag(uint32 tag)
{
	for (GameObject& gameObject : App->modGameObject->gameObjects)
	{
		if (gameObject.tag == tag)
		{
			return &gameObject;
		}
	}

	return nullptr;
}

GameObject * Instantiate()
{
	GameObject *result = ModuleGameObject::Instantiate();
	return result;
}

void Destroy(GameObject * gameObject)
{
	ModuleGameObject::Destroy(gameObject);
}

void Destroy(GameObject * gameObject, float delaySeconds)
{
	ModuleGameObject::Destroy(gameObject, delaySeconds);
}

