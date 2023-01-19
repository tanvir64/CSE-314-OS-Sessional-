#include<cstdio>
#include<iostream>
#include<fstream>
#include<pthread.h>
#include<semaphore.h>
#include<queue>
#include<cstdlib>
#include <unistd.h>
#include<string>
#include<random>
#include<utility>
#include<chrono>

using namespace std;

#define TOTAL_PASSENGERS 20

default_random_engine generator;
poisson_distribution<int> distribution(3.1);

//semaphore to control sleep and wake up
sem_t boarding_pass_check;
sem_t vip_walk;
sem_t special_kiosk;
sem_t print_msg;
sem_t* security_belts;
pthread_mutex_t * check_in_kiosk;
pthread_mutex_t vip_channel_lock[2];


ifstream infile;//("input.txt",ios::in);
ofstream outfile;//("output.txt",ios::out || ios::app);
int arrival_time = 0;
int passenger_id = 1;
int vip_waiting = 0;
bool passenger_returning = false;
int number_of_kiosk,number_of_belts,passenger_per_belt;
int check_in_time,security_check_time,boarding_time,vip_channel_time;
chrono::steady_clock::time_point start_time = chrono::steady_clock::now();

class Passenger{
	int id;
	int arrival_time;
	string status = "";
	bool has_boarding_pass = false;
	bool has_boarded = false;
	public:
	Passenger(int id){
		this->id = id;
	}
	Passenger(int id,string status){
		this->id = id;
		this->status = status;
	}
	void set_id(int id){
		this->id = id;
	}
	int get_id(){
		return this->id;
	}
	void set_arrival_time(int arrival_time){
		this->arrival_time = arrival_time;
	}
	int get_arrival_time(){
		return this->arrival_time;
	}
	void set_status(string status){
		this->status = status;
	}
	string get_status(){
		return this->status;
	}
	void set_boarding_pass(bool pass){
		this->has_boarding_pass = pass;
	}
	bool get_boarding_pass(){
		return this->has_boarding_pass;
	}
	void set_boarding_status(bool pass){
		this->has_boarded = pass;
	}
	bool get_boarding_status(){
		return this->has_boarded;
	}
};

vector<Passenger*> passenger_info;

