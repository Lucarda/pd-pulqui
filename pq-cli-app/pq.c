/* pq command line interface
// Written by Lucas Cordiviola May 2019 v0.1.0
// No warranties
// See License.txt
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>




// infoweneed
struct infoweneed 
{
   uint32_t samplebytes;
   uint8_t bitdepht;
   uint8_t channels;
   uint8_t bytespersample;
   uint32_t sampleperchannel;
};

struct infoweneed linfo;

// read
struct bits24 
{
   uint8_t byt1;
   uint8_t byt2;
   uint8_t byt3;
};

struct bits24 num24;

uint32_t love1 (int32_t bytes333);
float love32 (float IEEE);

int bee (uint32_t *ramptr);
int bee32 (float *IEEEramptr);

int positiveram (void);
int positiveramIEEE (void);

int createram (void);
int createramIEEE (void);

FILE *fptr; // file for reading
FILE *wptr; // file for writing

uint32_t n;
uint32_t bytes333;
uint32_t sampVokmain;
float sampVokmainIEEE;
const int MIN16 = 327;
const int MIN24 = 83886;
const float MIN32 = 0.01;
uint8_t *hcbyte;
float IEEE;
 
uint32_t *ramch1;
uint32_t *ramch2;
uint32_t **ramptr;
   
float *IEEEramch1;
float *IEEEramch2;
float **IEEEramptr;

char outname [100];


int main(int argc, char *argv[])
{

   
   
   printf("******************\n") ;
   sprintf(outname,"%sf\0", argv[1]);
   printf("file: %s\n", argv[1]);
   
   
   if ((fptr = fopen(argv[1],"rb")) == NULL){
       printf("Error! opening file");
		fclose(fptr);
       // Program exits if the file pointer returns NULL.
       exit(1);
   }
   

    // get channels
   fseek(fptr, 22, SEEK_SET);
   fread(&linfo.channels, 1, 1, fptr);
   printf("channels: %d\n" , linfo.channels);
   
    // get bit-depth
   fseek(fptr, 34, SEEK_SET);
   fread(&linfo.bitdepht, 1, 1, fptr);
   printf("bit-depth: %d\n" , linfo.bitdepht);
   
   if (!((linfo.bitdepht == 16) || ((linfo.bitdepht == 24) || (linfo.bitdepht == 32)))     ) {
   printf("Error , doesn't look like a 16, 24 or 32 bit WAV file.");
   goto pq_end ;
   }
   
   
      
    //  bit-depth to bytes
   linfo.bytespersample = linfo.bitdepht / 8;
   //printf("bytespersample: %d\n" , linfo.bytespersample);
   
    // get databytes 
   fseek(fptr, 40, SEEK_SET);
   fread(&linfo.samplebytes, 4, 1, fptr);
   //printf("samplebytes: %d\n" , linfo.samplebytes);
   
   //  samples per channel
   linfo.sampleperchannel = (linfo.samplebytes / linfo.bytespersample) / linfo.channels;
   //printf("samples per channel: %d\n" , linfo.sampleperchannel);
   

   
   
   
   
   
   
   
   
   
   ///////////////////////ram thing2
	
	
  if (linfo.bitdepht == 32) {
   createramIEEE();
  }else {
	createram();	
	}
   
   
   /////////////////////////////////
   

   if ((wptr = fopen(outname,"wb")) == NULL){
       printf("Error opening file for writing");
		fclose(wptr);
       exit(1);
   }   
   
   
   
   /////////////////////////////////
   
   printf("RAM allocation: %dMB\n" , ((linfo.sampleperchannel * 4) * linfo.channels) / 1000000);
   printf("started scanning...\n") ;  
  
   
   
   ////////////////////////////// load all positive to ram array
   
  if (linfo.bitdepht == 32) {
   positiveramIEEE();
  }else {
	positiveram();	
	}
   
  
   
////////////////////////////// do the thing
  if (linfo.bitdepht == 32) {
	  
		IEEEramptr = &IEEEramch1;   
		bee32 (*IEEEramptr);
			if (linfo.channels == 2){
				IEEEramptr = &IEEEramch2;
				bee32 (*IEEEramptr);	  
	  } 
	  
		}else {
		
		ramptr = &ramch1;   
		bee (*ramptr);
			if (linfo.channels == 2){
				ramptr = &ramch2;
				bee (*ramptr);

			}	
		}


   
   //////////////////////////////////////////// write the file
   
   
   //copy the header
   
   fseek(fptr, 0, SEEK_SET);
   fseek(wptr, 0, SEEK_SET);
   
	for(n = 0; n < 44 ; n++) {
		fread(&hcbyte, 1, 1, fptr);
		fwrite(&hcbyte, 1, 1, wptr);
	}

  // copy the data
  
  if (linfo.bitdepht == 32) {
  
   for(n = 0; n < linfo.sampleperchannel ; n++)
   {
	  fwrite(&IEEEramch1[n], linfo.bytespersample, 1, wptr); 
			if (linfo.channels == 2){
			fwrite(&IEEEramch2[n], linfo.bytespersample, 1, wptr);
			}	          
		}
  } else {
	  
	for(n = 0; n < linfo.sampleperchannel ; n++)
   {
	  fwrite(&ramch1[n], linfo.bytespersample, 1, wptr); 
			if (linfo.channels == 2){
			fwrite(&ramch2[n], linfo.bytespersample, 1, wptr);
			}  
	  }
	
  }
	
  printf("Ok, file written.\n") ;
  
  
  fclose(fptr);
  fclose(wptr);
  if (linfo.bitdepht == 32) {
	  
	  free(IEEEramch1);
	  //freebytes(IEEEramch1, (linfo.sampleperchannel * 4 + 1));
	   

		   if (linfo.channels == 2){
			free(IEEEramch2);
			//freebytes(IEEEramch2, (linfo.sampleperchannel * 4 + 1));
			
						
					}		
			}; 

		    if ((linfo.bitdepht == 16) || (linfo.bitdepht == 24)) {
				free(ramch1);
				//freebytes(ramch1, (linfo.sampleperchannel * 4 + 1));
				
			if (linfo.channels == 2){
					free(ramch2);
					//freebytes(ramch2, (linfo.sampleperchannel * 4 + 1));
					
					}		
			};
  
  
  
  
  
  printf("RAM deallocated.\n") ;
  printf("******************\n") ;
  
  
  pq_end:
  
   return 0;

   };


/////////////////////////////////// Functions //////////////////////




int createram (void) {
   ramch1 = (void *) malloc (linfo.sampleperchannel * 4 + 1);  
   if (ramch1 == NULL){
       printf("Error! memory trouble");
	   free(ramch1);
       exit(1);
   }   
   
   if (linfo.channels == 2){
	   ramch2 = (void *) malloc (linfo.sampleperchannel * 4 + 1);
       if (ramch2 == NULL){
       printf("Error! memory trouble");
	   free(ramch2);
       exit(1);
	   }

   }
return 0;
}



int createramIEEE (void) {
   IEEEramch1 = (void *) malloc (linfo.sampleperchannel * 4 + 1);  
   if (IEEEramch1 == NULL){
       printf("Error! memory trouble");
	   free(IEEEramch1);
       exit(1);
   }   
   
   if (linfo.channels == 2){
	   IEEEramch2 = (void *) malloc (linfo.sampleperchannel * 4 + 1);
       if (IEEEramch2 == NULL){
       printf("Error! memory trouble");
	   free(IEEEramch2);
       exit(1);
	   }
   }
return 0;
}






//////////////// for 16 or 24 bit
int bee (uint32_t *ramptr) {
	
	
uint32_t pos;
uint32_t startpos;
uint32_t endpos;
uint32_t peak;
int miniv;

if (linfo.bitdepht == 24){miniv = MIN24; }
	else{miniv = MIN16;}

	startpos = 0;
	endpos = 0;
	pos = 0;
   

   
   LOOP:while( pos < linfo.sampleperchannel ) {
	   
	  ramptr[pos] = 0; 
  
      pos++;
		
      if( ramptr[pos] > miniv) {
         
         break;
      }
   }
   
   startpos = pos;
   peak = 0;
   
   while( pos < linfo.sampleperchannel ) {
  
      pos++;
	  
	  if (ramptr[pos] > peak){
		  peak = ramptr[pos];
	  }
		
      if( ramptr[pos] < miniv) {
         
         break;
      }
   }
   
   endpos = pos;
   for(pos = startpos; pos < endpos ; pos++){
	   
	ramptr[pos] = peak;
   }
   endpos = pos;
   
   
   if (pos < linfo.sampleperchannel) {
	   goto LOOP;
   }
  
   


return 0;
  

}


//////////////// for 32bit


int bee32 (float *IEEEramptr) {
	
	
uint32_t pos;
uint32_t startpos;
uint32_t endpos;
float peakIEEE;
float miniv32;

    miniv32 = MIN32;
	startpos = 0;
	endpos = 0;
	pos = 0;
   

   
   LOOP:while( pos < linfo.sampleperchannel ) {
	   
	  IEEEramptr[pos] = 0; 
  
      pos++;
		
      if( IEEEramptr[pos] > miniv32) {
         
         break;
      }
   }
   
   startpos = pos;
   peakIEEE = 0;
   
   while( pos < linfo.sampleperchannel ) {
  
      pos++;
	  
	  if (IEEEramptr[pos] > peakIEEE){
		  peakIEEE = IEEEramptr[pos];
	  }
	  
	  
		
      if( IEEEramptr[pos] < miniv32) {
         
         break;
      }
   }
   
   endpos = pos;
   for(pos = startpos; pos < endpos ; pos++){
	   
	IEEEramptr[pos] = peakIEEE;
   }
   endpos = pos;
   
   
   if (pos < linfo.sampleperchannel) {
	   goto LOOP;
   }
  
   


return 0;
  

}




/////////////////////////////////////////////////////////////////////


uint32_t love1 (int32_t bytes333) {
	
		
uint8_t msb;
uint32_t sampVok;



if (linfo.bitdepht == 24){msb = bytes333 >> 23; }
	else{msb = bytes333 >> 15;}


 
if (msb == 0){
	sampVok = bytes333;} 
	else { 
		if (linfo.bitdepht == 24){
			sampVok = (bytes333 - 16777216) * -1;}
			else{sampVok = (bytes333 - 65536) * -1;}
	}

   return sampVok;
  
	
};// love1

/////////////////////////////////////////////////////////////////////


float love32 (float IEEE) {
	
		
float sampVokIEEE;

 
if (IEEE < 0){
	sampVokIEEE = IEEE * -1;}
	else{sampVokIEEE = IEEE;}
 return sampVokIEEE;

};// love32

///////////////////////////

int positiveram (void) {
	
   fseek(fptr, 44, SEEK_SET);
   
   for(n = 0; n < linfo.sampleperchannel ; n++)
   {
	   
	  fread(&num24, linfo.bytespersample, 1, fptr); 
	  bytes333 = num24.byt1 + (num24.byt2 << 8) + (num24.byt3 << 16); 
	  sampVokmain = love1(bytes333);
	  *(ramch1 + n) = sampVokmain ;
	  
	  
		if (linfo.channels == 2){
	   
			fread(&num24, linfo.bytespersample, 1, fptr); 
			bytes333 = num24.byt1 + (num24.byt2 << 8) + (num24.byt3 << 16); 
			sampVokmain = love1(bytes333);
			*(ramch2 + n) = sampVokmain ;
       
		}
	 
   }
   return 0;
}


///////////////////////////

int positiveramIEEE (void) {
	
   fseek(fptr, 44, SEEK_SET);
   
   for(n = 0; n < linfo.sampleperchannel ; n++)
   {
	  fread(&IEEE, linfo.bytespersample, 1, fptr);
	  sampVokmainIEEE = love32(IEEE);
	  *(IEEEramch1 + n) = sampVokmainIEEE ;
	  
	  
	  
		if (linfo.channels == 2){
			
			fread(&IEEE, linfo.bytespersample, 1, fptr); 
			sampVokmainIEEE = love32(IEEE);
			*(IEEEramch2 + n) = sampVokmainIEEE ;	   

      
		}
	 
   }
   
   
   return 0;
}