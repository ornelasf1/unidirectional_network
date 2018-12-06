#include <iostream>
#include <stdio.h>
#include <thread>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unistd.h>

using namespace std;

struct Packet{
    static int maxPackets;
    string type;
    int arrivalTime;
    string direction;
    int travelTime;
};
int Packet::maxPackets = 0;

static pthread_mutex_t bsem;
static pthread_cond_t channelAccess = PTHREAD_COND_INITIALIZER;
static string direction;
pthread_t mainT;
pthread_t channel;
pthread_t packet;
vector<Packet> packets;

void* packet_thread(void* void_ptr){
    return NULL;
}

void *channel_thread(void *void_ptr)
{
    int i = 0;
    while (1)
    {
        pthread_mutex_lock(&bsem);
        switch (i)
        {
        case 0:
            direction = "The channel is now open from A to B";
            break;
        case 1:
            direction = "The channel is now closed to ALL traffic";
            break;
        case 2:
            direction = "The channel is now open from B to A";
            break;
        case 3:
            direction = "The channel is now closed to ALL traffic";
            break;
        }
        printf("%s\n", direction.c_str());
        pthread_mutex_unlock(&bsem);
        sleep(5);
        i = (i + 1) % 4;
    }
    return NULL;
}



void *main_thread(void *void_ptr){
    pthread_t channel_tid; 
    if(!pthread_create(&channel_tid, NULL, channel_thread, (void*)NULL)){
        
        printf("Failed to create channel thread\n");
        return NULL;
    }
    pthread_join(channel_tid, NULL);

    return NULL;
}

int main(int argc, char* argv[]){
    if(argc != 1){
		printf("%s < <input.txt>\n", argv[0]);
		return 0;
	}

    string line;
    if(getline(cin, line)){
        Packet::maxPackets = atoi(line.c_str());
    }

    while(getline(cin, line)){
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

    pthread_t main_tid;
    pthread_mutex_init(&bsem, NULL); // Initialize access to 1

    if (!pthread_create(&main_tid, NULL, main_thread, (void *)NULL)){
        fprintf(stderr, "Error creating main thread\n");
        return 1;
    }



    pthread_join(main_tid, NULL);

    return 0;
}

