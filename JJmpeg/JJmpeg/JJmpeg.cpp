#include <windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <cctype>
#include <map>
#include <Shellapi.h>
#pragma comment(lib, "Shell32.lib")

namespace fs = std::filesystem;

const std::string JJMPG_VERSION = "2025.11.10";

std::string RunCommand3(const std::string& command) {
    std::string result;

    SECURITY_ATTRIBUTES saAttr{};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE hStdOutRead = NULL;
    HANDLE hStdOutWrite = NULL;
    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &saAttr, 0)) {
        std::cerr << "CreatePipe failed\n";
        return "";
    }
    SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);
    PROCESS_INFORMATION piProcInfo{};
    STARTUPINFOA siStartInfo{};
    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdError = hStdOutWrite;
    siStartInfo.hStdOutput = hStdOutWrite;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    std::string cmdLine = command;
    if (!cmdLine.empty() && cmdLine.front() != '"') {
        size_t spacePos = cmdLine.find(' ');
        if (spacePos != std::string::npos) {
            std::string exePart = cmdLine.substr(0, spacePos);
            if (exePart.find(' ') != std::string::npos) {
                std::string rest = cmdLine.substr(spacePos);
                cmdLine = "\"" + exePart + "\"" + rest;
            }
        }
    }

    char* cmdLineMutable = cmdLine.data();

    BOOL bSuccess = CreateProcessA(
        NULL,
        cmdLineMutable,
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &siStartInfo,
        &piProcInfo
    );

    CloseHandle(hStdOutWrite);

    if (!bSuccess) {
        DWORD err = GetLastError();
        std::cerr << "CreateProcess failed: \"" << cmdLine << "\" (error " << err << ")\n";
        CloseHandle(hStdOutRead);
        return "";
    }

    const DWORD bufSize = 4096;
    char buffer[bufSize];
    DWORD bytesRead;

    while (ReadFile(hStdOutRead, buffer, bufSize - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::string output = buffer;
        size_t pos = 0;
        while ((pos = output.find("ffmpeg", pos)) != std::string::npos) {
            output.replace(pos, 6, "ffmpeg");
            pos += 6;
        }

        std::cout << output;
        result += output;
    }

    WaitForSingleObject(piProcInfo.hProcess, INFINITE);

    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    CloseHandle(hStdOutRead);

    return result;
}

std::string RunCommand(const std::string& command) {
    std::string result;

    SECURITY_ATTRIBUTES saAttr{};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    HANDLE hStdOutRead = NULL;
    HANDLE hStdOutWrite = NULL;
    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &saAttr, 0)) return "";

    SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);

    PROCESS_INFORMATION piProcInfo{};
    STARTUPINFOA siStartInfo{};
    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdError = hStdOutWrite;
    siStartInfo.hStdOutput = hStdOutWrite;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    BOOL bSuccess = CreateProcessA(
        NULL,
        const_cast<char*>(command.c_str()),
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &siStartInfo,
        &piProcInfo
    );

    CloseHandle(hStdOutWrite);

    if (!bSuccess) return "";

    const DWORD bufSize = 4096;
    char buffer[bufSize];
    DWORD bytesRead;
    while (ReadFile(hStdOutRead, buffer, bufSize - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::string output = buffer;
        size_t pos = 0;
        while ((pos = output.find("ffmpeg", pos)) != std::string::npos) {
            output.replace(pos, 6, "ffmpeg");
            pos += 6;
        }

        std::cout << output;
        result += output;
    }

    WaitForSingleObject(piProcInfo.hProcess, INFINITE);

    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    CloseHandle(hStdOutRead);

    return result;
}

std::string RunCommand2(const std::string& exe, const std::string& args) {
    std::string result;

    SECURITY_ATTRIBUTES saAttr{};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    HANDLE hStdOutRead = NULL;
    HANDLE hStdOutWrite = NULL;
    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &saAttr, 0)) return "";

    SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);

    PROCESS_INFORMATION piProcInfo{};
    STARTUPINFOA siStartInfo{};
    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdError = hStdOutWrite;
    siStartInfo.hStdOutput = hStdOutWrite;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    std::string cmdLine = "\"" + exe + "\" " + args;

    BOOL bSuccess = CreateProcessA(
        exe.c_str(),
        cmdLine.data(),
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &siStartInfo,
        &piProcInfo
    );

    CloseHandle(hStdOutWrite);

    if (!bSuccess) {
        DWORD err = GetLastError();
        std::cerr << "CreateProcess failed for " << exe << " (error " << err << ")\n";
        CloseHandle(hStdOutRead);
        return "";
    }

    const DWORD bufSize = 4096;
    char buffer[bufSize];
    DWORD bytesRead;
    while (ReadFile(hStdOutRead, buffer, bufSize - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        result += buffer;
    }

    WaitForSingleObject(piProcInfo.hProcess, INFINITE);

    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    CloseHandle(hStdOutRead);

    return result;
}

std::string GetParentDir() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string fullPath(path);
    size_t pos = fullPath.find_last_of("\\/");
    if (pos == std::string::npos) return "";
    std::string binDir = fullPath.substr(0, pos);
    size_t pos2 = binDir.find_last_of("\\/");
    if (pos2 == std::string::npos) return "";
    return binDir.substr(0, pos2);
}
void DVDRip(const std::string& dvdDrive, const std::string& customOutput = "") {
    std::string outputFile;
    if (!customOutput.empty()) {
        outputFile = customOutput;
    }
    else {
        char volumeName[MAX_PATH + 1] = { 0 };
        if (GetVolumeInformationA(dvdDrive.c_str(), volumeName, sizeof(volumeName),
            NULL, NULL, NULL, NULL, 0)) {
            outputFile = std::string(volumeName);
        }
        else {
            outputFile = "Unknown_Disc";
        }
        for (auto& c : outputFile) {
            if (!isalnum(static_cast<unsigned char>(c)) && c != '-' && c != '_')
                c = '_';
        }

        outputFile += ".mp4";
    }

    std::string baseDir = GetParentDir();
    std::string exePath = baseDir + "\\dvd-rip.exe";
    std::string command = "\"" + exePath + "\" -i " + dvdDrive + " -o " + outputFile;
    std::cout << "Starting DVD rip..." << std::endl;
    std::cout << "Running command:\n" << command << std::endl;
    int result = system(command.c_str());

    if (result == 0) {
        std::cout << "\nDVD rip finished successfully." << std::endl;
        std::cout << "Saved as: " << outputFile << std::endl;
    }
    else {
        std::cerr << "\nDVD rip failed (exit code " << result << ")." << std::endl;
    }
}

void ConvertFile(const std::string& input, const std::string& output) {
    std::string baseDir = GetParentDir();
    std::string command = baseDir + "\\main.exe -i \"" + input + "\" \"" + output + "\"";
    std::cout << "Converting " << input << " to " << output << "...\n";
    RunCommand(command);
    std::cout << "\nConversion finished.\n";
}

void CombineFiles(const std::string& folder, const std::string& output) {
    std::string listFile = folder + "\\filelist.txt";
    std::ofstream ofs(listFile);
    for (auto& entry : fs::directory_iterator(folder)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".mp4" || ext == ".mkv" || ext == ".mov")
                ofs << "file '" << entry.path().string() << "'\n";
        }
    }
    ofs.close();

    std::string baseDir = GetParentDir();
    std::string command = baseDir + "\\..\\main.exe -f concat -safe 0 -i \"" + listFile + "\" -c copy \"" + output + "\"";
    std::cout << "Combining files in " << folder << " to " << output << "...\n";
    RunCommand(command);
    std::cout << "\nCombine finished.\n";
}

