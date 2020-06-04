#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <cmath>
#include <unistd.h>
#include <vector>
#include <limits>
#include <fstream>
#include "queue.cpp"
#include "gel.cpp"

#define TIMEOUT_CHANCE 10 //can set chance that timeout occurs and no ACK is received, between 0 to 100
#define COLL_CHANCE 10 //define collison chance, 0 to 100

#define ARRIVING 0
#define DEPARTING 1
#define PACKET_READY 2
#define INIT_WAIT_DONE 3
#define TRANSMIT 4
#define ACK 5
#define ACK_SEND 6
#define ACK_ARR 7
#define TIME_CHECK 8

#define DONE 10

/*event object containing:
time: when event occurs. For arrival it is time packet arrives at transmitter and 
for departure event it is time when the server is finished transmitting the packet.
type: arriving or departing event
nxt: pointer to next event
prev: pointer to previous event*/
/*struct event{
  int time;
  int type;
  event* nxt;
  event* prev;
};*/

/*General idea for implementation: set for loops for events, event list
will hold event types for checking channel, waiting, transmitter arriving,
departing, setting acknowledge, and all other events. In each loop, check next
event type, and do the next logical step. May not need too many event types
since can do many of the steps sequentially. 
*/

int drop = 0;
double u = 1;
double lamda = 0.8;
int len = 0;
int total_len = 0;
int cur_time = 0;   //in (us)
int MAXBUFFER = std::numeric_limits<int>::max();
int service_time = 0;
int next_time;

double server_busy_time = 0;
int pre_len = 0;
int pre_event_time = 0;
double area_under_curve = 0;
int total_bytes = 0;

bool channel_open = true;
bool init_wait_done_s = false;
bool init_wait_done_r = false;

GEL<event> *gel_list = new GEL<event>();

//Wait methods for sender(DIFS) receiver (SIFS)
//set next event time for after DIFS, return
void waitDIFS(){
    //set next time for event depending on DIFS value;
    int DIFS = 100;
    next_time = cur_time + DIFS;
    event* wait = new event(next_time, INIT_WAIT_DONE);     
    gel_list->insert(wait);
    server_busy_time += DIFS;
    
}

void waitSIFS(){
    int SIFS = 50;
    next_time = cur_time + SIFS;
    event* wait = new event(next_time, INIT_WAIT_DONE);     
    gel_list->insert(wait);
    server_busy_time += SIFS;
    
}

int transmit_time(){
    int r = 1544*drand48();
    total_bytes = total_bytes + r;
    int time = (1000 * r * 8) / (11 * 106); //times 1000 to make usec
    server_busy_time += time;
    return time;
}

void collision_chance(int* k){
    int c = 100 * drand48(); //random number 0 - 100
    if(c <= COLL_CHANCE){
        *k += 1;
    }
}

//returns 1 in case of timeout, 0 if no timeout before ACK
int timeout_occur(){
    int c = 100 * drand48();
    if(c <= TIMEOUT_CHANCE){
        return 1;
    }else{
        return 0;
    }
}

//transmitter receive
void transmitterReceive(queue_t* buffer_queue, event* packet){
    //processing arrival event
            
    cur_time = packet->time;
            
    area_under_curve += ((cur_time-pre_event_time)*pre_len);
    if(pre_len > 1) {
        server_busy_time += cur_time-pre_event_time;
    }
            
    pre_event_time = cur_time;
    pre_len = len;
            
    //schedule next event
    next_time = cur_time + negative_exponentially_distributed_time(lamda);
    service_time = negative_exponentially_distributed_time(u);

    //packet received, create ack
    if(packet->type == ACK_ARR){
        event* receive_ack = new event(next_time, TIME_CHECK);
    }else{
        event* ack = new event(next_time, ACK);
    }
            
    gel_list->insert(ack);
            
    //process the arrival event
    if (len == 0) {
        //when server is free
        //get service time of the packet
        //create departure event
        //event* dep_event = new event(cur_time+service_time, 1);
                
        //insert event into GEL
        //gel_list->insert(dep_event);
        len++;
                
    }
    else if (len > 0){
        //when server is busy
        if (len-1 < MAXBUFFER) {
        //the que is not full
                    
        //put packet into queue
        queue_enqueue(*buffer_queue, (void*)packet);
        len++;
        }
        else {
            //que is full, drop packet
            drop ++;
        }
    }
    else {
        std::cout << "error len less than 0" << std::endl;
    }
}

void transmitterSend(queue_t* buffer_queue, event* packet){
    //processing departure event
        
    cur_time = packet->time;
            
    area_under_curve += (cur_time-pre_event_time)*pre_len;
    if(pre_len > 1) {
        server_busy_time += cur_time-pre_event_time;
    }
            
    pre_event_time = cur_time;
    pre_len = len;        
            
    if(len > 1) {
        queue_dequeue(*buffer_queue, (void**)packet);
        if(packet->type == ACK_SEND){
            event* ack_arr = new event(cur_time + service_time, ACK_ARR);
        }else{
            event* arr_event = new event(cur_time + service_time, 0);
        }
                
        len--;
        //insert arrive event into GEL
        gel_list->insert(arr_event);
    }

}

int exp_backoff (int* k){ //k = number of collisions
    int backoff_time = 0;
    int bkoff = 100 * drand48(); //Choose a non-zero integer bkoff randomly from range (0, 100]
    int exp = pow(2,*k) * drand48(); //Choose a non-zero integer exp randomly from range (0, 2k]
    backoff_time = bkoff * exp;
    return backoff_time;
}

