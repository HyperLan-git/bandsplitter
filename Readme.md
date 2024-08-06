# BandSplitter
Multiband processing that is actually adaptable and doesn't limit your options. Takes in a mono or stereo input and splits it into as many channels are needed so that you can individually process bands using any other plugin you desire. To limit intersection of bands you will be able to increase the order of the splitting filters, this way you don't have to worry about affecting neighbouring frequencies. If you can't figure out how to mix back the bands together or how to process each channel, shoot me a message.

## Dependencies
On windows, you just need Visual Studio 2022 and Juce.
In addition to juce's main dependencies on linux, you'll need libasan to compile in debug mode.

## How to compile
On windows, simply open the project from projucer and build the solution you need.

On linux, use the makefile :
```sh
make
```

To compile in Release mode (with optimisations and no memory sanitizer), use `make CONFIG=Release`.
You can clean binaries with `make clean`.