void ShowHelp() {
    std::cout << "=============================\n";
    std::cout << "   JJmpeg - Command Guide\n";
    std::cout << "=============================\n\n";
    std::cout << "Available Commands:\n\n";
    std::cout << "Single File Operations:\n";
    std::cout << "  convert <input> to <output>\n";
    std::cout << "      Convert a file to another format.\n";
    std::cout << "  resize <input> to <width>x<height>\n";
    std::cout << "      Resize a video to the specified dimensions.\n";
    std::cout << "  trim <input> from <start> to <end> to <output>\n";
    std::cout << "      Cut a specific segment from a video or audio file.\n";
    std::cout << "  extract audio from <input> to <output>\n";
    std::cout << "      Extract the audio track without re-encoding.\n";
    std::cout << "  extract-audio <input> <output>\n";
    std::cout << "      Alternate extract command, encodes to MP3 by default.\n";
    std::cout << "  rotate <input> by <degrees> to <output>\n";
    std::cout << "      Rotate a video by 90, 180, or 270 degrees.\n";
    std::cout << "  speed <input> <factor> <output>\n";
    std::cout << "      Change playback speed (e.g., 0.5 = half, 2 = double).\n";
    std::cout << "  volume <input> <multiplier> <output>\n";
    std::cout << "      Adjust audio volume (e.g., 2.0 = double, 0.5 = half).\n";
    std::cout << "  upscale <input> to <resolution/fps> <quality> <output>\n";
    std::cout << "      Upscale a video to higher resolution (e.g., 4K60 high).\n";
    std::cout << "  screenshot <input> <timestamp> <output>\n";
    std::cout << "      Capture a frame at the given timestamp (hh:mm:ss).\n";
    std::cout << "  gif <input> <start> <duration> <output>\n";
    std::cout << "      Convert part of a video into an animated GIF.\n";
    std::cout << "  watermark <input> <image> <output>\n";
    std::cout << "      Add a watermark overlay (default: top-left).\n";
    std::cout << "  replace-audio <video> <audio> <output>\n";
    std::cout << "      Replace video’s original audio track.\n";
    std::cout << "  mix-audio <video> <audio> <output>\n";
    std::cout << "      Mix a new audio track into the video.\n";
    std::cout << "  reverse <input> <output>\n";
    std::cout << "      Reverse video and audio.\n";
    std::cout << "  stabilize <input> <output>\n";
    std::cout << "      Stabilize shaky footage.\n";
    std::cout << "  color <input> <brightness> <contrast> <saturation> <output>\n";
    std::cout << "      Adjust color levels.\n";
    std::cout << "  interpolate <input> <fps> <output>\n";
    std::cout << "      Increase framerate using frame interpolation.\n\n";
    std::cout << "DVD and Downloads:\n";
    std::cout << "  dvd-rip <drive> [to <output>]\n";
    std::cout << "      Rip a DVD to MP4 format using HandBrake.\n";
    std::cout << "  dvd copy <drive> to <folder> [encode]\n";
    std::cout << "      Copy or encode a full DVD to another location.\n";
    std::cout << "  copy <url> to <output>\n";
    std::cout << "      Download videos/audios (YouTube, playlists, etc.).\n\n";
    std::cout << "Utilities:\n";
    std::cout << "  play <file>\n";
    std::cout << "      Play a media file with ffplay.\n";
    std::cout << "  calculate\n";
    std::cout << "      Open Windows Calculator.\n";
    std::cout << "  duf\n";
    std::cout << "      Show disk usage with duf.\n";
    std::cout << "  probe <file>\n";
    std::cout << "      Show detailed information about a media file.\n\n";
    std::cout << "Coding / Scripting:\n";
    std::cout << "  python\n";
    std::cout << "      Run the latest Python version\n";
    std::cout << "  pip install <package>\n";
    std::cout << "      Install a Python package using pip.\n";
    std::cout << "  code view <path>\n";
    std::cout << "      View code file as plain text with syntax highlighting in console.\n";
    std::cout << "  html view <path>\n";
    std::cout << "      Open HTML file in default web browser.\n\n";
    std::cout << "Streaming Service Rip (Requires CMD to be ran as Administrator):\n";
    std::cout << "  vt install\n";
    std::cout << "      Install and build the streaming service ripper (necessary to use).\n";
    std::cout << "  vt <service name> name: <username> pw: <password>\n";
    std::cout << "      Supported services: iTunes, Hotstar, DisneyPlus, ParamountPlus, Sunnxt, and RakutenTV\n";
    std::cout << "  vt poetry run vt dl <command> #<target path>\n";
    std::cout << "      Run any command supported by the streaming service ripper (advanced users).\n";
    std::cout << "      Command examples:\n";
    std::cout << "        poetry run vt dl -q <quality *> -al <audio language **> -sl <OPTIONAL subtitle language> -m (for movie; -w for tv show) Hulu <link>\n";
    std::cout << "        * Quality options depend on the service (e.g., 1080p, 720p, etc.)\n";
    std::cout << "        ** Audio language codes depend on the service (e.g., en, es, fr, etc.)\n";
    std::cout << "      For more options, run: -h or see official github repo: https://github.com/chu23465/VT-PR\n";
    std::cout << "  vt <service name> <cookie path>\n";
    std::cout << "      Use the **Netscape** cookie file exported from the homepage of most streaming services not listed above.\n";
    std::cout << "      This may require you to reset the cookies every few days depending on the service.\n\n";
    std::cout << "Folder / Batch Operations:\n";
    std::cout << "  folder convert <folder> to <ext>\n";
    std::cout << "      Convert all files to given extension.\n";
    std::cout << "  folder play <folder>\n";
    std::cout << "      Play all media files in a folder.\n";
    std::cout << "  folder upscale <folder> <resolution/fps> <quality>\n";
    std::cout << "      Batch upscale all videos.\n";
    std::cout << "  folder extract-audio <folder> <ext>\n";
    std::cout << "      Extract audio from all videos in a folder.\n";
    std::cout << "  folder thumbnail <folder> <pos>\n";
    std::cout << "      Capture thumbnails (pos: mid/start).\n";
    std::cout << "  folder info <folder>\n";
    std::cout << "      Print media info for each file.\n";
    std::cout << "  folder concat <folder> <output>\n";
    std::cout << "      Merge multiple videos.\n";
    std::cout << "  folder subtitles <folder>\n";
    std::cout << "      Extract all subtitle tracks.\n";
    std::cout << "  folder normalize <folder>\n";
    std::cout << "      Normalize audio volume.\n";
    std::cout << "  folder gif <folder>\n";
    std::cout << "      Convert all videos to GIFs.\n";
    std::cout << "  folder probe <folder>\n";
    std::cout << "      Show info for all media files.\n";
    std::cout << "  folder watermark <folder> <image> <pos>\n";
    std::cout << "      Add watermark to all videos.\n";
    std::cout << "  folder replace-audio <folder> <audio>\n";
    std::cout << "      Replace audio in all videos.\n";
    std::cout << "  folder mix-audio <folder> <audio>\n";
    std::cout << "      Mix audio into all videos.\n";
    std::cout << "  folder reverse <folder>\n";
    std::cout << "      Reverse all videos.\n";
    std::cout << "  folder stabilize <folder>\n";
    std::cout << "      Stabilize all videos.\n";
    std::cout << "  folder color <folder> <b> <c> <s>\n";
    std::cout << "      Adjust colors in all videos.\n";
    std::cout << "  folder interpolate <folder> <fps>\n";
    std::cout << "      Apply interpolation to all videos.\n\n";
    std::cout << "Other:\n";
    std::cout << "  help           Show basic help (this menu).\n";
    std::cout << "  help_advanced <path>  Show advanced help and write it to a txt file.\n";
    std::cout << "  --version      Display JJmpeg version.\n\n";
}

