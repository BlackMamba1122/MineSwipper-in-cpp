#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <Windows.h>
#include <string>
#include <thread>
#define BLACK 0
#define WHITE 15
#define GREEN 2
#define RED 4
#define YELLOW 6
#define LPURPLE 13

void SetClr(int tcl, int bcl)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (tcl + (bcl * 16)));
}
int getRowColbyAnyClick(int& rpos, int& cpos)
{
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD prevMode;
    GetConsoleMode(hInput, &prevMode);
    SetConsoleMode(hInput, ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS | (prevMode & ~ENABLE_QUICK_EDIT_MODE));
    int buttonType = -1;
    do {
        DWORD Events;
        INPUT_RECORD InputRecord;
        ReadConsoleInput(hInput, &InputRecord, 1, &Events);

        if (InputRecord.EventType == MOUSE_EVENT && InputRecord.Event.MouseEvent.dwButtonState != 0) {
            cpos = InputRecord.Event.MouseEvent.dwMousePosition.X;
            rpos = InputRecord.Event.MouseEvent.dwMousePosition.Y;
            buttonType = (InputRecord.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) ? 0 : 1;
            break;
        }
    } while (true);
    SetConsoleMode(hInput, prevMode);
    return buttonType;
}
void gotoRowCol(int rpos, int cpos)
{
    COORD scrn;
    HANDLE hOuput = GetStdHandle(STD_OUTPUT_HANDLE);
    scrn.X = cpos;
    scrn.Y = rpos;
    SetConsoleCursorPosition(hOuput, scrn);
}

struct position {
    int ri, ci;
};
struct pos {
    int val = 0;
    bool isMine = false;
    bool isOpen = false;
    bool isNumber = false;
    bool Flag = false;
};
struct table {
    pos** ps;
    int size;
    int side;
    int mine;
    int count;
    int width;
    int startR;
    int startC;
};

table level;
bool gameover = false;
bool win = false;

struct progress {
    int Tplays;
    int Twins;
    int time;
};
struct detail {
    char name[20];
    char id[20];
    progress prog;
};
detail player;

