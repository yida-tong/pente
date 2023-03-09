#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <tuple>
#include <string>
#include <utility>
#include <unordered_map>
#include <unordered_set>

using namespace std;

unordered_map<string,int> scoreDef = {
        {"0a", 100000},
        {"1a", 1250},
        {"2a", 250},
        {"3a", 50},
        {"4a", 10},
        {"0", 100000},
        {"1", 10200},
        {"2", 5400},
        {"3", 2800},
        {"4", 1400},
        {"5", 700}
};

struct captured {
    int byWhite;
    int byBlack;
};

// node ds
struct node {
    tuple<int,int, char> posAdded;
    vector<tuple<int,int, char>> posGone;
    int depth;
    node* whoGive;
    captured capture;
    bool isRoot;
};


struct inputFile {
    char myTurn; // b or w
    int turnNum;
    double time;
    char board[19][19];
    captured capture;
};

int min_value(node*, int, int, int);
inputFile inputInfo {};
int meRadius = 1;
int opRadius = 1;
bool secWhite=false;

void process_input(inputFile* ptr) {
    ifstream input_file("input.txt");
    string scanLine;

    getline(input_file, scanLine);
    ptr->myTurn = scanLine=="BLACK"? 'b':'w';
    getline(input_file, scanLine);
    ptr->time = stod(scanLine);

    getline(input_file, scanLine,',');
    ptr->capture.byWhite = stoi(scanLine);
    getline(input_file, scanLine);
    ptr->capture.byBlack = stoi(scanLine);

    for (auto & k : ptr->board) {
        getline(input_file, scanLine);
        stringstream stream(scanLine);
        for (char & q : k) {
            stream >> q;
        }
    }
    input_file.close();

    fstream playDataFile;
    playDataFile.open("playdata.txt", fstream::in);
    if (playDataFile.is_open()) {
        getline(playDataFile, scanLine);
        ptr->turnNum=stoi(scanLine);
    } else {
        // first turn
        ptr->turnNum = 1;
    }
    playDataFile.close();
    playDataFile.open("playdata.txt", fstream::out);
    playDataFile << ptr->turnNum+1;
    playDataFile.close();
}

bool inRange(int x, int y) {
    return x>=0 && x<19 && y>=0 && y<19;
}

