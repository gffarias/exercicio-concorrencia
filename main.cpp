#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>
#include <iostream>
#include <list>
#include <vector>
#include <exception>
#include <string>
#include <set>
#include <algorithm>
#include <functional>
#include <random>
#include <chrono>

std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
std::uniform_int_distribution<int> uid;

enum Prompt {
    PROMPT_SELECT,
    PROMPT_ADD,
    PROMPT_REMOVE,
    PROMPT_PAUSE,
    PROMPT_CONTINUE,
    PROMPT_QUIT,
    PROMPT_SKIP,
    PROMPT_RANDOM,
    PROMPT_SEQUENTIAL
};

enum MenuState {
    PAUSED,
    HOME,
    ADDING,
    REMOVING,
    QUITTED,
    EMPTY
};

enum Order {
    RANDOM,
    SEQUENTIAL
};

std::string promptToString (Prompt p) {
    std::string ans;
    switch (p) {
        case PROMPT_SELECT:
            ans = "Enter selecionar";
            break;
        case PROMPT_PAUSE:
            ans = "p pausar";
            break;
        case PROMPT_REMOVE:
            ans = "Delete remover";
            break;
        case PROMPT_CONTINUE:
            ans = "p retomar";
            break;
        case PROMPT_ADD:
            ans = "Insert adicionar";
            break;
        case PROMPT_QUIT:
            ans = "Backspace sair";
            break;
        case PROMPT_RANDOM:
            ans = "r ordem aleatória";
            break;
        case PROMPT_SKIP:
            ans = "s pular";
            break;
        case PROMPT_SEQUENTIAL:
            ans = "r ordem sequencial";
            break;
    }
    return ans;
}

const int ID_F6 = 's';
const int ID_BACKSPACE = 263;
const int ID_INSERT = 331;
const int ID_DELETE = 330;
const int ID_F5 = 'p';
const int ID_ENTER = 10;
const int ID_UP = 259;
const int ID_DOWN = 258;
const int ID_F7 = 'r';

class Song {
    std::string artist;
    std::string album;
    std::string title;
    int duration;
public:
    Song (
        const std::string& artist,
        const std::string& album,
        const std::string& title,
        int duration
    ) :
        artist(artist),
        album(album),
        title(title),
        duration(duration)
    {}
    std::string getArtist () const {
        return artist;
    }
    std::string getAlbum () const {
        return album;
    }
    std::string getTitle () const {
        return title;
    }
    // acessa a duração da música (em segundos)
    int getDuration () const {
        return duration;
    }
    /* operator overloading que define a relação
     * de igualdade entre dois objetos Song */
    bool operator== (const Song& o) const {
        return artist == o.artist && album == o.album && title == o.title;
    }
    bool operator< (const Song& o) const {
        return std::make_tuple(artist, album, title) < std::make_tuple(o.artist, o.album, o.title);
    }
};

// Variáveis na região crítica

// Lista encadeada que representa a playlist das músicas sendo tocadas
std::list<Song> playlist;
// Iterator que aponta para a música sendo tocada atualmente
std::list<Song>::const_iterator currentlyPlaying;

std::set<Song> alreadySelected;

struct ScreenPosition {
    int m, n, r, c;
};

// Flag que controla o loop da função play
bool toQuit;
bool toPause;
bool toSkip;
bool removeAt;

int seconds;

Order order;

pthread_mutex_t mutex;
pthread_cond_t emptyPlaylist;
pthread_cond_t paused;

int titleWidth = 5, artistWidth = 6, albumWidth = 5;
ScreenPosition pos[5];
WINDOW* win[5];

/* Por simplicidade, representamos as músicas
* disponíveis dentro de um vetor na memória principal
* Além disso, para efeito de melhor visualização do funcionamento
* do programa, colocamos cada música com uma duração
* fictícia de apenas 3 segundos */
const std::vector<Song> database = {
    Song("The Beatles", "Help!", "Yesterday", 125),
    Song("Pink Floyd", "The Dark Side of The Moon", "Time", 408),
    Song("Led Zeppelin", "Physical Graffiti", "Kashmir", 517),
    Song("Red Hot Chili Peppers", "Stadium Arcadium", "Slow Cheetah", 319),
    Song("Dua Lipa", "Future Nostalgia", "Don't Start Now", 182),
    Song("Arctic Monkeys", "AM", "Arabella", 209),
    Song("Tame Impala", "Currents", "Let it Happen", 256),
    Song("Ed Sheeran", "X", "Don't", 219),
    Song("Gotye", "Making Mirrors", "Somebody That I Used to Know", 243),
    Song("Queen", "Hot Space", "Under Pressure", 248),
    Song("Yes", "Fragile", "Roundabout", 515),
    Song("R.E.M.", "Out of Time", "Losing My Religion", 269),
    Song("21 Pilots", "Stressed Out", "Stressed Out", 225),
    Song("Adele", "25", "Hello", 366),
    Song("Childish Gambino", "This is America", "This is America", 244),
    Song("Nirvana", "Nevermind", "In Bloom", 255),
    Song("Black Sabbath", "Paranoid", "Iron Man", 353),
    Song("Ariana Grande", "Positions", "positions", 158),
    Song("The Smiths", "The Queen is Dead", "Bigmouth Strikes Again", 191),
    Song("A-ha", "Hunting High and Low", "Take on Me", 244),
    Song("Eminem", "8 Mile - Soundtrack", "Love Yourself", 323)
};

