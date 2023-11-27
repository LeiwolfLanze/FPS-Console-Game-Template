#include<vector>
#include<iostream>
#include<string>
#include<memory>
#include<conio.h>
#include"Position.h"
#include"ANSI.h"
#include"rxWindow.h"

using namespace std;

class Game {
public:

    class MapBlock : public enable_shared_from_this<MapBlock> {
    private:
        int blockWidth, blockHeight, blockCenterX, blockCenterY;
        static int blockCount;
    public:
        int blockTag;
        string blockName;
        vector<vector<char>> blockboard;
        shared_ptr<MapBlock> north, south, east, west;
        MapBlock(string name, int blockWidth, int blockHeight) : north(nullptr), south(nullptr), east(nullptr), west(nullptr), blockName(name),
                                            blockWidth(blockWidth), blockHeight(blockHeight),
                                            blockCenterX(blockWidth / 2), blockCenterY(blockHeight / 2) {
            blockCount++;
            blockTag = blockCount;
            blockboard.resize(blockHeight);
            for(int i = 0; i < blockHeight; i++) blockboard[i].resize(blockWidth);

            for(int i = 0; i < blockHeight; i++) {
                for(int j = 0; j < blockWidth; j++) {
                    if(i == 0 || i == blockHeight - 1) blockboard[i][j] = '-';
                    else if(j == 0 || j == blockWidth - 1) blockboard[i][j] = '|';
                    else blockboard[i][j] = ' ';
                }
            }
        }

        ~MapBlock() {}

        void insertNorth(shared_ptr<MapBlock> newBlock) {
            north = newBlock;
            newBlock->south = shared_from_this();
            for(int i = blockCenterX-5; i < blockCenterX+5; i++) blockboard[0][i] = ' ', newBlock->blockboard[blockHeight - 1][i] = ' ';
        }

        void insertSouth(shared_ptr<MapBlock> newBlock) {
            south = newBlock;
            newBlock->north = shared_from_this();
            for(int i = blockCenterX-5; i < blockCenterX+5; i++) blockboard[blockHeight - 1][i] = ' ', newBlock->blockboard[0][i] = ' ';
        }

        void insertEast(shared_ptr<MapBlock> newBlock) {
            east = newBlock;
            newBlock->west = shared_from_this();
            for(int i = blockCenterY-3; i < blockCenterY+3; i++) blockboard[i][blockWidth - 1] = ' ', newBlock->blockboard[i][0] = ' ';
        }

        void insertWest(shared_ptr<MapBlock> newBlock) {
            west = newBlock;
            newBlock->east = shared_from_this();
            for(int i = blockCenterY-3; i < blockCenterY+3; i++) blockboard[i][0] = ' ', newBlock->blockboard[i][blockWidth - 1] = ' ';
        }
    };

    Game(int FPS, int scrWidth, int scrHeight) {

        this->screenWidth = (scrWidth > gameFrameWidth) ? scrWidth : gameFrameWidth;
	    this->screenHeight = (scrHeight > gameFrameHeight) ? scrHeight : gameFrameHeight;
        this->gameFPS = FPS;

        rxW.init(screenWidth, screenHeight, gameFrameWidth, gameFrameHeight, gameFPS); // Initialize rxWindow

        // Initialize the general messages
        currentGoal = "Get your first weapon!"; // Initialize the current goal
        publicMsg = make_shared<GeneralMessage>(ANSI::render(FG_BLUE, BG_WHITE, "-[ Welcome to the game! ]-"), 4, 6, "p1"); // Initialize the public message
        goalMsg = make_shared<GeneralMessage>(ANSI::render(FG_BRIGHT_YELLOW, BG_BLACK, "Task: " + currentGoal), 2, 3, "g1"); // Initialize the goal message
        rxW.addGeneralMessage(publicMsg); // Add the public message to the message list
        rxW.addGeneralMessage(goalMsg); // Add the goal message to the message list

        // Initialize commandline
        this->isCommandInput = false;
        this->commandLine = "";

        // Initialize the game
        this->mapWidth = 40;
        this->mapHeight = 20;

        playerPos = Position(mapWidth / 2, mapHeight / 2);

        currentBlock = make_shared<MapBlock>("Starter's Village", mapWidth, mapHeight);
        currentBlock->insertNorth(make_shared<MapBlock>("North Kingdom", mapWidth, mapHeight));
        currentBlock->insertSouth(make_shared<MapBlock>("South Coast", mapWidth, mapHeight));
        currentBlock->insertEast(make_shared<MapBlock>("East Forest", mapWidth, mapHeight));
        currentBlock->insertWest(make_shared<MapBlock>("Desert Flames", mapWidth, mapHeight));

        currentBlock->south->insertEast(make_shared<MapBlock>("Lost Kingdom", mapWidth, mapHeight));
        currentBlock->south->insertWest(make_shared<MapBlock>("Misty Mountains", mapWidth, mapHeight));

        gameboard.resize(mapHeight);
        for(int i = 0; i < mapHeight; i++) gameboard[i].resize(mapWidth);

        setGameboard();
    }
    ~Game() {}

    void setGameboard() {
        for(int i = 0; i < mapHeight; i++)
                for(int j = 0; j < mapWidth; j++) gameboard[i][j] = currentBlock->blockboard[i][j];
    }
    
