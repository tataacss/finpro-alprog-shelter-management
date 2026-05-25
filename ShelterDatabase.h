#ifndef SHELTER_DATABASE_H
#define SHELTER_DATABASE_H

#include "Animal.h"
#include <string>

struct Node {
    ShelterAnimal* data;
    Node* next;

    Node(ShelterAnimal* d) {
        data = d;
        next = nullptr;
    }
};

class ShelterDatabase {
private:
    Node* head;

public:
    ShelterDatabase() {
        head = nullptr;
    }

    ~ShelterDatabase() {
        Node* cur = head;
        while (cur) {
            Node* next = cur->next;
            delete cur->data;
            delete cur;
            cur = next;
        }
    }

    void sortByPriority() {
    if (!head || !head->next) return;

    bool swapped;
    do {
        swapped = false;
        Node* cur = head;

        while (cur->next) {
            if (cur->data->getPriorityScore() < cur->next->data->getPriorityScore()) {
                ShelterAnimal* temp = cur->data;
                cur->data = cur->next->data;
                cur->next->data = temp;
                swapped = true;
            }
            cur = cur->next;
        }
    } while (swapped);
}

    void registerAnimal(ShelterAnimal* animal) {
    if (!animal) return;

    Node* newNode = new Node(animal);

    if (!head) {
        head = newNode;
    } else {
        Node* cur = head;
        while (cur->next) {
            cur = cur->next;
        }
        cur->next = newNode;
    }
    sortByPriority();
}

    ShelterAnimal* findAnimal(const std::string& id) {
        Node* cur = head;
        while (cur) {
            if (cur->data->getAnimalId() == id) {
                return cur->data;
            }
            cur = cur->next;
        }
        return nullptr;
    }

    std::string toJSONArray() {
        std::string result = "[";
        Node* cur = head;
        bool first = true;

        while (cur) {
            if (!first) {
                result += ",";
            }
            result += cur->data->toJSON();
            first = false;
            cur = cur->next;
        }

        result += "]";
        return result;
    }
};

#endif