#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <string>
#include <iostream>
#include <list>
#include <vector>
#include <exception>

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
    // acessa a duração da música (em segundos)
    int getDuration () const {
        return duration;
    }
    /* operator overloading que define a relação
     * de igualdade entre dois objetos Song */
    bool operator== (const Song& o) const {
        return artist == o.artist && album == o.album && title == o.title;
    }
    // representação de objeto Song como string
    friend std::ostream& operator<< (std::ostream& os, const Song& o) {
        return os << "Título: " << o.title <<
        " | Artista: " << o.artist <<
        " | Álbum: " << o.album <<
        " | Duração: " << o.duration << "s";
    }
    std::string toString () {
        return title
        + " | " + artist
        + " | " + album
        + " | " + std::to_string(duration) + "s";
    }
};

// Variáveis na região crítica

// Lista encadeada que representa a playlist das músicas sendo tocadas
std::list<Song> playlist;
// Iterator que aponta para a música sendo tocada atualmente
std::list<Song>::const_iterator currentlyPlaying;
// Flag que controla o loop da função play
bool userQuitted;

pthread_mutex_t mutex;
pthread_cond_t emptyPlaylist;

// insere a música ao final da playlist
void addSong (const Song& song) {
    /* Como a variável playlist pode estar sendo acessada
     * também pela thread playlistLoop, é preciso que
     * a exclusão mútua seja garantida */
    pthread_mutex_lock(&mutex);
    playlist.push_back(song);
    /* Uma vez que pelo menos música foi inserida, sinalizamos
     * para a função play (que poderia estar aguardando)
     * que a playlist não está mais vazia */
    pthread_cond_signal(&emptyPlaylist);
    pthread_mutex_unlock(&mutex);
}

// remove a primeira música da playlist
void removeSong () {
    /* Como as variáveis playlist e currentPlaying
     * podem estar sendo acessadas
     * também pela thread playlistLoop, é preciso que
     * a exclusão mútua seja garantida */
    pthread_mutex_lock(&mutex);
    bool coincidence = currentlyPlaying == playlist.begin();
    playlist.pop_front();
    /* Aqui tratamos o caso excepcional em que a música sendo removida
     * (a primeira) é justamente aquela sendo executada no momento
     * Garantimos assim que o iterator não estará apontando
     * para uma posição inválida, no momento que a função play o avançar */
    if (coincidence) {
        currentlyPlaying = playlist.end();
    }
    pthread_mutex_unlock(&mutex);
}

// checa se a música song já está na playlist ou não
bool playlistContainsSong (const Song& song) {
    for (const auto& it : playlist) {
        if (it == song) {
            return true;
        }
    }
    return false;
}

// encerra o loop da função play
void quit () {
    /* Como a variável userQuitted pode estar sendo
     * acessada também pela thread playlistLoop,
     * é preciso que a exclusão mútua seja garantida */
    pthread_mutex_lock(&mutex);
    userQuitted = true;
    /* Poderia ocorrer de a playlist estar vazia,
     * aguardando um sinal da variável emptyPlaylist
     * Se não sinalizarmos, a função play ficará
     * aguardando indefinidamente, e não encerrará,
     * conforme desejado */
    pthread_cond_signal(&emptyPlaylist);
    pthread_mutex_unlock(&mutex);
}

class NoSongException : public std::exception {
public:  
    const char* what () const throw () {
        return "A playlist está vazia.";
    }
};

/* Se a playlist está vazia, levanta uma exceção
 * Senão, retorna a música que está sendo executada no momento */
Song getCurrentSong () {
    /* Como a variável currentlyPlaying pode estar sendo
     * acessada também pela thread playlistLoop,
     * é preciso que a exclusão mútua seja garantida */
    pthread_mutex_lock(&mutex);
    if (playlist.empty()) {
        pthread_mutex_unlock(&mutex);
        throw NoSongException();
    } else {
        Song song = *currentlyPlaying;
        pthread_mutex_unlock(&mutex);
        return song;
    }
}

/* A função play é um loop que simula a "execução" das músicas
 * Ela é executada numa thread separada do main: playlistLoop */
