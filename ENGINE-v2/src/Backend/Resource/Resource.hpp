#pragma once

enum class ResourceType {
    Food,
    Wood,
    Stone,
    Gold,
    Iron
};

class ResourceStack
{
public:
    ResourceStack(ResourceType type, int amount, int maxAmount);

    ResourceType getType() const;
    int getAmount() const;
    int getMaxAmount() const;

    void add(int value);
    int remove(int value);

private:
    ResourceType type;
    int amount;
    int maxAmount;
};
