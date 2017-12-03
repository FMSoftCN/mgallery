# NAME

    mGallery - mGallery is a PMP application platform which is based on the MiniGUI graphical user interface. 


## TABLE OF CONTENTS

1. Introduction
2. Preparations before movement 
3. About the authors
4. If you have a problem
5. Copying


## INTRODUCTION
    
mGallery is a PMP application platform which is based on the MiniGUI graphical
user interface. It has complete functions and simple operations.
It aims to provide a fast,lightweight,complete Windows systems and helps 
the manufacturer and the designer to construct the PMP solution more conveniently. 
    
mGallery has provided many applications, for example: audio, video, picture,
recorder,  FM broadcast, electricity book, tools (calculator, calendar, stopwatch),
system setup, games (Russia, worm, housekeeper). 
    
mGallery provides many public modules interface such as menu, resource management, 
caption bar and has the biggest flexibility.

Based on MiniGUI graphics library. mGallery can be easily ported to many operating 
system which MiniGUI supports. What's more, it is fast and lightweight.
    

## Preparations before movement 

### Install the engine     
    
If you want to use our native fbcon engine, there is no need to install
a specific library. The native fbcon engine is built in MiniGUI.

If you want to use QVFB as your graphics engine, please download 
the qvfb-1.0.tar.gz from our HTTP site:
    
    http://www.minigui.com/download/index.shtml
    
### Install the lib of MINIGUI as MINIGUI-Standalone.

If you any qusetion about installing the lib of MINIGUI. More information,
please browse our home page:

    http://www.minigui.com

### Configure and compile mGallery

mGallery uses `automake' and `autoconf', so configuration and compilation
of mGallery are very easy:
    
1. Use `tar' to extract `mGallery-1.0.tar.gz' to a new directory:

    $ tar zxvf mGallery-1.0.tar.gz

2. Change to the new directory and run `./autogen.sh':

    $ ./autogen.sh

3. Change to the new directory and run `./configure':

    $ ./configure

4. Run the following commands to compile mGallery:

    $ make;
       
5. Default all material files are stored in '/media' dirctory. Before 
run mgallery, you should create '/media' directory and its 
subdirectories: music, video, picture, recoder, ebook, cfg.

    $ mkdir /media
    $ mkdir /media/music
    $ mkdir /media/video
    $ mkdir /media/picture
    $ mkdir /media/ebook
    $ mkdir /media/recorder
    $ mkdir /media/cfg
     
Alternately you can define RES_TOP_DIR MACRO to set custom path. 

6. Run mGallery 
            
    $ qvfb
    $ cd /src
    $ ./startpmp

### Operation guide

There are some keys to help to operate it.

    Up: UP
    Down: "DOWN"
    Left: "LEFT"
    Right: "RIGHT"
    Exit: "ESC"
    MENU: "F1"


## ABOUT THE AUTHORS

The author of mGallery is the group of solution. More information,
please browse our home page:

    http://www.minigui.com


## IF YOU HAVE A PROBLEM

If you have any technical problem, advice or comment, please send 
messages to solution@minigui.com.

If you are interested in the commercial MiniGUI products and licensing, 
please write to sales@minigui.com.  Other mail can be sent to 
info@minigui.com.


## COPYING

    Copyright (C ) 2007-2017 Beijing FMSoft Technologies Co., Ltd.
  
