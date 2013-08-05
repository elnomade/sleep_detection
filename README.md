Sleep Detection by Globant Labs
===============================

**Sleep Detection** is a proof of concept application for Windows that will detect when a driver is falling asleep and alert him or her accordingly.

We keep a project backlog in our [Trello board](https://trello.com/board/sleep-detection/51cd7c144853d97c230013c2) in case you feel like watching what we are up to.

====================
**Compilation prerequisites:**

This project is developed under Visual Studio 2012, but should be easily compiled with minimal configuration changes.

**OpenCV:**
  1. Download [OpenCV](http://opencv.org/) for Windows.
  2. Extract it to directory: *`<clone dir>`\3rdParty\opencv\246*, replace the version number with yours.
  3. Ready!

**Boost:**
  1. Download [Boost](http://www.boost.org/) boost source code.
  2. Extract it to *`<clone dir>`\3rdParty\boost\boost_1_54_0*, replace the version number with yours.
  3. Compile boost, optionally you can use 3rdParty's `build.bat`.
  4. Ready!

====================
**Run within Visual Studio:**

1. Go to SleepDetection project properties.
2. Edit Debugging value Environment to, for example, *PATH=$(OPENCV_DIR)\x86\vc11\bin;%PATH%*.
3. Run it!

====================
**Run without Visual Studio:**

1. Download and install [Visual C++ Redist](http://lmgtfy.com/?q=visual+c%2B%2B+redist).
2. Make sure you have the needed OpenCV dll and xml files in the application's directory.
3. Run it!
