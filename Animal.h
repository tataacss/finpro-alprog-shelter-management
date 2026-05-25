#ifndef ANIMAL_H
#define ANIMAL_H

#include <iostream>
#include <string>

class ShelterAnimal {
protected:
    std::string animalId;
    std::string name;
    double baseCareCost;
    bool adopted;
    int priorityScore;

public:
    ShelterAnimal(const std::string& id, const std::string& n, double cost, int priority)
        : animalId(id), name(n), baseCareCost(cost),
          adopted(false), priorityScore(priority) {}

    virtual ~ShelterAnimal() {}

    virtual double calculateMonthlyExpense() = 0;
    virtual std::string getSpecies() = 0;
    virtual void calculatePriority() = 0;

    std::string getAnimalId() { return animalId; }
    std::string getName() { return name; }
    bool getAdoptionStatus() { return adopted; }

    void processAdoption() {
        adopted = true;
    }

    int getPriorityScore() {
        return priorityScore;
    }

    virtual std::string toJSON() {
        return
            "{\"id\":\"" + animalId +
            "\",\"name\":\"" + name +
            "\",\"species\":\"" + getSpecies() +
            "\",\"expense\":" + std::to_string((int)calculateMonthlyExpense()) +
            ",\"priority\":" + std::to_string(priorityScore) +
            ",\"adopted\":\"" + std::string(adopted ? "YES" : "NO") +
            "\"}";
    }
};

class Cat : public ShelterAnimal {
public:
    Cat(const std::string& id, const std::string& n, double cost)
        : ShelterAnimal(id, n, cost, 70) {}

    double calculateMonthlyExpense() override {
        return baseCareCost + 50000;
    }

    std::string getSpecies() override {
        return "CAT";
    }

    void calculatePriority() override {
        priorityScore = 70;
    }
};

class Dog : public ShelterAnimal {
public:
    Dog(const std::string& id, const std::string& n, double cost)
        : ShelterAnimal(id, n, cost, 90) {}

    double calculateMonthlyExpense() override {
        return (baseCareCost * 1.3) + 100000;
    }

    std::string getSpecies() override {
        return "DOG";
    }

    void calculatePriority() override {
        priorityScore = 90;
    }
};

#endif