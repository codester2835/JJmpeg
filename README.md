# JJmpeg
## Description
JJmpeg is a custom C++ library that provies a simplified interface for working with FFmpeg, FFplay, FFprobe, HandBrake, yt-dlp, and much more.
## Binaries
* [FFmpeg, FFplay, FFprobe](https://www.gyan.dev/ffmpeg/builds/ffmpeg-git-full.7z)
* [HandBrake](https://handbrake.fr/downloads2.php)
* [yt-dlp](https://github.com/yt-dlp/yt-dlp)
* [y-cruncher](https://www.numberworld.org/y-cruncher/)
* [python](https://www.python.org/downloads/)
* [duf](https://github.com/muesli/duf)
* [bat](https://github.com/sharkdp/bat)
* [VT-PR](https://github.com/chu23465/VT-PR)

## Features
JJmpeg has many features, none of which I claim as my own. I have simply wrapped existing tools and libraries to provide a simplified interface for users. I do __not__ claim responsibility for any file or system damage.  Similiarly, I do __not__ claim responsibility for creating any DRM bypassing tools.  I __do **not**__ promote piracy in any way shape or form.
* Video and audio encoding and decoding using FFmpeg
* Video and audio transcoding using HandBrake
* Video downloading using yt-dlp
* Video and audio analysis using FFprobe

* For more options, run:
```
jjmpeg help
```
* <u>or</u> visit the [advanced help] (Advanced_Help.txt) file

## Installation
__Insure you are running in administrator command prompt.__
1. Clone the repository:
```
git clone https://github.com/codester2835/JJmpeg.git
```
2. Navigate to the project directory:
```
cd JJmpeg\bin
```
3. Add the `bin` directory to your system PATH.
```
for /f "usebackq tokens=2*" %A in (`reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path`) do reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path /t REG_EXPAND_SZ /d "%B;%CD%" /f
```


__Or__
> Download the installer file from the [Releases](https://github.com/codester2835/JJmpeg/releases) page and run it.

## Disclaimer
By using this project you agree that:
`The developer shall not be held responsible for any account suspensions, terminations, penalties or legal action taken/imposed by third-party platforms. The User acknowledges and agrees that they are solely responsible for complying with all terms, policies, copyright and guidelines of any such platforms.`

## Usage
Once installed, you can use JJmpeg from the command line. Run the followning command for options:
```
jjmpeg help
```	
For a full list of commands, run:
```
jjmpeg help_advanced <insert path here>
```
## How to Contribute

If you want to contribute to this project, follow the steps below:

1. Fork this repository.
2. Create a branch: `git checkout -b <branch_name>`.
3. Make your changes and confirm them: `git commit -m '<commit_message>'`
4. Send to the original branch: `git push origin <project_name> / <location>`
5. Create the pull request.

Alternatively, consult the GitHub documentation on [how to create a pull request](https://help.github.com/en/github/collaborating-with-issues-and-pull-requests/creating-a-pull-request).


## Licence
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Legal
I (the creator) do __NOT__ promote piracy, or theft in any way.  This project is to be used **ONLY** for archival\personal uses.