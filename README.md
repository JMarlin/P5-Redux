P5-Redux
========

A new leap into playing around with P5OS for 2014.

I'd like to expand this section, maybe get some screenshots in here once I get WYG running again, but in the meantime I just wanted to update this to let any OSDev people stumbling in here know that all the working sources are in /P5OSPPB

The P5R* folders are a weird relic of the fact that this started as an embarassingly rudimentary text-mode bootsector CLI thing when I was 15, and I called that P5OS Release 0 (P5OSR0), packaged it up into a tidy .zip and uploaded it to sourceforge. After that, I started working on a fat12 boot floppy and rewriting the same CLI thing in C to be loaded from the filesystem. I called the floppy PBoot and so my first iteration of that that I put up on sourceforge was P5OS- Plus PBoot (P5OSPPB). I did a bunch more work in the year following that I remember got me into protected mode with a pmode fat12 floppy filesystem driver, but I really wanted to get a GUI going (because that's why every teenager wants to write an 'OS', right?) but got scared off by the thought of implementing v86 to support VESA and stopped working on it, that iteration of code sadly being lost to the sands of time. But then, most of a decade later, I re-found my sourceforge page and pulled down those old code samples. I was putting together this github page for resume purposes and decided I might as well include those sources, and that lead to me playing with them some more and developing the code you can find today in P5OSPPB.

So now you know.

The rest of the source tree clutter I really have no excuse for.

Oh, and don't ask about the name. It's completely meaningless. I came up with it when I was 15 for chrsit's sake.