// children generation
vector<node*> children(node* curNode, bool isOppo) {
    char me;
    char oppo;

    if (isOppo) {
        oppo = inputInfo.myTurn;
        me = oppo=='b'? 'w': 'b';
    } else {
        me = inputInfo.myTurn;
        oppo = me=='b'? 'w': 'b';
    }

    unordered_set<string> pool;

    if (secWhite) {
        for (int i=6;i<=12;++i) {
            if (i==6 || i==12) {
                for (int j=6;j<=12;j++) {
                    pool.insert(to_string(i)+"-"+ to_string(j));
                }
            } else {
                pool.insert(to_string(i)+"-"+ to_string(6));
                pool.insert(to_string(i)+"-"+ to_string(12));
            }
        }
        secWhite=false;
    } else {
        for (int row=0;row<19;++row) {
            for (int col=0;col<19;++col) {
                char cell = inputInfo.board[row][col];
                if (cell==me) {
                    for (int x=row-meRadius;x<=row+meRadius;++x) {
                        for (int y=col-meRadius;y<=col+meRadius;++y) {
                            if (inRange(x,y) && inputInfo.board[x][y]=='.') {
                                pool.insert(to_string(x)+"-"+ to_string(y));
                            }
                        }
                    }
                } else if (cell==oppo) {
                    for (int x=row-opRadius;x<=row+opRadius;++x) {
                        for (int y=col-opRadius;y<=col+opRadius;++y) {
                            if (inRange(x,y) && inputInfo.board[x][y]=='.') {
                                pool.insert(to_string(x)+"-"+ to_string(y));
                            }
                        }
                    }
                }
            }
        }
    }

    vector<node*> childList;
    for ( auto it = pool.begin(); it != pool.end(); ++it ) {
        tuple<int, int, char> newCoord;
        string temp;
        stringstream st(*it);
        getline(st, temp,'-');
        get<0>(newCoord) = stoi(temp);
        getline(st, temp);
        get<1>(newCoord) = stoi(temp);
        get<2>(newCoord) = me;

        vector<tuple<int,int, char>> goneList;
        
        int capByWhite=0;
        int capByBlack=0;
        
        if (
                inRange(get<0>(newCoord)-1, get<1>(newCoord)) &&
                inRange(get<0>(newCoord)-2, get<1>(newCoord)) &&
                inRange(get<0>(newCoord)-3, get<1>(newCoord)) &&
                inputInfo.board[get<0>(newCoord)-1][get<1>(newCoord)]==oppo &&
                inputInfo.board[get<0>(newCoord)-2][get<1>(newCoord)]==oppo &&
                inputInfo.board[get<0>(newCoord)-3][get<1>(newCoord)]==me
                ) 
        {
            goneList.push_back({get<0>(newCoord)-1, get<1>(newCoord), oppo});
            goneList.push_back({get<0>(newCoord)-2, get<1>(newCoord), oppo});
            if (oppo=='w') {
                capByBlack+=2;
            } else {
                capByWhite+=2;
            }
        }

        if (
                inRange(get<0>(newCoord)+1, get<1>(newCoord)) &&
                inRange(get<0>(newCoord)+2, get<1>(newCoord)) &&
                inRange(get<0>(newCoord)+3, get<1>(newCoord)) &&
                inputInfo.board[get<0>(newCoord)+1][get<1>(newCoord)]==oppo &&
                inputInfo.board[get<0>(newCoord)+2][get<1>(newCoord)]==oppo &&
                inputInfo.board[get<0>(newCoord)+3][get<1>(newCoord)]==me
                )
        {
            goneList.push_back({get<0>(newCoord)+1, get<1>(newCoord), oppo});
            goneList.push_back({get<0>(newCoord)+2, get<1>(newCoord), oppo});
            if (oppo=='w') {
                capByBlack+=2;
            } else {
                capByWhite+=2;
            }
        }

        if (
                inRange(get<0>(newCoord), get<1>(newCoord)-1) &&
                inRange(get<0>(newCoord), get<1>(newCoord)-2) &&
                inRange(get<0>(newCoord), get<1>(newCoord)-3) &&
                inputInfo.board[get<0>(newCoord)][get<1>(newCoord)-1]==oppo &&
                inputInfo.board[get<0>(newCoord)][get<1>(newCoord)-2]==oppo &&
                inputInfo.board[get<0>(newCoord)][get<1>(newCoord)-3]==me
                )
        {
            goneList.push_back({get<0>(newCoord), get<1>(newCoord)-1, oppo});
            goneList.push_back({get<0>(newCoord), get<1>(newCoord)-2, oppo});
            if (oppo=='w') {
                capByBlack+=2;
            } else {
                capByWhite+=2;
            }
        }
        
        if (
                inRange(get<0>(newCoord), get<1>(newCoord)+1) &&
                inRange(get<0>(newCoord), get<1>(newCoord)+2) &&
                inRange(get<0>(newCoord), get<1>(newCoord)+3) &&
                inputInfo.board[get<0>(newCoord)][get<1>(newCoord)+1]==oppo &&
                inputInfo.board[get<0>(newCoord)][get<1>(newCoord)+2]==oppo &&
                inputInfo.board[get<0>(newCoord)][get<1>(newCoord)+3]==me
                )
        {
            goneList.push_back({get<0>(newCoord), get<1>(newCoord)+1, oppo});
            goneList.push_back({get<0>(newCoord), get<1>(newCoord)+2, oppo});
            if (oppo=='w') {
                capByBlack+=2;
            } else {
                capByWhite+=2;
            }
        }

        if (
                inRange(get<0>(newCoord)-1, get<1>(newCoord)-1) &&
                inRange(get<0>(newCoord)-2, get<1>(newCoord)-2) &&
                inRange(get<0>(newCoord)-3, get<1>(newCoord)-3) &&
                inputInfo.board[get<0>(newCoord)-1][get<1>(newCoord)-1]==oppo &&
                inputInfo.board[get<0>(newCoord)-2][get<1>(newCoord)-2]==oppo &&
                inputInfo.board[get<0>(newCoord)-3][get<1>(newCoord)-3]==me
                )
        {
            goneList.push_back({get<0>(newCoord)-1, get<1>(newCoord)-1, oppo});
            goneList.push_back({get<0>(newCoord)-2, get<1>(newCoord)-2, oppo});
            if (oppo=='w') {
                capByBlack+=2;
            } else {
                capByWhite+=2;
            }
        }

        if (
                inRange(get<0>(newCoord)-1, get<1>(newCoord)+1) &&
                inRange(get<0>(newCoord)-2, get<1>(newCoord)+2) &&
                inRange(get<0>(newCoord)-3, get<1>(newCoord)+3) &&
                inputInfo.board[get<0>(newCoord)-1][get<1>(newCoord)+1]==oppo &&
                inputInfo.board[get<0>(newCoord)-2][get<1>(newCoord)+2]==oppo &&
                inputInfo.board[get<0>(newCoord)-3][get<1>(newCoord)+3]==me
                )
        {
            goneList.push_back({get<0>(newCoord)-1, get<1>(newCoord)+1, oppo});
            goneList.push_back({get<0>(newCoord)-2, get<1>(newCoord)+2, oppo});
            if (oppo=='w') {
                capByBlack+=2;
            } else {
                capByWhite+=2;
            }
        }

        if (
                inRange(get<0>(newCoord)+1, get<1>(newCoord)-1) &&
                inRange(get<0>(newCoord)+2, get<1>(newCoord)-2) &&
                inRange(get<0>(newCoord)+3, get<1>(newCoord)-3) &&
                inputInfo.board[get<0>(newCoord)+1][get<1>(newCoord)-1]==oppo &&
                inputInfo.board[get<0>(newCoord)+2][get<1>(newCoord)-2]==oppo &&
                inputInfo.board[get<0>(newCoord)+3][get<1>(newCoord)-3]==me
                )
        {
            goneList.push_back({get<0>(newCoord)+1, get<1>(newCoord)-1, oppo});
            goneList.push_back({get<0>(newCoord)+2, get<1>(newCoord)-2, oppo});
            if (oppo=='w') {
                capByBlack+=2;
            } else {
                capByWhite+=2;
            }
        }
        
        if (
                inRange(get<0>(newCoord)+1, get<1>(newCoord)+1) &&
                inRange(get<0>(newCoord)+2, get<1>(newCoord)+2) &&
                inRange(get<0>(newCoord)+3, get<1>(newCoord)+3) &&
                inputInfo.board[get<0>(newCoord)+1][get<1>(newCoord)+1]==oppo &&
                inputInfo.board[get<0>(newCoord)+2][get<1>(newCoord)+2]==oppo &&
                inputInfo.board[get<0>(newCoord)+3][get<1>(newCoord)+3]==me
                )
        {
            goneList.push_back({get<0>(newCoord)+1, get<1>(newCoord)+1, oppo});
            goneList.push_back({get<0>(newCoord)+2, get<1>(newCoord)+2, oppo});
            if (oppo=='w') {
                capByBlack+=2;
            } else {
                capByWhite+=2;
            }
        }

        node* child = new node (
                {
                    newCoord,
                    goneList,
                    curNode->depth+1,
                    nullptr,
                    {
                        curNode->capture.byWhite+capByWhite,
                        curNode->capture.byBlack+capByBlack,
                    },
                    false,
                });
        childList.push_back(child);
    }
    return childList;
}