void* flight_boarding(void* arg) {
	int id = *((int*)arg);
	delete (int*)arg;
	arg = NULL;	
	Passenger* passenger;
	int arrival;

	for(int i=0;i<passenger_info.size();i++){
		Passenger* p = passenger_info[i];		
		fflush(stdout);
		if(p->get_id() == id){			
			passenger = p;
			break;
		}	
	}

	arrival = passenger->get_arrival_time();

	// ---self-check in at kiosks---
	int kiosk_num = (rand() % number_of_kiosk);
	chrono::steady_clock::time_point kiosk_wait_time = chrono::steady_clock::now();
	pthread_mutex_lock(&check_in_kiosk[kiosk_num]); //down(M[0]/M[1]/M[2])
		
	if(passenger->get_status() == ""){
		sem_wait(&print_msg);
		outfile<<"Passenger "<<passenger->get_id()<<" has started self-check in at kiosk "<<kiosk_num+1 <<" at time "<<(chrono::duration_cast<chrono::seconds>(kiosk_wait_time - start_time).count()+arrival)<<endl;
		printf("Passenger %d has started self-check in at kiosk %d at time %d\n",passenger->get_id(),kiosk_num+1,(chrono::duration_cast<chrono::seconds>(kiosk_wait_time - start_time).count()+arrival));
		fflush(stdout);
		sem_post(&print_msg);
		// ---performing the check in operation
		sleep(check_in_time);
		chrono::steady_clock::time_point kiosk_end_time = chrono::steady_clock::now();
		sem_wait(&print_msg);
		outfile<<"Passenger "<<passenger->get_id()<<" has finished self-check in at kiosk "<<kiosk_num+1 <<" at time "<<(chrono::duration_cast<chrono::seconds>(kiosk_end_time - start_time).count()+arrival)<<endl;
		printf("Passenger %d has finished self-check in at kiosk %d at time %d\n",passenger->get_id(),kiosk_num+1,(chrono::duration_cast<chrono::seconds>(kiosk_end_time - start_time).count()+arrival));
		fflush(stdout);
		sem_post(&print_msg);
		passenger->set_boarding_pass(true);
		pthread_mutex_unlock(&check_in_kiosk[kiosk_num]);
	}	
	else{
		sem_wait(&print_msg);
		outfile<<"Passenger "<<passenger->get_id()<<" has started self-check in at kiosk "<<kiosk_num+1 <<" at time "<<(chrono::duration_cast<chrono::seconds>(kiosk_wait_time - start_time).count()+arrival)<<endl;
		printf("Passenger %d(VIP) has started self-check in at kiosk %d at time %d\n",passenger->get_id(),kiosk_num+1,(chrono::duration_cast<chrono::seconds>(kiosk_wait_time - start_time).count()+arrival));
		fflush(stdout);
		sem_post(&print_msg);
		// ---performing the check in operation
		sleep(check_in_time);
		chrono::steady_clock::time_point kiosk_end_time = chrono::steady_clock::now();
		sem_wait(&print_msg);
		outfile<<"Passenger "<<passenger->get_id()<<" has finished self-check in at kiosk "<<kiosk_num+1 <<" at time "<<(chrono::duration_cast<chrono::seconds>(kiosk_end_time - start_time).count()+arrival)<<endl;
		printf("Passenger %d(VIP) has finished self-check in at kiosk %d at time %d\n",passenger->get_id(),kiosk_num+1,(chrono::duration_cast<chrono::seconds>(kiosk_end_time - start_time).count()+arrival));
		fflush(stdout);
		sem_post(&print_msg);
		passenger->set_boarding_pass(true);
		pthread_mutex_unlock(&check_in_kiosk[kiosk_num]);
	}
		
	// ---security check through belts(normal passenger)---		
	if(passenger->get_status() == ""){
		int belt_num = (rand() % number_of_belts);
		chrono::steady_clock::time_point kiosk_end_time = chrono::steady_clock::now();
		sem_wait(&print_msg);
		outfile<<"Passenger "<<passenger->get_id()<<" has started waiting for security check in belt "<<(belt_num+1) <<" from time "<<(chrono::duration_cast<chrono::seconds>(kiosk_end_time - start_time).count()+1+arrival)<<endl;
		printf("Passenger %d has started waiting for security check in belt %d from time %d\n",passenger->get_id(),(belt_num+1),(chrono::duration_cast<chrono::seconds>(kiosk_end_time - start_time).count()+1+arrival));
		fflush(stdout);
		sem_post(&print_msg);
		//wait is over!
		chrono::steady_clock::time_point belt_wait_time = chrono::steady_clock::now();
		sem_wait(&security_belts[belt_num]);
		sem_wait(&print_msg);
		outfile<<"Passenger "<<passenger->get_id()<<" has started the security check in belt "<<(belt_num+1) <<" at time "<<(chrono::duration_cast<chrono::seconds>(belt_wait_time - start_time).count()+arrival)<<endl;
		printf("Passenger %d has started the security check in belt %d at time %d\n",passenger->get_id(),(belt_num+1),(chrono::duration_cast<chrono::seconds>(belt_wait_time - start_time).count()+arrival));
		fflush(stdout);
		sem_post(&print_msg);
		//---security check time!---
		sleep(security_check_time);
		chrono::steady_clock::time_point belt_end_time = chrono::steady_clock::now();
		sem_wait(&print_msg);
		outfile<<"Passenger "<<passenger->get_id()<<" has crossed the security check at time "<<(chrono::duration_cast<chrono::seconds>(belt_end_time - start_time).count()+arrival)<<endl;
		printf("Passenger %d has crossed the security check at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(belt_end_time - start_time).count()+arrival));
		fflush(stdout);
		sem_post(&print_msg);
		sem_post(&security_belts[belt_num]);
	}
	//VIP passengers skipping the security check
	else{
		chrono::steady_clock::time_point kiosk_end_time = chrono::steady_clock::now();
		sem_wait(&print_msg);
		outfile<<"Passenger "<<passenger->get_id()<<"(VIP) has started waiting on the VIP channel at time "<<(chrono::duration_cast<chrono::seconds>(kiosk_end_time - start_time).count()+1+arrival)<<endl;
		printf("Passenger %d(VIP) has started waiting on the VIP channel at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(kiosk_end_time - start_time).count()+1+arrival));
		fflush(stdout);
		sem_post(&print_msg);	
		//acquiring the VIP channel			
		if(!passenger_returning) pthread_mutex_lock(&vip_channel_lock[0]); //down L-R direction
		chrono::steady_clock::time_point vip_start_time = chrono::steady_clock::now();
		sem_wait(&print_msg);		
		outfile<<"Passenger "<<passenger->get_id()<<"(VIP) has started crossing the VIP channel at time "<<(chrono::duration_cast<chrono::seconds>(vip_start_time - start_time).count()+1+arrival)<<endl;
		printf("Passenger %d(VIP) has started crossing the VIP channel at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(vip_start_time - start_time).count()+1+arrival));
		fflush(stdout);
		sem_post(&print_msg);
		vip_waiting++;
		//walking through the VIP channel
		sleep(vip_channel_time);
		chrono::steady_clock::time_point vip_end_time = chrono::steady_clock::now();
		sem_wait(&print_msg);		
		outfile<<"Passenger "<<passenger->get_id()<<"(VIP) has crossed the VIP channel at time "<<(chrono::duration_cast<chrono::seconds>(vip_end_time - start_time).count()+arrival)<<endl;
		printf("Passenger %d(VIP) has crossed the VIP channel at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(vip_end_time - start_time).count()+arrival));
		fflush(stdout);
		sem_post(&print_msg);
		vip_waiting--;
		pthread_mutex_unlock(&vip_channel_lock[0]); //up L-R direction
	}

	// ---boarding at the gate---	
	while(!passenger->get_boarding_status()){
		chrono::steady_clock::time_point belt_end_time = chrono::steady_clock::now();
		sem_wait(&print_msg);
		if(passenger->get_status() == "") {
			outfile<<"Passenger "<<passenger->get_id()<<" has started waiting to be boarded at time "<<(chrono::duration_cast<chrono::seconds>(belt_end_time - start_time).count()+1+arrival)<<endl;
			printf("Passenger %d has started waiting to be boarded at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(belt_end_time - start_time).count()+1+arrival));
			fflush(stdout);
		}
		else{
			outfile<<"Passenger "<<passenger->get_id()<<"(VIP) has started waiting to be boarded at time "<<(chrono::duration_cast<chrono::seconds>(belt_end_time - start_time).count()+1+arrival)<<endl;
			printf("Passenger %d(VIP) has started waiting to be boarded at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(belt_end_time - start_time).count()+1+arrival));
			fflush(stdout);
		}		
		sem_post(&print_msg);

		int random_pass = rand() % 5;
		//---passengers randomly losing boarding pass ---
		if(random_pass == 1) passenger->set_boarding_pass(false);				
		//---passengers with boarding pass---
		if(passenger->get_boarding_pass()){
			chrono::steady_clock::time_point boarding_wait_time = chrono::steady_clock::now();			
			sem_wait(&boarding_pass_check);
			sem_wait(&print_msg);
			if(passenger->get_status() == ""){
				outfile<<"Passenger "<<passenger->get_id()<<" has started boarding the plane at time "<<(chrono::duration_cast<chrono::seconds>(boarding_wait_time - start_time).count()+1+arrival)<<endl;
				printf("Passenger %d has started boarding the plane at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(boarding_wait_time - start_time).count()+1+arrival));
				fflush(stdout);
			}
			else{
				outfile<<"Passenger "<<passenger->get_id()<<"(VIP) has started boarding the plane at time "<<(chrono::duration_cast<chrono::seconds>(boarding_wait_time - start_time).count()+1+arrival)<<endl;
				printf("Passenger %d(VIP) has started boarding the plane at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(boarding_wait_time - start_time).count()+1+arrival));
				fflush(stdout);
			}			
			sem_post(&print_msg);
			//boarding the plane
			sleep(boarding_time);
			chrono::steady_clock::time_point boarding_end_time = chrono::steady_clock::now();
			sem_wait(&print_msg);
			if(passenger->get_status() == ""){
				outfile<<"Passenger "<<passenger->get_id()<<" has boarded the plane at time "<<(chrono::duration_cast<chrono::seconds>(boarding_end_time - start_time).count()+arrival)<<endl;
				printf("Passenger %d has started boarded the plane at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(boarding_end_time - start_time).count()+arrival));
				fflush(stdout);
			}
			else{
				outfile<<"Passenger "<<passenger->get_id()<<" has boarded the plane at time "<<(chrono::duration_cast<chrono::seconds>(boarding_end_time - start_time).count()+arrival)<<endl;
				printf("Passenger %d(VIP) has boarded the plane at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(boarding_end_time - start_time).count()+arrival));
				fflush(stdout);
			}
			sem_post(&print_msg);
			sem_post(&boarding_pass_check);
			passenger->set_boarding_status(true);
		}
		//---returned passengers who have lost their boarding pass---
		else{			
			if(vip_waiting == 0) pthread_mutex_lock(&vip_channel_lock[1]); //change direction to R-L
			passenger_returning = true;	
			chrono::steady_clock::time_point recheck_start_time = chrono::steady_clock::now();					
			sem_wait(&print_msg);
			if(passenger->get_status() == ""){
				outfile<<"Passenger "<<passenger->get_id()<<"(returned) has started waiting to cross the VIP channel at time "<<(chrono::duration_cast<chrono::seconds>(recheck_start_time - start_time).count()+arrival)<<endl;
				printf("Passenger %d(returned) has started waiting to cross the VIP channel at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(recheck_start_time - start_time).count()+arrival));
				fflush(stdout);
			} 
			else{
				outfile<<"Passenger "<<passenger->get_id()<<"(VIP)(returned) has started waiting to cross the VIP channel at time "<<(chrono::duration_cast<chrono::seconds>(recheck_start_time - start_time).count()+arrival)<<endl;
				printf("Passenger %d(VIP)(returned) has started waiting to cross the VIP channel at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(recheck_start_time - start_time).count()+arrival));
				fflush(stdout);
			} 			
			sem_post(&print_msg);
			//---using the VIP channel to go to special kiosk---
			sleep(vip_channel_time);
			chrono::steady_clock::time_point recheck_end_time = chrono::steady_clock::now();
			sem_wait(&print_msg);
			if(passenger->get_status() == ""){
				outfile<<"Passenger "<<passenger->get_id()<<"(returned) has reached special kiosk at time "<<(chrono::duration_cast<chrono::seconds>(recheck_end_time - start_time).count()+arrival)<<endl;
				printf("Passenger %d(returned) has reached special kiosk at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(recheck_end_time - start_time).count()+arrival));
				fflush(stdout);
			} 
			else{
				outfile<<"Passenger "<<passenger->get_id()<<"(VIP)(returned) has reached special kiosk at time "<<(chrono::duration_cast<chrono::seconds>(recheck_end_time - start_time).count()+arrival)<<endl;
				printf("Passenger %d(VIP)(returned) has reached special kiosk at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(recheck_end_time - start_time).count()+arrival));
				fflush(stdout);
			} 
			
			sem_post(&print_msg);
			passenger_returning = false;
			pthread_mutex_unlock(&vip_channel_lock[1]);
			//---using the special kiosk to get the boarding pass---
			sem_wait(&special_kiosk);
			chrono::steady_clock::time_point repass_start_time = chrono::steady_clock::now();					
			sem_wait(&print_msg);
			if(passenger->get_status() == ""){
				outfile<<"Passenger "<<passenger->get_id()<<"(returned) has started using the special kiosk at time "<<(chrono::duration_cast<chrono::seconds>(repass_start_time - start_time).count()+arrival)<<endl;
				printf("Passenger %d(returned) has started using the special kiosk at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(repass_start_time - start_time).count()+arrival));
				fflush(stdout);
			} 
			else{
				outfile<<"Passenger "<<passenger->get_id()<<"(VIP)(returned) has started using the special kiosk at time "<<(chrono::duration_cast<chrono::seconds>(repass_start_time - start_time).count()+arrival)<<endl;
				printf("Passenger %d(VIP)(returned) has started using the special kiosk at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(repass_start_time - start_time).count()+arrival));
				fflush(stdout);
			} 
			sem_post(&print_msg);
			//using the kiosk
			sleep(check_in_time);
			chrono::steady_clock::time_point repass_end_time = chrono::steady_clock::now();					
			sem_wait(&print_msg);
			if(passenger->get_status() == ""){
				outfile<<"Passenger "<<passenger->get_id()<<"(returned) has finished using the special kiosk at time "<<(chrono::duration_cast<chrono::seconds>(repass_end_time - start_time).count()+arrival)<<endl;
				printf("Passenger %d(returned) has finished using the special kiosk at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(repass_end_time - start_time).count()+arrival));
				fflush(stdout);
			} 
			else{
				outfile<<"Passenger "<<passenger->get_id()<<"(VIP)(returned) has finished using the special kiosk at time "<<(chrono::duration_cast<chrono::seconds>(repass_end_time - start_time).count()+arrival)<<endl;
				printf("Passenger %d(VIP)(returned) has finished using the special kiosk at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(repass_end_time - start_time).count()+arrival));	
				fflush(stdout);			
			} 
			sem_post(&print_msg);			
			sem_post(&special_kiosk);
			//regained the boarding pass
			passenger->set_boarding_pass(true);
			//returning to the boarding gate through VIP channel
			sem_wait(&print_msg);
			if(passenger->get_status() == ""){
				outfile<<"Passenger "<<passenger->get_id()<<"(returned) has started waiting on the VIP channel at time "<<(chrono::duration_cast<chrono::seconds>(repass_end_time - start_time).count()+1+arrival)<<endl;
				printf("Passenger %d(returned) has started waiting on the VIP channel at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(repass_end_time - start_time).count()+1+arrival));
				fflush(stdout);
			} 
			else{
				outfile<<"Passenger "<<passenger->get_id()<<"(VIP)(returned) has started waiting on the VIP channel at time "<<(chrono::duration_cast<chrono::seconds>(repass_end_time - start_time).count()+1+arrival)<<endl;
				printf("Passenger %d(VIP)(returned) has started waiting on the VIP channel at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(repass_end_time - start_time).count()+1+arrival));
				fflush(stdout);
			} 
			sem_post(&print_msg);			
			if(vip_waiting == 0 && !passenger_returning) pthread_mutex_lock(&vip_channel_lock[0]); //change direction to L-R							
			chrono::steady_clock::time_point revisit_start_time = chrono::steady_clock::now();					
			sem_wait(&print_msg);
			if(passenger->get_status() == ""){
				outfile<<"Passenger "<<passenger->get_id()<<"(returned) has started crossing the VIP channel at time "<<(chrono::duration_cast<chrono::seconds>(revisit_start_time - start_time).count()+arrival)<<endl;
				printf("Passenger %d(returned) has started crossing the VIP channel at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(revisit_start_time - start_time).count()+arrival));
				fflush(stdout);
			} 
			else{
				outfile<<"Passenger "<<passenger->get_id()<<"(VIP)(returned) has started crossing the VIP channel at time "<<(chrono::duration_cast<chrono::seconds>(revisit_start_time - start_time).count()+arrival)<<endl;
				printf("Passenger %d(VIP)(returned) has started crossing the VIP channel at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(revisit_start_time - start_time).count()+arrival));
				fflush(stdout);
			}
			sem_post(&print_msg);
			//passing through the VIP channel
			sleep(vip_channel_time);
			chrono::steady_clock::time_point revisit_end_time = chrono::steady_clock::now();					
			sem_wait(&print_msg);
			if(passenger->get_status() == ""){
				outfile<<"Passenger "<<passenger->get_id()<<"(returned) has crossed the VIP channel at time "<<(chrono::duration_cast<chrono::seconds>(revisit_end_time - start_time).count()+arrival)<<endl;
				printf("Passenger %d(returned) has crossed the VIP channel at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(revisit_end_time - start_time).count()+arrival));
				fflush(stdout);
			}
			else{
				outfile<<"Passenger "<<passenger->get_id()<<"(VIP)(returned) has crossed the VIP channel at time "<<(chrono::duration_cast<chrono::seconds>(revisit_end_time - start_time).count()+arrival)<<endl;
				printf("Passenger %d(VIP)(returned) has crossed the VIP channel at time %d\n",passenger->get_id(),(chrono::duration_cast<chrono::seconds>(revisit_end_time - start_time).count()+arrival));
				fflush(stdout);
			}
			sem_post(&print_msg);
			pthread_mutex_unlock(&vip_channel_lock[0]);
		}
	}
}

int main(void)
{
	int value;
	srand(time(NULL));	

	infile.open("input.txt",ios::in);
	outfile.open("output.txt",ios::app);
	
	infile >> number_of_kiosk >> number_of_belts >> passenger_per_belt >> check_in_time >> security_check_time >> boarding_time >> vip_channel_time;	
	
	// ---initializing semaphore and locks(mutex)---	
	security_belts = new sem_t[number_of_belts];
	check_in_kiosk = new pthread_mutex_t[number_of_kiosk];	
	sem_init(&boarding_pass_check,0,1);
	sem_init(&print_msg,0,1);
	sem_init(&special_kiosk,0,1);
	for(int i=0; i<number_of_kiosk; i++)
		pthread_mutex_init(&check_in_kiosk[i], NULL);
	for(int i=0; i<2; i++)
		// 0:left-right on, 1:right-left on
		pthread_mutex_init(&vip_channel_lock[i], NULL);
	for(int i=0; i<number_of_belts; i++)		
		sem_init(&security_belts[i],0,passenger_per_belt);			
    
	// ---creating passenger threads---
	pthread_t passengers[TOTAL_PASSENGERS];
	for(int i=0; i<TOTAL_PASSENGERS; i++) {
		int* id = new int(i+1);
		string status = "";
		int number = distribution(generator);
		if(number%3==0 || number%5==0) status = "(VIP)";
		Passenger* p = new Passenger(i+1);
		p->set_status(status);
		p->set_boarding_pass(false);
		p->set_boarding_status(false);			
		arrival_time += number;
		p->set_arrival_time(arrival_time);
		passenger_info.push_back(p);		
		// cout<<p->get_boarding_status()<<endl;
		if((i+1) % 3 == 0) {
			sleep(2);
			arrival_time += 2;
		}
		sem_wait(&print_msg);
		outfile<<"Passenger "<< p->get_id() << p->get_status() << " has arrived at the airport at time "<<arrival_time<<endl;
		cout<<"Passenger "<< p->get_id() << p->get_status() << " has arrived at the airport at time "<<arrival_time<<endl;
		sem_post(&print_msg);
		pthread_create(&passengers[i], NULL, flight_boarding, (void*)id);
	}

	// ---synchronizing the passenger threads---
	for(int i=0; i<TOTAL_PASSENGERS; i++)
		pthread_join(passengers[i], NULL);

	infile.close();
	outfile.close();

	// ---uninitializing semaphores and locks(mutex)---
	sem_destroy(&boarding_pass_check);
	sem_destroy(&print_msg);
	for(int i=0; i<number_of_kiosk; i++)
		pthread_mutex_destroy(&check_in_kiosk[i]);
	for(int i=0; i<number_of_belts; i++)
		/* 0: in_service & 1: in_waiting */
		sem_destroy(&security_belts[i]);
	return 0;
}
