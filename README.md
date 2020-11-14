# exercicio-concorrencia
Music Player by Michael Felipe dos Santos & Gustavo Farani de Farias

Programa desenvolvido em C++ para o projeto de concorrência válido para a EE1 da disciplina Infraestrutura de Software.

Funcionamento do programa:

Obs.: Usar o terminal em tela cheia para que a exibição não fique muito condensada e, portanto, a visualização seja prejudicada.

Ao iniciar o programa, o usuário será apresentado a uma janela com três subdivisões: Avaliable Songs, Playlist e Playing Now, além de, uma linha (a última) com os possíveis comandos (cada um seguido pela sua função) que o usuário pode usar para interagir com o programa.

Avaliable Songs: é a janela onde as músicas passiveis de serem adicionadas na playlist são mostradas. Nela, as músicas são mostradas separadas por Title, Artist e Album, sendo cada linha representando uma música.

Playlist: é a janela que exibe as músicas atualmente presentes na playlist, separadas por Title, Artist e Album.

Playing Now: é a janela que mostra qual música está sendo reproduzida no momento. Semelhante as janelas anteriores, esta representa a musica tocada no momento separada por Title, Artist e Album, com adição da contagem de segundos que representa a duração da música.

A navegação por entre as músicas apresentadas nas janelas Avaliable Songs e Playlist é feita pelas setas para cima e para baixo do teclado. Porém, essa navegação só é possível quando o usuário está adicionando ou removendo músicas.
Se o usuário estiver em uma função e desejar voltar ao menu principal, basta que ele pressione a tecla “Backspace”, e já estando no menu principal essa tecla sai do programa.

Adicionar à playlist:
Para adicionar uma música a playlist, basta que o usuário, estando no menu principal, aperte a tecla “Insert”. Nesse momento, o cursor irá para a janela Avaliable Songs, onde, com as setas para cima e para baixo do teclado, é possível navegar por entre as músicas disponíveis. Tendo escolhido uma música, basta que o usuário aperte a tecla “Enter” e ela será adicionada a playlist, sendo mostrada na janela Playlist.
Se essa for a primeira música a ser adicionada na playlist, ela imediatamente começará a ser reproduzida, sendo mostrada na janela Playing Now.
É válido ressaltar que, o usuário não pode adicionar na playlist uma música adicionada anteriormente, pois, como é mostrado na linha de exibição de comandos a tecla “Enter” (que é responsável por essa adição) não é lida pelo programa, quando o cursor está sobre uma música já presente na playlist.
Para voltar ao menu principal basta que o usuário pressione a tecla “Backspace”.

Remover da playlist:
Para remover uma música da playlist, basta que o usuário, estando no menu principal, aperte a tecla “Delete”. Nesse momento, o cursor irá para a janela Playlist, onde, com as setas para cima e para baixo do teclado, é possível navegar por entre as músicas presentes. Tendo escolhido uma música, basta que o usuário aperte a tecla “Enter” e ela será removida da playlist, sendo retirada de exibição na janela Playlist.
Se a música a ser retirada da playlist for a que estive em reprodução no momento, ela será removida normalmente, e a próxima música começará a ser reproduzida.
Após a remoção da playlist, a música removida, pode ser adicionada novamente a partir da janela Avaliable Songs.

Pausar/Retomar reprodução:
Para pausar a reprodução de uma música, basta que o usuário, estando no menu principal, aperte a tecla “P”. Nesse momento, a reprodução da música atual será pausada e a janela Playing Now exibirá o segundo em que a música parou sua reprodução. Para retomar a reprodução, basta que o usuário, aperta novamente a tecla “P”. Nesse momento a reprodução continuará do segundo onde parou na janela Playing Now.

Pular/Avançar reprodução:
Para Pular/Avançar a reprodução de uma música, basta que o usuário, estando no menu principal, aperte a tecla “S”. Nesse momento, a música atualmente em execução na janela Playing Now, será alterada para a próxima música da Playlist, que por sua vez passará a ser reproduzida a partir de seu início.
Se a música atualmente em reprodução for a última da playlist, ao avançar, a próxima música a entrar em reprodução será a primeira da playlist.

Alternar entre reprodução sequencial e aleatória:
Para Alternar entre reprodução sequencial e aleatória, basta que o usuário, estando no menu principal, aperte a tecla “R”. Nesse momento, as músicas passaram a ser reproduzidas em ordem aleatória e não mais em ordem sequencial como é o padrão. Para retornar a ordem de reprodução sequencial, basta que o usuário aperte a tecla “R” novamente.
