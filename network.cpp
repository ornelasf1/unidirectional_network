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
    int packetNum;
    bool waited = false;
    string type;
    int arrivalTime;
    string direction;
    int travelTime;
};
int Packet::maxPackets = 0;

static pthread_mutex_t bsem;
static pthread_mutex_t asem;
static pthread_cond_t channelAccess = PTHREAD_COND_INITIALIZER;
static pthread_cond_t openChannel = PTHREAD_COND_INITIALIZER;
static string direction;
pthread_t mainT;
pthread_t channel;
pthread_t packet;
vector<Packet*> packets;
bool allPacketsDone = false;
int activeThreads = 0;
int i = 0;
void printSummary();

void* packet_thread(void *packet){
    //pthread_mutex_lock(&asem);
    //activeThreads++;
    //pthread_mutex_unlock(&asem);

    Packet* packet_t = (Packet*) packet;
    int timeElapsed = 0;
    int direc;
    if(packet_t->direction[1] == 'B') direc = 0;
    else if(packet_t->direction[1] == 'A') direc = 2;
    else direc = -1;
    
    //printf("Thread created with packet info: %s %i %s %i\n", packet_t->type.c_str(), packet_t->arrivalTime, packet_t->direction.c_str(), packet_t->travelTime);
    printf("Packet #%i (%s) going to %c arrives at the system.\n", packet_t->packetNum, packet_t->type.c_str(), packet_t->direction[1]);
    while(timeElapsed < packet_t->travelTime){
        if(i == direc) printf("Packet #%i (%s) going to %c is using the channel.\n", packet_t->packetNum, packet_t->type.c_str(), packet_t->direction[1]);
        while(i == direc && timeElapsed < packet_t->travelTime){
            sleep(1);
            timeElapsed++;
        }
        if(timeElapsed >= packet_t->travelTime) break;
        printf("Packet thread %i %s is waiting\n", packet_t->packetNum, packet_t->type.c_str());
        pthread_cond_wait(&channelAccess, &bsem);
    }
    printf("Packet #%i (%s) going to %c exits the channel.\n", packet_t->packetNum, packet_t->type.c_str(), packet_t->direction[1]);
    //pthread_mutex_lock(&asem);
    //activeThreads--;
    //pthread_mutex_unlock(&asem);

    return NULL;
}

void *channel_thread(void *void_ptr)
{
    while (1)
    {
        if(allPacketsDone) break;
        printf("mark 1\n");
        pthread_mutex_lock(&bsem);
        printf("mark 2\n");
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
        pthread_cond_broadcast(&channelAccess);
        //pthread_cond_broadcast(&openChannel);
        printf("%s\n", direction.c_str());
        printf("mark 3\n");
        pthread_mutex_unlock(&bsem);
        printf("mark 4\n");
        sleep(5);
        i = (i + 1) % 4;
    }
    return NULL;
}



void *main_thread(void *void_ptr){
    pthread_t channel_tid; 
    if(pthread_create(&channel_tid, NULL, channel_thread, (void*)NULL)){
        
        printf("Failed to create channel thread\n");
        return NULL;
    }
    int numOfPackets = packets.size();

    Packet* packet;
    pthread_t packet_tid[numOfPackets];
    //int i = 0;
    for(int i = 0; i < numOfPackets; i++){
        //packet_t->waited = true;
        packet = packets[i];
        //printf("info of element : %s %i %s %i\n", packet->type.c_str(), packet->arrivalTime, packet->direction.c_str(), packet->travelTime);
        sleep(packet->arrivalTime);
        if(activeThreads >= Packet::maxPackets){
            printf("Pausing main packet creating cause max packet exceeded\n");
            packet->waited = true;
            pthread_cond_wait(&openChannel, &asem); 
        }
        if(pthread_create(&packet_tid[i], NULL, packet_thread, (void*)packet)){
            printf("Failed to create packet thread\n");
            return NULL;
        }
        //packets.erase(packets.begin());
        //i++;
    }

    for(int i = 0; i < numOfPackets; i++){
        pthread_join(packet_tid[i], NULL);
    }
    printf("All packets done\n");
    allPacketsDone = true;
    pthread_join(channel_tid, NULL);

    printSummary();

    return NULL;
}

