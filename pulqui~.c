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
    int x_scanlen, x_len;
    int x_pulquiblock;
} t_pulqui_tilde;

static void pulqui_tilde_alloc(t_pulqui_tilde *x)
{
    x->x_scanlen = x->x_len * 2;
    x->x_ramchpositive = (t_sample *)getbytes(x->x_scanlen * sizeof(t_sample));
    x->x_ramchnegative = (t_sample *)getbytes(x->x_scanlen * sizeof(t_sample));
    x->x_ramch = (t_sample *)getbytes(x->x_len * sizeof(t_sample));
    x->x_bufsignal = (t_sample *)getbytes(x->x_len * sizeof(t_sample));
    x->x_bufsignalout = (t_sample *)getbytes(x->x_len * sizeof(t_sample));
    x->x_bufpulqui = (t_sample *)getbytes(x->x_len * sizeof(t_sample));
    for (int i = 0; i < x->x_len; i++)
    {
        x->x_bufpulqui[i] = 1;
    }
}

static void *pulqui_tilde_new(t_floatarg len)
{
    t_pulqui_tilde *x = (t_pulqui_tilde *)pd_new(pulqui_tilde_class);
    if (len < 512)
    {
        logpost(x,2,"pulqui~: block resized to a minimum of 512 samples.\
            ignoring requested size of '%d'", (int)len);
        x->x_len = 512;
    }
    else
        x->x_len = (int)len;
    pulqui_tilde_alloc(x);
    x->x_out1=outlet_new(&x->x_obj, &s_signal);
    x->x_out2=outlet_new(&x->x_obj, &s_signal);

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
    if (x->x_len < sp[0]->s_n)
    {
        x->x_len = sp[0]->s_n;
        pulqui_tilde_alloc(x);
        logpost(x,2,"pulqui~: reallocated to a bigger block size of %d samples", x->x_len);
    }
    dsp_add(pulqui_tilde_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
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
}