void ShowHelpAdvanced(const std::string& outPath) {
    std::ofstream fout(outPath);
    if (!fout.is_open()) {
        std::cerr << "Failed to open file: " << outPath << "\n";
        return;
    }

    fout << "=============================\n";
    fout << "   JJmpeg - Advanced Commands\n";
    fout << "=============================\n\n";
    fout << "This lists all commands, including raw ffmpeg options.\n";
    fout << "Note: Folder and batch commands are identical to basic help.\n";
    fout << "Unknown arguments are forwarded directly to ffmpeg.\n\n";

    std::string baseDir = GetParentDir();

    fout << "FFmpeg Help:\n------------------------------------\n";
    fout << RunCommand2(baseDir + "\\main.exe", "-h full") << "\n";

    fout << "FFplay Help:\n------------------------------------\n";
    fout << RunCommand2(baseDir + "\\play.exe", "-h") << "\n";

    fout << "FFprobe Help:\n------------------------------------\n";
    fout << RunCommand2(baseDir + "\\probe.exe", "-h") << "\n";

    fout << "YT-DLP Help:\n------------------------------------\n";
    fout << RunCommand2(baseDir + "\\yt-dlp.exe", "--help") << "\n";

    fout << "HandBrakeCLI Help:\n------------------------------------\n";
    fout << RunCommand2(baseDir + "\\dvd-rip.exe", "--help") << "\n";

    fout << "DUF Help:\n------------------------------------\n";
    fout << RunCommand2(baseDir + "\\tools\\duf.exe", "--help") << "\n";

    fout << "Aria2c Help:\n------------------------------------\n";
    fout << RunCommand2(baseDir + "\\aria2c.exe", "--help") << "\n";

    fout.close();
    std::cout << "Advanced help written to " << outPath << "\n";
}


void ConvertFolder(const std::string& folder, const std::string& ext) {
    for (auto& entry : fs::directory_iterator(folder)) {
        if (entry.is_regular_file()) {
            std::string input = entry.path().string();
            std::string filename = entry.path().stem().string();
            std::string output = entry.path().parent_path().string() + "\\" + filename + "." + ext;

            std::cout << "Converting " << input << " -> " << output << "\n";
            std::string baseDir = GetParentDir();
            std::string command = baseDir + "\\main.exe -i \"" + input + "\" \"" + output + "\"";
            RunCommand(command);
        }
    }
    std::cout << "\nBatch conversion finished.\n";
}
void FolderUpscale(const std::string& folderPath, const std::string& resolutionFps, const std::string& quality) {
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA((folderPath + "\\*.*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Could not open folder " << folderPath << "\n";
        return;
    }

    std::string res, fps;
    if (resolutionFps == "4K60") { res = "3840:2160"; fps = "60"; }
    else if (resolutionFps == "1080p30") { res = "1920:1080"; fps = "30"; }
    else { std::cerr << "Unknown resolution/fps\n"; return; }

    std::string crf;
    if (quality == "low") crf = "28";
    else if (quality == "medium") crf = "23";
    else if (quality == "high") crf = "18";
    else if (quality == "ultra") crf = "14";
    else { std::cerr << "Unknown quality\n"; return; }

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::string inputFile = folderPath + "\\" + findFileData.cFileName;

            std::string filename = findFileData.cFileName;
            size_t dotPos = filename.find_last_of('.');
            std::string baseName = (dotPos == std::string::npos) ? filename : filename.substr(0, dotPos);

            std::string outputFile = folderPath + "\\" + baseName + "_upscaled.mp4";

            if (GetFileAttributesA(outputFile.c_str()) != INVALID_FILE_ATTRIBUTES) {
                std::cerr << "Error: " << outputFile << " already exists. Skipping.\n";
                continue;
            }
            std::string baseDir = GetParentDir();
            std::string command = baseDir + "\\main.exe -i \"" + inputFile + "\" -vf scale=" + res + " -r " + fps +
                " -c:v libx265 -crf " + crf + " -preset slow -c:a copy \"" + outputFile + "\"";

            std::cout << "Upscaling: " << inputFile << " -> " << outputFile << "\n";
            RunCommand(command);
        }
    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);
    std::cout << "\nBatch upscale finished.\n";
}

void FolderProbe(const std::string& folderPath) {
    for (auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string inputFile = entry.path().string();
            std::cout << "Probing: " << inputFile << "\n";
            std::string baseDir = GetParentDir();
            std::string command = baseDir + "\\probe.exe \"" + inputFile + "\"";
            RunCommand(command);
        }
    }
    std::cout << "\nBatch probe finished.\n";
}
void FolderPlay(const std::string& folderPath) {
    for (auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string inputFile = entry.path().string();
            std::cout << "Playing: " << inputFile << "\n";
            std::string baseDir = GetParentDir();
            RunCommand(baseDir + "\\play.exe \"" + inputFile + "\"");
        }
    }
}

void FolderExtractAudio(const std::string& folderPath, const std::string& ext) {
    for (auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string inputFile = entry.path().string();
            std::string filename = entry.path().stem().string();
            std::string outputFile = folderPath + "\\" + filename + "." + ext;
            std::cout << "Extracting audio: " << inputFile << " -> " << outputFile << "\n";
            std::string baseDir = GetParentDir();
            std::string cmd = baseDir + "\\main.exe -i \"" + inputFile + "\" -vn -acodec copy \"" + outputFile + "\"";
            RunCommand(cmd);
        }
    }
}

void FolderThumbnail(const std::string& folderPath, const std::string& position) {
    for (auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string inputFile = entry.path().string();
            std::string filename = entry.path().stem().string();
            std::string outputFile = folderPath + "\\" + filename + ".jpg";
            std::string seek = (position == "mid") ? "00:00:10" : "00:00:01";
            std::cout << "Creating thumbnail: " << inputFile << " -> " << outputFile << "\n";
            std::string baseDir = GetParentDir();
            std::string cmd = baseDir + "\\main.exe -i \"" + inputFile + "\" -ss " + seek + " -vframes 1 \"" + outputFile + "\"";
            RunCommand(cmd);
        }
    }
}

void FolderInfo(const std::string& folderPath) {
    for (auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string inputFile = entry.path().string();
            std::cout << "Probing: " << inputFile << "\n";
            std::string baseDir = GetParentDir();
            RunCommand(baseDir + "\\probe.exe \"" + inputFile + "\"");
        }
    }
}

void FolderConcat(const std::string& folderPath, const std::string& outputFile) {
    std::string listFile = folderPath + "\\filelist.txt";
    std::ofstream ofs(listFile);
    std::string baseDir = GetParentDir();
    for (auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".mp4" || ext == ".mkv" || ext == ".mov") {
                ofs << "file '" << entry.path().string() << "'\n";
            }
        }
    }
    ofs.close();
    std::string cmd = baseDir + "\\main.exe -f concat -safe 0 -i \"" + listFile + "\" -c copy \"" + outputFile + "\"";
    std::cout << "Concatenating files in " << folderPath << " -> " << outputFile << "\n";
    RunCommand(cmd);
}

void FolderSubtitles(const std::string& folderPath) {
    for (auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string inputFile = entry.path().string();
            std::string filename = entry.path().stem().string();
            std::string outputFile = folderPath + "\\" + filename + ".srt";
            std::cout << "Extracting subtitles: " << inputFile << " -> " << outputFile << "\n";
            std::string baseDir = GetParentDir();
            std::string cmd = baseDir + "\\main.exe -i \"" + inputFile + "\" -map 0:s:0 \"" + outputFile + "\"";
            RunCommand(cmd);
        }
    }
}