// goal
bool isGoal(node* curNode) {
    if (curNode->isRoot) return false;
    if (curNode->capture.byWhite>=10 || curNode->capture.byBlack>=10) return true;
    int x = get<0>(curNode->posAdded);
    int y = get<1>(curNode->posAdded);
    char c = get<2>(curNode->posAdded);

    // vert
    int t_c=0;
    for (int k=x-1; inRange(k,y)&&inputInfo.board[k][y]==c;--k) {
        t_c++;
    }

    int b_c=0;
    for (int k=x+1;inRange(k,y)&&inputInfo.board[k][y]==c;++k) {
        b_c++;
    }
    if (t_c+b_c+1>=5) return true;
    // horiz
    int l_c=0;
    for (int k=y-1; inRange(x,k)&&inputInfo.board[x][k]==c;--k) {
        l_c++;
    }

    int r_c=0;
    for (int k=y+1;inRange(x,k)&&inputInfo.board[x][k]==c;++k) {
        r_c++;
    }
    if (l_c+r_c+1>=5) return true;

    // first dia
    int tl_c=0;
    for (int k1=x-1,k2=y-1; inRange(k1,k2)&&inputInfo.board[k1][k2]==c;--k1,--k2) {
        tl_c++;
    }

    int br_c=0;
    for (int k1=x+1,k2=y+1; inRange(k1,k2)&&inputInfo.board[k1][k2]==c;++k1,++k2) {
        br_c++;
    }
    if (tl_c+br_c+1>=5) return true;

    // second dia
    int tr_c=0;
    for (int k1=x-1,k2=y+1; inRange(k1,k2)&&inputInfo.board[k1][k2]==c;--k1,++k2) {
        tr_c++;
    }

    int bl_c=0;
    for (int k1=x+1,k2=y-1; inRange(k1,k2)&&inputInfo.board[k1][k2]==c;++k1,--k2) {
        bl_c++;
    }
    if (tr_c+bl_c+1>=5) return true;

    return false;
}

