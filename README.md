# pd-pulqui #

An audio limiter algorithm developed on [Pure-Data](https://github.com/pure-data/pure-data).

It is named "Pulqui" because it sounds the same as "Pull key". It's the [Mapuche](https://en.wikipedia.org/wiki/Mapuche) word for "arrow" and was also used to name an Argentine jet aircraft prototype [->](https://en.wikipedia.org/wiki/FMA_I.Ae._27_Pulqui_I).


The algorithm works scanning audio files or live signal. In both cases the scan generates a "side-chain" signal that is used to manipulate the original signal. The live signal process takes at least a theoretical latency of a little more than 25ms (half of a 20hz period).



## The "side-chain" signal. ##

From a given signal:

```
     /\
/\  /  \    
  \/    \  /
         \/
```

We scan which was the **highest value** that we have for the lap that **starts** whenever the sample exceeds a zero value and **stops** whenever the sample is very near a zero value. We write the highest value on all that lap. We do the same for the negative laps but we invert the lowest value :

```
    ___ ___
_ _      

               
```

## Combination. ##

We use the "side-chain" values to do the attenuation math on the original signal.

The "original signal" first waves from 0.5 to -0.5 and then from 1 to -1. We want to "limit" it so that it never exceeds 0.5. 

We use the "side-chain signal" to anticipate how much attenuation we need for that exact lap of time. i.e we know how much to attenuate as soon as the lap starts.

```
1     ___ ___
  _ _
0

-1
               
```
We ignore the first two values because they are equal or less than 0.5 but we use the following two because they are grater than 0.5.

Limited output:

```
1
     
0 /\  /\      
    \/  \/    
-1         

```

## Intermodulation Distortion. ##

Intermodulation distortion may be introduced with this limiter.

For example in an edge case:

In a single vacuum tube triode circuit only one side of the signal enters the saturation area (this is the intermodulation distortion produced by the triode) so the distorted signal, along with other things, waves less on the positive side than on the negative one. 
 
```
1
       
0_._ _._ _  
    .   .
-1  

```
If we limit such a signal with this limiter it starts limiting only the negative side (this is the intermodulation distortion produced by this limiter). The more we limit it at some point both sides are equal in amplitude.


----------------------------------

## Installation. ##



The easiest way is to open Pd (you must have Pd installed to use pulqui) and go to the menu **help/find externals** and type "pulqui" on the search bar. Hopefully your search results on a high-lighted pulqui version that matches your Pd and OS architecture. Click on it and the automatic installation will start.

Alternatively you can find the binaries for your Pd/OS architecture in the [release](https://github.com/Lucarda/pd-pulqui/releases) section. Download and extract the one you need and place it on your Pd's externals folder.


## Compiling ##

Download and extract the sources. Then it should be straight-forward (using Linux GCC, macOS Xcode or Windows MinGW) with:

```
cd <path/to/the/pulqui-sources>
make install
```

If you need to specify Pd's location and an output dir do:

```
make install PDDIR=<path/to/your/pd> PDLIBDIR=<path/you/want/the/output>
```

## Repository ##

https://github.com/Lucarda/pd-pulqui