void FolderNormalizeAudio(const std::string& folderPath) {
    for (auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string inputFile = entry.path().string();
            std::string filename = entry.path().stem().string();
            std::string outputFile = folderPath + "\\" + filename + "_normalized.mp4";
            std::cout << "Normalizing audio: " << inputFile << " -> " << outputFile << "\n";
            std::string baseDir = GetParentDir();
            std::string cmd = baseDir + "\\main.exe -i \"" + inputFile + "\" -af loudnorm -c:v copy \"" + outputFile + "\"";
            RunCommand(cmd);
        }
    }
}

void Watermark(const std::string& input, const std::string& overlay, const std::string& position, const std::string& output) {
    std::string posFilter;
    if (position == "top-left") posFilter = "10:10";
    else if (position == "top-right") posFilter = "main_w-overlay_w-10:10";
    else if (position == "bottom-left") posFilter = "10:main_h-overlay_h-10";
    else if (position == "bottom-right") posFilter = "main_w-overlay_w-10:main_h-overlay_h-10";
    else if (position == "center") posFilter = "(main_w-overlay_w)/2:(main_h-overlay_h)/2";
    else { std::cerr << "Unknown position\n"; return; }

    std::string baseDir = GetParentDir();
    std::string command = baseDir + "\\main.exe -i \"" + input + "\" -i \"" + overlay + "\" -filter_complex \"overlay=" + posFilter + "\" \"" + output + "\"";
    std::cout << "Applying watermark: " << input << " + " << overlay << " -> " << output << "\n";
    RunCommand(command);
}

void ReplaceAudio(const std::string& video, const std::string& audio, const std::string& output) {
    std::string baseDir = GetParentDir();
    std::string cmd = baseDir + "\\main.exe -i \"" + video + "\" -i \"" + audio + "\" -c:v copy -map 0:v:0 -map 1:a:0 \"" + output + "\"";
    std::cout << "Replacing audio: " << video << " + " << audio << " -> " << output << "\n";
    RunCommand(cmd);
}

void MixAudio(const std::string& video, const std::string& audio, const std::string& output) {
    std::string baseDir = GetParentDir();
    std::string cmd = baseDir + "\\main.exe -i \"" + video + "\" -i \"" + audio + "\" -filter_complex \"[0:a][1:a]amix=inputs=2:duration=first:dropout_transition=2[a]\" -map 0:v -map \"[a]\" -c:v copy \"" + output + "\"";
    std::cout << "Mixing audio: " << video << " + " << audio << " -> " << output << "\n";
    RunCommand(cmd);
}

void SpeedRamp(const std::string& input, const std::string& start, const std::string& end, const std::string& factor, const std::string& output) {
    std::string baseDir = GetParentDir();
    std::string cmd = baseDir + "\\main.exe -i \"" + input + "\" -filter_complex \"[0:v]trim=start=" + start + ":end=" + end + ",setpts=" + factor + "*PTS[v];[0:a]atrim=start=" + start + ":end=" + end + ",atempo=" + factor + "[a]\" -map \"[v]\" -map \"[a]\" \"" + output + "\"";
    std::cout << "Applying speed ramp: " << input << " -> " << output << "\n";
    RunCommand(cmd);
}

void Stabilize(const std::string& input, const std::string& output) {
    std::string baseDir = GetParentDir();
    std::string cmd = baseDir + "\\main.exe -i \"" + input + "\" -vf vidstabdetect=shakiness=5:accuracy=15 -f null - && \\..\\main.exe -i \"" + input + "\" -vf vidstabtransform=smoothing=30:input=\"transforms.trf\" \"" + output + "\"";
    std::cout << "Stabilizing: " << input << " -> " << output << "\n";
    RunCommand(cmd);
}

void AdjustColor(const std::string& input, const std::string& brightness, const std::string& contrast, const std::string& saturation, const std::string& output) {
    std::string filter = "eq=brightness=" + brightness + ":contrast=" + contrast + ":saturation=" + saturation;
    std::string baseDir = GetParentDir();
    std::string cmd = baseDir + "\\main.exe -i \"" + input + "\" -vf \"" + filter + "\" \"" + output + "\"";
    std::cout << "Adjusting color: " << input << " -> " << output << "\n";
    RunCommand(cmd);
}

void ReverseVideo(const std::string& input, const std::string& output) {
    std::string baseDir = GetParentDir();
    std::string cmd = baseDir + "\\main.exe -i \"" + input + "\" -vf reverse -af areverse \"" + output + "\"";
    std::cout << "Reversing video: " << input << " -> " << output << "\n";
    RunCommand(cmd);
}

void FrameInterpolate(const std::string& input, const std::string& fps, const std::string& output) {
    std::string baseDir = GetParentDir();
    std::string cmd = baseDir + "\\main.exe -i \"" + input + "\" -filter_complex \"minterpolate='fps=" + fps + ":mi_mode=mci:mc_mode=aobmc:me_mode=bidir:vsbmc=1'\" \"" + output + "\"";
    std::cout << "Frame interpolation: " << input << " -> " << output << "\n";
    RunCommand(cmd);
}

void FolderWatermark(const std::string& folder, const std::string& overlay, const std::string& position) {
    for (auto& entry : fs::directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;
        std::string input = entry.path().string();
        std::string output = entry.path().parent_path().string() + "\\" + entry.path().stem().string() + "_wm" + entry.path().extension().string();
        Watermark(input, overlay, position, output);
    }
}

void FolderReplaceAudio(const std::string& folder, const std::string& audio) {
    for (auto& entry : fs::directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;
        std::string input = entry.path().string();
        std::string output = entry.path().parent_path().string() + "\\" + entry.path().stem().string() + "_newaudio" + entry.path().extension().string();
        ReplaceAudio(input, audio, output);
    }
}

void FolderMixAudio(const std::string& folder, const std::string& audio) {
    for (auto& entry : fs::directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;
        std::string input = entry.path().string();
        std::string output = entry.path().parent_path().string() + "\\" + entry.path().stem().string() + "_mixaudio" + entry.path().extension().string();
        MixAudio(input, audio, output);
    }
}

void FolderReverse(const std::string& folder) {
    for (auto& entry : fs::directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;
        std::string input = entry.path().string();
        std::string output = entry.path().parent_path().string() + "\\" + entry.path().stem().string() + "_rev" + entry.path().extension().string();
        ReverseVideo(input, output);
    }
}

void FolderStabilize(const std::string& folder) {
    for (auto& entry : fs::directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;
        std::string input = entry.path().string();
        std::string output = entry.path().parent_path().string() + "\\" + entry.path().stem().string() + "_stab" + entry.path().extension().string();
        Stabilize(input, output);
    }
}

void FolderAdjustColor(const std::string& folder, const std::string& brightness, const std::string& contrast, const std::string& saturation) {
    for (auto& entry : fs::directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;
        std::string input = entry.path().string();
        std::string output = entry.path().parent_path().string() + "\\" + entry.path().stem().string() + "_color" + entry.path().extension().string();
        AdjustColor(input, brightness, contrast, saturation, output);
    }
}

void FolderFrameInterpolate(const std::string& folder, const std::string& fps) {
    for (auto& entry : fs::directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;
        std::string input = entry.path().string();
        std::string output = entry.path().parent_path().string() + "\\" + entry.path().stem().string() + "_intp" + entry.path().extension().string();
        FrameInterpolate(input, fps, output);
    }
}

void FolderGif(const std::string& folderPath) {
    for (auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string inputFile = entry.path().string();
            std::string filename = entry.path().stem().string();
            std::string outputFile = folderPath + "\\" + filename + ".gif";
            std::cout << "Converting to GIF: " << inputFile << " -> " << outputFile << "\n";
            std::string baseDir = GetParentDir();
            std::string cmd = baseDir + "\\main.exe -i \"" + inputFile + "\" -vf \"fps=15,scale=640:-1:flags=lanczos\" -loop 0 \"" + outputFile + "\"";
            RunCommand(cmd);
        }
    }
}
void runCommand(const std::string& cmd) {
    std::cout << "[JJMPEG] Running: " << cmd << std::endl;
    int result = std::system(cmd.c_str());
    if (result != 0) {
        std::cerr << "[JJMPEG] Command failed with code " << result << std::endl;
    }
}