void info(int choice)
{
    if (choice == 1)
    {
        level.side = 10;
        level.mine = 10;
        level.width = 4;
        level.startR = 20;
        level.startC = 30;
    }
    else if (choice == 2)
    {
        level.side = 14;
        level.mine = 40;
        level.width = 3;
        level.startR = 20;
        level.startC = 30;
    }
    else
    {
        level.side = 18;
        level.mine = 100;
        level.width = 2;
        level.startR = 20;
        level.startC = 30;
    }
}
void declare()
{
    level.ps = new pos * [level.side];
    for (int i = 0; i < level.side; i++)
        level.ps[i] = new pos[level.side];
}
void put_bomb(position* mines)
{
    for (int i = 0; i < level.mine; i++)
    {
        level.ps[mines[i].ri][mines[i].ci].isMine = true;
    }
}
bool bomb_present(position* mines, int randomX, int randomY)
{
    for (int i = 0; i < level.count; i++)
        if (mines[i].ri == randomX && mines[i].ci == randomY)
            return false;
    return true;
}
void bomb_position(position*& mines)
{
    srand(time(0));
    while (level.count <= level.mine)
    {
        int randomX = rand() % level.side;
        int randomY = rand() % level.side;
        if (bomb_present(mines, randomX, randomY))
        {
            position* mine = new position[level.count + 1];
            for (int i = 0; i < level.count; i++)
            {
                mine[i] = mines[i];
            }
            mine[level.count].ri = randomX;
            mine[level.count].ci = randomY;
            // delete[] mines;
            mines = mine;
            level.count++;
        }
    }
    put_bomb(mines);
}
bool cordinate_check(int ri, int ci)
{
    return ((ri >= 0) && (ci >= 0) && (ri < level.side) && (ci < level.side));
}
void increment2(int ri, int ci)
{
    if (cordinate_check(ri, ci) && !level.ps[ri][ci].isMine)
        level.ps[ri][ci].val++, level.ps[ri][ci].isNumber = true;
}
void increment(position* mines)
{
    for (int i = 0; i < level.mine; i++)
    {
        int ri = mines[i].ri;
        int ci = mines[i].ci;
        increment2(ri - 1, ci - 1);
        increment2(ri - 1, ci);
        increment2(ri - 1, ci + 1);
        increment2(ri, ci - 1);
        increment2(ri, ci + 1);
        increment2(ri + 1, ci - 1);
        increment2(ri + 1, ci);
        increment2(ri + 1, ci + 1);
    }
}
void set_game(int choice)
{
    info(choice);
    declare();
    position* mines;
    bomb_position(mines);
    increment(mines);
}
int total_left;
void print(int ri, int ci, int row, int col, int len, char ch = -37)
{
    bool found = true;
    int temp1, temp2;
    for (int i = 0; i < len; i++)
    {
        gotoRowCol(row + i, col);
        for (int j = 0; j < len; j++)
        {
            if (gameover && level.ps[ri][ci].isMine)
                SetClr(BLACK, LPURPLE);
            else if (!level.ps[ri][ci].isOpen && !level.ps[ri][ci].Flag)
                SetClr(YELLOW, LPURPLE);
            else if (level.ps[ri][ci].isOpen)
                SetClr(WHITE, LPURPLE);
            else if (level.ps[ri][ci].Flag)
                SetClr(RED, LPURPLE);
            if (!(level.ps[ri][ci].isOpen && level.ps[ri][ci].isNumber))
                std::cout << ch;
            else
            {
                if (found)
                {
                    temp1 = i, temp2 = j;
                    found = false;
                }
                if (i == temp1 + len / 2 && j == temp2 + len / 2)
                {
                    SetClr(RED, WHITE);
                    std::cout << level.ps[ri][ci].val;
                    found = true;
                }
                else
                    std::cout << ch;
            }
        }
    }
}
void grid(char ch = -37)
{
    int r = level.startR;
    int c = level.startC;
    int d = level.width;
    for (int i = 0; i < level.side; i++)
    {
        r = level.startR + i;
        r += (i * d);
        for (int j = 0; j < level.side; j++)
        {
            c = level.startC + j;
            c += j * d;
            print(i, j, r, c, d, ch);
        }
    }
}
void flood(int, int);
void further_flood(position*& pos, int& size, int x, int y)
{
    if (cordinate_check(x, y) && !level.ps[x][y].isOpen)
    {
        level.ps[x][y].isOpen = true;
        total_left--;
        if (!level.ps[x][y].isNumber)
        {
            position* pos_temp = new position[size + 1];
            for (int i = 0; i < size; i++)
            {
                pos_temp[i].ri = pos[i].ri;
                pos_temp[i].ci = pos[i].ci;
            }
            pos_temp[size].ri = x;
            pos_temp[size].ci = y;
            // delete[] pos;
            pos = pos_temp;
            size++;
        }
    }
    for (int i = 0; i < size; i++)
        flood(pos[i].ri, pos[i].ci);
}
void flood(int x, int y)
{
    position* pos;
    int size = 0;
    if (level.ps[x][y].isMine && !level.ps[x][y].isOpen)
    {
        gameover = true;
        return;
    }
    else if (level.ps[x][y].isNumber)
    {
        level.ps[x][y].isOpen = true;
        total_left--;
        return;
    }
    else
    {
        if (!level.ps[x][y].isOpen)
            total_left--;
        level.ps[x][y].isOpen = true;
        further_flood(pos, size, x - 1, y - 1);
        further_flood(pos, size, x - 1, y);
        further_flood(pos, size, x - 1, y + 1);
        further_flood(pos, size, x, y - 1);
        further_flood(pos, size, x, y + 1);
        further_flood(pos, size, x + 1, y - 1);
        further_flood(pos, size, x + 1, y);
        further_flood(pos, size, x + 1, y + 1);
    }
}
void flag(int x, int y)
{
    level.ps[x][y].Flag = !level.ps[x][y].Flag;
}
void getClick()
{
    int x = -1, y = -1, button, a, b;
    do {
        button = getRowColbyAnyClick(x, y);
        a = (x - level.startR) / (level.width + 1);
        b = (y - level.startC) / (level.width + 1);
    } while (!cordinate_check(a, b));
    if (!button)
        flood(a, b);
    else
        flag(a, b);
}
void winCheck()
{
    if (total_left == level.mine)
        win = true;
}
void text(bool, bool, bool, bool, bool);
void signUpIn();
int start()
{
    int x = 0, y = 0;
    text(1, 1, 0, 0, 0);
    SetClr(WHITE, LPURPLE);
    text(0, 0, 1, 0, 0);
    while (1)
    {
        getRowColbyAnyClick(x, y);
        if (x > 65 && x < 72)
            if ((y >= 0 && y < 23) || (y > 40 && y < 64) || (y > 85 && y < 108))
                break;
    }
    return y;
}
int i = 0;
std::fstream file;
bool search()
{
    char container[20];
    bool found = false;
    file.open("info.bin", std::ios::binary | std::ios::in);
    i = 0;

    while (file.read(container, 20))
    {
        if (strcmp(container, player.name) == 0)
        {
            file.read(container, 20);
            if (strcmp(container, player.id) == 0)
            {
                found = true;
                break;
            }
        }
        i += sizeof(detail);
        file.seekg(i);
    }

    file.close();
    return found;
}

