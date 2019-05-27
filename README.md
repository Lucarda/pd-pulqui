#pd-pulqui

An audio limiter algorithm for soundfiles developed on [Pure-Data](https://github.com/pure-data/pure-data).

It is named "Pulqui" because it sounds the same as "Pull key". It's the [Mapuche](https://en.wikipedia.org/wiki/Mapuche) word for "arrow" and was also used to name a famous Argentine jet aircraft prototype [->](https://en.wikipedia.org/wiki/FMA_I.Ae._27_Pulqui_I).

The algorithm first scans the given audio file and generates a "side-chain" file that is later combined with the original file.

**pulqui** is a Pd patch with a Pd external. The patch is for real-time combination and outputs to file(s), other software (with Jack for example) or to audio hardware via the sound-card. The external is for generating the "side-chain" file.

The Pd patch has 6 stereo channels (can also be mono) so that it can be used as a "Multi-Band Limiter". A file for each cross-over band must be supplied.

##The "side-chain" file.

We copy the original file:

```
     /\
/\  /  \    
  \/    \  /
         \/
```

We make all values positive:

```
     /\  /\
/\/\/  \/  \    
               
```

Then we scan which was the **highest value** that we have for the lap that **starts** whenever the sample exceeds a near zero value and **stops** whenever the sample is smaller than the near zero value. We write the highest value on all that lap:

```
    ___ ___
_ _|   |   |
 | |   |   |
               
```

##Combination.

We playback both files together and we use the "side-chain" values to do the attenuation math on the original file.

As an example the "original file" first waves from 0.5 to -0.5 and then from 1 to -1. 

```
1
       /\
0 /\  /  \    
    \/    \  /
-1         \/

```
We want to "limit" it so that it never exceeds 0.5. 

We use the "side-chain file" to anticipate how much attenuation we need for that exact lap of time. i.e we know how much it will climb as soon as it starts to climb.

```
1     ___ ___
  _ _|   |   |
0  | |   |   |
               
```
We ignore the first two values because they are equal or less than 0.5 but we use the following two because they are grater than 0.5.

Limited output:

```
1
     
0 /\  /\      
    \/  \/    
-1         

```

## Intermodulation Distortion.

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