void* play (void* args) {
    while (true) {
        /* O lock no mutex garante um acesso seguro à região crítica,
         * no caso, às variáveis playlist, currentPlaying e userQuitted */
        pthread_mutex_lock(&mutex);
        if (userQuitted) {
            pthread_mutex_unlock(&mutex);
            /* Quando o usuário deseja encerrar o ciclo da playlist,
            * a flag userQuitted é atribuída com true,
            * e o loop da função play é parado */
            break;
        } else if (playlist.empty()) {
            /* Enquanto a playlist estiver vazia, não há motivo para o
             * loop prosseguir, já que não há músicas para iterar
             * Aguardamos então o sinal da variável condicional emptyPlaylist,
             * que ocorre quando a primeira música é finalmente adicionada */
            pthread_cond_wait(&emptyPlaylist, &mutex);
            pthread_mutex_unlock(&mutex);
            /* Reinicializamos a variável condicional emptyPlaylist, pois
             * a playlist pode voltar a ficar vazia */
            pthread_cond_init(&emptyPlaylist, NULL);
        } else {
            /* O loop possui comportamento cíclico
             * quando chega ao final, volta ao começo */
            if (currentlyPlaying == playlist.cend()) {
                currentlyPlaying = playlist.cbegin();
            } else {
                currentlyPlaying++;
            }
            int duration = currentlyPlaying->getDuration();
            pthread_mutex_unlock(&mutex);
            // simulamos a "execução" da música com um thread sleep
            sleep(duration);
        }
    }
    pthread_exit(NULL);
}

