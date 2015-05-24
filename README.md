# DirectShow-vfwFilter
VFW Video Compression Wrapper Filter for DirectShow

##Background
This filter was written to get around what looks like a bug in Microsoft DirectShow in 64-bit mode.

Usually (in 32-bit apps) when DirectShow enumerates filters it also include legacy audio and video VFW codecs.  For some reason in 64-bit apps it fails to enumerate any of the legacy VFW video codecs but it enumerates the audio codecs just fine.


.........................
CLSID_VideoCompressorCategory 

It uses the filter 'AVI Compressor' to wrap the functions of the VFW codesc.
