# exercicio-concorrencia
Music Player by Michael Felipe dos Santos & Gustavo Farani de Farias

Programa desenvolvido em C++ para o projeto de concorrência válido para a EE1 da disciplina Infraestrutura de Software.

Funcionamento do programa:

Ao iniciar o programa apresenta um menu com as seguintes opções de interação:

+ : Para adicionar músicas a playlist
- : Para remover músicas da playlist
? : Para ver a música em execução atualmente na playlist
. : Para encerrar a execução da playlist

Ao digitar “+”, serão mostradas as opções de músicas advindas de uma base de dados, para que o usuário escolha uma por vez para adicionar a playlist.
Cada música possui um índice, que será informado pelo usuário para adição na playlist.
Se o usuário informar um índice que não esteja na database, a seguinte mensagem será retornada: “Índice inválido”, e logo após,
o menu reaparecerá para uma nova interação.
Se o usuário informar um índice válido, a música que possui este índice no database, será adicionada no final da playlist,
a menos que ela já esteja na mesma, pois, há uma checagem disto anteriormente a sua adição e exibirá a mensagem:
“A música selecionada foi adicionada na playlist com sucesso”, em que “selecionada” será a música escolhida.
Por fim, o menu do programa reaparecerá para uma nova interação.

Ao digitar “-”, a playlist verificará se existe pelo menos uma música na playlist para poder então removê-la.
Em caso negativo a seguinte mensagem de erro é exibida: “A playlist está vazia.”.
Em caso afirmativo, a primeira música da playlist será removida, dessa forma, sempre haverá remoção de músicas pelo início da playlist.
Logo após remover a música, será mostrado uma mensagem confirmando a remoção seguido pelo estado atual da playlist.
Após uma mensagem ou outra o menu reaparecerá para uma nova interação do usuário.

Ao digitar “?”, o programa exibirá a música que está “tocando” na playlist no momento, a menos que, não exista nenhuma música na mesma.
Nesse caso aparecerá a mensagem: “A playlist está vazia”. E novamente reaparecerá o menu do programa para que o usuário possa realizar uma nova ação.

Ao digitar “.”, o programa irá verificar se a playlist possui não está vazia, caso esteja, irá exibir a mensagem: “A playlist foi encerrada vazia.”
e o programa será encerrado. Caso a playlist não esteja vazia o programa irá interromper o ciclo da playlist, ou seja, irá impedir que a próxima música
(se houver) seja executada e mostrará a seguinte mensagem: “O ciclo da playlist foi encerrado.".
Logo após exibirá a mensagem: “Espere até a última música acabar.” e esperará o fim da “reprodução” da música para encerrar o programa.

Ao digitar um caractere diferentes destes apresentados no menu do programa, a seguinte mensagem será retornada: “Entrada inválida” e o programa
voltará ao menu.
