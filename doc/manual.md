# Manual 
IggyLabs VCV Modules


## Table
![Table screenshot](img/table_1.png)

### Intro
Table is a wavetable oscillator for user-provided wavetables, like those you might import into a VST wavetable synthesizer. It supports 16 channels of polyphony.

I recommend trying out [WaveEdit](https://synthtech.com/waveedit) by Synthesis Technology; it is a free, open-source wavetable editor for PC/Mac/Linux. If you are an Xfer Serum user, you are also already probably aware of the powerful wavetable editor available. If you're not interested in making your own wavetables, you can also find countless wavetables online. 

Wherever you get your wavetables, be sure to note the length of a frame or cycle. This is necessary information for correctly importing into Table.

### Use
To import a wavetable, right click on the module, and select "Load wavetable." Load in your .wav file, and you're good to go!

### Parameters
Table has three parameters:

1. pos: The position in the wavetable
2. fine: Fine frequency tuning
3. V/oct: Coarse, semitone frequency tuning

### Developer Notes

#### Aliasing
You may experience aliasing with some wavetables, especially at higher frequencies. In future updates of this module, I may implement an option for creating bandlimited tables that stop upper harmonics from folding back down on the spectrum. For more information, see [EarLevel Engineering's series](https://www.earlevel.com/main/2020/01/04/further-thoughts-on-wave-table-oscillators/) on wavetable synthesis optimizations. If you want to avoid aliasing, you may try creating oversampled waveforms or using waveforms with fewer high harmonics.

#### CPU Optimizations

Thanks to SquinkyLabs for their [demo](https://github.com/squinkylabs/Demo) oscillator for suggestions on reducing CPU load.