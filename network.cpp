#include <iostream>
#include <stdio.h>
#include <thread>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

struct Packet{
    static int maxPackets;
    string type;
    int arrivalTime;
    string direction;
    int travelTime;
};
int Packet::maxPackets = 0;

int main(){
    pthread_t main;
    pthread_t channel;
    pthread_t packet;
    vector<Packet> packets;
    ifstream inputFile;
    string line;

    inputFile.open("input.txt");
    if(inputFile.is_open()){
        if(getline(inputFile, line)){
            Packet::maxPackets = atoi(line.c_str());
        }
        while(getline(inputFile, line)){
            stringstream sline(line);
            Packet packet;
            sline >> line;
            packet.type = line;
            sline >> line;
            packet.arrivalTime = atoi(line.c_str());
            sline >> line;
            packet.direction = line;
            sline >> line;
            packet.travelTime = atoi(line.c_str());
            packets.push_back(packet);
        }
    }else{
        printf("File failed to open\n");
    }
    return 0;
}