void printRow (int w, const Song& song, int i) {
    wmove(win[w], 3 + i, 1);
    wprintw(win[w], " %*s %*s %*s ",
        titleWidth,
        song.getTitle().c_str(),
        artistWidth,
        song.getArtist().c_str(),
        albumWidth,
        song.getAlbum().c_str()
    );
    int cnt = pos[w].n - 2 - (4 + titleWidth + artistWidth + albumWidth);
    while (cnt--) {
        wprintw(win[w], " ");
    }
}

void next () {
    currentlyPlaying++;
    if (currentlyPlaying == playlist.cend()) {
        currentlyPlaying = playlist.cbegin();
    }
}

void skip () {
    if (playlist.size() > 1) {
        switch (order) {
            case RANDOM: {
                int k = 1 + uid(rng) % (playlist.size() - 1);
                while (k--) {
                    next();
                }
                break;
            }
            case SEQUENTIAL: {
                next();
                break;
            }
        }
    }
    seconds = 0;
}

void startPlaying () {
    if (playlist.empty()) {
        printRow(3, Song("", "", "", 0), 0);
        mvwprintw(win[3], 4, pos[3].n/2 - 2, "--:--");
    } else {
        printRow(3, *currentlyPlaying, 0);
        mvwprintw(win[3], 4, pos[3].n/2 - 2, "00:00");
    }
}

void* playlistLoop (void* args) {
    while (true) {
        pthread_mutex_lock(&mutex);
        if (toQuit) {
            toQuit = false;
            pthread_mutex_unlock(&mutex);
            break;
        } else if (toPause) {
            pthread_cond_wait(&paused, &mutex);
            toPause = false;
            pthread_mutex_unlock(&mutex);
            pthread_cond_init(&paused, NULL);
        } else if (playlist.empty()) {
            pthread_cond_wait(&emptyPlaylist, &mutex);
            currentlyPlaying = playlist.cbegin();
            seconds = 0;
            startPlaying();
            pthread_mutex_unlock(&mutex);
            wrefresh(win[3]);
            pthread_cond_init(&emptyPlaylist, NULL);
        } else if (removeAt) {
            removeAt = false;
            if (playlist.size() == 1) {
                playlist.clear();
            } else {
                std::list<Song>::const_iterator it = currentlyPlaying;
                skip();
                playlist.erase(it);
            }
            startPlaying();
            pthread_mutex_unlock(&mutex);
            wrefresh(win[3]);
        } else if (toSkip) {
            toSkip = false;
            skip();
            startPlaying();
            pthread_mutex_unlock(&mutex);
            wrefresh(win[3]);
        } else {
            pthread_mutex_unlock(&mutex);
            sleep(1);
            pthread_mutex_lock(&mutex);
            if (++seconds == currentlyPlaying->getDuration()) {
                skip();
                startPlaying();
            } else {
                mvwprintw(win[3], 4, pos[3].n/2 - 2, "%02d:%02d", seconds/60, seconds % 60);
            }
            pthread_mutex_unlock(&mutex);
            wrefresh(win[3]);
        }
    }
    pthread_exit(NULL);
}

