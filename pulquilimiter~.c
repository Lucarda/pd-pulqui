/* pulquilimiter~ external for Pure-Data
// Written by Lucas Cordiviola 11-2023
// No warranties
// See License.txt
*/

#include "m_pd.h"

static t_class *pulquilimiter_tilde_class;

typedef struct _pulquilimiter_tilde {
    t_object x_obj;
    t_float x_thresh;
    t_float x_makeup;
    t_float f;
    t_inlet *x_in2;
    t_inlet *x_in3;
    t_inlet *x_in4;
    t_outlet*x_out;
} t_pulquilimiter_tilde;

static t_int *pulquilimiter_tilde_perform(t_int *w)
{
    t_pulquilimiter_tilde *x = (t_pulquilimiter_tilde *)(w[1]);
    t_sample        *in1 =            (t_sample *)(w[2]);
    t_sample        *in2 =            (t_sample *)(w[3]);
    t_sample        *out =            (t_sample *)(w[4]);
    int                n =            (int)(w[5]);
    t_sample f;
    for (int i = 0; i < n; i++)
    {
        if (in2[i] > x->x_thresh)
            f = in1[i]*(x->x_thresh / in2[i]);
        else 
            f = in1[i];
        if (x->x_makeup)
            out[i] = f*((1-x->x_thresh)+0.998);
        else
            out[i] = f;        
    }

    return (w+6);
}

static void pulquilimiter_tilde_dsp(t_pulquilimiter_tilde *x, t_signal **sp)
{
    dsp_add(pulquilimiter_tilde_perform, 5, x,
                    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void pulquilimiter_tilde_free(t_pulquilimiter_tilde *x)
{
    inlet_free(x->x_in2);
    inlet_free(x->x_in3);
    inlet_free(x->x_in4);
    outlet_free(x->x_out);
}

static void *pulquilimiter_tilde_new(t_floatarg thresh, t_floatarg makeup)
{
    t_pulquilimiter_tilde *x = (t_pulquilimiter_tilde *)pd_new(pulquilimiter_tilde_class);

    x->x_thresh = thresh;

    x->x_in2=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->x_in3=floatinlet_new (&x->x_obj, &x->x_thresh);
    x->x_in4=floatinlet_new (&x->x_obj, &x->x_makeup);
    x->x_out=outlet_new(&x->x_obj, &s_signal);

    return (void *)x;
}

void pulquilimiter_tilde_setup(void) 
{
    logpost(NULL,2,"---");
    logpost(NULL,2,"  pulquilimiter~ v0.1.0");
    logpost(NULL,2,"---");
    pulquilimiter_tilde_class = class_new(gensym("pulquilimiter~"),
                (t_newmethod)pulquilimiter_tilde_new,
                0, sizeof(t_pulquilimiter_tilde),
                CLASS_DEFAULT,
                A_DEFFLOAT, A_DEFFLOAT, 0);

    class_addmethod(pulquilimiter_tilde_class,
                (t_method)pulquilimiter_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(pulquilimiter_tilde_class, t_pulquilimiter_tilde, f);
}