double negative_exponentially_distributed_time (double rate) {
    double u;
    u = drand48();
    return ((-1/rate)*log(1-u));
}

void receiver(event* evnt, int* k){
    if(*k > 0){
        event* packet = new event(cur_time, DEPARTING);     
        gel_list->insert(packet);
        *k -= 1;
        return;
    }
    //working on this
    for(int i = 0; i < 2; i++){
        if(!init_wait_done_r){
            waitSIFS();
            if(!channel_open){
                event* evnt = gel_list->removeFirst();         
                //remove last event, create new event for added expbackoff time
                next_time = cur_time + exp_backoff(*k);
                event* wait = new event(next_time, INIT_WAIT_DONE);    
                gel_list->insert(wait);
            }
            init_wait_done_r = true;

        }else if(evnt->type == INIT_WAIT_DONE && channel_open){
            //transmit the ACK packet
            next_time = cur_time + 439; //time to transmit 64 bit long ACK
            server_busy_time += 439;
            total_bytes = total_bytes + 64;
            
            event* ack = new event(next_time, ACK_SEND);     
            gel_list->insert(ack);
            return;
        }
        evnt = gel_list->removeFirst(); //get next state before loop again
    }

}

void sender (event* evnt, int k){
    for(int i = 0; i < 3; i++){
        if(!init_wait_done_s){
            waitDIFS();
            if(!channel_open){
                event* evnt = gel_list->removeFirst();
                //remove last event, create new event for added expbackoff time
                next_time = cur_time + exp_backoff(*k);
                event* wait = new event(next_time, INIT_WAIT_DONE);     
                gel_list->insert(wait);
            }
            init_wait_done_s = true;

        }else if(evnt->type == INIT_WAIT_DONE && channel_open){
            //transmit the packet, takes time r?
            next_time = cur_time + transmit_time();
            event* transmit = new event(next_time, TRANSMIT);     
            gel_list->insert(transmit);

        }else if(evnt->type == TRANSMIT){
            next_time = cur_time;
            event* new_packet = new event(next_time, DEPARTING);     
            gel_list->insert(new_packet);
            return;
        }

       evnt = gel_list->removeFirst(); 
    }
    
}

int main() {
    
    std::ofstream myfile;
    myfile.open ("result.txt");
    myfile << "Writing this to a file.\n";
    
    //initialization
    int* k = 0; //number of collisions
    bool timeout = false;
    event* ack;

    event* initial_event = new event(0, 0); //create arriving event
    gel_list->insert(initial_event);

    event* send_packet = initial_event;

    queue_t* buffer_queue_host1 = queue_create();
    queue_t* buffer_queue_host2 = queue_create();

    
    for (int i = 0; i < 100000; i++) {
        
        std::cout << "len: " << len << std::endl;
        std::cout << "drop: " << drop << std::endl;
        std::cout << "time: " << cur_time << std::endl;
        
        myfile << "len: " << len << std::endl;
        myfile << "drop: " << drop << std::endl;
        myfile << "time: " << cur_time << std::endl;
        
        //get next event
        event* evnt = gel_list->removeFirst();

        if(evnt->type == PACKET_READY){
            //start sender code
            //only dequeue send packet if there was no timeout
            if(!timeout){
                queue_enqueue(*buffer_queue_host1, (void*)send_packet);
            }
            sender(evnt, k);
        }
        else if (evnt->type == ARRIVING) {
            transmitterReceive(buffer_queue_host2, evnt);
        }
        else if (evnt->type == DEPARTING){
            //every time departure happens, chance for collision.
            collision_chance(k);
            transmitterSend(buffer_queue_host1, evnt);
        }else if (evnt->type == ACK){
            ack = evnt;
            queue_enqueue(*buffer_queue_host2, (void*)evnt); //enqueue ACK
            receiver(evnt, k);
        }
        else if(evnt->type == ACK_SEND){
            transmitterSend(buffer_queue_host2, evnt);
            queue_dequeue(*buffer_queue_host2, (void**)ack); //dequeue sent ACK
        }
        else if(evnt->type == ACK_ARR){
            transmitterReceive(buffer_queue_host1, evnt);
        }
        else if(evnt->type == TIME_CHECK){
            if(timeout_occur()){
                event* retransmit = new event(cur_time, PACKET_READY);
                gel_list->insert(retransmit);
                timeout = true;
            }else{
                //no timeout, remove from buffer and make new packet to send
                timeout = false;
                queue_dequeue(*buffer_queue_host1, (void**)send_packet);
                event* new_packet = new event(cur_time, PACKET_READY);
                gel_list->insert(new_packet);
            }
        }
        else {
            std::cout << "type error" << std::endl;
        }
        
    }
    
    //std::cout << total_len/100000 << std::endl;
    myfile.close();
    
    
    double utilization = server_busy_time/cur_time;
    double mean_queue_length = area_under_curve/cur_time;

    double throughput = total_bytes / cur_time;
    double network_delay = server_busy_time / throughput;
    
    std::cout << "utilization: " << server_busy_time/cur_time << std::endl;
    std::cout << "mean_queue_length: " << mean_queue_length << std::endl;
    std::cout << "dropped: " << drop << std::endl;
    
    //std::cout << "time: " << cur_time << std::endl;
    
    return 0;
    
}