void printSummary(){
    int ftp_ab = 0, ftp_ba = 0, ftp_ab_w = 0, ftp_ba_w = 0;
    int http_ab = 0, http_ba = 0, http_ab_w = 0, http_ba_w = 0;
    int ssh_ab = 0, ssh_ba = 0, ssh_ab_w = 0, ssh_ba_w = 0;
    int smtp_ab = 0, smtp_ba = 0, smtp_ab_w = 0, smtp_ba_w = 0;
    for(int i = 0; i < packets.size(); i++){
        if(packets[i]->direction == "AB"){
            if(packets[i]->type == "FTP"){
                ftp_ab++;
                if(packets[i]->waited) ftp_ab_w++;
            }else if(packets[i]->type == "HTTP"){
                http_ab++;
                if(packets[i]->waited) http_ab_w++;
            }else if(packets[i]->type == "SSH"){
                ssh_ab++;
                if(packets[i]->waited) ssh_ab_w++;
            }else if(packets[i]->type == "SMTP"){
                smtp_ab++;
                if(packets[i]->waited) smtp_ab_w++;
            }
        }else if(packets[i]->direction == "BA"){
            if(packets[i]->type == "FTP"){
                ftp_ba++;
                if(packets[i]->waited) ftp_ba_w++;
            }else if(packets[i]->type == "HTTP"){
                http_ba++;
                if(packets[i]->waited) http_ba_w++;
            }else if(packets[i]->type == "SSH"){
                ssh_ba++;
                if(packets[i]->waited) ssh_ba_w++;
            }else if(packets[i]->type == "SMTP"){
                smtp_ba++;
                if(packets[i]->waited) smtp_ba_w++;
            }
        }
    }
    printf("Packets transmitted:\n");
    printf("\tFrom A to B:\n");
    printf("\t\t%i FTP, %i HTTP, %i SSH, %i SMTP.\n", ftp_ab, http_ab, ssh_ab, smtp_ab);
    printf("\tFrom B to A:\n");
    printf("\t\t%i FTP, %i HTTP, %i SSH, %i SMTP.\n", ftp_ba, http_ba, ssh_ba, smtp_ba);
    printf("Packets that waited:\n");
    printf("\tFrom A to B:\n");
    printf("\t\t%i FTP, %i HTTP, %i SSH, %i SMTP.\n", ftp_ab_w, http_ab_w, ssh_ab_w, smtp_ab_w);
    printf("\tFrom B to A:\n");
    printf("\t\t%i FTP, %i HTTP, %i SSH, %i SMTP.\n", ftp_ba_w, http_ba_w, ssh_ba_w, smtp_ba_w);
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

    int num = 0;
    while(getline(cin, line)){
        stringstream sline(line);
        Packet* packet = new Packet();
        num++;
        packet->packetNum = num;
        sline >> line;
        packet->type = line;
        sline >> line;
        packet->arrivalTime = atoi(line.c_str());
        sline >> line;
        packet->direction = line;
        sline >> line;
        packet->travelTime = atoi(line.c_str());
        packets.push_back(packet);
        printf("Insert %s %i %s %i\n", packet->type.c_str(), packet->arrivalTime, packet->direction.c_str(), packet->travelTime);
    }

    pthread_t main_tid;
    pthread_mutex_init(&bsem, NULL); // Initialize access to 1
    //pthread_mutex_init(&asem, NULL); // Initialize access to 1

    if (pthread_create(&main_tid, NULL, main_thread, (void *)NULL)){
        fprintf(stderr, "Error creating main thread\n");
        return 1;
    }



    pthread_join(main_tid, NULL);

    return 0;
}

