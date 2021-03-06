/*
 * CaroloCup.
 *
 * This software is open source. Please see COPYING and AUTHORS for further information.
 */

#include <iostream>

#include "core/data/Container.h"
#include "core/io/ContainerConference.h"
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <cstdio>   /* Standard input/output definitions */
#include <linux/serial.h>
#include <sys/ioctl.h>
#include "LidarData.h"
#include "Lidar.h"
#include <algorithm>


#include <bitset>

#define XV11_CHECKSUM_INDEX 20



using namespace std;


unsigned char buffer[22];

unsigned char starter[1];

int init=0;
int *init_level = &init;

unsigned char byte1, byte2, byte3, byte4, byte5, 
	      byte6, byte7, byte8, byte9, byte10, 
              byte11, byte12, byte13, byte14, byte15, byte16, 
              byte17, byte18, byte19, byte20, byte21, byte22;

unsigned char buffing[22] = {byte1, byte2, byte3, byte4, byte5, 
	      byte6, byte7, byte8, byte9, byte10, 
              byte11, byte12, byte13, byte14, byte15, byte16, 
              byte17, byte18, byte19, byte20, byte21, byte22};

unsigned int sid, sra1, sra2, srb1, srb2, src1, src2, srd1, srd2;

int fd; 			//File descriptor for the port
bool portState = false;

int fow = 0;

reading Distance1;			//The first distance info from the packet 
reading *Pointer1 = &Distance1;	//Pointer to the first distance

reading Distance2;			//The second distance info from the packet 
reading *Pointer2 = &Distance2;	//Pointer to the second distance

reading Distance3;			//The third distance info from the packet 
reading *Pointer3 = &Distance3;	//Pointer to the third distance

reading Distance4;			//The fourth distance info from the packet 
reading *Pointer4 = &Distance4;	//Pointer to the fourth distance

int32_t ldArray[360];


namespace carolocup {

    void  *lidarData(void *argument);

    using namespace std;
    using namespace core::base;
    using namespace core::data;


    Lidar::Lidar(const int32_t &argc, char **argv) :
        ConferenceClientModule(argc, argv, "Lidar"),
        m_fifo()
{}

    Lidar::~Lidar() {}

    void Lidar::setUp() {
        // This method will be call automatically _before_ running body().
    }

    void Lidar::tearDown() {
        // This method will be call automatically _after_ return from body().
    }