void login();
void signup_detail()
{
    std::cout << "Enter Name\n";
    std::cin.ignore();
    std::cin.getline(player.name, 20);
    std::cout << "Enter Password\n";
    std::cin >> player.id;
    player.prog.Tplays = 0;
    player.prog.Twins = 0;
    player.prog.time = 0;
    file.open("info.bin", std::ios::binary | std::ios::app | std::ios::out);
    file.write(player.name, 20);
    i = file.tellg();
    i -= 20;
    file.write(player.id, 20);
    file.write((char*)&player.prog.Tplays, 4);
    file.write((char*)&player.prog.Twins, 4);
    file.write((char*)&player.prog.time, 4);
    file.close();

}
void login_detail()
{
    std::cout << "Enter Name\n";
    std::cin.ignore();
    std::cin.getline(player.name, 20);
    std::cout << "Enter Password\n";
    std::cin >> player.id;
    if (!search())
        login();
    else
    {
        file.open("info.bin", std::ios::binary | std::ios::in);
        file.seekg(i + 40);
        file.read((char*)&player.prog.Tplays, 4);
        file.read((char*)&player.prog.Twins, 4);
        file.read((char*)&player.prog.time, 4);
        file.close();
    }
}
void login()
{
    system("cls");
    gotoRowCol(50, 50);
    signUpIn();
    int x = 0, y = 0;
    while (1)
    {
        getRowColbyAnyClick(x, y);
        if (x > 50 && x < 57)
            if ((y > 31 && y < 55) || (y > 76 && y < 100))
                break;
    }
    if (y > 31 && y < 55)
        signup_detail();
    else
        login_detail();
}
void cleanupMemory() {
    for (int i = 0; i < level.side; ++i) {
        delete[] level.ps[i];
    }
    delete[] level.ps;
    gameover = false;
    win = false;
    level.ps = nullptr;
    level.count = 0;
    level.side = 0;
    level.mine = 0;
    level.width = 0;
    level.startR = 0;
    level.startC = 0;
}
void detail_fill()
{
    file.open("info.bin", std::ios::binary | std::ios::in | std::ios::out);
    file.seekg(i + 40);
    file.write((char*)&player.prog.Tplays, 4);
    file.write((char*)&player.prog.Twins, 4);
    file.write((char*)&player.prog.time, 4);
    file.close();
}
int Time;
void Timee(long long startTime, bool& printtime, bool& stoptime)
{
    while (!stoptime)
    {
        Sleep(1000);
        int current = time(0);
        Time = current - startTime;
        if (printtime)
        {
            gotoRowCol(12, 50);
            SetClr(WHITE, LPURPLE);
            std::cout << std::setw(4) << Time;
        }
    }
}
void print()
{
    file.open("info.bin", std::ios::binary | std::ios::in | std::ios::out);
    system("cls");
    int fileSize, times = 0;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    fileSize /= 52;
    file.seekg(0, std::ios::beg);
    for (int t = 0; t < fileSize; t++)
    {
        file.read(player.name, 20);
        file.read(player.id, 20);
        file.read((char*)&player.prog.Tplays, 4);
        file.read((char*)&player.prog.Twins, 4);
        file.read((char*)&player.prog.time, 4);
        gotoRowCol(25, 50);
        std::cout << "Name " << std::setw(10) << "time";
        if (player.prog.time != 0)
        {
            gotoRowCol(30 + times, 50);
            std::cout << player.name << "         " << player.prog.time;
            times += 2;
        }

    }
    file.close();

}
void leadership()
{
    char a[52];
    char b[52];
    char t1[4];
    char t2[4];
    bool change;
    int fileSize;

    std::fstream file("info.bin", std::ios::binary | std::ios::in | std::ios::out);
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    do {
        change = false;
        file.seekg(0);
        for (int t = 0; t < fileSize - 52; t += sizeof(detail))
        {
            file.read(a, 52);
            file.seekg(-4, std::ios::cur);
            file.read(t1, 4);
            file.read(b, 52);
            file.seekg(-4, std::ios::cur);
            file.read(t2, 4);
            if (strcmp(t1, t2) > 0)
            {
                file.seekg(-104, std::ios::cur);
                file.write(b, 52);
                file.write(a, 52);
                change = true;
            }
            file.seekg(-52, std::ios::cur);
        }

    } while (change);
    file.close();
    print();
}

