#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <list>
#include <vector>

class Song {
    std::string artist;
    std::string album;
    std::string title;
    int duration;     // em segundos
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
};

std::list<Song> playlist;

// inserir uma música na playlist
void addSong () {
    system("clear");
    std::cout << "Aqui você escolhe qual música você quer adicionar" << std::endl;
}

// remover uma música da playlist
void removeSong () {
    system("clear");
    std::cout << "Aqui você escolhe qual música você quer remover" << std::endl;
}

int main () {
    /* Por simplicidade, representamos as músicas
     * disponíveis dentro de um vetor na memória principal */
    std::vector<Song> database = {
        Song("Iron Maiden", "Fear of the Dark", "Fear of the Dark", 438),
        Song("The Beatles", "Abbey Road", "Here Comes the Sun", 185),
        Song("Pink Floyd", "Wish You Were Here", "Wish You Were Here", 317),
        Song("Elis Regina & Tom Jobim", "Elis & Tom", "Águas de Março", 212)
    };
    // flag que controla se deve permanecer no menu principal ou não
    bool keep;
    do {
        // imprimir opções de caracteres a serem digitados pelo usuário
        std::cout << "Olá, bem-vindo ao Music Player!" << std::endl;
        std::cout << "Opções..." << std::endl;
        // Lê o caractere que o usuário digitou
        char input;
        std::cin >> input;

        switch (input) {
            case '+': {
                addSong();
                sleep(2);
                keep = true;
            } break;
            case '-': {
                removeSong();
                sleep(2);
                keep = true;
            } break;
            case 'q': { // sair do menu
                std::cout << "Flw!" << std::endl;
                sleep(2);
                keep = false;
            } break;
            case 'c': { // consultar música em execução
                std::cout << "A musica em reprodução é: " << std::endl;
                sleep(2);
                break;
            }
            default: {
                std::cout << "Entrada inválida, tente novamente" << std::endl;
                sleep(2);
                keep = true;
            }
        }

        system("clear");   // apaga tudo para recomeçar
    } while (keep);
}