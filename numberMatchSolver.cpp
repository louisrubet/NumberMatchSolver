#include <algorithm>
#include <getopt.h>
#include <iostream>
#include <set>
#include <string>
#include <vector>
using namespace std;

static const char TERM_INVERSE[] = "\x1b[7m";
static const char TERM_RESET[] = "\x1b[0m";

class NumberMatch {
  public:
    NumberMatch(int lineLength = 9, int add = 4, bool stopAtFirstSuccess = true, bool diagonals = true)
        : _lineLength(lineLength), _add(add), _stopAtFirstSuccess(stopAtFirstSuccess), _diagonals(diagonals) {}

    bool fromString(string& input) {
        for_each(input.begin(), input.end(), [&](const char& c) {
            if (c >= '0' && c <= '9')
                _grid.push_back(c - '0');
            else
                _grid.push_back(0);
        });
        return true;
    }

    struct Cut {
        int firstIdx;
        int secondIdx;
    };

    vector<Cut>& cuts() { return _cuts; }

    void show(Cut cut = {-1, -1}) {
        for (int i = 0; i < _grid.size(); i++) {
            if (i == cut.firstIdx || i == cut.secondIdx)
                cout << TERM_INVERSE;
            if (_grid[i] != 0)
                cout << _grid[i];
            else
                cout << '.';
            if (i == cut.firstIdx || i == cut.secondIdx)
                cout << TERM_RESET;
            if (i > 0 && ((i + 1) % _lineLength) == 0)
                cout << endl;
        }
        if ((_grid.size() % _lineLength) != 0)
            cout << endl;
    }

    bool play(Cut cut) {
        // play: disable cut indexes
        if (cut.firstIdx != -1 && cut.secondIdx != -1) {
            _grid[cut.firstIdx] = 0;
            _grid[cut.secondIdx] = 0;
        } else if (_add > 0) {
            // or duplicate the grid (cut -1,-1 is magic)
            size_t imax = _grid.size();
            for (size_t i = 0; i < imax; i++)
                if (_grid[i] != 0)
                    _grid.push_back(_grid[i]);
            _add--;
        }

        // clear lines
        bool lineCleared;
        do {
            size_t j;
            lineCleared = false;
            for (size_t i = 0; i < _grid.size(); i += (size_t)_lineLength) {
                size_t jMax = i + _lineLength < _grid.size() ? i + _lineLength : _grid.size();
                for (j = i; j < jMax && _grid[j] == 0; j++)
                    ;
                if (j == jMax) { // this line is full of disabled boxes -> clear it
                    _grid.erase(_grid.begin() + i, _grid.begin() + jMax);
                    lineCleared = true;
                }
            }
        } while (lineCleared);

        if (any_of(_grid.cbegin(), _grid.cend(), [](const int& entry) {
            return entry != 0; }))
            return false; // game is not successful
        return true;          // all entries are disabled or grid is empty ->success!
    }

    bool areEqual(int index1, int index2) {
        return (_grid[index2] != 0 && (_grid[index1] == _grid[index2] || _grid[index1] + _grid[index2] == 10));
    }

    void findCuts(vector<Cut>& found) {
        int first = 0;

        // find next enabled box
        while (first < _grid.size() - 1) {
            for (; _grid[first] == 0 && first < _grid.size() - 1; first++)
                ;
            if (first == _grid.size() - 1)
                break;

            // find horizontal box
            int second;
            for (second = first + 1; second < _grid.size() && _grid[second] == 0; second++)
                ;
            if (second < _grid.size() && areEqual(first, second))
                found.push_back({first, second});

            // find vertical box
            for (second = first + _lineLength; second < _grid.size() && _grid[second] == 0; second += _lineLength)
                ;
            if (second < _grid.size() && areEqual(first, second))
                found.push_back({first, second});

            if (_diagonals) {
                // find diagonal right box
                if (first % _lineLength != _lineLength - 1) {
                    bool preventWrap = false;
                    for (second = first + _lineLength + 1; second < _grid.size() && _grid[second] == 0;
                         second += _lineLength + 1)
                        if ((second + 1) % _lineLength == 0) {
                            preventWrap = true;
                            break;
                        }
                    if (!preventWrap && second < _grid.size() && areEqual(first, second))
                        found.push_back({first, second});
                }

                // find diagonal left box
                if (first % _lineLength != 0) {
                    bool preventWrap = false;
                    for (second = first + _lineLength - 1; second < _grid.size() && _grid[second] == 0;
                         second += _lineLength - 1) {
                        if ((second - 1) % _lineLength == _lineLength - 1) {
                            preventWrap = true;
                            break;
                        }
                    }
                    if (!preventWrap && second < _grid.size() && areEqual(first, second))
                        found.push_back({first, second});
                }
            }

            first++;
        }

        // nothing found? A grid duplication is still possible
        if (found.size() == 0)
            found.push_back({-1, -1});
    }