int main()
{
    login();
    char again = 'y';
    while (again == 'y' || again == 'Y') {
        SetClr(BLACK, LPURPLE);
        system("cls");
        gotoRowCol(0, 0);
        int startTime = time(0);
        bool stoptime = false;
        bool printtime = true;
        int choice = 0;
        choice = start();
        if (choice >= 0 && choice < 23)
            choice = 1;
        else if (choice > 40 && choice < 64)
            choice = 2;
        else
            choice = 3;
        set_game(choice);
        total_left = level.side * level.side;
        SetClr(WHITE, LPURPLE);
        system("cls");
        grid();
        std::thread timer(Timee, startTime, std::ref(printtime), std::ref(stoptime));
        while (!gameover && !win)
        {
            getClick();
            printtime = false;
            winCheck();
            SetClr(WHITE, LPURPLE);
            system("cls");
            grid();
            printtime = true;
        }
        stoptime = true;
        timer.join();
        if (gameover) {
            ++player.prog.Tplays;
            gotoRowCol(40, 40);
            SetClr(WHITE, LPURPLE);
            text(0, 0, 0, 0, 1);
        }
        if (win) {
            ++player.prog.Twins;
            ++player.prog.Tplays;
            if (player.prog.time > Time || player.prog.time == 0)
                player.prog.time = Time;
            gotoRowCol(40, 40);
            SetClr(WHITE, LPURPLE);
            text(0, 0, 0, 1, 0);
        }
        detail_fill();
        std::cout << "\n\n\nYou want to play again y/n\n\n\n";
        again = _getch();
        cleanupMemory();
    }
    leadership();
    std::cout << std::endl;
    system("pause");
    return 0;
}

////////////////////////////////////////////////////////////////////////////

