/* pulqui~ external for Pure-Data
// Written by Lucas Cordiviola 11-2023
// No warranties
// See License.txt
*/

#include "m_pd.h"

static t_class *pulqui_tilde_class;

typedef struct _pulqui_tilde
{
    t_object x_obj;
    t_float x_f; //
    t_sample *x_ramchpositive;
    t_sample *x_ramchnegative;
    t_sample *x_ramch;
    t_sample *x_bufsignal;
    t_sample *x_bufsignalout;
    t_sample *x_bufpulqui;
    t_outlet *x_out1;
    t_outlet *x_out2;
    t_outlet *x_outlet3;
    int x_scanlen, x_len, x_autoblk, x_blkchange;
    int x_pulquiblock;
    t_float x_infoblk, x_infomslat;
    char x_requestchangeblock;
    int x_oldscanlen, x_oldlen;
} t_pulqui_tilde;


static void pulqui_tilde_alloc(t_pulqui_tilde *x)
{
    x->x_scanlen = x->x_len * 2;
    x->x_ramchpositive = (t_sample *)resizebytes(x->x_ramchpositive,
		x->x_oldscanlen * sizeof(t_sample), x->x_scanlen * sizeof(t_sample));
    x->x_ramchnegative = (t_sample *)resizebytes(x->x_ramchnegative,
		x->x_oldscanlen * sizeof(t_sample), x->x_scanlen * sizeof(t_sample));
    x->x_ramch = (t_sample *)resizebytes(x->x_ramch,
		x->x_oldlen * sizeof(t_sample), x->x_len * sizeof(t_sample));
    x->x_bufsignal = (t_sample *)resizebytes(x->x_bufsignal,
		x->x_oldlen * sizeof(t_sample), x->x_len * sizeof(t_sample));
    x->x_bufsignalout = (t_sample *)resizebytes(x->x_bufsignalout,
		x->x_oldlen * sizeof(t_sample),x->x_len * sizeof(t_sample));
    x->x_bufpulqui = (t_sample *)resizebytes(x->x_bufpulqui,
		x->x_oldlen * sizeof(t_sample),x->x_len * sizeof(t_sample));
    for (int i = 0; i < x->x_len; i++)
    {
        x->x_bufpulqui[i] = 1;
    }
    x->x_oldscanlen = x->x_len * 2;
    x->x_oldlen = x->x_len;
}

static char powerof2(int x)
{
   return (x != 0) && ((x & (x - 1)) == 0); 
}

static void *pulqui_tilde_new(t_floatarg size)
{
    int len = size;
    t_pulqui_tilde *x = (t_pulqui_tilde *)pd_new(pulqui_tilde_class);
    x->x_requestchangeblock = 0;
    x->x_oldscanlen = 0;
    x->x_oldlen = 0;
    if (len > 1 && len < 512)
    {
        logpost(x,2,"pulqui~: block resized to a minimum of 512 samples. ignoring requested size of '%d'.", len);
        x->x_len = 512;
        x->x_autoblk=0;
        x->x_requestchangeblock = 1;
    }
    if (len >= 512)
    {       
        x->x_len = len;
        if (!powerof2(x->x_len)) 
        {
            logpost(x,2,"pulqui~: requested size is not a power of 2 and this could be unsafe. setting size to 4096 samples.");
            x->x_len = 4096;
        }
        x->x_autoblk=0;
        x->x_requestchangeblock = 1;
    }     
    if(len <= 0) x->x_autoblk=1;
    x->x_out1=outlet_new(&x->x_obj, &s_signal);
    x->x_out2=outlet_new(&x->x_obj, &s_signal);
    x->x_outlet3 = outlet_new(&x->x_obj, &s_list);

    return (x);
}

static void pq_bee32(t_pulqui_tilde *x)
{
    int pos;
    int startpos;
    int endpos;
    t_sample peakIEEE;
    startpos = 0;
    endpos = 0;
    pos = 0;

    LOOP:while (pos < x->x_scanlen)
    {
        if ( x->x_ramchpositive[pos] > 0.0001) break;
        pos++;
    }
    startpos = pos;
    peakIEEE = 0;
    while (pos < x->x_scanlen)
    {
        if (x->x_ramchpositive[pos] > peakIEEE) peakIEEE = x->x_ramchpositive[pos];
        if ( x->x_ramchpositive[pos] < 0.0001) break;
        pos++;
    }
    endpos = pos;
    for (pos = startpos; pos < endpos ; pos++)
    {
        x->x_ramchpositive[pos] = peakIEEE;
    }
    //endpos = pos;
    if (pos < x->x_scanlen) goto LOOP;
}

static void pq_bee32_negative(t_pulqui_tilde *x)
{
    int pos;
    int startpos;
    int endpos;
    t_sample peakIEEE;
    startpos = 0;
    endpos = 0;
    pos = 0;

    LOOP:while (pos < x->x_scanlen)
    {
        if ( x->x_ramchnegative[pos] < -0.0001) break;
        pos++;
    }
    startpos = pos;
    peakIEEE = 0;
    while (pos < x->x_scanlen)
    {
        if (x->x_ramchnegative[pos] < peakIEEE) peakIEEE = x->x_ramchnegative[pos];
        if ( x->x_ramchnegative[pos] > -0.0001) break;
        pos++;
    }
    endpos = pos;
    for (pos = startpos; pos < endpos ; pos++)
    {
        x->x_ramchnegative[pos] = peakIEEE;
    }
    //endpos = pos;
    if (pos < x->x_scanlen) goto LOOP;
}

