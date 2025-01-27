/*
4th order Linkwitz-Riley filters 
taken from pseudo code in:
https://www.musicdsp.org/en/latest/Filters/266-4th-order-linkwitz-riley-filters.html
(the first example)
adapted to Pd by Lucas Cordiviola <lucarda27@hotmail.com> 01-2025
No warranties
See License.txt
*/


#include "m_pd.h"
#include <math.h>




static t_class *pqcrossover_tilde_class;

struct filter {
    double a0;
    double a1;
    double a2;
    double a3;
    double a4;
    //------------------------------    
    double tempx;
    double tempy;
    double xm4;
    double xm3;
    double xm2;
    double xm1;
    double ym4;
    double ym3;
    double ym2;
    double ym1;    
};

typedef struct _pqcrossover_tilde {
    t_object x_obj;
    t_float x_f;
    t_outlet *x_outlp, *x_outhp;
    //------------------------------
    t_float fc; // cutoff frequency
    double pi;
    t_float srate;  // sample rate
    //------------------------------
    double wc;
    double wc2;
    double wc3;
    double wc4;
    double k;
    double k2;
    double k3;
    double k4;
    double sqrt2;
    double sq_tmp1;
    double sq_tmp2;
    double a_tmp;
    double b1;
    double b2;
    double b3;
    double b4;
    //------------------------------
    struct filter lp;
    struct filter hp;

} t_pqcrossover_tilde;  

static double pqcrossover_tilde_lp(t_pqcrossover_tilde *x, t_sample in)
{
    x->lp.tempx=in;

    x->lp.tempy=x->lp.a0*x->lp.tempx+x->lp.a1*x->lp.xm1+x->lp.a2*x->lp.xm2+x->lp.a3*x->lp.xm3+x->lp.a4*x->lp.xm4-x->b1*x->lp.ym1-x->b2*x->lp.ym2-x->b3*x->lp.ym3-x->b4*x->lp.ym4;
    x->lp.xm4=x->lp.xm3;
    x->lp.xm3=x->lp.xm2;
    x->lp.xm2=x->lp.xm1;
    x->lp.xm1=x->lp.tempx;
    x->lp.ym4=x->lp.ym3;
    x->lp.ym3=x->lp.ym2;
    x->lp.ym2=x->lp.ym1;
    x->lp.ym1=x->lp.tempy;

    return (x->lp.tempy);
    
}

static double pqcrossover_tilde_hp(t_pqcrossover_tilde *x, t_sample in)
{
    x->hp.tempx=in;

    x->hp.tempy=x->hp.a0*x->hp.tempx+x->hp.a1*x->hp.xm1+x->hp.a2*x->hp.xm2+x->hp.a3*x->hp.xm3+x->hp.a4*x->hp.xm4-x->b1*x->hp.ym1-x->b2*x->hp.ym2-x->b3*x->hp.ym3-x->b4*x->hp.ym4;
    x->hp.xm4=x->hp.xm3;
    x->hp.xm3=x->hp.xm2;
    x->hp.xm2=x->hp.xm1;
    x->hp.xm1=x->hp.tempx;
    x->hp.ym4=x->hp.ym3;
    x->hp.ym3=x->hp.ym2;
    x->hp.ym2=x->hp.ym1;
    x->hp.ym1=x->hp.tempy;

    return (x->hp.tempy);
    
}