void cmdTrim(const std::string& input, const std::string& start, const std::string& duration, const std::string& output) {
    std::string cmd = "ffmpeg -i \"" + input + "\" -ss " + start + " -t " + duration + " -c copy \"" + output + "\"";
    runCommand(cmd);
}

void cmdExtractAudio(const std::string& input, const std::string& output) {
    std::string cmd = "ffmpeg -i \"" + input + "\" -vn -c:a libmp3lame -q:a 2 \"" + output + "\"";
    runCommand(cmd);
}

void cmdScreenshot(const std::string& input, const std::string& time, const std::string& output) {
    std::string cmd = "ffmpeg -ss " + time + " -i \"" + input + "\" -vframes 1 \"" + output + "\"";
    runCommand(cmd);
}

void cmdGif(const std::string& input, const std::string& start, const std::string& duration, const std::string& output) {
    std::string cmd = "ffmpeg -ss " + start + " -t " + duration + " -i \"" + input + "\" -vf \"fps=10,scale=640:-1:flags=lanczos\" -gifflags +transdiff \"" + output + "\"";
    runCommand(cmd);
}

void cmdVolume(const std::string& input, const std::string& factor, const std::string& output) {
    std::string cmd = "ffmpeg -i \"" + input + "\" -filter:a \"volume=" + factor + "\" \"" + output + "\"";
    runCommand(cmd);
}
void JJMPegDVDCopy(const std::string& sourceDrive, const std::string& destPath, bool encode = true, bool useCSS = true) {
    fs::path dest(destPath);

    if (!fs::exists(dest)) fs::create_directories(dest);

    std::string volumeName = "DVD_Rip";

    char volName[MAX_PATH + 1] = { 0 };
    if (GetVolumeInformationA(sourceDrive.c_str(), volName, sizeof(volName), NULL, NULL, NULL, NULL, 0)) {
        volumeName = volName;
        for (auto& c : volumeName) if (!isalnum(static_cast<unsigned char>(c))) c = '_';
    }

    std::string outputFile = (dest / (volumeName + ".mp4")).string();

    if (encode) {
        std::stringstream cmd;
        std::string baseDir = GetParentDir();
        cmd << baseDir + "HandBrakeCLI.exe -i " << sourceDrive
            << " -o \"" << outputFile << "\""
            << " --preset \"Super HQ 2160p60 4K HEVC Surround\"";

        if (useCSS) {
            cmd << " --enable-libdvdcss";
        }

        std::cout << "[JJMPEG] Encoding DVD: " << cmd.str() << "\n";
        RunCommand(cmd.str());
    }
    else {
        std::cout << "[JJMPEG] Copying DVD files to folder: " << dest.string() << "\n";
        for (const auto& entry : fs::directory_iterator(sourceDrive)) {
            if (entry.is_regular_file()) {
                fs::copy_file(entry.path(), dest / entry.path().filename(), fs::copy_options::overwrite_existing);
            }
        }
    }

    std::cout << "[JJMPEG] DVD copy finished: " << outputFile << "\n";
}

void cmdSpeed(const std::string& input, const std::string& factor, const std::string& output) {
    std::string cmd = "ffmpeg -i \"" + input + "\" -filter_complex \"[0:v]setpts=" + std::to_string(1.0 / std::stod(factor)) + "*PTS[v];[0:a]atempo=" + factor + "[a]\" -map \"[v]\" -map \"[a]\" \"" + output + "\"";
    runCommand(cmd);
}

void cmdWatermark(const std::string& input, const std::string& watermark, const std::string& output) {
    std::string cmd = "ffmpeg -i \"" + input + "\" -i \"" + watermark + "\" -filter_complex \"overlay=10:10\" \"" + output + "\"";
    runCommand(cmd);
}
void FolderConvert(const std::string& folderPath, const std::string& targetExtension) {
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA((folderPath + "\\*.*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Could not open folder " << folderPath << "\n";
        return;
    }

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::string inputFile = folderPath + "\\" + findFileData.cFileName;

            std::string filename = findFileData.cFileName;
            size_t dotPos = filename.find_last_of('.');
            std::string baseName = (dotPos == std::string::npos) ? filename : filename.substr(0, dotPos);

            std::string outputFile = folderPath + "\\" + baseName + "." + targetExtension;

            if (GetFileAttributesA(outputFile.c_str()) != INVALID_FILE_ATTRIBUTES) {
                std::cerr << "Error: " << outputFile << " already exists. Skipping.\n";
                continue;
            }

            std::string baseDir = GetParentDir();
            std::string command = baseDir + "\\main.exe -y -i \"" + inputFile + "\" \"" + outputFile + "\"";

            std::cout << "Converting: " << inputFile << " -> " << outputFile << "\n";

            RunCommand(command);
        }
    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);
}

