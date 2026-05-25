#include <iostream>
#include <string>
#include <thread>
#include <limits>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 4096



void receiveMessages(SOCKET sock) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = recv(sock, buffer, BUFFER_SIZE, 0);

        if (valread <= 0) {
            cout << "\n[INFO] Terputus dari server.\n";
            break;
        }

        cout << "\n\n[SERVER RESPONSE]\n" << buffer << "\n";
        cout << "Pilih menu: " << flush;
    }
}

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) return 1;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        cout << "Gagal terhubung ke server!\n";
        WSACleanup();
        return 1;
    }

    cout << "Berhasil terhubung ke server.\n";

    thread recvThread(receiveMessages, sock);
    recvThread.detach();

    while (true) {
        cout << "\n=== ANIMAL SHELTER CLIENT ===\n";
        cout << "1. Tambah Hewan\n";
        cout << "2. Adopsi Hewan\n";
        cout << "3. Cari Hewan\n";
        cout << "0. Keluar\n";
        cout << "Pilih menu: ";

        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Masukkan angka!\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 0) break;

        if (choice == 1) {
            string species, id, name;
            cout << "Species (CAT/DOG): "; cin >> species;
            cout << "ID: "; cin >> id;
            cout << "Nama: "; cin.ignore(); getline(cin, name);

            string request = "{\"action\":\"ADD\",\"species\":\"" + species + "\",\"id\":\"" + id + "\",\"name\":\"" + name + "\"}\n";
            send(sock, request.c_str(), request.size(), 0);
        }
        else if (choice == 2) {
            string id;
            cout << "ID Hewan: "; cin >> id;
            string request = "{\"action\":\"ADOPT\",\"id\":\"" + id + "\"}\n";
            send(sock, request.c_str(), request.size(), 0);
        }
        else if (choice == 3) {
            string id;
            cout << "Cari ID: "; cin >> id;
            string request = "{\"action\":\"SEARCH\",\"id\":\"" + id + "\"}\n";
            send(sock, request.c_str(), request.size(), 0);
        }
        
        this_thread::sleep_for(chrono::milliseconds(200));
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}