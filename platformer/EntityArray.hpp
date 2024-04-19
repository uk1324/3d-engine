#pragma once

#include <RefOptional.hpp>
#include <Types.hpp>
#include <Assertions.hpp>

#include <vector>

// New entity version can only be taken once per frame, because the entites are removed and added at the end of the frame. So to overflow a 32 bit version number running at 60 fps it would take 828 days. So this is how long an entity would need to be kept for between frames for the entity array to think that an old entity is a new entity.
template<typename Entity>
struct EntityArray {
	struct Id {
		friend EntityArray;
		Id() : index_{ 0xCDCDCDCD }, version_{ 0xCDCDCDCD } {}
		auto operator==(const Id&) const -> bool = default;
		auto index() const->i32;
		auto version() const->i32;
	private:
		Id(u32 index, u32 version) : index_{ index }, version_{ version } {}
		u32 index_;
		u32 version_;
	};

	// TODO: Maybe make iterator and pair the same thing. They essentially do the same thing. One issues is that you wouldn't be able to do structured bindings on the object, because it contains array (which you wouldn't want). It uses the index so you would need to call a function to get the id. It also can't store a reference to the entity only a pointer because it needs to modify it. Technically you could reinterpret the reference as a pointer.
	struct Pair {
		Id id;
		Entity& entity;
		auto operator->()->Entity*;
		auto operator->() const -> const Entity*;
	};

	auto update() -> void;
	auto get(const Id& id) -> std::optional<Entity&>;
	auto isAlive(const Id& id) -> bool;
	auto isAlive(const Entity& entity) -> bool;
	auto create() -> Pair;
	auto create(Entity&& entity) -> Pair;
	auto destroy(const Id& id) -> void;
	auto destroy(const Entity& entity) -> void;
	auto validate(u32 index) -> std::optional<Id>;
	auto reset() -> void;
	auto aliveCount() -> i64 { return aliveCount_; };

	struct Iterator {
		auto operator++()->Iterator&;
		auto operator!=(const Iterator& other) const -> bool;
		auto operator->()->Entity*;
		auto operator->() const -> const Entity*;
		auto operator*()->Pair;

		u32 index;
		EntityArray& array;
	};

	auto begin() -> Iterator;
	auto end() -> Iterator;
	std::vector<Entity> entities;
	std::vector<u32> entityVersions;
	// Could use a set instead of entityIsFree and freeEntities, but it would probably be slower. Don't want to do a red-black tree search for each iterator increment.
	std::vector<bool> entityIsFree;
private:

	i64 aliveCount_ = 0;

	std::vector<u32> freeEntities;
	// To make pooling more efficient could make a templated function that wouold reset the state of an object. So for example if the type stored a vector it would just clear it instead of calling the constructor, which would cause a reallocation.

	// Delaying entity removal till the end of the frame so systems like for example the collision system can assume that the entites it received as added last frame are still alive at the end of the frame.
	// First add entites the remove so if the entites that was created in one frame and deleted in the same frame gets deleted. This shouldn't even be able to happen, because the entity adding is also buffered till the end of the frame.

	std::vector<Id> entitiesToRemove;
	std::vector<Id> entitiesAddedLastFrame_;
	std::vector<Id> entitiesAddedThisFrame;
	// Could delay the creating of entites until the end of frame. One advantage of doing this is that you can loop over entities and add new ones. The entites would still be created inside the entites list, but the versions would be updated (this also requires the version zero to always be an invalid version, could also make sure it wraps around to 1 on overflow). The created entity ids would be added to at toAdd list, which would be iterated in the update function and the versions would be updated there. Would need to make sure that there aren't any issues if an entity was destroyed on the same frame it was created. Could either first add entites the destroy them or remove all the entites, which are both inside the add and remove list. !!! This wouldn't actually work, when adding an entity pointers could get invalidated so you would need to only allow iterating over indices and only allow access using indices, could make a class that just stores the index and on operator -> gives access to the entity or could just save them into a separate vector, but if I wanted to implmenet the pooling of more complex types so types that store for exapmle vector don't need to get reallocated then this pooling would also need to work for this list.
public:
	auto entitiesAddedLastFrame() const -> const std::vector<Id>& { return entitiesAddedLastFrame_; }
};

namespace std {

	template<typename Entity>
	// For some reason it doesn't work if you put EntityArray<Entity>::Id as the specialization (typename doesn't help), but it does work if you just put anything in here so I put EntityArray<Entity>.
	struct hash<EntityArray<Entity>> {
		using argument_type = typename EntityArray<Entity>::Id;
		using result_type = size_t;

		result_type operator ()(const argument_type& key) const {
			return std::hash<i32>()(key.index()) * std::hash<i32>()(key.index());
		}
	};

}