bool IsRunningAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&NtAuthority, 2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &adminGroup))
    {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin == TRUE;
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        ShowHelp();
        return 0;
    }

    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exeDir = buffer;
    exeDir = exeDir.substr(0, exeDir.find_last_of("\\/"));


    std::string first = argv[1];
    std::transform(first.begin(), first.end(), first.begin(), ::tolower);

    if (first == "dvd-rip") {
        if (argc >= 5 && std::string(argv[3]) == "to") {
            DVDRip(argv[2], argv[4]);
        }
        else {
            DVDRip(argv[2]);
        }
    }

    else if (first == "convert" && argc >= 5 && std::string(argv[3]) == "to") {
        ConvertFile(argv[2], argv[4]);
    }
    else if (first == "resize" && argc >= 5 && std::string(argv[3]) == "to") {
        std::string input = argv[2];
        std::string size = argv[4];
        std::string output = input.substr(0, input.find_last_of(".")) + "_resized.mp4";
        std::string baseDir = GetParentDir();
        std::string cmd = baseDir + "\\main.exe -i \"" + input + "\" -vf scale=" + size + " \"" + output + "\"";
        RunCommand(cmd);
    }
    else if (first == "trim" && argc >= 9 &&
        std::string(argv[3]) == "from" && std::string(argv[5]) == "to" && std::string(argv[7]) == "to") {
        std::string input = argv[2];
        std::string start = argv[4];
        std::string end = argv[6];
        std::string output = argv[8];
        std::string baseDir = GetParentDir();
        std::string cmd = baseDir + "\\main.exe -i \"" + input + "\" -ss " + start + " -to " + end + " -c copy \"" + output + "\"";
        RunCommand(cmd);
    }
    else if (first == "extract" && argc >= 7 && std::string(argv[2]) == "audio" &&
        std::string(argv[3]) == "from" && std::string(argv[5]) == "to") {
        std::string input = argv[4];
        std::string output = argv[6];
        std::string baseDir = GetParentDir();
        std::string cmd = baseDir + "\\main.exe -i \"" + input + "\" -vn -acodec copy \"" + output + "\"";
        RunCommand(cmd);
    }
    else if (first == "calculate") {
        std::cout << "Opening scientific calculator...\n";
        std::string baseDir = GetParentDir();
        RunCommand(baseDir + "\\calc.exe");
    }

    else if (first == "upscale" && argc >= 7 && std::string(argv[3]) == "to") {
        std::string input = argv[2];
        std::string resolutionFps = argv[4];
        std::string quality = argv[5];
        std::string output = argv[6];

        std::string res = (resolutionFps == "4K60") ? "3840:2160" : "1920:1080";
        std::string fps = (resolutionFps == "4K60") ? "60" : "30";
        std::string crf = (quality == "low") ? "28" : (quality == "medium") ? "23" : (quality == "high") ? "18" : "14";

        std::string baseDir = GetParentDir();
        std::string cmd = baseDir + "\\main.exe -i \"" + input + "\" -vf scale=" + res + " -r " + fps +
            " -c:v libx265 -crf " + crf + " -preset slow \"" + output + "\"";
        RunCommand(cmd);
    }

    else if (first == "ffmpeg" || first == "ffprobe" || first == "ffplay" ||
        first == "yt-dlp" || first == "handbrake" || first == "y-cruncher" || first == "duf" || first == "aria2") {

        std::string baseDir = GetParentDir();
        std::string toolExe;
        if (first == "ffmpeg") toolExe = baseDir + "\\main.exe";
        else if (first == "ffprobe") toolExe = baseDir + "\\probe.exe";
        else if (first == "ffplay") toolExe = baseDir + "\\play.exe";
        else if (first == "yt-dlp") toolExe = baseDir + "\\yt-dlp.exe";
        else if (first == "handbrake") toolExe = baseDir + "\\dvd-rip.exe";
        else if (first == "y-cruncher") toolExe = exeDir + baseDir + "\\y-cruncher\\y-cruncher.exe";
        else if (first == "duf") toolExe = exeDir + baseDir + "\\tools\\duf.exe";
        else if (first == "aria2") toolExe = baseDir + "\\aria2c.exe";
        else {
            std::cerr << "Unknown tool: " << first << "\n";
            return 1;
        }

        std::string cmd = toolExe;
        for (int i = 2; i < argc; i++) {
            cmd += " ";
            cmd += argv[i];
        }

        RunCommand(cmd);
    }
    else if (first == "play" && argc >= 3) {
        std::string baseDir = GetParentDir();
        std::string cmd = baseDir + "\\play.exe \"" + std::string(argv[2]) + "\"";
        RunCommand(cmd);
    }
    else if (first == "probe" && argc >= 3) {
        std::string baseDir = GetParentDir();
        std::string cmd = baseDir + "\\probe.exe \"" + std::string(argv[2]) + "\"";
        RunCommand(cmd);
    }
    else if (first == "trim" && argc >= 6) {
        std::string cmd = "ffmpeg -i \"" + std::string(argv[2]) +
            "\" -ss " + std::string(argv[3]) +
            " -t " + std::string(argv[4]) +
            " -c copy \"" + std::string(argv[5]) + "\"";
        RunCommand(cmd);
    }
    else if (first == "duf") {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        std::string exeDir = buffer;
        exeDir = exeDir.substr(0, exeDir.find_last_of("\\/"));

        std::string baseDir = GetParentDir();
        std::string dufExe = exeDir + baseDir + "\\tools\\duf.exe";

        std::string cmd = "\"" + dufExe + "\" -json";
        for (int i = 2; i < argc; i++) {
            cmd += " \"" + std::string(argv[i]) + "\"";
        }

        RunCommand(cmd);
    }
    else if (first == "extract-audio" && argc >= 4) {
        std::string baseDir = GetParentDir();
        std::string cmd = baseDir + "ffmpeg -i \"" + std::string(argv[2]) +
            "\" -vn -c:a libmp3lame -q:a 2 \"" + std::string(argv[3]) + "\"";
        RunCommand(cmd);
    }
    else if (first == "screenshot" && argc >= 5) {
        std::string baseDir = GetParentDir();
        std::string cmd = baseDir + "ffmpeg -ss " + std::string(argv[3]) +
            " -i \"" + std::string(argv[2]) +
            "\" -vframes 1 \"" + std::string(argv[4]) + "\"";
        RunCommand(cmd);
    }
    else if (first == "gif" && argc >= 6) {
        std::string baseDir = GetParentDir();
        std::string cmd = baseDir + "ffmpeg -ss " + std::string(argv[3]) +
            " -t " + std::string(argv[4]) +
            " -i \"" + std::string(argv[2]) +
            "\" -vf \"fps=10,scale=640:-1:flags=lanczos\" -gifflags +transdiff \"" + std::string(argv[5]) + "\"";
        RunCommand(cmd);
    }
    else if (first == "jjmpeg" && argc >= 6 && std::string(argv[2]) == "dvd" && std::string(argv[3]) == "copy" && std::string(argv[5]) == "to") {
        std::string sourceDrive = argv[4];
        std::string destPath = argv[6];
        bool encode = (argc >= 8 && std::string(argv[7]) == "encode");
        JJMPegDVDCopy(sourceDrive, destPath, encode);
    }
    else if (first == "vt" && argc >= 3) {
        std::string sub = argv[2];
        std::transform(sub.begin(), sub.end(), sub.begin(), ::tolower);

        std::map<std::string, std::string> svcMap = {
            {"itunes", "iTunes"},
            {"hotstar", "Hotstar"},
            {"disneyplus", "DisneyPlus"},
            {"disney", "DisneyPlus"},
            {"paramountplus", "ParamountPlus"},
            {"paramount", "ParamountPlus"},
            {"sunnxt", "Sunnxt"},
            {"rakutentv", "RakutenTV"},
            {"rakuten", "RakutenTV"},
            {"install", "INSTALL_MARKER"}
        };

        if (sub == "install") {
            if (!IsRunningAsAdmin()) {
                std::cerr << "[JJMPEG] Error: This command requires administrator privileges.\n";
                std::cerr << "Please open Command Prompt as Administrator and run again.\n";
                return 1;
            }

            std::string baseDir = GetParentDir();
            std::string installDir = baseDir + "\\tools\\VT-PR";
            std::string installPath = installDir + "\\install.bat";

            if (GetFileAttributesA(installPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
                std::cerr << "Error: " << installPath << " not found.\n";
                return 1;
            }

            std::cout << "[JJMPEG] Running VT installer...\n";

            SetCurrentDirectoryA(installDir.c_str());
            std::string cmd = "cmd.exe /c install.bat";
            int result = system(cmd.c_str());

            if (result == 0)
                std::cout << "[JJMPEG] VT installation completed successfully.\n";
            else
                std::cerr << "[JJMPEG] VT installation failed (exit code " << result << ").\n";

            return 0;
        }
        if (argc == 4) {
            std::string service = sub;
            std::string cookiePath = argv[3];
            if (!IsRunningAsAdmin()) {
                std::cerr << "[JJMPEG] Error: Updating cookies requires administrator privileges.\n";
                std::cerr << "Please run Command Prompt as Administrator and try again.\n";
                return 1;
            }
            if (!service.empty()) service[0] = toupper(service[0]);
            std::string baseDir = GetParentDir();
            std::string destDir = baseDir + "\\tools\\VT-PR\\vinetrimmer\\cookies\\" + service;
            std::string destPath = destDir + "\\default.txt";
            DWORD attr = GetFileAttributesA(destDir.c_str());
            std::error_code ec;
            std::filesystem::create_directories(destDir, ec);
            if (ec) {
                std::cerr << "[JJMPEG] Error: Failed to create directory: " << destDir << "\n";
                std::cerr << "Reason: " << ec.message() << "\n";
                return 1;
            }
            if (!CopyFileA(cookiePath.c_str(), destPath.c_str(), FALSE)) {
                std::cerr << "[JJMPEG] Error: Failed to copy cookie file.\n";
                std::cerr << "From: " << cookiePath << "\n";
                std::cerr << "To:   " << destPath << "\n";
                return 1;
            }
            std::cout << "[JJMPEG] Cookie file copied successfully.\n";
            std::cout << "Service: " << service << "\n";
            std::cout << "Destination: " << destPath << "\n";
            return 0;
        }

        if (sub == "poetry" && argc >= 4) {
            if (!IsRunningAsAdmin()) {
                std::cerr << "[JJMPEG] Error: Poetry commands require administrator privileges.\n";
                std::cerr << "Please run Command Prompt as Administrator and try again.\n";
                return 1;
            }
            std::string poetryCmd;
            std::string targetPath;
            for (int i = 3; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg.rfind("#", 0) == 0) {
                    targetPath = arg.substr(1);
                }
                else {
                    poetryCmd += arg;
                    if (i < argc - 1) poetryCmd += " ";
                }
            }
            std::string baseDir = GetParentDir();
            std::string pythonPath = baseDir + "\\tools\\python 312\\python.exe";
            std::string workingDir = baseDir + "\\tools\\VT-PR";
            std::string downloadsDir = workingDir + "\\Downloads";
            if (GetFileAttributesA(pythonPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
                std::cerr << "[JJMPEG] Error: Python executable not found at:\n" << pythonPath << "\n";
                return 1;
            }
            if (!SetCurrentDirectoryA(workingDir.c_str())) {
                std::cerr << "[JJMPEG] Error: Failed to set working directory to:\n" << workingDir << "\n";
                return 1;
            }
            std::string cmd = "\"" + pythonPath + "\" -m poetry " + poetryCmd;
            std::cout << "[JJMPEG] Running poetry command:\n" << cmd << "\n";
            int result = system(cmd.c_str());
            if (result != 0) {
                std::cerr << "[JJMPEG] Poetry command failed (exit code " << result << ").\n";
                return 1;
            }
            std::cout << "[JJMPEG] Poetry command completed successfully.\n";
            if (!targetPath.empty()) {
                std::filesystem::path target(targetPath);
                std::filesystem::path fullTarget;
                if (target.is_absolute()) {
                    fullTarget = target;
                }
                else {
                    fullTarget = std::filesystem::path(baseDir) / target;
                }
                std::error_code ec;
                std::filesystem::create_directories(fullTarget, ec);
                if (ec) {
                    std::cerr << "[JJMPEG] Error: Failed to create target directory: " << fullTarget << "\n";
                    std::cerr << "Reason: " << ec.message() << "\n";
                    return 1;
                }
                for (const auto& entry : std::filesystem::directory_iterator(downloadsDir)) {
                    std::filesystem::path src = entry.path();
                    std::filesystem::path dst = fullTarget / src.filename();
                    std::error_code moveErr;
                    if (entry.is_regular_file()) {
                        std::filesystem::rename(src, dst, moveErr);
                    }
                    else if (entry.is_directory()) {
                        std::filesystem::rename(src, dst, moveErr);
                    }
                    if (moveErr) {
                        std::cerr << "[JJMPEG] Failed to move: " << src << "\n";
                        std::cerr << "Reason: " << moveErr.message() << "\n";
                    }
                    else {
                        std::cout << "[JJMPEG] Moved: " << src.filename().string() << "\n";
                    }
                }

            }

            return 0;
        }
        auto it = svcMap.find(sub);
        if (it != svcMap.end()) {
            if (sub == "install") {
                std::cerr << "Usage: jjmpeg vt install\n";
                return 1;
            }
            if (!IsRunningAsAdmin()) {
                std::cerr << "[JJMPEG] Error: Updating credentials requires administrator privileges.\n";
                std::cerr << "Please run Command Prompt as Administrator and try again.\n";
                return 1;
            }
            if (argc < 7 || std::string(argv[3]) != "name:" || std::string(argv[5]) != "pw:") {
                std::cerr << "Usage: jjmpeg vt " << sub << " name: <username> pw: <password>\n";
                return 1;
            }
            std::string username = argv[4];
            std::string password = argv[6];
            std::string combined = username + ":" + password;
            std::string baseDir = GetParentDir();
            std::string ymlPath = baseDir + "\\tools\\VT-PR\\vinetrimmer\\vinetrimmer.yml";
            std::ifstream fin(ymlPath);
            if (!fin.is_open()) {
                std::cerr << "Error: Could not open " << ymlPath << "\n";
                return 1;
            }
            std::vector<std::string> lines;
            std::string line;
            while (std::getline(fin, line)) lines.push_back(line);
            fin.close();
            if (lines.empty()) {
                std::cerr << "Error: vinetrimmer.yml appears empty.\n";
                return 1;
            }
            auto tolower_copy = [](const std::string& s) {
                std::string r = s;
                std::transform(r.begin(), r.end(), r.begin(), ::tolower);
                return r;
                };
            int credIndex = -1;
            for (size_t i = 0; i < lines.size(); ++i) {
                std::string tmp = lines[i];
                size_t pos = tmp.find_first_not_of(" \t");
                if (pos != std::string::npos) tmp = tmp.substr(pos);
                else tmp.clear();
                std::string lower = tolower_copy(tmp);
                if (lower.rfind("credentials:", 0) == 0) {
                    credIndex = static_cast<int>(i);
                    break;
                }
            }
            bool replaced = false;
            std::string yamlKey = it->second;
            std::string yamlKeyLower = tolower_copy(yamlKey);
            if (credIndex != -1) {
                for (int i = credIndex + 1; i < static_cast<int>(lines.size()) && i <= credIndex + 40; ++i) {
                    std::string trimmed = lines[i];
                    size_t p = trimmed.find_first_not_of(" \t");
                    std::string trimmedNoIndent = (p == std::string::npos) ? std::string() : trimmed.substr(p);
                    std::string trimmedLower = tolower_copy(trimmedNoIndent);

                    if (trimmedLower.rfind(yamlKeyLower + ":", 0) == 0) {
                        std::string indent = (p == std::string::npos) ? "  " : lines[i].substr(0, p);
                        std::string newLine = indent + yamlKey + ": '" + combined + "'";
                        lines[i] = newLine;
                        replaced = true;
                        break;
                    }
                }
            }
            if (!replaced) {
                int startIdx = 26;
                int endIdx = 33;
                int sizeMinusOne = static_cast<int>(lines.size()) - 1;
                int maxIndex = (endIdx < sizeMinusOne) ? endIdx : sizeMinusOne;

                for (int i = startIdx; i <= maxIndex; ++i) {
                    std::string trimmed = lines[i];
                    size_t p = trimmed.find_first_not_of(" \t");
                    std::string trimmedNoIndent = (p == std::string::npos) ? std::string() : trimmed.substr(p);
                    std::string trimmedLower = tolower_copy(trimmedNoIndent);

                    if (trimmedLower.rfind(yamlKeyLower + ":", 0) == 0) {
                        std::string indent = (p == std::string::npos) ? "  " : lines[i].substr(0, p);
                        std::string newLine = indent + yamlKey + ": '" + combined + "'";
                        lines[i] = newLine;
                        replaced = true;
                        break;
                    }
                }
            }
            if (!replaced) {
                std::cerr << "Error: Could not find a suitable line to replace for service " << yamlKey << ".\n";
                return 1;
            }
            std::ofstream fout(ymlPath, std::ios::trunc);
            if (!fout.is_open()) {
                std::cerr << "Error: Could not write to " << ymlPath << "\n";
                return 1;
            }
            for (const auto& l : lines) fout << l << "\n";
            fout.close();
            std::cout << "[JJMPEG] Updated " << yamlKey << " credentials in vinetrimmer.yml.\n";
            std::cout << "Username: " << username << "\n";
            std::cout << "Password: ********\n";

            return 0;
        }
        std::cerr << "Unknown vt subcommand: " << sub << "\n";
        std::cerr << "Usage: jjmpeg vt install\n";
        std::cerr << "       jjmpeg vt <service> name: <username> pw: <password>\n";
        return 1;
    }
    else if (first == "volume" && argc >= 5) {
        std::string baseDir = GetParentDir();
        std::string cmd = baseDir + "ffmpeg -i \"" + std::string(argv[2]) +
            "\" -filter:a \"volume=" + std::string(argv[3]) +
            "\" \"" + std::string(argv[4]) + "\"";
        RunCommand(cmd);
    }
    else if (first == "speed" && argc >= 5) {
        std::string baseDir = GetParentDir();
        std::string cmd = baseDir + "ffmpeg -i \"" + std::string(argv[2]) +
            "\" -filter_complex \"[0:v]setpts=" +
            std::to_string(1.0 / std::stod(argv[3])) +
            "*PTS[v];[0:a]atempo=" + std::string(argv[3]) +
            "[a]\" -map \"[v]\" -map \"[a]\" \"" + std::string(argv[4]) + "\"";
        RunCommand(cmd);
    }
    else if (first == "watermark" && argc >= 5) {
        std::string baseDir = GetParentDir();
        std::string cmd = baseDir + "ffmpeg -i \"" + std::string(argv[2]) +
            "\" -i \"" + std::string(argv[3]) +
            "\" -filter_complex \"overlay=10:10\" \"" + std::string(argv[4]) + "\"";
        RunCommand(cmd);
    }
    else if (first == "copy" && argc >= 5 && std::string(argv[3]) == "to") {
        std::string url = argv[2];
        std::string output = argv[4];
        std::string cookiesFile;

        if (argc >= 9 && std::string(argv[5]) == "with" && std::string(argv[6]) == "cookies" && std::string(argv[7]) == "at") {
            cookiesFile = argv[8];
        }

        std::filesystem::create_directories(output);

        std::string baseDir = GetParentDir();
        std::string cmd = baseDir + "\\yt-dlp.exe ";
        if (!cookiesFile.empty()) cmd += "--cookies \"" + cookiesFile + "\" ";
        cmd += "-f \"bv*+ba/b\" --yes-playlist -o \"" + output + "\\%(playlist_index)03d - %(title)s.%(ext)s\" \"" + url + "\"";
        RunCommand(cmd);
    }
    else if (first == "html" && argc >= 3) {
        std::string action = argv[2];
        if (action == "view" && argc >= 4) {
            std::string htmlPath = argv[3];
            ShellExecuteA(NULL, "open", htmlPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
            std::cout << "Opening HTML file in browser: " << htmlPath << std::endl;
        }
    }
    else if (first == "python") {
        std::string baseDir = GetParentDir();
        std::string pythonExe = baseDir + "\\python\\python.exe";
        if (GetFileAttributesA(pythonExe.c_str()) == INVALID_FILE_ATTRIBUTES) {
            std::cerr << "[JJMPEG] Error: Python not found\n";
            return 1;
        }
        std::string cmd = "\"" + pythonExe + "\"";
        for (int i = 2; i < argc; ++i) {
            cmd += " \"" + std::string(argv[i]) + "\"";
        }
        PROCESS_INFORMATION pi;
        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        ZeroMemory(&pi, sizeof(pi));

        BOOL success = CreateProcessA(
            NULL,
            (LPSTR)cmd.c_str(),
            NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

        if (!success) {
            std::cerr << "[JJMPEG] Failed to start Python process.\n";
            return 1;
        }
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return (int)exitCode;
    }
    else if (first == "pip") {
        std::string baseDir = GetParentDir();
        std::string pythonExe = baseDir + "\\python\\python.exe";
        if (GetFileAttributesA(pythonExe.c_str()) == INVALID_FILE_ATTRIBUTES) {
            std::cerr << "[JJMPEG] Error: Python not found\n";
            return 1;
        }
        std::string cmd = "\"" + pythonExe + "\" -m pip";
        for (int i = 2; i < argc; ++i) {
            cmd += " \"" + std::string(argv[i]) + "\"";
        }
        RunCommand(cmd);
    }
    else if (first == "combine" && argc >= 6) {
        std::string input1 = argv[2];
        std::string input2 = argv[4];
        std::string output = argv[6];

        std::string baseDir = GetParentDir();
        std::string ffmpegPath = baseDir + "\\main.exe";
        std::string listFile = "inputs.txt";
        std::ofstream fout(listFile);
        fout << "file '" << input1 << "'\n";
        fout << "file '" << input2 << "'\n";
        fout.close();
        std::string cmd = "cmd.exe /c \"\"" + ffmpegPath +
            "\" -y -f concat -safe 0 -i \"" + listFile +
            "\" -c copy \"" + output + "\"\"";

        std::cout << "Combining " << input1 << " and " << input2
            << " into " << output << "..." << std::endl;
        int result = system(cmd.c_str());
        if (result == 0)
            std::cout << "Combine complete: " << output << std::endl;
        else
            std::cerr << "FFmpeg combine failed (code " << result << ")" << std::endl;
        std::remove(listFile.c_str());

        return result;
    }

    else if (first == "y-cruncher") {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        std::string exeDir = buffer;
        exeDir = exeDir.substr(0, exeDir.find_last_of("\\/"));
        std::string baseDir = GetParentDir();
        std::string yCruncherExe = exeDir + baseDir + "\\y - cruncher\\y - cruncher.exe";
        std::string yCruncherDir = exeDir + baseDir + "\\y - cruncher";
        SHELLEXECUTEINFOA sei = { 0 };
        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_DEFAULT;
        sei.hwnd = NULL;
        sei.lpVerb = "runas";
        sei.lpFile = yCruncherExe.c_str();
        sei.lpParameters = "";
        sei.lpDirectory = yCruncherDir.c_str();
        sei.nShow = SW_SHOWNORMAL;
        if (!ShellExecuteExA(&sei)) {
            DWORD err = GetLastError();
            if (err == ERROR_CANCELLED) {
                std::cerr << "User refused elevation.\n";
            }
            else {
                std::cerr << "Failed to launch y-cruncher. Error: " << err << "\n";
            }
        }
    }

    else if (first == "folder" && argc >= 3) {
        std::string sub = argv[2];
        std::transform(sub.begin(), sub.end(), sub.begin(), ::tolower);

        if (sub == "convert" && argc >= 6 && std::string(argv[4]) == "to") {
            FolderConvert(argv[3], argv[5]);
        }
        else if (sub == "play" && argc >= 4) FolderPlay(argv[3]);
        else if (sub == "upscale" && argc >= 7) FolderUpscale(argv[3], argv[4], argv[5]);
        else if (sub == "extract-audio" && argc >= 5) FolderExtractAudio(argv[3], argv[4]);
        else if (sub == "thumbnail" && argc >= 5) FolderThumbnail(argv[3], argv[4]);
        else if (sub == "info" && argc >= 4) FolderInfo(argv[3]);
        else if (sub == "concat" && argc >= 5) FolderConcat(argv[3], argv[4]);
        else if (sub == "subtitles" && argc >= 4) FolderSubtitles(argv[3]);
        else if (sub == "normalize" && argc >= 4) FolderNormalizeAudio(argv[3]);
        else if (sub == "gif" && argc >= 4) FolderGif(argv[3]);
        else if (sub == "probe" && argc >= 4) FolderProbe(argv[3]);
        else std::cout << "Unknown folder command.\n";
    }

    else if (first == "help") ShowHelp();
    else if (first == "help_advanced" && argc >= 3) {
        ShowHelpAdvanced(argv[2]);
    }
    else if (first == "--version") std::cout << "JJmpeg version " << JJMPG_VERSION << "\n";

    else {
        std::string baseDir = GetParentDir();
        const char* executables[] = {
            "main.exe",
            "aria2c.exe",
            "play.exe",
            "probe.exe",
            "yt-dlp.exe",
            "dvd-rip.exe",
            "tools\\duf.exe",
            "y-cruncher\\y-cruncher.exe"
        };
        for (const auto& exe : executables) {
            std::string cmd = exe;
            for (int i = 1; i < argc; ++i) {
                cmd += " ";
                cmd += argv[i];
            }
            RunCommand(cmd);
        }
    }
    return 0;
}