    void movePlayer(const Position& deltaPos) {
        Position nextPos = playerPos + deltaPos;
        bool resetGameboard = false;

        if(nextPos.x < 0 && currentBlock->west != nullptr) {
            currentBlock = currentBlock->west;
            playerPos.x = mapWidth - 1, resetGameboard = true;
        }
        else if(nextPos.x >= mapWidth && currentBlock->east != nullptr) {
            currentBlock = currentBlock->east;
            playerPos.x = 0, resetGameboard = true;
        }
        else if(nextPos.y < 0 && currentBlock->north != nullptr) {
            currentBlock = currentBlock->north;
            playerPos.y = mapHeight - 1, resetGameboard = true;
        }
        else if(nextPos.y >= mapHeight && currentBlock->south != nullptr) {
            currentBlock = currentBlock->south;
            playerPos.y = 0, resetGameboard = true;
        }
        else if (gameboard[nextPos.y][nextPos.x] == ' ') playerPos = nextPos;
        else msg.newMessage("< Invalid Position.", 2); // Make a new message

        if(resetGameboard) setGameboard();
    }

    bool processCommand(string& command) {

        if (command.empty()) {
            msg.newMessage("< Unknown command for /" + commandLine, 3);
            return true;
        }

        // You can use other libraries, such as <regex> for better command processing
        if(!command.compare("quit")) return false;
        else if(!command.compare("save")) msg.newMessage("< Saved!", 2);
        else if(!command.compare("help")) {
            string helpMsg = "< Available commands:\n";
            helpMsg += "  /quit - To quit the game.\n";
            helpMsg += "  /save - To save the current achievement.\n";
            helpMsg += "  /help - To display this help message.\n";
            msg.newMessage(helpMsg, 7);
        }
        else msg.newMessage("< Unknown command for /" + commandLine, 3);
        
        return true;
    }

    void render() {
        
        rxW.resetCursor(); // Reset the cursor
        
        rxW.displayGeneralMessage(); // Display the general messages

        rxW.println("Current Block: " + currentBlock->blockName);
        rxW.println("Hero Position: (" + to_string(playerPos.x) + ", " + to_string(playerPos.y) + ")");
        for(int i = 0; i < mapHeight; i++) {
            string str = "";
            for(int j = 0; j < mapWidth; j++) str += (i == playerPos.y && j == playerPos.x) ? 'H' : gameboard[i][j];
            rxW.println(str);
        }

        // Display command input
        rxW.println("========[Commandbar]========");
        rxW.println(isCommandInput ? ("Input Command > /" + commandLine) : "");

        // Show the action messages on console screen
        msg.messageToBuffer(rxW); // Display the action messages
        rxW.screenBufferOut(); // Output screen
    }

    bool update() {

        bool isGameOver = false;

        msg.updateMessageList();

        // Do something here. For example, check if the player is dead, update general messages, etc.
        string newGoal = goalList[currentBlock->blockTag];
        if(newGoal.compare(currentGoal)) {
            currentGoal = newGoal;
            goalMsg->message = ANSI::render(FG_YELLOW, BG_BLACK, "Task: " + currentGoal);
            msg.newMessage("< Goal has changed: " + currentGoal, 5);
        }

        if(isGameOver) return false;
        return true;
    }

    bool run() {
        
        rxW.clearScreen(); // Clear the console and reset the cursor

        bool isInGame = true, canMove = true;
        while(isInGame) {

            rxW.startFrameMeasuring(); // Start measuring the time of this frame

            if (_kbhit() && canMove && !isCommandInput) {
                char input = tolower(_getch());
                switch (input) {
					case 'w':
						movePlayer(Position(0, -1)); // Move the player up
						break;
					case 's':
						movePlayer(Position(0, 1)); // Move the player down
						break;
					case 'a':
						movePlayer(Position(-1, 0)); // Move the player left
						break;
					case 'd':
						movePlayer(Position(1, 0)); // Move the player right
						break;
                    case 27:
                        isInGame = false; // ESC to quit
                        break;
                    case '/':
						isCommandInput = true;
						break;
                    default:
                        break;
                }
                canMove = false;
            }

            // Handle user command input
			if (_kbhit() && isCommandInput) {
				char input = _getch();
				switch (input) {
				case 8: // Backspace
					if (!commandLine.empty()) commandLine.pop_back();
					break;
				case 13: // Enter
					isInGame = processCommand(commandLine);
					commandLine = "";
					isCommandInput = false;
					break;
				default:
					commandLine += input;
					break;
				}
			}

            if(!update()) isInGame = false;
            render();

			if(rxW.checkFrame()) canMove = true; // Check if the current frame is shorter than the desired frame duration, if so, sleep for the remaining time to achieve the desired FPS
        }

        return true;
    }

private:
    rxWindow rxW; // Build a rxWindow object
    rxMessage msg; // Build a rxMessage object
    
    const int gameFrameWidth = 80, gameFrameHeight = 30; // The frame size of the game, also the minimum requirement of the screen size
    int screenWidth, screenHeight, gameFPS;
    int mapWidth, mapHeight;
    Position playerPos;
    vector<vector<char>> gameboard;
    shared_ptr<MapBlock> currentBlock;

    // General messages
    shared_ptr<GeneralMessage> goalMsg;
    shared_ptr<GeneralMessage> publicMsg;

    // Goal list
    string currentGoal;
    vector<string> goalList = { "Get your first weapon in starter's village!",
                                "Meet the king in north kingdom.",
                                "Get a boat in south coast.",
                                "Hunt the dragon in the dark forest.",
                                "Find the dessert temple",
                                "Unfold the mystery of the lost kingdom.",
                                "Meet the hermit in the misty mountains." };

    // Commandline display
	bool isCommandInput = false;
	string commandLine = "";
};

int Game::MapBlock::blockCount = -1;

int main() {
    
    Game game(60, 100, 30); // Set FPS, screen width and height
    if(!game.run()) cout << "Game run failed!" << endl;

    return 0;
}