template<typename Entity>
auto EntityArray<Entity>::update() -> void {
	ASSERT(entities.size() == entityVersions.size());
	ASSERT(entityVersions.size() == entityIsFree.size());

	std::swap(entitiesAddedThisFrame, entitiesAddedLastFrame_);
	entitiesAddedThisFrame.clear();

	for (const auto id : entitiesToRemove) {
		if (id.index_ >= entities.size()) {
			ASSERT_NOT_REACHED();
			return;
		}

		if (id.version_ != entityVersions[id.index_]) {
			// Should double free be an error?
			ASSERT_NOT_REACHED();
			return;
		}

		freeEntities.push_back(id.index_);
		entityVersions[id.index_]++;
		entityIsFree[id.index_] = true;
		aliveCount_--;
	}
	entitiesToRemove.clear();
}

template<typename Entity>
auto EntityArray<Entity>::get(const Id& id) -> std::optional<Entity&> {
	if (id.index_ >= entities.size()) {
		ASSERT_NOT_REACHED();
		return std::nullopt;
	}

	if (id.version_ != entityVersions[id.index_]) {
		return std::nullopt;
	}

	return entities[id.index_];
}

template<typename Entity>
auto EntityArray<Entity>::isAlive(const Id& id) -> bool {
	return get(id).has_value();
}

template<typename Entity>
auto EntityArray<Entity>::isAlive(const Entity& entity) -> bool {
	const auto index = &entity - entities.data();
	const auto id = validate(index);
	if (!id.has_value()) {
		ASSERT_NOT_REACHED();
		return false;
	}
	return isAlive(id);
}

template<typename Entity>
auto EntityArray<Entity>::create() -> Pair {
	create(Entity{});
}

template<typename Entity>
auto EntityArray<Entity>::create(Entity&& entity) -> Pair {
	Id id;
	if (freeEntities.size() == 0) {
		id = Id{ static_cast<u32>(entities.size()), 0 };
		entities.push_back(Entity{ std::move(entity) });
		entityVersions.push_back(0);
		entityIsFree.push_back(false);
	} else {
		const auto index = freeEntities.back();
		freeEntities.pop_back();
		entityIsFree[index] = false;
		id = Id{ index, entityVersions[index] };
		new (&entities[index]) Entity{ std::move(entity) };
	}

	aliveCount_++;
	entitiesAddedThisFrame.push_back(id);
	return { id, entities[id.index_] };
}

template<typename Entity>
auto EntityArray<Entity>::destroy(const Id& id) -> void {
	entitiesToRemove.push_back(id);
}

template<typename Entity>
auto EntityArray<Entity>::destroy(const Entity& entity) -> void {
	const auto index = static_cast<u32>(&entity - entities.data());
	const auto id = validate(index);
	if (!id.has_value()) {
		ASSERT_NOT_REACHED();
		return;
	}
	destroy(*id);
}

template<typename Entity>
auto EntityArray<Entity>::validate(u32 index) -> std::optional<Id> {
	if (index >= entities.size()) {
		return std::nullopt;
	}

	if (entityIsFree[index]) {
		return std::nullopt;
	}

	return Id{ index, entityVersions[index] };
}

template<typename Entity>
auto EntityArray<Entity>::reset() -> void {
	freeEntities.clear();
	// Iterating backwards so the ids a given out in order, which is required for level loading.
	for (i32 i = static_cast<i32>(entities.size()) - 1; i >= 0; i--) {
		freeEntities.push_back(i);
		entityIsFree[i] = true;
	}
	entitiesToRemove.clear();
	entitiesAddedLastFrame_.clear();
	entitiesAddedThisFrame.clear();
}

template<typename Entity>
auto EntityArray<Entity>::begin() -> Iterator {
	u32 index = 0;
	while (index < entityIsFree.size() && entityIsFree[index]) {
		index++;
	}
	return Iterator{ index, *this };
}

template<typename Entity>
auto EntityArray<Entity>::end() -> Iterator {
	return Iterator{ static_cast<u32>(entities.size()), *this };
}

template<typename Entity>
auto EntityArray<Entity>::Iterator::operator++() -> Iterator& {
	if (*this != array.end()) {
		index++;
	}
	while (*this != array.end() && array.entityIsFree[index]) {
		index++;
	}

	return *this;
}

template<typename Entity>
auto EntityArray<Entity>::Iterator::operator!=(const Iterator& other) const -> bool {
	ASSERT(&array == &other.array);
	return index != other.index;
}

template<typename Entity>
auto EntityArray<Entity>::Iterator::operator->() -> Entity* {
	return &array.entities[index];
}

template<typename Entity>
inline auto EntityArray<Entity>::Iterator::operator->() const -> const Entity* {
	return &array.entities[index];
}

template<typename Entity>
auto EntityArray<Entity>::Iterator::operator*() -> Pair {
	return Pair{ Id{ index, array.entityVersions[index] }, array.entities[index] };
}

template<typename Entity>
auto EntityArray<Entity>::Id::index() const -> i32 {
	return index_;
}

template<typename Entity>
auto EntityArray<Entity>::Id::version() const -> i32 {
	return version_;
}

template<typename Entity>
auto EntityArray<Entity>::Pair::operator->() -> Entity* {
	return &entity;
}

template<typename Entity>
auto EntityArray<Entity>::Pair::operator->() const -> const Entity* {
	return &entity;
}
