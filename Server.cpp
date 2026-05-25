#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "ShelterDatabase.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 4096

ShelterDatabase shelterDb;
mutex shelterMutex;
mutex clientsMutex;
vector<SOCKET> clients;

string getJSONValue(string json, string key) {
    string target = "\"" + key + "\":\"";
    size_t start = json.find(target);
    if (start == string::npos) return "";

    start += target.length();
    size_t end = json.find("\"", start);
    if (end == string::npos) return "";

    return json.substr(start, end - start);
}

void broadcastAnimals() {
    string data;
    {
        lock_guard<mutex> lock(shelterMutex);
        data = "{\"status\":\"UPDATE\",\"animals\":" + shelterDb.toJSONArray() + "}\n";
    }

    lock_guard<mutex> lock(clientsMutex);
    for (SOCKET s : clients) {
        send(s, data.c_str(), data.size(), 0);
    }
}

void handleClient(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        if (valread <= 0) {
            cout << "[INFO] Client terputus (Socket: " << clientSocket << ")\n";
            break;
        }

        string request(buffer);
        string action = getJSONValue(request, "action");

        if (action == "ADD") {
            string species = getJSONValue(request, "species");
            string id = getJSONValue(request, "id");
            string name = getJSONValue(request, "name");
            string response;
            
            {
                lock_guard<mutex> lock(shelterMutex);
                if (shelterDb.findAnimal(id) != nullptr) {
                    response = "{\"status\":\"FAILED\",\"message\":\"Gagal: ID sudah digunakan\"}\n";
                } else {
                    ShelterAnimal* animal = nullptr;
                    if (species == "CAT") {
                        animal = new Cat(id, name, 150000);
                    } else {
                        animal = new Dog(id, name, 300000);
                    }
                    shelterDb.registerAnimal(animal);
                    response = "{\"status\":\"SUCCESS\",\"message\":\"Hewan berhasil ditambahkan\"}\n";
                }
            }

            send(clientSocket, response.c_str(), response.size(), 0);
            if (response.find("SUCCESS") != string::npos) broadcastAnimals();
        }
        else if (action == "ADOPT") {
            string id = getJSONValue(request, "id");
            string response;

            {
                lock_guard<mutex> lock(shelterMutex);
                ShelterAnimal* animal = shelterDb.findAnimal(id);

                if (animal) {
                    if (!animal->getAdoptionStatus()) {
                        animal->processAdoption();
                        response = "{\"status\":\"SUCCESS\",\"message\":\"Adopsi berhasil\"}\n";
                    } else {
                        response = "{\"status\":\"FAILED\",\"message\":\"Sudah diadopsi\"}\n";
                    }
                } else {
                    response = "{\"status\":\"FAILED\",\"message\":\"ID tidak ditemukan\"}\n";
                }
            }

            send(clientSocket, response.c_str(), response.size(), 0);
            if (response.find("SUCCESS") != string::npos) broadcastAnimals();
        }
        else if (action == "SEARCH") {
            string id = getJSONValue(request, "id");
            string response;

            {
                lock_guard<mutex> lock(shelterMutex);
                ShelterAnimal* animal = shelterDb.findAnimal(id);

                if (animal) {
                    response = "{\"status\":\"FOUND\",\"animal\":" + animal->toJSON() + "}\n";
                } else {
                    response = "{\"status\":\"FAILED\",\"message\":\"Tidak ditemukan\"}\n";
                }
            }
            send(clientSocket, response.c_str(), response.size(), 0);
        }
    }

    {
        lock_guard<mutex> lock(clientsMutex);
        clients.erase(remove(clients.begin(), clients.end(), clientSocket), clients.end());
    }
    closesocket(clientSocket);
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    shelterDb.registerAnimal(new Cat("CAT-01", "Milo", 150000));
    shelterDb.registerAnimal(new Dog("DOG-01", "Buddy", 300000));

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    char opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        cout << "Bind gagal!\n";
        return 1;
    }

    listen(server_fd, 5);
    cout << "=== SHELTER SERVER ACTIVE ON PORT " << PORT << " ===\n";

    while (true) {
        SOCKET clientSocket = accept(server_fd, nullptr, nullptr);
        if (clientSocket != INVALID_SOCKET) {
            cout << "[INFO] Client baru terhubung (Socket: " << clientSocket << ")\n";
            {
                lock_guard<mutex> lock(clientsMutex);
                clients.push_back(clientSocket);
            }
            thread t(handleClient, clientSocket);
            t.detach();
        }
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}