static void pulqui_tilde_do_pulqui(t_pulqui_tilde *x)
{
    int i;
    for (i = 0; i < x->x_len; i++)
    {
         x->x_ramchpositive[x->x_len + i] = x->x_ramch[i];
         x->x_ramchnegative[x->x_len + i] = x->x_ramch[i];
    }

    pq_bee32(x);
    pq_bee32_negative(x);

    for (i = 0; i < x->x_len; i++)
    {
        x->x_bufsignalout[i] = x->x_bufsignal[i];
        if (x->x_ramchpositive[i] >  0.0001)
        {
            x->x_bufpulqui[i] = x->x_ramchpositive[i];
        }
        else if (x->x_ramchnegative[i] <  -0.0001)
        {
            x->x_bufpulqui[i] = x->x_ramchnegative[i] * -1;
        }
        else
        {
            x->x_bufpulqui[i] = 1;
        }
    }

    for (i = 0; i < x->x_len; i++)
    {
         x->x_ramchpositive[i] = x->x_ramchpositive[x->x_len + i];
         x->x_ramchnegative[i] = x->x_ramchnegative[x->x_len + i];
         x->x_bufsignal[i] = x->x_ramch[i];
    }
}

static t_int *pulqui_tilde_perform(t_int *w)
{
    t_pulqui_tilde *x = (t_pulqui_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    t_sample *out1 = (t_sample *)(w[3]);
    t_sample *out2 = (t_sample *)(w[4]);
    int n = (int)w[5];

    for (int i = 0; i < n; i++)
    {
        t_sample f = in[i];
        if (PD_BIGORSMALL(f))
            f = 0;
        x->x_ramch[i + x->x_pulquiblock] = f;
        out1[i] = x->x_bufsignalout[i + x->x_pulquiblock];
        out2[i] = x->x_bufpulqui[i + x->x_pulquiblock];
    }

    if(x->x_pulquiblock > ((x->x_len - n) - 1))
    {
        pulqui_tilde_do_pulqui(x);
        x->x_pulquiblock = 0;
    }
    else x->x_pulquiblock += n;

    return (w+6);
}


static void pulqui_tilde_dsp(t_pulqui_tilde *x, t_signal **sp)
{
    t_float sr = sp[0]->s_sr / 1000.;
    int block = sp[0]->s_n;

    if(x->x_autoblk)
    {
        int blkarray[100] = {'0'};
        int ms = (1000/20)/2; // half of 20hz in ms: 25ms
        t_float nsamples = ms * sr; // how much samples we need to store 25ms
        int blockaccum = 0;
        for(int i = 0; blockaccum < nsamples; i++)
        {
            blockaccum += block;
            blkarray[i] = blockaccum;
        }
        int blk = 0;
        for(int i = 0; blkarray[i] < (nsamples/2); i++)
        {
            blk = blkarray[i+1];
        }
        x->x_len = blk;
        if(x->x_len != x->x_oldlen) x->x_requestchangeblock = 1;
    }
    else x->x_pulquiblock = 0;
    if (x->x_len < block)
    {
        x->x_len = block;
        x->x_requestchangeblock = 1;
        logpost(x,2,"pulqui~: reallocated to a bigger block size of %d samples", x->x_len);
    }
    x->x_infoblk = x->x_len*2;
    x->x_infomslat = ((1000/sr)*(x->x_len*2))/1000;    
    if(x->x_requestchangeblock)
    {
        pulqui_tilde_alloc(x);
        x->x_pulquiblock = 0;
        x->x_requestchangeblock = 0;
    }
    dsp_add(pulqui_tilde_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void pulqui_tilde_info_outlet(t_pulqui_tilde *x)
{
    t_atom info[2];
    SETFLOAT(info+0, (int)x->x_infoblk);
    SETFLOAT(info+1, x->x_infomslat);
    outlet_list(x->x_outlet3, &s_list, 2, info);
}

static void pulqui_tilde_info(t_pulqui_tilde *x)
{
    logpost(x,2,"pulqui~ latency: %d samples (%.2fms)", (int)x->x_infoblk, x->x_infomslat);
}

static void pulqui_tilde_free(t_pulqui_tilde *x)
{
    freebytes(x->x_ramch,sizeof(x->x_len * sizeof(t_sample)));
    freebytes(x->x_ramchpositive,sizeof(x->x_scanlen * sizeof(t_sample)));
    freebytes(x->x_ramchnegative,sizeof(x->x_scanlen * sizeof(t_sample)));
    freebytes(x->x_bufsignal,sizeof(x->x_len * sizeof(t_sample)));
    freebytes(x->x_bufsignalout,sizeof(x->x_len * sizeof(t_sample)));
    freebytes(x->x_bufpulqui,sizeof(x->x_len * sizeof(t_sample)));
}

void pulqui_tilde_setup(void)
{
    logpost(NULL,2,"---");
    logpost(NULL,2,"  pulqui~ v0.1.0");
    logpost(NULL,2,"---");
    pulqui_tilde_class = class_new(gensym("pulqui~"), (t_newmethod)pulqui_tilde_new,
        (t_method)pulqui_tilde_free, sizeof(t_pulqui_tilde), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(pulqui_tilde_class, t_pulqui_tilde, x_f);
    class_addmethod(pulqui_tilde_class, (t_method)pulqui_tilde_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(pulqui_tilde_class, (t_method)pulqui_tilde_info, gensym("info"), 0);
    class_addmethod(pulqui_tilde_class, (t_method)pulqui_tilde_info_outlet, gensym("info_outlet"), 0);
}