    void solve(vector<NumberMatch>& winningGames) {
        if (NumberMatch::s_stopRecurrence)
            return;

        // find a cut
        vector<Cut> cuts;
        findCuts(cuts);
        for (auto& cut : cuts) {
            NumberMatch next = *this;  // duplicate the grid
            next._cuts.push_back(cut); // add this cut to the cuts history
            bool playResult = next.play(cut);
            if (NumberMatch::s_playedGrids.find(next._grid) != NumberMatch::s_playedGrids.end())
                continue; // trash if already played
            else
                s_playedGrids.insert(next._grid);
            // finished, keep winning game
            if (playResult) {
                winningGames.push_back(next);
                cout << "Found " << winningGames.size() << " winning game" << (winningGames.size() > 1 ? "s" : "")
                     << endl
                     << endl;
                if (next._stopAtFirstSuccess)
                    NumberMatch::s_stopRecurrence = true;
            } else
                next.solve(winningGames);
        }
    }

  private:
    const int _lineLength;
    vector<int> _grid;
    int _add;
    bool _stopAtFirstSuccess;
    bool _diagonals;
    vector<Cut> _cuts; // access to the cuts which lead to this game

    static set<vector<int>> s_playedGrids;
    static bool s_stopRecurrence;
};

set<vector<int>> NumberMatch::s_playedGrids;
bool NumberMatch::s_stopRecurrence = false;

void syntax() {
    cout << endl << "NumberMatchSolver" << endl << endl;
    cout << "syntax: numberMatchSolver [options] <grid>" << endl;
    cout << "Options can be:" << endl;
    cout << "-o, --one-cut: find the first playable cut only" << endl;
    cout << "-c, --continue: try to find all winning sequences (can be very long)" << endl;
    cout << "-i, --interactive: ask user for next step" << endl;
    cout << "-l, --line-length <line length>: set the grid line length, must be >=2 (default 9)" << endl;
    cout << "-a, --add <num>: max times numbers can be added, must be >=0 (default 4)" << endl;
    cout << "-d, --diagonals: make diagonal numbers match too, can be y or n (default y)" << endl;
    cout << "-h, --help: this help message" << endl;
    cout << endl << "ex: ./numberMatchSolver --interactive 262557596931762454465213897" << endl;
}

int main(int argc, char* argv[]) {
    bool oneCut = false;
    bool continueGame = false;
    bool interactive = false;
    int lineLength = 9;
    int add = 4;
    bool diagonals = true;

    struct option long_options[] = {
        {"one-cut", no_argument, nullptr, 'o'},     {"continue", no_argument, nullptr, 'c'},
        {"interactive", no_argument, nullptr, 'i'}, {"line-length", required_argument, nullptr, 'l'},
        {"add", required_argument, nullptr, 'a'},   {"diagonals", no_argument, nullptr, 'd'},
        {"help", no_argument, nullptr, 'h'},        {nullptr, 0, nullptr, 0}};

    bool parse = true;
    while (parse) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "ocil:n:d:h", long_options, &option_index);
        switch (c) {
        case 'h':
            syntax();
            return EXIT_SUCCESS;
        case 'o':
            oneCut = true;
            break;
        case 'c':
            continueGame = true;
            break;
        case 'd':
            if (string(optarg) == "y")
                diagonals = true;
            else if (string(optarg) == "n")
                diagonals = false;
            else {
                syntax();
                return EXIT_FAILURE;
            }
            break;
        case 'i':
            interactive = true;
            break;
        case 'l':
            lineLength = atoi(optarg);
            if (lineLength < 2) {
                syntax();
                return EXIT_FAILURE;
            }
            break;
        case 'a':
            add = atoi(optarg);
            if (add < 0) {
                syntax();
                return EXIT_FAILURE;
            }
            break;
        case -1:
            parse = false;
            break;
        default:
            syntax();
            return EXIT_FAILURE;
        }
    }

    if (optind == argc) {
        // missing mandatory grid init
        syntax();
        return EXIT_FAILURE;
    }

    NumberMatch numberMatch(lineLength, add, !continueGame, diagonals);
    string strTray(argv[argc - 1]);
    if (!numberMatch.fromString(strTray)) {
        syntax();
        return EXIT_FAILURE;
    }

    if (oneCut) {
        // find and show just one cut
        vector<NumberMatch::Cut> cuts;
        bool oneCutAtLeast = false;

        numberMatch.findCuts(cuts);
        for (auto& cut : cuts) {
            if (cut.firstIdx != -1 && cut.secondIdx != -1) { // cut -1,-1 means duplication
                cout << "A cut was found at " << cut.firstIdx + 1 << " and " << cut.secondIdx + 1 << endl;
                oneCutAtLeast = true;
                numberMatch.show(cut);
            }
        }
        if (!oneCutAtLeast) {
            cout << "No cut found" << endl;
            numberMatch.show();
        }
    } else {
        // find winning cut sequences
        vector<NumberMatch> winningGames;
        numberMatch.solve(winningGames);

        if (winningGames.size() == 0)
            cout << endl << "No winning cut sequences found" << endl;
        else {
            // show them all
            int gameIdx = 1;
            for (auto& match : winningGames) {
                NumberMatch game = numberMatch;
                cout << "Game " << gameIdx << endl << endl;
                gameIdx++;
                for (auto& cut : match.cuts()) {
                    game.show(cut);
                    game.play(cut);
                    if (interactive) {
                        if (cut.firstIdx == -1 && cut.secondIdx == -1)
                            cout << "[duplicate]" << endl;
                        cout << "[press enter]" << endl;
                        string input;
                        getline(cin, input);
                    } else
                        cout << endl;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
