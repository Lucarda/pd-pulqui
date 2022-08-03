/* pulqui external for Pure-Data
// Written by Lucas Cordiviola 05-2019 07-2022 v0.1.1
// No warranties
// See License.txt
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "m_pd.h"

static t_class *pulqui_class;

typedef struct infoweneed 
{
    t_object  x_obj;
    t_canvas  *x_canvas;
    t_outlet *symbol_out, *bang_out;
    uint32_t samplebytes;
    uint8_t bitdepht;
    uint8_t channels;
    uint8_t bytespersample;
    uint32_t sampleperchannel;
    char filename[512];
    char outfilename[512];
    char datafoo[5];
    int startdatachunk;
    uint8_t audioformat;
    FILE *fptr; // file for reading
    FILE *wptr; // file for writing
    float *ramch1;
    float *ramch2;
} t_pulqui;

struct bytes 
{
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3; 
} xbytes;

const char *refuse ="pulqui: error. refusing to open file. does not look like a WAV file.";


static char pq_streq(const char *str1, const char *str2) 
{    
    if (0 == strcmp(str1, str2)) { 
           return 1;
       } else {
           return 0;
       }  
}

static char pq_wavinfo(t_pulqui *x)
{
    int i; 
        
    if ((x->fptr = fopen(x->filename,"rb")) == NULL){
        logpost(x,PD_ERROR,"pulqui: Error! opening file");
        logpost(x,2,"******************");
        fclose(x->fptr);
        outlet_bang(x->bang_out);
        return 0;
    }
    
    // file sanity checks
    fseek(x->fptr, 0, SEEK_SET);
    fread(&x->datafoo, 4, 1, x->fptr);
    if (!pq_streq(x->datafoo, "RIFF")) {
        logpost(x,PD_ERROR,"%s",refuse);
        logpost(x,2,"******************");
        fclose(x->fptr);
        outlet_bang(x->bang_out);
        return 0;
    }
        
    fseek(x->fptr, 8, SEEK_SET);
    fread(&x->datafoo, 4, 1, x->fptr);
    if (!pq_streq(x->datafoo, "WAVE")) {
        logpost(x,PD_ERROR,"%s",refuse);
        logpost(x,2,"******************");
        fclose(x->fptr);
        outlet_bang(x->bang_out);
        return 0;
    }
    
    fseek(x->fptr, 12, SEEK_SET);
    fread(&x->datafoo, 4, 1, x->fptr);
    if (!pq_streq(x->datafoo, "fmt ")) {
        logpost(x,PD_ERROR,"%s",refuse);
        logpost(x,2,"******************");
        fclose(x->fptr);
        outlet_bang(x->bang_out);
        return 0;
    }
        
    fseek(x->fptr, 20, SEEK_SET);
    fread(&x->audioformat, 2, 1, x->fptr);
    //logpost(x,2,"audioformat: %d" , x->audioformat);
    
    
    // get channels
    fseek(x->fptr, 22, SEEK_SET);
    fread(&x->channels, 1, 1, x->fptr);
    logpost(x,2,"channels: %d" , x->channels);


    // get bit-depth
    fseek(x->fptr, 34, SEEK_SET);
    fread(&x->bitdepht, 1, 1, x->fptr);
    logpost(x,2,"bit-depth: %d" , x->bitdepht);

    //  bit-depth to bytes
    x->bytespersample = x->bitdepht / 8;
    //logpost(x,2,"bytespersample: %d" , x->bytespersample);

    // go to the "data" chunk
    for (i=0; i<200; i++) {
        fseek(x->fptr, i, SEEK_SET);
        fread(&x->datafoo, 4, 1, x->fptr);
        if (pq_streq(x->datafoo, "data")) { 
            x->startdatachunk = i + 4;
            break;
        }
    }

    // get databytes 
    fseek(x->fptr, x->startdatachunk, SEEK_SET);
    fread(&x->samplebytes, 4, 1, x->fptr);
    //logpost(x,2,"samplebytes: %d" , x->samplebytes);   

    //  samples per channel
    x->sampleperchannel = (x->samplebytes / x->bytespersample) / x->channels;
    logpost(x,2,"samples per channel: %d" , x->sampleperchannel);
   
    fclose(x->fptr);
    
    if ((x->audioformat == 1) && (x->bitdepht == 32)) {
        logpost(x,PD_ERROR,"pulqui: error. refusing to open 32bit 'pcm' file. only 32bit 'float' files are supported.");
        logpost(x,2,"******************");
        outlet_bang(x->bang_out);
        return 0;
    }
        
    if (x->channels > 2) {
        logpost(x,PD_ERROR,"pulqui: error. refusing to open wav file with more than 2 chahnnels");
        logpost(x,2,"******************");
        outlet_bang(x->bang_out);
        return 0;
    }
    return 1;
}

static char pq_allocate_array(t_pulqui *x)
{
    x->ramch1 = (float *) getbytes((x->sampleperchannel * sizeof(float)) );
    if (x->ramch1 == NULL) {
        logpost(x,PD_ERROR,"pulqui: Error! memory trouble");
        logpost(x,2,"******************");
        freebytes(x->ramch1,sizeof(x->ramch1));
        outlet_bang(x->bang_out);
        return 0;
    }
    //logpost(x,2,"allocate bytes for channel 1: %d", (x->sampleperchannel * sizeof(float)) );
    
    if (x->channels == 2) {
        x->ramch2 = (float *) getbytes((x->sampleperchannel * sizeof(float)) );
        if (x->ramch2 == NULL) {
            logpost(x,PD_ERROR,"pulqui: Error! memory trouble");
            logpost(x,2,"******************");
            freebytes(x->ramch2,sizeof(x->ramch2));
            outlet_bang(x->bang_out);
            return 0;
        }
        //logpost(x,2,"allocate bytes for channel 2: %d", (x->sampleperchannel * sizeof(float)) );
    }
    logpost(x,2,"allocated bytes in RAM: %d", (x->sampleperchannel * sizeof(float)) );
    
    return 1;
}

static float pq_positive_16(t_pulqui *x)
{
    int sample;
    char sign;
    
    fread(&xbytes, 2, 1, x->fptr);
    sample = xbytes.byte1 + (xbytes.byte2 << 8);
    sign = sample >> 15;
    if (sign) {
        return (((sample - 65536.) / 32768) * -1);
    } else {
        return (sample / 32768.);
    }   
}

static void pq_wavtoarray16(t_pulqui *x)
{
    uint32_t i;
    
    for (i=0; i < (x->sampleperchannel); i++) {     
       x->ramch1[i] = pq_positive_16(x);
       if (x->channels == 2){
           x->ramch2[i] = pq_positive_16(x);
       }    
   } 
}

static float pq_positive_24(t_pulqui *x)
{
    int sample;
    char sign;
    
    fread(&xbytes, 3, 1, x->fptr);
    sample = xbytes.byte1 + (xbytes.byte2 << 8) + (xbytes.byte3 << 16);
    sign = sample >> 23;
    if (sign) {
        return (((sample - 16777216.) / 8388608) * -1);
    } else {
        return (sample / 8388608.);
    }   
}

static void pq_wavtoarray24(t_pulqui *x)
{
    uint32_t i;
   
    for (i=0; i < (x->sampleperchannel); i++) {
       x->ramch1[i] = pq_positive_24(x);
       if (x->channels == 2){
           x->ramch2[i] = pq_positive_24(x);
       }   
    }
}

static float pq_positive_32(t_pulqui *x)
{
    float sample;   
    
    fread(&sample, 4, 1, x->fptr);    
    if (sample < 0) {
        return (sample * -1);
    } else {
        return (sample);
    }   
}

static void pq_wavtoarray32(t_pulqui *x)
{
    uint32_t i;
   
    for (i=0; i < (x->sampleperchannel); i++) {
        x->ramch1[i] = pq_positive_32(x);
        if (x->channels == 2){
            x->ramch2[i] = pq_positive_32(x);
       }
   }
}

static void pq_zero_last_sample(t_pulqui *x)
{
    x->ramch1[x->sampleperchannel] = 0;
    if (x->channels == 2){
            x->ramch2[x->sampleperchannel] = 0;
        }    
}

static void pq_bee32(t_pulqui *x, float *IEEEramptr) {

    uint32_t pos;
    uint32_t startpos;
    uint32_t endpos;
    float peakIEEE;
    float miniv32 = 0.01;
    
    startpos = 0;
    endpos = 0;
    pos = 0;
   
    LOOP:while (pos < x->sampleperchannel ) {

        IEEEramptr[pos] = 0;   
        pos++;
        if ( IEEEramptr[pos] > miniv32) {       
            break;
        }
    }   
    startpos = pos;
    peakIEEE = 0;
    while (pos < x->sampleperchannel ) {
  
        pos++;
        if (IEEEramptr[pos] > peakIEEE){
            peakIEEE = IEEEramptr[pos];
        }
        if( IEEEramptr[pos] < miniv32) {        
            break;
        }
    }   
    endpos = pos;
    for (pos = startpos; pos < endpos; pos++){

        IEEEramptr[pos] = peakIEEE;
    }
    //endpos = pos;     
    if (pos < x->sampleperchannel) {
        goto LOOP;
    }
}

static void pq_do_pq(t_pulqui *x) 
{   
    pq_bee32(x, x->ramch1);
    if (x->channels == 2){
        pq_bee32(x, x->ramch2);
    }
}

static char pq_openwritefile(t_pulqui *x)
{
    int n;
    uint8_t *byte;
    x->fptr = fopen(x->filename,"rb");
    if ((x->wptr = fopen(x->outfilename,"wb")) == NULL){
        logpost(x,PD_ERROR,"pulqui: Error opening file for writing");
        logpost(x,2,"******************");
        fclose(x->wptr);
        outlet_bang(x->bang_out);
        return 0;
    }
   
    fseek(x->fptr, 0, SEEK_SET);
    fseek(x->wptr, 0, SEEK_SET);
    
    //copy the header   
    for(n = 0; n < x->startdatachunk+4; n++) {
        fread(&byte, 1, 1, x->fptr);
        fwrite(&byte, 1, 1, x->wptr);
    }
    
    fclose(x->fptr);
    return 1;
}

static void pq_write16(t_pulqui *x)
{
    uint32_t i;
    int sample;
    
    for (i=0; i < (x->sampleperchannel); i++) {
        sample = (int)(x->ramch1[i] * 32768);
        fwrite(&sample, 2, 1, x->wptr);
        if (x->channels == 2){
            sample = (int)(x->ramch2[i] * 32768);
            fwrite(&sample, 2, 1, x->wptr);
        }
    }    
}

static void pq_write24(t_pulqui *x)
{
    uint32_t i;
    int sample;
    
    for (i=0; i < (x->sampleperchannel); i++) {
        sample = (int)(x->ramch1[i] * 8388608);
        fwrite(&sample, 3, 1, x->wptr);
        if (x->channels == 2){
            sample = (int)(x->ramch2[i] * 8388608);
            fwrite(&sample, 3, 1, x->wptr);
        }
    }    
}

static void pq_write32(t_pulqui *x)
{
    uint32_t i;
    
    for (i=0; i < (x->sampleperchannel); i++) {
        fwrite(&x->ramch1[i], 4, 1, x->wptr);
        if (x->channels == 2){
            fwrite(&x->ramch2[i], 4, 1, x->wptr);
        }
    }    
}

static void pq_do_writefile(t_pulqui *x)
{

    if (x->bitdepht == 16) {
        pq_write16(x);        
    } else if (x->bitdepht == 24) {
        pq_write24(x);        
    } else if (x->bitdepht == 32) {
        pq_write32(x);
    }
    fclose(x->wptr);
    freebytes(x->ramch1,sizeof(x->ramch1));
    freebytes(x->ramch2,sizeof(x->ramch2));
}

static void pq_do_positive_array(t_pulqui *x)
{
    
    x->fptr = fopen(x->filename,"rb");
    fseek(x->fptr, x->startdatachunk+4, SEEK_SET);
        
    if (x->bitdepht == 16) {
        pq_wavtoarray16(x);        
    } else if (x->bitdepht == 24) {
        pq_wavtoarray24(x);        
    } else if (x->bitdepht == 32) {
        pq_wavtoarray32(x);
    }
    fclose(x->fptr);
}


static char pq_filer(t_pulqui *x, t_symbol *name) {
    
    char completefilename[MAXPDSTRING];

    const char* filename = name->s_name;
    char realdir[MAXPDSTRING], *realname = NULL;
    int fd;
    fd = canvas_open(x->x_canvas, filename, "", realdir, &realname, MAXPDSTRING, 0);
        if(fd < 0){
            logpost(NULL,2," ");
            logpost(NULL,2,"***** pulqui *****");
            logpost(x, PD_ERROR, "pulqui: can't find file: %s", filename);
            logpost(NULL,2,"******************");
            outlet_bang(x->bang_out);
            return 0;
        }

    sys_close(fd);
    strcpy(completefilename, realdir);
    strcat(completefilename, "/");
    strcat(completefilename, realname);
    strcpy(x->filename, completefilename);
    strcpy(x->outfilename, x->filename);
    strcat(x->outfilename, "f");
    return 1;
}                                                                                   

void pulqui_main(t_pulqui *x, t_symbol *filename)
{
    
    if(!pq_filer(x, filename)) {
    return;
    }
    logpost(x,2," ");
    logpost(x,2,"***** pulqui *****");
    logpost(x,2,"file: %s", filename->s_name);    

    if(!pq_wavinfo(x)) {
    return;
    }
    
    if(!pq_allocate_array(x)) {
    return;
    }
    
    pq_do_positive_array(x);
    pq_zero_last_sample(x);
    logpost(x,2,"started scanning...");
    pq_do_pq(x);
    
    if(!pq_openwritefile(x)) {
    return;
    }

    pq_do_writefile(x);
    logpost(x,2,"RAM deallocated.");
    logpost(x,2,"******************");
    outlet_symbol(x->symbol_out, gensym(x->outfilename));
    
}




void *pulqui_new(void)
{

    t_pulqui *x = (t_pulqui *)pd_new(pulqui_class);
   
    x->x_canvas = canvas_getcurrent();
    x->symbol_out = outlet_new(&x->x_obj, &s_symbol);
    x->bang_out = outlet_new(&x->x_obj, &s_bang);
   
    return (void *)x;
}



void pulqui_setup(void) {
    
    logpost(NULL,2," ");    
    logpost(NULL,2,"***** pulqui *****");
    logpost(NULL,2,"loaded version 0.1.1");
    logpost(NULL,2,"******************");

    pulqui_class = class_new(gensym("pulqui"),      
                    (t_newmethod)pulqui_new,
                    0 /*(t_method)pulqui_free*/,                          
                    sizeof(t_pulqui),       
                    CLASS_DEFAULT,               
                    0);                        

   
    class_addmethod(pulqui_class, (t_method)pulqui_main, gensym("read"), A_SYMBOL, 0);
 
}