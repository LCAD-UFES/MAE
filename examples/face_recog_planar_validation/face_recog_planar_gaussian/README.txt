Este arquivo relata como o experimento adotado em "Robust Face Recognition via Sparse Representation" foi realizado no face_recog_planar_gaussian. 


################## Arquivo de Treino ##################

> faces_session1.txt: 
	* contém 700 poses, de 50 homens e 50 mulheres, coletadas na primeira sessão;
	* as poses são: 1) iluminação esquerda, 2) ilumicação direita, 3) iluminação por todos os lados; 4) sem expressão; 5) bravo; 6) sorrindo; 7) gritando.

Há um define (RAMDOM_FACES_TRAIN) em face_recog_user_functions.h especificando este arquivo como o utilizado para treino.


################## Arquivo de Teste ##################

> faces_session2.txt:
        * contém 700 poses, de 50 homens e 50 mulheres, coletadas na primeira segunda;
        * as poses são: 1) iluminação esquerda, 2) ilumicação direita, 3) iluminação por todos os lados; 4) sem expressão; 5) bravo; 6) sorrindo; 7) gritan
do.

Há um define (RAMDOM_FACES_RECALL) em face_recog_user_functions.h especificando este arquivo como o utilizado para teste.


################## Configuração da Rede Neural Sem Peso ##################

OUT_WIDTH x OUT_HEIGHT = 32x32
SYNAPSES = 256


################## Arquivo de saída de Erros ##################

> errors.out

Contém todas as poses e coordenadas, utilizadas durante o teste, que não estão presentes em /home/hallysson/img/ARDataBase/ARPhotoDataBase/