void updateBoard(node* curNode, bool reverse) {
    if (curNode->isRoot) return;
    if (!reverse) {
        inputInfo.board[get<0>(curNode->posAdded)][get<1>(curNode->posAdded)] = get<2>(curNode->posAdded);
        for (auto & k:curNode->posGone) {
            inputInfo.board[get<0>(k)][get<1>(k)]='.';
        }
    } else {
        inputInfo.board[get<0>(curNode->posAdded)][get<1>(curNode->posAdded)] = '.';
        for (auto & k:curNode->posGone) {
            inputInfo.board[get<0>(k)][get<1>(k)]=get<2>(k);
        }
    }
}

int hScan(vector<vector<char>> &board, int away, char who) {
    int cnt=0;
    for (int i=0;i<19;++i) {
        for (int lPtr=0,rPtr=4;rPtr<19;lPtr++,rPtr++) {
            int holeCNT=0;
            bool legal = true;
            for (int mPtr=lPtr;mPtr<=rPtr;mPtr++) {
                if (board[i][mPtr]=='.') {
                    holeCNT++;
                }
                if (board[i][mPtr]!=who && board[i][mPtr]!='.') {legal=false;break;}
            }
            if (legal&& holeCNT==away) {
                cnt++;
                for (int mPtr=lPtr;mPtr<=rPtr;mPtr++) {
                    if (board[i][mPtr]=='.') board[i][mPtr]='O';
                }
            }
        }
    }
    return cnt;
}

int vScan(vector<vector<char>> &board, int away, char who) {
    int cnt=0;
    for (int j=0;j<19;++j) {
        for (int lPtr=0,rPtr=4;rPtr<19;lPtr++,rPtr++) {
            int holeCNT=0;
            bool legal = true;
            for (int mPtr=lPtr;mPtr<=rPtr;mPtr++) {
                if (board[mPtr][j]=='.') {
                    holeCNT++;
                }
                if (board[mPtr][j]!=who && board[mPtr][j]!='.') {legal=false;break;}
            }
            if (legal&& holeCNT==away) {
                cnt++;
                for (int mPtr=lPtr;mPtr<=rPtr;mPtr++) {
                    if (board[mPtr][j]=='.') board[mPtr][j]='O';
                }
            }
        }
    }
    return cnt;
}

