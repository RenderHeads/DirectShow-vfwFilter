# DirectShow-vfwFilter
VFW Video Compression Wrapper Filter for DirectShow

##Background
This filter was written to get around what seems to be a bug in the 64-bit implementation  of Microsoft DirectShow.

Usually (in 32-bit apps) when DirectShow enumerates filters it also include legacy audio and video VFW codecs.  For some reason in 64-bit apps it fails to enumerate any of the legacy VFW video codecs but it enumerates the audio codecs just fine.

##Detail
When enumeration video compression filters in the category 'CLSID_VideoCompressorCategory' DirectShow creates wrapper filters for all legacy VFW video codecs using the 'AVI Compressor' filter.  For legacy audio codecs it does the same thing using 'ACM Wrapper'.  In 64-bit mode all legacy video codecs are missing.

![alt tag](Docs/graph.png)

##Solution
This filter allows you to select any 64-bit VFW video compression codec and use it to build a DirectShow graph.
