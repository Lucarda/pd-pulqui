/* pq command line interface
// Written by Lucas Cordiviola 05-2019 10-2023
// No warranties
// See License.txt
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define FULL16BIT 65536.0
#define HALF16BIT 32768.0
#define HICLIP16 32767
#define LOWCLIP16 -32767
#define FULL24BIT 16777216.0
#define HALF24BIT 8388608.0
#define HICLIP24 8388607
#define LOWCLIP24 -8388607

struct infoweneed
{
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
} info;

struct bytes
{
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
} xbytes;

static const char *version = "0.1.2";

static const char *refuse ="\nerror: refusing to open file. does not look like a WAV file.\n";

static const char *usage = "usage: pq <file> \n\
\t <file> \t input wav file \n\
example: \n\
\t pq foo.wav\n\
";


static char pq_streq(const char *str1, const char *str2)
{
    if (0 == strcmp(str1, str2)) return 1;
    else return 0;
}


static void pq_wavinfo(void)
{
    int i;
    if ((info.fptr = fopen(info.filename,"rb")) == NULL)
    {
        printf("Error opening file for reading");
        fclose(info.fptr);
        exit(1);
    }

    // file sanity checks
    fseek(info.fptr, 0, SEEK_SET);
    fread(&info.datafoo, 4, 1, info.fptr);
    if (!pq_streq(info.datafoo, "RIFF"))
    {
        printf("%s",refuse);
        fclose(info.fptr);
        exit(1);
    }

    fseek(info.fptr, 8, SEEK_SET);
    fread(&info.datafoo, 4, 1, info.fptr);
    if (!pq_streq(info.datafoo, "WAVE"))
    {
        printf("%s",refuse);
        fclose(info.fptr);
        exit(1);
    }

    fseek(info.fptr, 12, SEEK_SET);
    fread(&info.datafoo, 4, 1, info.fptr);
    if (!pq_streq(info.datafoo, "fmt "))
    {
        printf("%s",refuse);
        fclose(info.fptr);
        exit(1);
    }

    fseek(info.fptr, 20, SEEK_SET);
    fread(&info.audioformat, 2, 1, info.fptr);
    //printf("audioformat: %d\n" , info.audioformat);


    // get channels
    fseek(info.fptr, 22, SEEK_SET);
    fread(&info.channels, 1, 1, info.fptr);
    printf("channels: %d\n" , info.channels);


    // get bit-depth
    fseek(info.fptr, 34, SEEK_SET);
    fread(&info.bitdepht, 1, 1, info.fptr);
    printf("bit-depth: %d\n" , info.bitdepht);

    //  bit-depth to bytes
    info.bytespersample = info.bitdepht / 8;
    //printf("bytespersample: %d\n" , info.bytespersample);

    // go to the "data" chunk
    for (i=0; i<200; i++)
    {
        fseek(info.fptr, i, SEEK_SET);
        fread(&info.datafoo, 4, 1, info.fptr);
        if (pq_streq(info.datafoo, "data"))
        {
            info.startdatachunk = i + 4;
            break;
        }
    }

    // get databytes
    fseek(info.fptr, info.startdatachunk, SEEK_SET);
    fread(&info.samplebytes, 4, 1, info.fptr);
    //printf("samplebytes: %d\n" , info.samplebytes);

    //  samples per channel
    info.sampleperchannel = (info.samplebytes / info.bytespersample) / info.channels;
    //printf("samples per channel: %d\n" , info.sampleperchannel);

    fclose(info.fptr);

    if ((info.audioformat == 1) && (info.bitdepht == 32))
    {
        printf("\nerror: refusing to open 32bit 'int' file. only 32bit 'float' files are supported.\n");
        exit(1);
    }

    if (info.channels > 2)
    {
        printf("\nerror: refusing to open wav file with more than 2 chahnnels\n");
        exit(1);
    }
}

static void pq_allocate_array(void)
{
    info.ramch1 = (float *) malloc((info.sampleperchannel * sizeof(float)) );
    if (info.ramch1 == NULL)
    {
        printf("Error! memory trouble");
        free(info.ramch1);
        exit(1);
    }

    if (info.channels == 2)
    {
        info.ramch2 = (float *) malloc((info.sampleperchannel * sizeof(float)) );
        if (info.ramch2 == NULL)
        {
            printf("Error! memory trouble");
            free(info.ramch2);
            exit(1);
        }
    }
    printf("allocated bytes in RAM: %d MB\n", ((info.sampleperchannel * 4) * info.channels) / 1000000);
}

static float pq_positive_16(void)
{
    int sample;
    char sign;
    float xsample;
    fread(&xbytes, 2, 1, info.fptr);
    sample = xbytes.byte1 + (xbytes.byte2 << 8);
    sign = sample >> 15;
    if (sign) xsample = (((sample - FULL16BIT) / HALF16BIT) * -1);
    else xsample =  (sample / HALF16BIT);
    // protect float to not be 1 or -1: if this is the case make them 0.998
    if (xsample > 0.998 || xsample == -1.0) xsample = 0.998;
    return (xsample);
}

static void pq_wavtoarray16(void)
{
    uint32_t i;
    for (i=0; i < (info.sampleperchannel); i++)
    {
       info.ramch1[i] = pq_positive_16();
       if (info.channels == 2) info.ramch2[i] = pq_positive_16();
    }
}

static float pq_positive_24(void)
{
    int sample;
    char sign;
    float xsample;
    fread(&xbytes, 3, 1, info.fptr);
    sample = xbytes.byte1 + (xbytes.byte2 << 8) + (xbytes.byte3 << 16);
    sign = sample >> 23;
    if (sign) xsample = (((sample - FULL24BIT) / HALF24BIT) * -1);
    else xsample = (sample / HALF24BIT);
    // protect float to not be 1 or -1: if this is the case make them 0.998
    if (xsample > 0.998 || xsample == -1.0) xsample = 0.998;
    return (xsample);
}

static void pq_wavtoarray24(void)
{
    uint32_t i;
    for (i=0; i < (info.sampleperchannel); i++)
    {
       info.ramch1[i] = pq_positive_24();
       if (info.channels == 2) info.ramch2[i] = pq_positive_24();
    }
}

static float pq_positive_32(void)
{
    float sample;
    fread(&sample, 4, 1, info.fptr);
    if (sample < 0) return (sample * -1);
    else return (sample);
}

static void pq_wavtoarray32(void)
{
    uint32_t i;
    for (i=0; i < (info.sampleperchannel); i++)
    {
        info.ramch1[i] = pq_positive_32();
        if (info.channels == 2) info.ramch2[i] = pq_positive_32();
    }
}

static void pq_zero_last_sample(void)
{
    info.ramch1[info.sampleperchannel] = 0;
    if (info.channels == 2) info.ramch2[info.sampleperchannel] = 0;
}

static void pq_bee32 (float *IEEEramptr)
{
    uint32_t pos;
    uint32_t startpos;
    uint32_t endpos;
    float peakIEEE;
    float miniv32 = 0.01;
    startpos = 0;
    endpos = 0;
    pos = 0;

    LOOP:while (pos < info.sampleperchannel )
    {
        IEEEramptr[pos] = 0;
        pos++;
        if ( IEEEramptr[pos] > miniv32) break;
    }
    startpos = pos;
    peakIEEE = 0;
    while (pos < info.sampleperchannel)
    {
        pos++;
        if (IEEEramptr[pos] > peakIEEE) peakIEEE = IEEEramptr[pos];
        if( IEEEramptr[pos] < miniv32) break;
    }
    endpos = pos;
    for (pos = startpos; pos < endpos ; pos++)
    {
        IEEEramptr[pos] = peakIEEE;
    }
    //endpos = pos;
    if (pos < info.sampleperchannel) goto LOOP;
}

static void pq_do_pq(void)
{
    pq_bee32(info.ramch1);
    if (info.channels == 2) pq_bee32(info.ramch2);
}

static void pq_openwritefile(void)
{
    int n;
    uint8_t *byte;
    info.fptr = fopen(info.filename,"rb");
    if ((info.wptr = fopen(info.outfilename,"wb")) == NULL)
    {
        printf("Error opening file for writing");
        fclose(info.wptr);
        exit(1);
    }
    fseek(info.fptr, 0, SEEK_SET);
    fseek(info.wptr, 0, SEEK_SET);
    //copy the header
    for(n = 0; n < info.startdatachunk+4 ; n++)
    {
        fread(&byte, 1, 1, info.fptr);
        fwrite(&byte, 1, 1, info.wptr);
    }
    fclose(info.fptr);
}

static void pq_write16()
{
    uint32_t i;
    int sample;
    for (i=0; i < (info.sampleperchannel); i++)
    {
        sample = (int)(info.ramch1[i] * HALF16BIT);
        fwrite(&sample, 2, 1, info.wptr);
        if (info.channels == 2)
        {
            sample = (int)(info.ramch2[i] * HALF16BIT);
            fwrite(&sample, 2, 1, info.wptr);
        }
    }
}

static void pq_write24()
{
    uint32_t i;
    int sample;
    for (i=0; i < (info.sampleperchannel); i++)
    {
        sample = (int)(info.ramch1[i] * HALF24BIT);
        fwrite(&sample, 3, 1, info.wptr);
        if (info.channels == 2)
        {
            sample = (int)(info.ramch2[i] * HALF24BIT);
            fwrite(&sample, 3, 1, info.wptr);
        }
    }
}

static void pq_write32()
{
    uint32_t i;
    for (i=0; i < (info.sampleperchannel); i++)
    {
        fwrite(&info.ramch1[i], 4, 1, info.wptr);
        if (info.channels == 2) fwrite(&info.ramch2[i], 4, 1, info.wptr);
    }
}

static void pq_do_writefile(void)
{
    if (info.bitdepht == 16) pq_write16();
    else if (info.bitdepht == 24) pq_write24();
    else if (info.bitdepht == 32) pq_write32();
    fclose(info.wptr);
    free(info.ramch1);
    free(info.ramch2);
}

static void pq_do_positive_array(void)
{
    info.fptr = fopen(info.filename,"rb");
    fseek(info.fptr, info.startdatachunk+4, SEEK_SET);
    if (info.bitdepht == 16) pq_wavtoarray16();
    else if (info.bitdepht == 24) pq_wavtoarray24();
    else if (info.bitdepht == 32) pq_wavtoarray32();
    fclose(info.fptr);
}

int main(int argc, char *argv[])
{
    printf("pq v%s\n", version);
    if (pq_streq(argv[1], "--help") || pq_streq(argv[1], "-h"))
    {
        printf("%s", usage);
        exit(1);
    }
    sprintf(info.filename,"%s", argv[1]);
    sprintf(info.outfilename,"%sf", argv[1]);
    printf("file: %s\n", argv[1]);
    pq_wavinfo();
    pq_allocate_array();
    pq_do_positive_array();
    pq_zero_last_sample();
    printf("started scanning...\n") ;
    pq_do_pq();
    pq_openwritefile();
    pq_do_writefile();
    printf("RAM deallocated.\n") ;
    printf("\nfile written: %s\n", info.outfilename);
}