bool add () {
    bool extended = false;
    int i = 0;
    while (true) {
        wattron(win[1], A_STANDOUT);
        printRow(1, database[i], i);
        wrefresh(win[1]);
        wclear(win[4]);
        wmove(win[4], 0, 0);
        if (!alreadySelected.count(database[i])) {
            wprintw(win[4], "\t%s\t", promptToString(PROMPT_SELECT).c_str());
        }
        wprintw(win[4], "\t%s\t", promptToString(PROMPT_QUIT).c_str());
        wrefresh(win[4]);
        int j;
        int key = getch();
        switch (key) {
            case ID_ENTER:
                if (!alreadySelected.count(database[i])) {
                    pthread_mutex_lock(&mutex);
                    playlist.push_back(database[i]);
                    if (playlist.size() == 1) {
                        pthread_cond_signal(&emptyPlaylist);
                    }
                    pthread_mutex_unlock(&mutex);
                    extended = true;
                    printRow(2, database[i], playlist.size() - 1);
                    wrefresh(win[2]);
                    alreadySelected.insert(database[i]);
                    j = -1;
                } else {
                    j = i;
                }
            break;
            case ID_UP:
                j = i == 0 ? i : i - 1;
            break;
            case ID_DOWN:
                j = i == database.size() - 1 ? i : i + 1;
            break;
            case ID_BACKSPACE:
                j = -1;
            break;
        }
        wattroff(win[1], A_STANDOUT);
        printRow(1, database[i], i);
        wrefresh(win[1]);
        if (j == -1) {
            break;
        }
        i = j;
    }
    return extended;
}

bool remove () {
    bool becameEmpty = false;
    int i = 0;
    std::list<Song>::const_iterator selector = playlist.cbegin();
    while (true) {
        wattron(win[2], A_STANDOUT);
        printRow(2, *selector, i);
        wrefresh(win[2]);
        wattroff(win[2], A_STANDOUT);
        wclear(win[4]);
        wmove(win[4], 0, 0);
        wprintw(win[4], "\t%s\t\t%s",
            promptToString(PROMPT_SELECT).c_str(),
            promptToString(PROMPT_QUIT).c_str()
        );
        wrefresh(win[4]);
        int j;
        std::list<Song>::const_iterator to;
        int key = getch();
        bool toErase = true;
        switch (key) {
            case ID_ENTER: {
                pthread_mutex_lock(&mutex);
                becameEmpty = playlist.size() == 1;
                alreadySelected.erase(*selector);
                printRow(2, Song("", "", "", 0), playlist.size() - 1);
                if (currentlyPlaying == selector) {
                    removeAt = true;
                    int k = 0;
                    auto it = playlist.cbegin();
                    for (auto it = playlist.cbegin(); it != playlist.cend(); it++) {
                        if (it == currentlyPlaying) {
                            continue;
                        }
                        printRow(2, *it, k);
                        k++;
                    }
                } else {
                    playlist.erase(selector);
                    auto it = playlist.cbegin();
                    for (int k = 0; k < playlist.size(); k++) {
                        printRow(2, *it, k);
                        it++;
                    }
                }
                pthread_mutex_unlock(&mutex);
                wrefresh(win[2]);
                j = -1;
                toErase = false;
                break;
            }
            case ID_UP:
                if (i == 0) {
                    j = i;
                    to = selector;
                } else {
                    j = i - 1;
                    to = std::prev(selector);
                }
                break;
            case ID_DOWN:
                if (i == playlist.size() - 1) {
                    j = i;
                    to = selector;
                } else {
                    to = std::next(selector);
                    j = i + 1;
                }
                break;
            case ID_BACKSPACE:
                j = -1;
                break;
        }
        if (toErase) {
            printRow(2, *selector, i);
            wrefresh(win[2]);
        }
        i = j;
        selector = to;
        if (j == -1) {
            break;
        }
    }
    return becameEmpty;
}

void quit () {
    pthread_mutex_lock(&mutex);
    toQuit = true;
    if (playlist.empty()) {
        pthread_cond_signal(&emptyPlaylist);
    }
    pthread_mutex_unlock(&mutex);
}

