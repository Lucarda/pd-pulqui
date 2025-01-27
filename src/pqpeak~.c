/*
Lucas Cordiviola <lucarda27@hotmail.com> 01-2025
No warranties
See License.txt
*/

#include "m_pd.h"
#include <math.h>


static t_class *pqpeak_tilde_class;

typedef struct _pqpeak_tilde {
    t_object x_obj;
    t_sample x_v;
    t_sample *x_buf;
    int x_block;
    t_clock *x_clock;
    t_outlet *x_out;
    t_float x_f;
} t_pqpeak_tilde;  

static void pqpeak_tilde_peak(t_pqpeak_tilde *x)
{
    int i;
    x->x_v = 0;    
    for (i=0; i<2048; i++)
    {
       if (x->x_buf[i] > x->x_v) x->x_v = x->x_buf[i];
    }    
}

static t_int *pqpeak_tilde_perform(t_int *w)
{
    t_pqpeak_tilde *x = (t_pqpeak_tilde *)(w[1]);
    t_sample    *in1 =      (t_sample *)(w[2]);
    int            n =             (int)(w[3]);
    int i;

    for (i=0; i<n; i++) 
    {
       t_sample f = in1[i];
       x->x_buf[x->x_block + i] = (f >= 0 ? f : -f);
    }
    
    x->x_block += n;
    
    if (x->x_block == 2048) 
    {
        pqpeak_tilde_peak(x);
        clock_delay(x->x_clock, 0);
        x->x_block = 0;
    }

    return (w+4);
}

static void pqpeak_tilde_dsp(t_pqpeak_tilde *x, t_signal **sp)
{
    dsp_add(pqpeak_tilde_perform, 3, x,
        sp[0]->s_vec, sp[0]->s_n);
}

static void pqpeak_tilde_tick(t_pqpeak_tilde *x)
{    
    t_float v = (t_float)20 * log10f ((float)x->x_v);
    
    outlet_float(x->x_out, v < -100 ? -100 : v);
}

static void pqpeak_tilde_free(t_pqpeak_tilde *x)
{
    clock_free(x->x_clock);
    freebytes(x->x_buf, 2048 * sizeof(t_sample));
}

static void *pqpeak_tilde_new(void)
{
    t_pqpeak_tilde *x = (t_pqpeak_tilde *)pd_new(pqpeak_tilde_class);
    x->x_clock = clock_new(x, (t_method)pqpeak_tilde_tick);
    x->x_out=outlet_new(&x->x_obj, &s_float);
    x->x_buf = getbytes(2048 * sizeof(t_sample));

    return (void *)x;
}

void pqpeak_tilde_setup(void) 
{
    logpost(NULL,2,"---");
    logpost(NULL,2,"  pqpeak~ v0.1.0");
    logpost(NULL,2,"---");
    pqpeak_tilde_class = class_new(gensym("pqpeak~"),
            (t_newmethod)pqpeak_tilde_new,
            (t_method)pqpeak_tilde_free, sizeof(t_pqpeak_tilde),
            CLASS_DEFAULT,
            0);

    class_addmethod(pqpeak_tilde_class,
            (t_method)pqpeak_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(pqpeak_tilde_class, t_pqpeak_tilde, x_f);
}