    // This method will do the main data processing job.
    ModuleState::MODULE_EXITCODE Lidar::body() {

   fd = open("/dev/ttyUSB2", O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd == -1){
	perror("cannot open");
     }

    else
    	fcntl(fd, F_SETFL, 0);
	struct termios options;
	tcgetattr(fd, &options);
	cfsetispeed(&options, B115200);
	cfsetospeed(&options, B115200);
	options.c_cflag |= (CLOCAL | CREAD);
	//tcsetattr(fd, TCSANOW, &options);
	options.c_cflag &= ~CSIZE;
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
 	tcflush(fd, TCIFLUSH);
   	tcflush(fd, TCIOFLUSH);

  if(tcsetattr(fd, TCSANOW, &options) != 0){

	cout << "Error " << errno << " from tcsetattr" << endl;
   }	
  
  int follow = 0;	

  pthread_t t1;
  pthread_create(&t1, NULL, lidarData, NULL);

  while (getModuleState() == ModuleState::RUNNING) {

   LidarData sendData;

///////////////////////READ FROM PORT//////////////////////////////////
 // while(1){
   
  //sleep(0.00001);

		/*sendData.setIndex(Pointer1->readingIndex);
		sendData.setFirstDeg(Pointer1->degree);
		sendData.setFirstDist(Pointer1->distance);
		sendData.setSecondDeg(Pointer2->degree);
		sendData.setSecondDist(Pointer2->distance);
		sendData.setThirdDeg(Pointer3->degree);
		sendData.setThirdDist(Pointer3->distance);
		sendData.setFourthDeg(Pointer4->degree);
		sendData.setFourthDist(Pointer4->distance);*/

		//Create container for sending the Lidar data
		memcpy(sendData.data, ldArray, 360*sizeof(int32_t));
		for(int i = 0; i < 360; i ++) {
			cout << sendData.data[i] << ",";
		}
		cout << endl;
		//cout << sendData.data[330] << endl;
		Container contData(Container::USER_DATA_2, sendData);
		// Send containers.
		getConference().send(contData);

	////////////////////////////////////////////////////

	      //}   //end of if statement when start byte is found

	   // } //End of infinite while loop


  } //End of ModuleState::RUNNING

    	return ModuleState::OKAY;
    }

bool validate_buffer(void){
    
    int incomming_checksum = (buffing[XV11_CHECKSUM_INDEX + 1] << 8) + buffing[XV11_CHECKSUM_INDEX];    
    
    int chk32 = 0;
    for(int i = 0; i < 10; i++)
        chk32 = (chk32 << 1) + (buffing[2*i + 1] << 8) + buffing[2*i + 0];
    
    int checksum = (chk32 & 0x7FFF) + (chk32 >> 15);
    checksum = checksum & 0x7FFF;
    
    return (checksum == incomming_checksum);
} //End of validate_buffer function

void getDistances(unsigned int id, unsigned int ra1, unsigned int ra2, unsigned int rb1, unsigned int rb2, unsigned int rc1, unsigned int rc2, unsigned int rd1, unsigned int rd2){

  if(id < 160 || id > 249){
        return;
 }


	Pointer1->readingIndex = id;
	Pointer2->readingIndex = id;
	Pointer3->readingIndex = id;
	Pointer4->readingIndex = id;

	unsigned int firstDeg = (id - 160)*4;

///////////////////////////First Reading////////////////////////////////////

          if(ra2 & 0x80){
	  Pointer1->degree = firstDeg;
	  Pointer1->distance = 7000;
	   } //Low quality reading  
    
           else if(ra2 & 0x40){
	  Pointer1->degree = firstDeg;
	  Pointer1->distance = 8000;
            } //Questionable quality reading

            else{
	  Pointer1->degree = firstDeg;
          Pointer1->distance = (((ra2 & 0x3F) << 8) + ra1) / 10.0;
             } //Good reading

///////////////////////////Second Reading//////////////////////////////////

 	 if(rb2 & 0x80){
	Pointer2->degree = firstDeg+1;
	Pointer2->distance = 7000;
	   } //Low quality reading   
   
           else if(rb2 & 0x40){
	Pointer2->degree = firstDeg+1;
	Pointer2->distance = 8000;
            } //Questionable quality reading

            else{
	Pointer2->degree = firstDeg+1;
        Pointer2->distance = (((rb2 & 0x3F) << 8) + rb1) / 10.0;
             } //Good reading

////////////////////////Third Reading///////////////////////////////////

 	if(rc2 & 0x80){
	  Pointer3->degree = firstDeg+2;
	  Pointer3->distance = 7000;		
	   } //Low quality reading      
           else if(rc2 & 0x40){
	  Pointer3->degree = firstDeg+2;
	  Pointer3->distance = 8000;		
            } //Questionable quality reading

            else{
	  Pointer3->degree = firstDeg+2;
          Pointer3->distance = (((rc2 & 0x3F) << 8) + rc1) / 10.0;     
             } //Good reading

//////////////////////Fourth Reading///////////////////////////////////

 	if(rd2 & 0x80){
	  Pointer4->degree = firstDeg+3;
	  Pointer4->distance = 7000;		
	 } //Low quality reading      

           else if(rd2 & 0x40){
	  Pointer4->degree = firstDeg+3;
	  Pointer4->distance = 8000;		
            } //Questionable quality reading

            else{
	 Pointer4->degree = firstDeg+3;
         Pointer4->distance = (((rd2 & 0x3F) << 8) + rd1) / 10.0;     
             } //Good reading

     }//End of get distance function
     