void* userInterface (void* args) {
    
    getmaxyx(stdscr, pos[0].m, pos[0].n);
    pos[1].m = pos[2].m = pos[0].m - 7;
    pos[1].n = pos[0].n/2, pos[2].n = pos[0].n - pos[1].n;
    pos[1].r = pos[1].c = 0;
    pos[2].r = 0, pos[2].c = pos[1].n;
    pos[3].m = 1 + 4 + 1, pos[3].n = pos[0].n;
    pos[3].r = pos[1].m, pos[3].c = 0;
    pos[4].m = 1, pos[4].c = pos[0].n;
    pos[4].r = pos[1].m + pos[3].m, pos[4].c = 0;
    
    for (int i = 1; i < 5; i++) {
        win[i] = newwin(pos[i].m, pos[i].n, pos[i].r, pos[i].c);
    }
    refresh();
    for (int i = 1; i < 4; i++) {
        box(win[i], '|', '-');
        wrefresh(win[i]);
    }
    for (const Song& it : database) {
        titleWidth = std::max(titleWidth, int(it.getTitle().length()));
        artistWidth = std::max(artistWidth, int(it.getArtist().length()));
        albumWidth = std::max(albumWidth, int(it.getAlbum().length()));
    }
    mvwprintw(win[1], 0, 1, "Available Songs");
    mvwprintw(win[1], 1, 1, " %*s %*s %*s ", titleWidth, "Title", artistWidth, "Artist", albumWidth, "Album");
    wmove(win[1], 2, 1);
    for (int j = 0; j < pos[1].n - 2; j++) {
        wprintw(win[1], "-");
    }
    for (int i = 0; i < database.size(); i++) {
        printRow(1, database[i], i);
    }
    wrefresh(win[1]);

    mvwprintw(win[2], 0, 1, "Playlist");
    mvwprintw(win[2], 1, 1, " %*s %*s %*s ", titleWidth, "Title", artistWidth, "Artist", albumWidth, "Album");
    wmove(win[2], 2, 1);
    for (int j = 0; j < pos[2].n - 2; j++) {
        wprintw(win[2], "-");
    }
    wrefresh(win[2]);

    mvwprintw(win[3], 0, 1, "Playing Now");
    mvwprintw(win[3], 1, 1, " %*s %*s %*s ", titleWidth, "Title", artistWidth, "Artist", albumWidth, "Album");
    wmove(win[3], 2, 1);
    for (int j = 0; j < pos[3].n - 2; j++) {
        wprintw(win[3], "-");
    }
    wmove(win[3], 4, 1);
    startPlaying();
    wrefresh(win[3]);

    MenuState currentState = EMPTY;
    do {
        wclear(win[4]);
        wmove(win[4], 0, 0);
        switch (currentState) {
            case EMPTY: {
                wprintw(win[4], "\t%s\t\t%s",
                    promptToString(PROMPT_ADD).c_str(),
                    promptToString(PROMPT_QUIT).c_str()
                );
                wrefresh(win[4]);
                int key = getch();
                switch (key) {
                    case ID_INSERT:
                        if (add()) {
                            currentState = HOME;
                        }
                        break;
                    case ID_BACKSPACE:
                        quit();
                        currentState = QUITTED;
                        break;
                }
                break;
            }
            case HOME: {
                wprintw(win[4], "\t%s\t\t%s\t\t%s\t\t%s\t\t%s\t\t%s",
                    promptToString(PROMPT_ADD).c_str(),
                    promptToString(PROMPT_REMOVE).c_str(),
                    promptToString(PROMPT_PAUSE).c_str(),
                    promptToString(PROMPT_SKIP).c_str(),
                    promptToString(order == RANDOM ? PROMPT_SEQUENTIAL : PROMPT_RANDOM).c_str(),
                    promptToString(PROMPT_QUIT).c_str()
                );
                wrefresh(win[4]);
                int key = getch();
                switch (key) {
                    case ID_INSERT:
                        add();
                        break;
                    case ID_DELETE:
                        if (remove()) {
                            currentState = EMPTY;
                        }
                        break;
                    case ID_F5:
                        pthread_mutex_lock(&mutex);
                        toPause = true;
                        pthread_mutex_unlock(&mutex);
                        currentState = PAUSED;
                        break;
                    case ID_F6:
                        pthread_mutex_lock(&mutex);
                        toSkip = true;
                        pthread_mutex_unlock(&mutex);
                        break;
                    case ID_F7:                    
                        pthread_mutex_lock(&mutex);
                        order = order == RANDOM ? SEQUENTIAL : RANDOM;
                        pthread_mutex_unlock(&mutex);
                        break;
                    case ID_BACKSPACE:
                        quit();
                        currentState = QUITTED;
                        break;
                }
                break;
            }
            case PAUSED: {
                wprintw(win[4], "\t%s",
                    promptToString(PROMPT_CONTINUE).c_str()
                );
                wrefresh(win[4]);
                int key = getch();
                switch (key) {
                    case ID_F5:
                        pthread_mutex_lock(&mutex);
                        pthread_cond_signal(&paused);
                        pthread_mutex_unlock(&mutex);
                        currentState = HOME;
                        break;
                }
                break;
            }
        }
    } while (currentState != QUITTED);
    pthread_exit(NULL);
}

int main () {

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&emptyPlaylist, NULL);
    pthread_cond_init(&paused, NULL);

    toQuit = false;
    toPause = false;
    toSkip = false;
    removeAt = false;
    order = SEQUENTIAL;

    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    pthread_t playlistLoopThread, userInterfaceThread;
    pthread_create(&playlistLoopThread, NULL, playlistLoop, NULL);
    pthread_create(&userInterfaceThread, NULL, userInterface, NULL);

    pthread_join(playlistLoopThread, NULL);
    pthread_join(userInterfaceThread, NULL);

    endwin();

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&emptyPlaylist);
    pthread_cond_destroy(&paused);
}