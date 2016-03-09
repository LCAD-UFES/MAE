Para rodar o face_recog_planar_yale você precisa da 
"Extended Yale Face Database B". Você pode obtê-la em:
http://vision.ucsd.edu/~leekc/ExtYaleDatabase/ExtYaleB.html

Esta versão do face_recog_planar usa as imagens recortadas 
(Cropped Images). Baixe-as neste site e desempacote no diretório:
/home/hallysson/img/ExtendedYaleB/CroppedYale

Você pode colocar em outro lugar, mas, para isso, tem que mudar
o face_recog.con de acordo (não recomendado pois, colocando
neste diretório, outros usuários poderão usar a base de figuras).

Para rodar o face_recog_planar_yale basta compila-lo (make) e
rodá-lo com o comando:
./face_recog face_recog.cml

O comando acima roda experimento equivalente ao do paper:
John Wright, Allen Y. Yang, Arvind Ganesh, S. Shankar Sastry, 
Yi Ma, "Robust Face Recognition via Sparse Representation," 
IEEE Transactions on Pattern Analysis and Machine Intelligence, 
vol. 31, no. 2, pp. 210-227, Feb. 2009, doi:10.1109/TPAMI.2008.79

Nosso resultado é, contudo, melhor: 99.34% contra 98.1% (ver Seção 4.1.1
do paper). Você encontra cópia do paper acima no diretório:
face_recog_planar_yale/papers/TPAMI-robust-face-recognition.pdf