void text(bool a, bool b, bool c, bool d, bool e)
{
    const char* bomb_pic = R"===(
                 %        ..                              
                 ***  *@: @ +*@:                          
               .-*  @    :    **@                         
            @@%   -   *#-...:#@%+@                        
               =. @:@+ .       -% *                       
               @   *@. @        %*@-*                     
              @      =+*     :*@@@@*@                     
             @:@:= + :     .@*.     @                     
             @   :# #-.     -       @                     
                 :=         -      -@.                    
                 @         @@@.     .@@@                  
                       -@@@.            @@                
                    #@@*.                *%               
                 =@@+                     -%              
               :@@:                        -@             
              #@.                    -@@    =@            
             @@                    *@@%%:    #@           
            @@                   -@@@%%@:     @:          
           :@  +                @@@@%@@@       @          
           @ : +@             :@@@@%%@@@       %=         
          @-    %+           %@@@@%+  .=        @         
         :@     ##          @@@@@.:@@ @         @:        
         @    .#-%        .@@@@     =@          +*        
        .@    @ :%       =@@@        +#         .@        
        *+   %. :#      #@@+          @          @        
        @    @ %.*  :  %@@            @.         @        
        @   ==#@ *  @ %@              %:         @        
        @   @ @@ @*%@@  @@            %:         @        
        @   @ @@ @.  + =@@@           @.         @        
        @   @=@@ @   # %@@@           @          @        
        @   @.@ @+   -.@@@@          .@          @        
        @   @  %@     %#@@@          @.         =#        
        #*  #@@@      % #%          @@          @:        
         @   .        -@          .@+=          @         
         @:            %@       +@@ %          *@         
          @             *@#@@@@@-  %           @          
          =@              @@.     .           %%          
           #@               =@@%              %           
            @*    .                  %@@=    @.           
             @= :   +@#               @ +   @=            
              @*      .@@:           .@    @+             
               *@=      @@@+         @    @=              
                 @@.     @@@@@%:  .#@   =@.               
                   @@-    #@@@@@@@@%   @@                 
                     @@%    :@@+   :: @%                  
                       %@%     @%:  @@:                   
                         =@@@@@@  =*#                     
                               *-   *@                    
                                %    @                    
                                @-  +@.                   
                                @     @                   
                                @  @@@@                   
                                %-@   @                   
                                .@.:#@.                   
                                 .=-.        )===";
    const char* text_pic = R"===(

   __  __  _                              _                            
  |  \/  |(_)                            (_)                           
  | \  / | _  _ __    ___  ___ __      __ _  _ __   _ __    ___  _ __  
  | |\/| || || '_ \  / _ \/ __|\ \ /\ / /| || '_ \ | '_ \  / _ \| '__| 
  | |  | || || | | ||  __/\__ \ \ V  V / | || |_) || |_) ||  __/| |    
  |_|  |_||_||_| |_| \___||___/  \_/\_/  |_|| .__/ | .__/  \___||_|    
                                            | |    | |                 
                                            |_|    |_|                 
)===";
    const char* text = R"===(
+---------------------+                  +---------------------+                     +---------------------+
|                     |                  |                     |                     |                     |
|        EASY         |                  |      MEDIUM         |                     |       HARD          |
|    Difficulty: 1    |                  |    Difficulty: 2    |                     |    Difficulty: 3    |
|                     |                  |                     |                     |                     |
+---------------------+                  +---------------------+                     +---------------------+
)===";
    const char* t1 = R"===(
__     __  ____   _    _  __          __ _____  _   _ 
\ \   / / / __ \ | |  | | \ \        / /|_   _|| \ | |
 \ \_/ / | |  | || |  | |  \ \  /\  / /   | |  |  \| |
  \   /  | |  | || |  | |   \ \/  \/ /    | |  | . ` |
   | |   | |__| || |__| |    \  /\  /    _| |_ | |\  |
   |_|    \____/  \____/      \/  \/    |_____||_| \_|                                                 
)===";
    const char* t2 = R"===(
__   __                    _                          
\ \ / /   ___    _   _    | |       ___    ___    ___ 
 \ V /   / _ \  | | | |   | |      / _ \  / __|  / _ \
  | |   | (_) | | |_| |   | |___  | (_) | \__ \ |  __/
  |_|    \___/   \__,_|   |_____|  \___/  |___/  \___|
)===";

    if (a)
        std::cout << bomb_pic;
    if (b)
        std::cout << text_pic;
    if (c)
        std::cout << text;
    if (d)
        std::cout << t1;
    if (e)
        std::cout << t2;
}

void signUpIn()
{
    const char* sign = R"===(
                                +---------------------+                      +---------------------+
                                |                     |                      |                     |
                                |      Sign Up        |                      |       Sign In       |
                                |        New          |                      |         Login       |
                                |                     |                      |                     |
                                +---------------------+                      +---------------------+
)===";
    SetClr(GREEN, BLACK);
    std::cout << sign;
}