int d1Scan(vector<vector<char>> &board, int away, char who) {
    int cnt=0;
    for (int j=0;j<19;++j) {
        for (int lPtr_x=0,lPtr_y=j,rPtr_x=4, rPtr_y=j+4; inRange(rPtr_x, rPtr_y);lPtr_x++,lPtr_y++,rPtr_x++,rPtr_y++) {
            int holeCNT=0;
            bool legal = true;
            for (int mPtr_x=lPtr_x,mPtr_y=lPtr_y;mPtr_y<=rPtr_y;mPtr_x++,mPtr_y++) {
                if (board[mPtr_x][mPtr_y]=='.') {
                    holeCNT++;
                }
                if (board[mPtr_x][mPtr_y]!=who && board[mPtr_x][mPtr_y]!='.') {legal=false;break;}
            }

            if (legal&& holeCNT==away) {
                cnt++;
                for (int mPtr_x=lPtr_x,mPtr_y=lPtr_y;mPtr_y<=rPtr_y;mPtr_x++,mPtr_y++) {
                    if (board[mPtr_x][mPtr_y]=='.') board[mPtr_x][mPtr_y]='O';
                }
            }
        }
    }

    for (int i=1;i<19;++i) {
        for (int lPtr_x=i,lPtr_y=0,rPtr_x=i+4, rPtr_y=4; inRange(rPtr_x, rPtr_y);lPtr_x++,lPtr_y++,rPtr_x++,rPtr_y++) {
            int holeCNT=0;
            bool legal = true;
            for (int mPtr_x=lPtr_x,mPtr_y=lPtr_y;mPtr_y<=rPtr_y;mPtr_x++,mPtr_y++) {
                if (board[mPtr_x][mPtr_y]=='.') {
                    holeCNT++;
                }
                if (board[mPtr_x][mPtr_y]!=who && board[mPtr_x][mPtr_y]!='.') {legal=false;break;}
            }

            if (legal&& holeCNT==away) {
                cnt++;
                for (int mPtr_x=lPtr_x,mPtr_y=lPtr_y;mPtr_y<=rPtr_y;mPtr_x++,mPtr_y++) {
                    if (board[mPtr_x][mPtr_y]=='.') board[mPtr_x][mPtr_y]='O';
                }
            }
        }
    }
    return cnt;
}

int d2Scan(vector<vector<char>> &board, int away, char who) {
    int cnt=0;
    for (int j=0;j<19;++j) {
        for (int lPtr_x=0,lPtr_y=j,rPtr_x=4, rPtr_y=j-4; inRange(rPtr_x, rPtr_y);lPtr_x++,lPtr_y--,rPtr_x++,rPtr_y--) {
            int holeCNT=0;
            bool legal = true;
            for (int mPtr_x=lPtr_x,mPtr_y=lPtr_y;mPtr_x<=rPtr_x;mPtr_x++,mPtr_y--) {
                if (board[mPtr_x][mPtr_y]=='.') {
                    holeCNT++;
                }
                if (board[mPtr_x][mPtr_y]!=who && board[mPtr_x][mPtr_y]!='.') {legal=false;break;}
            }

            if (legal&& holeCNT==away) {
                cnt++;
                for (int mPtr_x=lPtr_x,mPtr_y=lPtr_y;mPtr_x<=rPtr_x;mPtr_x++,mPtr_y--) {
                    if (board[mPtr_x][mPtr_y]=='.') board[mPtr_x][mPtr_y]='O';
                }
            }
        }
    }

    for (int i=1;i<19;++i) {
        for (int lPtr_x=i,lPtr_y=18,rPtr_x=i+4, rPtr_y=14; inRange(rPtr_x, rPtr_y);lPtr_x++,lPtr_y--,rPtr_x++,rPtr_y--) {
            int holeCNT=0;
            bool legal = true;
            for (int mPtr_x=lPtr_x,mPtr_y=lPtr_y;mPtr_x<=rPtr_x;mPtr_x++,mPtr_y--) {
                if (board[mPtr_x][mPtr_y]=='.') {
                    holeCNT++;
                }
                if (board[mPtr_x][mPtr_y]!=who && board[mPtr_x][mPtr_y]!='.') {legal=false;break;}
            }

            if (legal&& holeCNT==away) {
                cnt++;
                for (int mPtr_x=lPtr_x,mPtr_y=lPtr_y;mPtr_x<=rPtr_x;mPtr_x++,mPtr_y--) {
                    if (board[mPtr_x][mPtr_y]=='.') board[mPtr_x][mPtr_y]='O';
                }
            }
        }
    }
    return cnt;
}

