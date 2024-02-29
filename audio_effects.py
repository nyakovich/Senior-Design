'''
Title: Audio Effects Code
Author: Nicholas Yakovich
Description: Applies reverb, echo, and distortion effects to wav files
Inputs: Wav files
Outputs: Wav files with all combination of effects applied

Program uses pysndfx library along with SoX for audio processing
Program uses librosa and sounfile for wav file reading and writing
'''

import librosa as lr
import soundfile as sf
from pysndfx.dsp import AudioEffectsChain

# Define echo function
echo_fx = (
    AudioEffectsChain()
    .custom('echo 0.8 0.9 1000 0.3')
)

# Define reverb function
reverb_fx = (
    AudioEffectsChain()
    .reverb(reverberance=100, wet_gain = 10)
)

# Create distortion function
distortion_fx = (
    AudioEffectsChain()
    .overdrive(gain = 10, colour = 20)
    .lowshelf()
    .highpass(frequency = 500)
)

# Read in samplerate and load file with same samplerate
d, samplerate = sf.read("test.wav")
infile = lr.load("test.wav", sr = samplerate)

# Apply three effects individually
echo = echo_fx(infile[0])
reverb = reverb_fx(infile[0])
distortion = distortion_fx(infile[0])

# Write to wav file
sf.write('output_e.wav', echo, samplerate)
sf.write('output_r.wav', reverb, samplerate)
sf.write('output_d.wav', distortion, samplerate)

# Apply all combinations of effects
echo_reverb = echo_fx(reverb)
echo_distortion = echo_fx(distortion)
echo_reverb_distortion = distortion_fx(echo_reverb)
reverb_distortion = reverb_fx(distortion)

# Write to wav file
sf.write('output_e_r.wav', echo_reverb, samplerate)
sf.write('output_e_d.wav', echo_distortion, samplerate)
sf.write('output_e_r_d.wav', echo_reverb_distortion, samplerate)
sf.write('output_r_d.wav', reverb_distortion, samplerate)