#pragma once

class Archive;

class Serializable {
public:
	~Serializable() = default;
	virtual void serialize(Archive& arch) const = 0;
	virtual void deserialize(const Archive& arch) = 0;
};