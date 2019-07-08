# lighthouse2tools
General tools for working with / figuring out the LH2 (index) technology stack

# Reading

- https://github.com/cnlohr/esptracker/issues/1 - Has a ton of information from one of Charles livestreams going over the new LH tech stack
- https://discord.gg/WYYJyc2 - Discord with a lot of information on steam lighthouses in general
- https://github.com/cnlohr/libsurvive - Libsurvive, which eventually will be the recepient of the tools / information from here
- https://gist.github.com/jdavidberger/324b13bc575de7b2a3a29e69b1440419 Data dump from libsurvive with a 2018 tracker

# Current Understanding

In contrast to LH1, LH2 has only one motor in it; but with two mirrors(?) setup in such a way to project a laser out of the rotor of the motor on a plane 45 degree(?) offset from the rotation. These two planes are also 90 degree offset from eachother. The rotation of the motor varies depending on mode, but is approximately 50hz. 

The basic idea is that if you know when each laser hits you relative to when the lasers are at '0', this makes a line from the lighthouse to a sensor, and with multiple sensors, you can do structure for motion and recover pose in the same way you could with the two motor design of LH1. This is somewhat more complicated than before, but doesn't change much of the fundamental math. 

## Time at sweep

An additional caveat is that LH2 also did away with the sync pulse which was instrumental in LH1 for figuring out the timing of when the laser passes over a sensor -- the sync pulse essentially told you the axis was passing by 0, and gave a start time to compare to when the laser swept the sensor. This is a more impactful implementation detail change than the single rotor.

The mechanism in play now is that the new lighthouse sensors have an envelop and a data pin, and the sweep laser is being modulated as it passes by the sensor. The sensor is able to read anywhere from 10 to 30 bits of this signal while the laser is passing by. The signal that the laser is sending out is, ultimately, the output of a 17 bit [Linear feedback shift register](https://en.wikipedia.org/wiki/Linear-feedback_shift_register). The polynomials used for this all have the maximal period for their size -- 131,071. The signal is at 6mhz, once demodulated, and so the entire period takes 21.85 milliseconds; just over the ~20 milliseconds per rotation. 

If you can capture 32 bits from this stream, you can solve for the shift register polynomial. In practice, there is noise and the window is short so it seems like you end up making do with roughly 20 bits or so. Given a discrete set of polynomial, you can use anything over 17 bits to calculate an error for each given polynomial and figure out which one was used and how 'far' away it is from some known initial value (currently unknown; but 1 or 0x1ffff would make some sense). 

Since you have envelop data for each sensor which got swept, which have start times, you only need to solve the LFSR problem for a single sensor, and then you can work forward and backward from there. 

Based on experimentation and data available from ligthouse_console and the TTY; the polynomials used in the internals of the lighthouse match this [impelmentation](https://github.com/jdavidberger/lighthouse2tools/blob/master/src/lfsr.cc#L3)

## OOTX 

In LH1, the sync pulse also carried a bit of data with it called the 'OOTX'. This gave the sensor device information on the lighthouse such as its serial number and calibration data. That still happens, except since there is no sync pulse this information is conveyed in which LFSR polynomial is used. There are 16 modes for LH2, each has a slightly different rotation frequency and two distinct polynomials used. Since you can recover which polynomial was used for a given pass, one polynomial is interpreted as a 0 and the other as 1. The OOTX packet is at most 128 bytes long. In theory the bitrate is 100bps -- assuming you can switch the used polynomial once per laser plane -- or 50bps if you cant. 

## Multiple lighthouses

As far as I know, there is no syncronization between lighthouses which is why each lighthouse mode has a different RPM. Sweeps are relatively rare -- a pulse could last roughly 50us each 10ms from one LH to one sensor. Even just randomly rotating, collisions would be very rare and since the rotation speed is slightly different they wouldn't happen in the same location each rotation.  

## USB protocol

### Report ID: 39
The USB protocol for a plugged in device still receives the same envelop record type 37; but there is also a new record type 39. So far this packet is decoded to look like this:

```C++
struct lh2_entry {
	uint8_t code; // sensor with some bit flag at 0x80. Continuation flag?
	uint32_t time; // 48mhz timecode
	uint8_t data[8]; // modulated signal? 
};
struct lh2_packet {
        lh2_entry entries[4];
        uint8_t some_bit_flag;
        uint8_t extra[5]; // Always '00 aa 0d e4 0d'??
};
```

The total size is 58 bytes since the first byte is the report type id. Note that HIDAPI will tell you it's 64 bytes but libusb reports 59; unclear what the deal with HID is. 

`some_bit_flag` changes somewhat often, and looks more like a bit flag than a counter or something. 

How to turn that modulated signal into the bitstream is currently unclear. Based on how the lighthouse_console tool works, it seems like those 64 bits is a sampling of the data line at 12mhz; and through some process turns into the 6mhz signal. There are a lot of current hints about differential manchester encoding but the signal is somewhat noisy and so this remains the missing chunk of reading in USB data. 

### Report ID: 40

The devices themselves, presumably for bandwidth reasons, are capable of deciphering the bistream themselves and giving raw timing data. You must send a configuration sequence to the device to get this report type. **TODO**: Figure out what that sequence is.

Report ID gives a heavily packed structure which reports both virtual syncs and collisions with the laser per sensor. See code to read it [here](https://github.com/cnlohr/libsurvive/blob/269d446659e81599dd092ff1baa17b2f6eb80004/src/driver_vive.c#L2039). 

The first byte gives the length of the packet, excluding itself. 

Subsequent bytes are read as a bitflag for the first 4 bits. `0bXXXX XFSC`. 

If the `C` bit is set, the data is a channel marker. The second octet gives the channel number. In this case; it's unclear if the remaining 3 bits are used for anything.

If the `S` bit is set, we are seeing a sensor hit. Read 4 bytes from the stream as such: `0bSSSS STTT TTTT TTTT TTTT TTTT TTTT TFSC`. The `T` section here is a 24bit timecode; you can recover the full timecode by using the most recent IMU timecode. The `S` portion denotes which sensor we are seeing. 

If the `S` bit is not set, we are seeing a sync event. Given my understanding, this event is mostly a made up thing just to give context to the timestamps after it -- it gives the starting point of the rotation. Read the 4 bytes from the stream as: `0bXXXX GOTT TTTT TTTT TTTT TTTT TTTT TTSC`. The `T` section is the 24bit timecode value. `O` is the ootx bit and `G` is something referred to as a generational value; presumably to track when channels are rediscovered(?). It's unclear if the `XXXX` bits refer to anything or are used for future proofing. 

It seems like the virtual syncs happen once per rotation; so you tend to get 2 hits per sensor per rotation -- one in the first half of the rotation and the other in the second half. This would imply the OOTX changes only once per rotation.

# Open questions

- How to demodulate the USB data into something useful for LFSR?
- How best to construct a reprojection model for the X sweep pattern?
- How to use the new calibration parameters?
- What 'initial value' is in play for each LFSR polynomial? 
- What are the miscellaneous bits in the USB packet for? 
- How do you enable report type 40? 

# Useful external tools

- If you plug into a lighthouses usb port, you get a TTY into it -- standard 115200 baud. It doesn't have a help command, but it has tab completion so you can hit tab a few times to see all commands. This lets you set the mode, and look at system paramters. 
- lighthouse_console.exe is provided as part of steam, and with 'sample'/'dump' commands it will spit out the demodulated bistream per sensor with envelop data. 'find_best_poly' uses the output format from this tool. 