static void pqcrossover_tilde_setup_filter(t_pqcrossover_tilde *x)
{
    //------------------------------
    x->wc=2*x->pi*x->fc;
    x->wc2=x->wc*x->wc;
    x->wc3=x->wc2*x->wc;
    x->wc4=x->wc2*x->wc2;
    x->k=x->wc/tan(x->pi*x->fc/x->srate);
    x->k2=x->k*x->k;
    x->k3=x->k2*x->k;
    x->k4=x->k2*x->k2;
    x->sqrt2=sqrt(2);
    x->sq_tmp1=x->sqrt2*x->wc3*x->k;
    x->sq_tmp2=x->sqrt2*x->wc*x->k3;
    x->a_tmp=4*x->wc2*x->k2+2*x->sq_tmp1+x->k4+2*x->sq_tmp2+x->wc4;

    x->b1=(4*(x->wc4+x->sq_tmp1-x->k4-x->sq_tmp2))/x->a_tmp;
    x->b2=(6*x->wc4-8*x->wc2*x->k2+6*x->k4)/x->a_tmp;
    x->b3=(4*(x->wc4-x->sq_tmp1+x->sq_tmp2-x->k4))/x->a_tmp;
    x->b4=(x->k4-2*x->sq_tmp1+x->wc4-2*x->sq_tmp2+4*x->wc2*x->k2)/x->a_tmp;   
    
    //================================================
    // low-pass
    //================================================
    x->lp.a0=x->wc4/x->a_tmp;
    x->lp.a1=4*x->wc4/x->a_tmp;
    x->lp.a2=6*x->wc4/x->a_tmp;
    x->lp.a3=x->lp.a1;
    x->lp.a4=x->lp.a0;
    //=====================================================
    // high-pass
    //=====================================================
    x->hp.a0=x->k4/x->a_tmp;
    x->hp.a1=-4*x->k4/x->a_tmp;
    x->hp.a2=6*x->k4/x->a_tmp;
    x->hp.a3=x->hp.a1;
    x->hp.a4=x->hp.a0;    
    //------------------------------
}

static void pqcrossover_tilde_setcrossf(t_pqcrossover_tilde *x, t_float freq)
{
    if (freq < 1) return;
    if (freq != x->fc) x->fc = freq;
    pqcrossover_tilde_setup_filter(x);  
}

static t_int *pqcrossover_tilde_perform(t_int *w)
{
    t_pqcrossover_tilde *x = (t_pqcrossover_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    t_sample *out1 = (t_sample *)(w[3]);
    t_sample *out2 = (t_sample *)(w[4]);
    int n = (int)w[5];
    int i;
    t_sample sample;
    for (i = 0; i < n; i++)
    {
        sample = in[i];
        out1[i] = (t_sample)pqcrossover_tilde_lp(x, sample);
        out2[i] = (t_sample)pqcrossover_tilde_hp(x, sample);
    }    
    return (w+6);
}

static void pqcrossover_tilde_dsp(t_pqcrossover_tilde *x, t_signal **sp)
{
    x->srate = sp[0]->s_sr;
    pqcrossover_tilde_setup_filter(x);
    dsp_add(pqcrossover_tilde_perform, 5,
        x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void pqcrossover_tilde_free(t_pqcrossover_tilde *x)
{

}

static void *pqcrossover_tilde_new(t_floatarg freq)
{
    t_pqcrossover_tilde *x = (t_pqcrossover_tilde *)pd_new(pqcrossover_tilde_class);
    x->pi = 3.14159265358979323846;
    x->fc = freq == 0 ? 1000: freq;
    x->x_outlp=outlet_new(&x->x_obj, &s_signal);
    x->x_outhp=outlet_new(&x->x_obj, &s_signal);

    return (void *)x;
}

void pqcrossover_tilde_setup(void) 
{
    logpost(NULL,2,"---");
    logpost(NULL,2,"  pqcrossover~ v0.1.0");
    logpost(NULL,2,"---");
    pqcrossover_tilde_class = class_new(gensym("pqcrossover~"),
            (t_newmethod)pqcrossover_tilde_new,
            (t_method)pqcrossover_tilde_free, sizeof(t_pqcrossover_tilde),
            0, A_DEFFLOAT, 0);

    class_addmethod(pqcrossover_tilde_class,
            (t_method)pqcrossover_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(pqcrossover_tilde_class, t_pqcrossover_tilde, x_f);
    class_addmethod(pqcrossover_tilde_class, (t_method)pqcrossover_tilde_setcrossf, gensym("freq"), A_FLOAT, 0);
}