int eval(node* curNode) {
    char me = inputInfo.myTurn;
    char oppo = me=='b'? 'w': 'b';

    int capByMe;
    int capByOppo;

    if (me=='w') {
        capByMe = curNode->capture.byWhite;
        capByOppo = curNode->capture.byBlack;
    } else {
        capByMe = curNode->capture.byBlack;
        capByOppo = curNode->capture.byWhite;
    }

    if (capByMe>10) {
        capByMe = 10;
    }

    if (capByOppo>10) {
        capByOppo = 10;
    }

    int capByMeToWin = -capByMe/2+5;
    int capByOppoToWin = -capByOppo/2+5;


    vector<vector<char>> boardOne;
    vector<vector<char>> boardTwo;


    for (auto & i : inputInfo.board) {
        vector<char> tep;
        for (char j : i) {
            tep.push_back(j);
        }
        boardOne.push_back(tep);
        boardTwo.push_back(tep);
    }

    // calc 0 away
    int zeroAwayMe = 0;
    int zeroAwayOppo = 0;

    zeroAwayMe+=hScan(boardOne, 0, me);
    zeroAwayOppo+=hScan(boardTwo, 0, oppo);

    zeroAwayMe+=vScan(boardOne, 0, me);
    zeroAwayOppo+=vScan(boardTwo, 0, oppo);

    zeroAwayMe+=d1Scan(boardOne, 0, me);
    zeroAwayOppo+=d1Scan(boardTwo, 0, oppo);

    zeroAwayMe+=d2Scan(boardOne, 0, me);
    zeroAwayOppo+=d2Scan(boardTwo, 0, oppo);

    // calc 1 away
    int oneAwayMe = 0;
    int oneAwayOppo = 0;

    oneAwayMe+=hScan(boardOne, 1, me);
    oneAwayOppo+=hScan(boardTwo, 1, oppo);

    oneAwayMe+=vScan(boardOne, 1, me);
    oneAwayOppo+=vScan(boardTwo, 1, oppo);

    oneAwayMe+=d1Scan(boardOne, 1, me);
    oneAwayOppo+=d1Scan(boardTwo, 1, oppo);

    oneAwayMe+=d2Scan(boardOne, 1, me);
    oneAwayOppo+=d2Scan(boardTwo, 1, oppo);

    // calc 2 away
    int twoAwayMe = 0;
    int twoAwayOppo = 0;

    twoAwayMe+=hScan(boardOne, 2, me);
    twoAwayOppo+=hScan(boardTwo, 2, oppo);

    twoAwayMe+=vScan(boardOne, 2, me);
    twoAwayOppo+=vScan(boardTwo, 2, oppo);

    twoAwayMe+=d1Scan(boardOne, 2, me);
    twoAwayOppo+=d1Scan(boardTwo, 2, oppo);

    twoAwayMe+=d2Scan(boardOne, 2, me);
    twoAwayOppo+=d2Scan(boardTwo, 2, oppo);


    // calc 3 away
    int threeAwayMe = 0;
    int threeAwayOppo = 0;

    threeAwayMe+=hScan(boardOne, 3, me);
    threeAwayOppo+=hScan(boardTwo, 3, oppo);

    threeAwayMe+=vScan(boardOne, 3, me);
    threeAwayOppo+=vScan(boardTwo, 3, oppo);

    threeAwayMe+=d1Scan(boardOne, 3, me);
    threeAwayOppo+=d1Scan(boardTwo, 3, oppo);

    threeAwayMe+=d2Scan(boardOne, 3, me);
    threeAwayOppo+=d2Scan(boardTwo, 3, oppo);

    // calc 4 away
    int fourAwayMe = 0;
    int fourAwayOppo = 0;

    fourAwayMe+=hScan(boardOne, 4, me);
    fourAwayOppo+=hScan(boardTwo, 4, oppo);

    fourAwayMe+=vScan(boardOne, 4, me);
    fourAwayOppo+=vScan(boardTwo, 4, oppo);

    fourAwayMe+=d1Scan(boardOne, 4, me);
    fourAwayOppo+=d1Scan(boardTwo, 4, oppo);

    fourAwayMe+=d2Scan(boardOne, 4, me);
    fourAwayOppo+=d2Scan(boardTwo, 4, oppo);

    int scoreMe = scoreDef[to_string(capByMeToWin)]+scoreDef["0a"]*zeroAwayMe+scoreDef["1a"]*oneAwayMe+scoreDef["2a"]*twoAwayMe+scoreDef["3a"]*threeAwayMe+scoreDef["4a"]*fourAwayMe;
    int scoreOppo = scoreDef[to_string(capByOppoToWin)]+scoreDef["0a"]*zeroAwayOppo+scoreDef["1a"]*oneAwayOppo+scoreDef["2a"]*twoAwayOppo+scoreDef["3a"]*threeAwayOppo+scoreDef["4a"]*fourAwayOppo;
    return scoreMe-scoreOppo;
}

