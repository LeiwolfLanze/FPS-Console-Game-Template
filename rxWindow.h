#pragma once
#include<vector>
#include<string>
#include<memory>
#include<chrono>
#include<thread>
#include"ANSI.h"

using namespace std;
using namespace chrono;

class GeneralMessage {
public:
bool showMessage = false;  // Flag to control whether to display the message
    string message, messageID;
    int lastingTime, period;
    steady_clock::time_point messageStart;
    GeneralMessage() {}
    GeneralMessage(string message, int lastingTime, int period, string messageID) : message(message), messageID(messageID), lastingTime(lastingTime), period(period), messageStart(steady_clock::now()) {}
    GeneralMessage(GeneralMessage& msg) : message(msg.message), messageID(msg.messageID), lastingTime(msg.lastingTime), period(msg.period), messageStart(msg.messageStart) {}
};

class rxWindow {
private:
    int screenWidth, screenHeight, gameFrameWidth, gameFrameHeight, gameFPS, outputLines = 0;
    vector<string> screen;
    milliseconds frameDuration;
    steady_clock::time_point frameStart;
    vector<shared_ptr<GeneralMessage>> generalMessageList;

public:
    rxWindow() {}

    void init(int screenWidth, int screenHeight, int gameFrameWidth, int gameFrameHeight, int gameFPS) {
        this->screenWidth = screenWidth;
        this->screenHeight = screenHeight;
        this->gameFrameWidth = gameFrameWidth;
        this->gameFrameHeight = gameFrameHeight;
        this->gameFPS = gameFPS;
        frameDuration = milliseconds(1000 / gameFPS);
        
        // Initialize screen
	    screen.resize(screenHeight, string(screenWidth, ' '));
    }

    void startFrameMeasuring() { frameStart = steady_clock::now(); }
    bool checkFrame() {
        // Calculate the time taken for this frame
        auto frameEnd = steady_clock::now();
        auto frameTime = duration_cast<milliseconds>(frameEnd - frameStart);

        // If the current frame is shorter than the desired frame duration
        // Sleep for the remaining time to achieve the desired FPS
        if (frameTime < frameDuration) {
            this_thread::sleep_for(frameDuration - frameTime);
            return true;
        }

        return false;
    }

    void clearScreen() { cout << "\033[2J\033[H"; } // Clear the console and reset the cursor
    void resetCursor() { cout << "\033[H"; } // Only reset the cursor: to move the cursor to the top left corner of the screen

    void screenBufferOut() {

        // Move the cursor to the top left corner of the screen
        cout << "\033[H";
        
        static int lastOutputLines = outputLines;

        // Output
        if (lastOutputLines > outputLines) {
            for (size_t row = outputLines; row < lastOutputLines; ++row) screen[row] = string(screenWidth, ' ');
            for (size_t row = 0; row < lastOutputLines; ++row) puts(screen[row].c_str());
        }
        else {
            for (size_t row = 0; row < outputLines; ++row) puts(screen[row].c_str());
        }

        // Reset screen, outputLines and lastOutputLines
        screen.resize(screenHeight, string(screenWidth, ' '));
        lastOutputLines = outputLines;
        outputLines = 0;
    }
    
    void println(const string& str, int row, int col) {

        // This println() can automatically newline if out of width or containing '\n' by using recursion

        // Out of screenHeight
        if (row > screenHeight -1) return;

        // Check if str contains '\n'
        size_t it = str.find('\n');

        if (it != string::npos) {
            string subStr = (it < screenWidth - col) ? str.substr(0, it) : str.substr(0, screenWidth - col);
            screen[row] = string(col, ' ') + subStr;
            screen[row].resize(screenWidth, ' ');
            if (row == outputLines) outputLines++;
            println((it < screenWidth - col) ? str.substr(it + 1) : str.substr(it), ++row, 0);
        }
        else {
            string subStr = (str.length() <= screenWidth - col) ? str : str.substr(0, screenWidth -col);
            screen[row] = string(col, ' ') + subStr;
            screen[row].resize(screenWidth, ' ');
            if (row == outputLines) outputLines++;
            if (str.length() > screenWidth - col) println(str.substr(screenWidth - col), ++row, 0);
        }
    }

    void println(const string& str) { println(str, outputLines, 0); }

    void addGeneralMessage(shared_ptr<GeneralMessage>& message) {
        generalMessageList.push_back(message);
        for (auto& it : generalMessageList) it->messageStart = steady_clock::now();
    }

    void displayGeneralMessage() {
        for (auto& it : generalMessageList) checkGeneralMessage(it, steady_clock::now());
    }

    void checkGeneralMessage(shared_ptr<GeneralMessage>& msg, steady_clock::time_point currentTime = steady_clock::now()) {

        if (msg->lastingTime == msg->period) msg->showMessage = true; // Always on display
        else {
            // Calculate the elapsed time since the message started
            auto elapsedSeconds = duration_cast<seconds>(currentTime - msg->messageStart);

            // Check if it's time to display the message
            msg->showMessage = (elapsedSeconds >= seconds(msg->period - msg->lastingTime) && elapsedSeconds < seconds(msg->period));

            // If the time exceeds total seconds, reset the start time to the current time
            if (elapsedSeconds >= seconds(msg->period)) msg->messageStart = currentTime;
        }

        int newLines = count_if(msg->message.begin(), msg->message.end(), [](char c) {return c == '\n'; });

        println(msg->showMessage ? msg->message : string(newLines, '\n'));
    }
};

class rxMessage {
private:
    int prevMessageListSize, curMessageListSize;
    vector<tuple<string, int, steady_clock::time_point>> messageList;
public:
    rxMessage() {}

    void newMessage(const string& str, int duration) { // duration in seconds
        messageList.emplace_back(str, duration, steady_clock::now());
    }

    void updateMessageList() {
         // Update the message list
        auto currentMsgTime = steady_clock::now();
        auto it = remove_if(messageList.begin(), messageList.end(), [&](const auto& t) { return (currentMsgTime - get<2>(t)) >= seconds(get<1>(t)); });
        messageList.erase(it, messageList.end());

        curMessageListSize = messageList.size();
        for (auto& it : messageList) curMessageListSize += count(get<0>(it).begin(), get<0>(it).end(), '\n');
    }

    void messageToBuffer(rxWindow& rxw) {
        // Display action messages
        rxw.println("========[Messages]========");
        auto currentMsgTime = steady_clock::now();

        for (auto it = messageList.rbegin(); it != messageList.rend(); ++it) {
            // Newest to oldest message rendered in different colors
            if (currentMsgTime - get<2>(*it) <= milliseconds(1000 * get<1>(*it)) / 3) rxw.println(ANSI::render(FG_YELLOW, BG_BLACK, get<0>(*it)));
            else if (currentMsgTime - get<2>(*it) <= milliseconds(1000 * get<1>(*it)) * 2 / 3) rxw.println(ANSI::render(FG_WHITE, BG_BLACK, get<0>(*it)));
            else rxw.println(ANSI::render(FG_GRAY, BG_BLACK, get<0>(*it)));
        }
        
        int clearMessage = prevMessageListSize - curMessageListSize;
        for (int i = 0; i < clearMessage; ++i) rxw.println("");

        prevMessageListSize = curMessageListSize;
    }
};