int main () {
    /* Por simplicidade, representamos as músicas
     * disponíveis dentro de um vetor na memória principal
     * Além disso, para efeito de melhor visualização do funcionamento
     * do programa, colocamos cada música com uma duração
     * fictícia de apenas 3 segundos */
    std::vector<Song> database = {
        Song("The Beatles", "Help!", "Yesterday", 3),
        Song("Pink Floyd", "The Dark Side of The Moon", "Time", 3),
        Song("Led Zeppelin", "Physical Graffiti", "Kashmir", 3),
        Song("Red Hot Chili Peppers", "Stadium Arcadium", "Dani California", 3),
        Song("Dua Lipa", "Future Nostalgia", "Don't Start Now", 3),
        Song("Caetano Veloso", "Cinema Transcendental", "Oração ao Tempo", 3),
        Song("Chico Buarque", "Opera do Malandro", "Geni e o Zepelim", 3),
        Song("Alcione", "A Voz do Samba", "Não Deixe o Samba Morrer", 3),
        Song("Legião Urbana", "Dois", "Tempo Perdido", 3),
        Song("Melim", "Melim", "Meu Abrigo", 3)
    };

    // inicializando o mutex e a variável condicional
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&emptyPlaylist, NULL);

    // inicializando as variáveis na região crítica
    userQuitted = false;
    currentlyPlaying = playlist.cbegin();
    // (a playlist começa vazia)

    // inicializando a thread que executa o loop da playlist
    pthread_t playlistLoop;
    pthread_create(&playlistLoop, NULL, play, NULL);

    // flag que controla o menu principal
    bool keep;

    do {
        initscr();

        int qtdLinhas, qtdColunas, y0, x0, y1, x1, y2, x2, y3, x3, y4, x4, y5, x5, y6 , x6, y7, x7;

        getmaxyx(stdscr, qtdLinhas, qtdColunas);

        y0 = 0, x0 = 0; // Posição inicial de linhas e colunas
        y1 = qtdLinhas-8, x1 = (qtdColunas / 2) - 1; //Total das linhas e colunas da janela 1

        y2 = 0, x2 = (qtdColunas / 2) + 1; // Posição inicial de linhas e colunas
        y3 = qtdLinhas-8, x3 = (qtdColunas / 2) - 1; //Total das linhas e colunas da janela 2

        y4 = qtdLinhas-8, x4 = 0; // Posição inicial de linhas e colunas
        y5 = 8; x5 = (qtdColunas/2)-1; //Total das linhas e colunas da janela 3

        y6 = qtdLinhas-8, x6 = (qtdColunas / 2) + 1;// Posição inicial de linhas e colunas
        y7 = 8; x7 = (qtdColunas/2)-1;//Total das linhas e colunas da janela 4

        WINDOW *win1 = newwin(y1, x1, y0, x0);
        WINDOW *win2 = newwin(y3, x3, y2, x2);
        WINDOW *win3 = newwin(y5 , x5, y4, x4);
        WINDOW *win4 = newwin(y7 , x7, y6, x6);
        refresh();

        box(win1, '|', '-');
        box(win2, '|', '-');
        box(win3, '|', '-');
        box(win4, '|', '-');

        wrefresh(win1);
        wrefresh(win2);
        wrefresh(win3);
        wrefresh(win4);
        
        mvwprintw(win1, 0, 0, "Músicas disponíveis: "/*Título: | Artista: | Álbum: | Duração: "*/);
        mvwprintw(win1, 1, 1, "Indice: | Título: | Artista: | Album: | Duração: ");
        for (int i = 0, l = 2; i < database.size(); i++, l++){
            // impressão das musicas disponíveis para adição na playlist
            wmove(win1, l, 1);
            wprintw(win1, "%d - %s", i, database[i].toString().c_str());
        }

        mvwprintw(win2, 0, 0, "Playlist: ");
        mvwprintw(win3, 0, 0, "Menu: "); // Menu do programa
        mvwprintw(win3, 1, 1, "Pressione uma letra referente a ação desejada: ");
        mvwprintw(win3, 2, 1, "+ : Para adicionar uma música à playlist");
        mvwprintw(win3, 3, 1, "- : Para remover uma música da playlist");
        mvwprintw(win3, 4, 1, "? : Para ver a música atualmente em execução na playlist");
        mvwprintw(win3, 5, 1, ". : Para encerrar o ciclo de execução da playlist");
        mvwprintw(win4, 0, 0, "Reproduzindo: ");
        
        wrefresh(win1);
        wrefresh(win2);
        wrefresh(win3);
        wrefresh(win4);
        
        wmove(win3,6,1);
        wrefresh(win3);

        //endwin(); // encerrar as janelas (colocar só no final)

        // Lê o caractere do menu digitado pelo usuário
        char input = wgetch(win3);

        switch (input) {
            case '+': {
                wmove(win1,2,1); // Topo da database
                wrefresh(win1);
                keypad(win3, TRUE);
                int move = wgetch(win3);
                
                int pos_database;

                while(move != 10){ // Enter na tabela ASCII
                    int l = getcury(win1);
                    if (move==KEY_UP){
                        if (l-1<2){ //Não passar dos limites das músicas da database
                            wmove(win1,2,1);
                            wrefresh(win1);
                            pos_database = 0;//primeira musica da database
                        } else{
                        wmove(win1,l-1,1); //sobe na database
                        wrefresh(win1);
                        pos_database = getcury(win1)-2;
                        }
                    } else if (move==KEY_DOWN){
                        if (l+1>11){//Não passar dos limites das músicas da database
                            wmove(win1,11,1);
                            wrefresh(win1);
                            pos_database = 9; //mudar caso aumente o numero de músicas da database;
                        } else{
                            wmove(win1,l+1,1); // desce na database
                            wrefresh(win1);
                            pos_database = getcury(win1)-2;
                        }
                    } else {
                        mvwprintw(win3, 6, 1, "Comando inválido");
                        wrefresh(win3);
                        //break;
                    }
                    noecho();
                    move = wgetch(win3);
                }

                mvwprintw(win3,6,1,"a posição da database é: %d",pos_database);
                wrefresh(win3);
                
                getch();// só pra ver os prints acima
                
                //Verificar cores
                
                /*
                if (playlistContainsSong(database[pos_database])){
                    std::cout << "Música já adicionada anteriormente." << std::endl;
                } else {
                    // Adiciona no final da playlist
                    addSong(database[pos_database]); 
                    std::cout << "A música:" << std::endl;
                    std::cout << database[pos_database] << std::endl;
                    std::cout << "Foi adicionada ao final da playlist com sucesso." << std::endl;
                }*/
                //std::cin.get();
                keep = true;
            } break;
            case '-': {
                printw("cheguei no -");
                getch();
                // interação com usuário para remoção de música
                if (playlist.empty()) {
                    std::cout << "A playlist está vazia." << std::endl;
                } else {
                    std::cout << "Removida a primeira música da playlist:" << std::endl;
                    std::cout << playlist.front() << std::endl;
                    removeSong();
                    if (playlist.empty()) {
                        std::cout << "A playlist agora está vazia." << std::endl;
                    } else {
                        int i = 0;
                        std::cout << "Estado atual da playlist:" << std::endl;
                        for (Song song: playlist){
                            // impressão das musicas disponíveis na playlist após a remoção
                            std::cout << i++ << " - " << song << std::endl;
                        }
                    }
                }
                std::cin.get();
                keep = true;
            } break;
            case '?': {
                printw("cheguei no ?");
                getch();
                
                try {
                    Song song = getCurrentSong();
                    std::cout << "Tocando agora:" << std::endl;
                    std::cout << song << std::endl;
                } catch (NoSongException& e) {
                    std::cout << e.what() << std::endl;
                }
                std::cin.get();
                keep = true;
            } break;
            case '.': {
                printw("cheguei no .");
                getch();
                quit();
                if (playlist.empty()) {
                    std::cout << "A playlist foi encerrada vazia." << std::endl;
                } else {
                    std::cout << "O ciclo da playlist foi encerrado." << std::endl;
                    std::cout << "Espere até a última música acabar." << std::endl;
                }
                keep = false;
            } break;
            default: {
                printw("nenhum");
                getch();
                std::cout << "Entrada inválida." << std::endl;
                std::cin.get();
                keep = true;
            }
        }
    } while (keep);
    
    pthread_join(playlistLoop, NULL);

    // destruindo o mutex e variável condicional
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&emptyPlaylist);
}