int max_value(node* nodePtr, int alpha, int beta, int depth) {
    updateBoard(nodePtr, false);
    if (isGoal(nodePtr) || nodePtr->depth==depth) {
        nodePtr->whoGive = nodePtr;
        int src = eval(nodePtr);
        updateBoard(nodePtr, true);
        return src;
    }

    int v = -2147483647;
    for (node* childPtr: children(nodePtr, false)) {
        int new_v = min_value(childPtr, alpha, beta, depth);
        if (new_v>v) {
            v = new_v;
            nodePtr->whoGive = childPtr;
        }
        if (v>=beta) {
            updateBoard(nodePtr, true);
            return v;
        }
        alpha = max(alpha, v);
    }
    updateBoard(nodePtr, true);
    return v;
}

int min_value(node* nodePtr, int alpha, int beta, int depth) {
    updateBoard(nodePtr, false);
    if (isGoal(nodePtr) || nodePtr->depth==depth) {
        nodePtr->whoGive = nodePtr;
        int src = eval(nodePtr);
        updateBoard(nodePtr, true);
        return src;
    }

    int v = 2147483647;
    for (node* childPtr: children(nodePtr, true)) {
        int new_v = max_value(childPtr, alpha, beta, depth);
        if (new_v<v) {
            v = new_v;
            nodePtr->whoGive = childPtr;
        }
        if (v<=alpha) {
            updateBoard(nodePtr, true);
            return v;
        }
        beta = min(beta, v);
    }
    updateBoard(nodePtr, true);
    return v;
}

// depth >=1
pair<int, int> alphaBetaSearch(int depth) {
    node root = {
            {},
            {},
            0,
            nullptr,
            inputInfo.capture,
            true
    };
    max_value(&root, -2147483647, 2147483647, depth);
    return {get<0>(root.whoGive->posAdded), get<1>(root.whoGive->posAdded)};
}

int main() {
    process_input(&inputInfo);
    pair<int, int> res;

    // special case
    if (inputInfo.turnNum==1&&inputInfo.myTurn=='w') {
        res = {9,9};
    } else if (inputInfo.turnNum==1&&inputInfo.myTurn=='b') {
        res = {10,10};
    } else {
        if (inputInfo.turnNum==2&&inputInfo.myTurn=='w') {
            secWhite = true;
        }

        if (inputInfo.time<20) {
            res = alphaBetaSearch(2);
        } else {
            if (inputInfo.turnNum<=8) {
                res = alphaBetaSearch(4);
            } else if (inputInfo.turnNum<=15) {
                res = alphaBetaSearch(3);
            } else {
                res = alphaBetaSearch(2);
            }
        }
    }

    unordered_map<int, char> infoT = {
            {0, 'A'},
            {1, 'B'},
            {2, 'C'},
            {3, 'D'},
            {4, 'E'},
            {5, 'F'},
            {6, 'G'},
            {7, 'H'},
            {8, 'J'},
            {9, 'K'},
            {10, 'L'},
            {11, 'M'},
            {12, 'N'},
            {13, 'O'},
            {14, 'P'},
            {15, 'Q'},
            {16, 'R'},
            {17, 'S'},
            {18, 'T'},
    };

    ofstream o_file("output.txt");
    o_file << 19-res.first << infoT[res.second];
    o_file.close();
    return 0;
}