     void *lidarData(void *argument){

     while(1) {
	sleep(0.01);


      int n = -1;
      if(*init_level == 0){
           n = read(fd, starter, 1);
      }
      if(n < 0 && *init_level == 0){
		cout << "Failed to read" << endl;

        }

	if(starter[0] == 0xFA){
	*init_level = 1;
	byte1 = starter[0];

	}
	else{
	  *init_level = 0;
	continue;
	}

	if(*init_level == 1){

	unsigned char start[1];
	       n = read(fd, start, 1);

	   if(start[0] >= 0xA0 && start[0] <= 0xF9){
	   byte2 = start[0];
	   byte1 = 0xFA;

	*init_level = 2;
	    }  

	else if(start[0] != 0xFA){
	      *init_level = 0;
	continue;
	}
	}
	if(*init_level == 2){
		 int p; 
	  	unsigned char star[1];
		p = read(fd, star, 1);
		byte3 = star[0];
	       	p = read(fd, starter, 1); 
		byte4 = starter[0];
	       	p = read(fd, starter, 1); 
		byte5 = starter[0];
	       	p = read(fd, starter, 1); 
		byte6 = starter[0];
	       	p = read(fd, starter, 1); 
		byte7 = starter[0];
	       	p = read(fd, starter, 1); 
		byte8 = starter[0];
	       	p = read(fd, starter, 1); 
		byte9 = starter[0];
	       	p = read(fd, starter, 1); 
		byte10 = starter[0];
	       	p = read(fd, starter, 1); 
		byte11 = starter[0];
	       	p = read(fd, starter, 1); 
		byte12 = starter[0];
	       	p = read(fd, starter, 1); 
		byte13 = starter[0];
	       	p = read(fd, starter, 1); 
		byte14 = starter[0];
	       	p = read(fd, starter, 1); 
		byte15 = starter[0];
	       	p = read(fd, starter, 1); 
		byte16 = starter[0];
	       	p = read(fd, starter, 1); 
	    	byte17 = starter[0];
	       	p = read(fd, starter, 1); 
		byte18 = starter[0];
	       	p = read(fd, starter, 1); 
		byte19 = starter[0];
	       	p = read(fd, starter, 1); 
		byte20 = starter[0];
	       	p = read(fd, starter, 1); 
		byte21 = starter[0];
	       	p = read(fd, starter, 1); 
		byte22 = starter[0];


		if(validate_buffer() == true){
		sid = byte2;
		sra1 = byte5; sra2 = byte6;
		srb1 = byte9; srb2 = byte10;
		src1 = byte13; src2 = byte14;
		srd1 = byte17; srd2 = byte18;

		getDistances(sid, sra1, sra2, srb1, srb2, src1, src2, srd1, srd2);

		/*printf("Index: %d   Degree: %d   Distance: %d\n", Pointer1->readingIndex, Pointer1->degree, Pointer1->distance);
		printf("Index: %d   Degree: %d   Distance: %d\n", Pointer2->readingIndex, Pointer2->degree, Pointer2->distance);
		printf("Index: %d   Degree: %d   Distance: %d\n", Pointer3->readingIndex, Pointer3->degree, Pointer3->distance);
		printf("Index: %d   Degree: %d   Distance: %d\n\n\n", Pointer4->readingIndex, Pointer4->degree, Pointer4->distance);*/

		ldArray[Pointer1->degree] = Pointer1->distance;
		ldArray[Pointer2->degree] = Pointer2->distance;
		ldArray[Pointer3->degree] = Pointer3->distance;
		ldArray[Pointer4->degree] = Pointer4->distance;
		}

		else{
		//cout<<"NOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"<<endl;
		}
	}
	}
}

} // carolocup

