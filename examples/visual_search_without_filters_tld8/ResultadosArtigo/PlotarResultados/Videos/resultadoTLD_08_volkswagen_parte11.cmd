/* Counters */

int i;
int retrain;
int numRetrain;
int numLastRetrained;
int JustTrained;
int num_pixels;
int x;
int y;
int x_retrain;
int y_retrain;
int best_x;
int best_y;
float confidence;
float best_confidence;
float confidenceLevel;
float scale_zoom;
float scale_before;
int out_of_scene;
float scale_to_retrain;
float max_scale;
float min_scale;
int num_frames;

i = 0;
/* Trainning */
#Treinando com a imagem 0
GetImage(i);
MoveToTargetCenter(0);
key t;
ForwardVisualSearchNetwork(0);
#forward outputs;
pause;
/* Testing */
numLastRetrained = -1;
i = i + 1;
GetImage(i);
MoveToTargetCenter(0);
JustTrained = 1;

max_scale = GetMaxScale(0);
min_scale = GetMinScale(0);

num_frames = GetNumFrames(0);

GetImage(5040);
SetScaleFactor(1.000000);
ForwardVisualSearchNetwork(0);
MoveToPoint(367,83); 
MoveToPoint(367,83);
#pause;
GetImage(5041);
SetScaleFactor(0.956843);
ForwardVisualSearchNetwork(0);
MoveToPoint(369,83); 
MoveToPoint(369,83);
#pause;
GetImage(5042);
SetScaleFactor(0.915088);
ForwardVisualSearchNetwork(0);
MoveToPoint(371,84); 
MoveToPoint(371,84);
#pause;
GetImage(5043);
SetScaleFactor(0.915088);
ForwardVisualSearchNetwork(0);
MoveToPoint(371,81); 
MoveToPoint(371,81);
#pause;
GetImage(5044);
SetScaleFactor(0.915088);
ForwardVisualSearchNetwork(0);
MoveToPoint(373,74); 
MoveToPoint(373,74);
#pause;
GetImage(5045);
SetScaleFactor(0.915088);
ForwardVisualSearchNetwork(0);
MoveToPoint(373,70); 
MoveToPoint(373,70);
#pause;
GetImage(5046);
SetScaleFactor(0.915088);
ForwardVisualSearchNetwork(0);
MoveToPoint(373,68); 
MoveToPoint(373,68);
#pause;
GetImage(5047);
SetScaleFactor(0.871512);
ForwardVisualSearchNetwork(0);
MoveToPoint(373,68); 
MoveToPoint(373,68);
#pause;
GetImage(5048);
SetScaleFactor(0.871512);
ForwardVisualSearchNetwork(0);
MoveToPoint(375,70); 
MoveToPoint(375,70);
#pause;
GetImage(5049);
SetScaleFactor(0.871512);
ForwardVisualSearchNetwork(0);
MoveToPoint(377,72); 
MoveToPoint(377,72);
#pause;
GetImage(5050);
SetScaleFactor(0.871512);
ForwardVisualSearchNetwork(0);
MoveToPoint(379,74); 
MoveToPoint(379,74);
#pause;
GetImage(5051);
SetScaleFactor(0.867494);
ForwardVisualSearchNetwork(0);
MoveToPoint(381,72); 
MoveToPoint(381,72);
#pause;
GetImage(5052);
SetScaleFactor(0.826185);
ForwardVisualSearchNetwork(0);
MoveToPoint(386,70); 
MoveToPoint(386,70);
#pause;
GetImage(5053);
SetScaleFactor(0.826185);
ForwardVisualSearchNetwork(0);
MoveToPoint(386,68); 
MoveToPoint(386,68);
#pause;
GetImage(5054);
SetScaleFactor(0.810001);
ForwardVisualSearchNetwork(0);
MoveToPoint(386,68); 
MoveToPoint(386,68);
#pause;
GetImage(5055);
SetScaleFactor(0.808152);
ForwardVisualSearchNetwork(0);
MoveToPoint(386,66); 
MoveToPoint(386,66);
#pause;
GetImage(5056);
SetScaleFactor(0.769668);
ForwardVisualSearchNetwork(0);
MoveToPoint(386,66); 
MoveToPoint(386,66);
#pause;
GetImage(5057);
SetScaleFactor(0.760802);
ForwardVisualSearchNetwork(0);
MoveToPoint(386,69); 
MoveToPoint(386,69);
#pause;
GetImage(5058);
SetScaleFactor(0.760802);
ForwardVisualSearchNetwork(0);
MoveToPoint(389,72); 
MoveToPoint(389,72);
#pause;
GetImage(5059);
SetScaleFactor(0.739253);
ForwardVisualSearchNetwork(0);
MoveToPoint(392,72); 
MoveToPoint(392,72);
#pause;
GetImage(5060);
SetScaleFactor(0.739253);
ForwardVisualSearchNetwork(0);
MoveToPoint(392,75); 
MoveToPoint(392,75);
#pause;
GetImage(5061);
SetScaleFactor(0.737464);
ForwardVisualSearchNetwork(0);
MoveToPoint(392,78); 
MoveToPoint(392,78);
#pause;
GetImage(5062);
SetScaleFactor(0.702346);
ForwardVisualSearchNetwork(0);
MoveToPoint(389,81); 
MoveToPoint(389,81);
#pause;
GetImage(5063);
SetScaleFactor(0.668901);
ForwardVisualSearchNetwork(0);
MoveToPoint(389,81); 
MoveToPoint(389,81);
#pause;
GetImage(5064);
SetScaleFactor(0.666047);
ForwardVisualSearchNetwork(0);
MoveToPoint(392,78); 
MoveToPoint(392,78);
#pause;
GetImage(5065);
SetScaleFactor(0.652661);
ForwardVisualSearchNetwork(0);
MoveToPoint(398,69); 
MoveToPoint(398,69);
#pause;
GetImage(5066);
SetScaleFactor(0.642543);
ForwardVisualSearchNetwork(0);
MoveToPoint(401,66); 
MoveToPoint(401,66);
#pause;
GetImage(5067);
SetScaleFactor(0.633126);
ForwardVisualSearchNetwork(0);
MoveToPoint(401,60); 
MoveToPoint(401,60);
#pause;
GetImage(5068);
SetScaleFactor(0.628938);
ForwardVisualSearchNetwork(0);
MoveToPoint(404,57); 
MoveToPoint(404,57);
#pause;
GetImage(5069);
SetScaleFactor(0.614612);
ForwardVisualSearchNetwork(0);
MoveToPoint(407,54); 
MoveToPoint(407,54);
#pause;
GetImage(5070);
SetScaleFactor(0.607530);
ForwardVisualSearchNetwork(0);
MoveToPoint(410,51); 
MoveToPoint(410,51);
#pause;
GetImage(5071);
SetScaleFactor(0.594807);
ForwardVisualSearchNetwork(0);
MoveToPoint(413,54); 
MoveToPoint(413,54);
#pause;
GetImage(5072);
SetScaleFactor(0.584654);
ForwardVisualSearchNetwork(0);
MoveToPoint(416,54); 
MoveToPoint(416,54);
#pause;
GetImage(5073);
SetScaleFactor(0.573880);
ForwardVisualSearchNetwork(0);
MoveToPoint(419,57); 
MoveToPoint(419,57);
#pause;
GetImage(5074);
SetScaleFactor(0.560426);
ForwardVisualSearchNetwork(0);
MoveToPoint(422,57); 
MoveToPoint(422,57);
#pause;
GetImage(5075);
SetScaleFactor(0.544710);
ForwardVisualSearchNetwork(0);
MoveToPoint(422,57); 
MoveToPoint(422,57);
#pause;
GetImage(5076);
SetScaleFactor(0.539983);
ForwardVisualSearchNetwork(0);
MoveToPoint(426,57); 
MoveToPoint(426,57);
#pause;
GetImage(5077);
SetScaleFactor(0.526187);
ForwardVisualSearchNetwork(0);
MoveToPoint(426,57); 
MoveToPoint(426,57);
#pause;
GetImage(5078);
SetScaleFactor(0.519014);
ForwardVisualSearchNetwork(0);
MoveToPoint(426,61); 
MoveToPoint(426,61);
#pause;
GetImage(5079);
SetScaleFactor(0.504462);
ForwardVisualSearchNetwork(0);
MoveToPoint(430,65); 
MoveToPoint(430,65);
#pause;
GetImage(5080);
SetScaleFactor(0.495841);
ForwardVisualSearchNetwork(0);
MoveToPoint(434,61); 
MoveToPoint(434,61);
#pause;
GetImage(5081);
SetScaleFactor(0.485627);
ForwardVisualSearchNetwork(0);
MoveToPoint(442,57); 
MoveToPoint(442,57);
#pause;
GetImage(5082);
SetScaleFactor(0.476199);
ForwardVisualSearchNetwork(0);
MoveToPoint(446,61); 
MoveToPoint(446,61);
#pause;
GetImage(5083);
SetScaleFactor(0.464361);
ForwardVisualSearchNetwork(0);
MoveToPoint(446,65); 
MoveToPoint(446,65);
#pause;
GetImage(5084);
SetScaleFactor(0.450387);
ForwardVisualSearchNetwork(0);
MoveToPoint(450,65); 
MoveToPoint(450,65);
#pause;
GetImage(5085);
SetScaleFactor(0.434012);
ForwardVisualSearchNetwork(0);
MoveToPoint(461,61); 
MoveToPoint(461,61);
#pause;
GetImage(5086);
SetScaleFactor(0.417831);
ForwardVisualSearchNetwork(0);
MoveToPoint(473,56); 
MoveToPoint(473,56);
#pause;
GetImage(5087);
SetScaleFactor(0.417831);
ForwardVisualSearchNetwork(0);
MoveToPoint(473,56); 
MoveToPoint(473,56);
#pause;
GetImage(5088);
SetScaleFactor(0.401781);
ForwardVisualSearchNetwork(0);
MoveToPoint(478,56); 
MoveToPoint(478,56);
#pause;
GetImage(5089);
SetScaleFactor(0.386186);
ForwardVisualSearchNetwork(0);
MoveToPoint(483,61); 
MoveToPoint(483,61);
#pause;
GetImage(5090);
SetScaleFactor(0.372462);
ForwardVisualSearchNetwork(0);
MoveToPoint(488,66); 
MoveToPoint(488,66);
#pause;
GetImage(5091);
SetScaleFactor(0.358391);
ForwardVisualSearchNetwork(0);
MoveToPoint(499,66); 
MoveToPoint(499,66);
#pause;
GetImage(5092);
SetScaleFactor(0.345299);
ForwardVisualSearchNetwork(0);
MoveToPoint(505,67); 
MoveToPoint(505,67);
#pause;
GetImage(5093);
SetScaleFactor(0.345299);
ForwardVisualSearchNetwork(0);
MoveToPoint(517,66); 
MoveToPoint(517,66);
#pause;
GetImage(5094);
SetScaleFactor(0.345299);
ForwardVisualSearchNetwork(0);
MoveToPoint(523,60); 
MoveToPoint(523,60);
#pause;
GetImage(5095);
SetScaleFactor(0.328856);
ForwardVisualSearchNetwork(0);
MoveToPoint(525,61); 
MoveToPoint(525,61);
#pause;
GetImage(5096);
SetScaleFactor(0.328372);
ForwardVisualSearchNetwork(0);
MoveToPoint(525,61); 
MoveToPoint(525,61);
#pause;
GetImage(5097);
SetScaleFactor(0.322601);
ForwardVisualSearchNetwork(0);
MoveToPoint(531,61); 
MoveToPoint(531,61);
#pause;
GetImage(5098);
SetScaleFactor(0.315506);
ForwardVisualSearchNetwork(0);
MoveToPoint(552,61); 
MoveToPoint(552,61);
#pause;
GetImage(5099);
SetScaleFactor(0.307291);
ForwardVisualSearchNetwork(0);
MoveToPoint(568,67); 
MoveToPoint(568,67);
#pause;
GetImage(5100);
SetScaleFactor(0.295808);
ForwardVisualSearchNetwork(0);
MoveToPoint(584,67); 
MoveToPoint(584,67);
#pause;
GetImage(5101);
SetScaleFactor(0.283505);
ForwardVisualSearchNetwork(0);
MoveToPoint(601,70); 
MoveToPoint(601,70);
#pause;
GetImage(5102);
SetScaleFactor(0.295846);
ForwardVisualSearchNetwork(0);
MoveToPoint(566,74); 
MoveToPoint(566,74);
#pause;
GetImage(5103);
SetScaleFactor(0.309389);
ForwardVisualSearchNetwork(0);
MoveToPoint(566,74); 
MoveToPoint(566,74);
#pause;
GetImage(5104);
SetScaleFactor(0.323782);
ForwardVisualSearchNetwork(0);
MoveToPoint(529,74); 
MoveToPoint(529,74);
#pause;
GetImage(5105);
SetScaleFactor(0.338204);
ForwardVisualSearchNetwork(0);
MoveToPoint(553,68); 
MoveToPoint(553,68);
#pause;
GetImage(5106);
SetScaleFactor(0.352965);
ForwardVisualSearchNetwork(0);
MoveToPoint(557,69); 
MoveToPoint(557,69);
#pause;
GetImage(5107);
SetScaleFactor(0.368734);
ForwardVisualSearchNetwork(0);
MoveToPoint(572,75); 
MoveToPoint(572,75);
#pause;
GetImage(5108);
SetScaleFactor(0.386670);
ForwardVisualSearchNetwork(0);
MoveToPoint(562,54); 
MoveToPoint(562,54);
#pause;
GetImage(5109);
SetScaleFactor(0.405410);
ForwardVisualSearchNetwork(0);
MoveToPoint(576,59); 
MoveToPoint(576,59);
#pause;
GetImage(5110);
SetScaleFactor(0.426044);
ForwardVisualSearchNetwork(0);
MoveToPoint(565,7); 
MoveToPoint(565,7);
#pause;
GetImage(5111);
SetScaleFactor(0.447806);
ForwardVisualSearchNetwork(0);
MoveToPoint(581,2); 
MoveToPoint(581,2);
#pause;
GetImage(5112);
SetScaleFactor(0.470778);
ForwardVisualSearchNetwork(0);
MoveToPoint(565,51); 
MoveToPoint(565,51);
#pause;
GetImage(5113);
SetScaleFactor(0.494330);
ForwardVisualSearchNetwork(0);
MoveToPoint(559,68); 
MoveToPoint(559,68);
#pause;
GetImage(5114);
SetScaleFactor(0.494330);
ForwardVisualSearchNetwork(0);
MoveToPoint(532,69); 
MoveToPoint(532,69);
#pause;
GetImage(5115);
SetScaleFactor(0.472550);
ForwardVisualSearchNetwork(0);
MoveToPoint(529,70); 
MoveToPoint(529,70);
#pause;
GetImage(5116);
SetScaleFactor(0.472550);
ForwardVisualSearchNetwork(0);
MoveToPoint(541,70); 
MoveToPoint(541,70);
#pause;
GetImage(5117);
SetScaleFactor(0.472550);
ForwardVisualSearchNetwork(0);
MoveToPoint(550,70); 
MoveToPoint(550,70);
#pause;
GetImage(5118);
SetScaleFactor(0.452265);
ForwardVisualSearchNetwork(0);
MoveToPoint(576,66); 
MoveToPoint(576,66);
#pause;
GetImage(5119);
SetScaleFactor(0.473742);
ForwardVisualSearchNetwork(0);
MoveToPoint(573,63); 
MoveToPoint(573,63);
#pause;
GetImage(5120);
SetScaleFactor(0.496064);
ForwardVisualSearchNetwork(0);
MoveToPoint(551,67); 
MoveToPoint(551,67);
#pause;
GetImage(5121);
SetScaleFactor(0.520947);
ForwardVisualSearchNetwork(0);
MoveToPoint(632,59); 
MoveToPoint(632,59);
#pause;
GetImage(5122);
SetScaleFactor(0.547595);
ForwardVisualSearchNetwork(0);
MoveToPoint(624,58); 
MoveToPoint(624,58);
#pause;
GetImage(5123);
SetScaleFactor(0.574167);
ForwardVisualSearchNetwork(0);
MoveToPoint(597,58); 
MoveToPoint(597,58);
#pause;
GetImage(5124);
SetScaleFactor(0.547991);
ForwardVisualSearchNetwork(0);
MoveToPoint(587,59); 
MoveToPoint(587,59);
#pause;
GetImage(5125);
SetScaleFactor(0.575608);
ForwardVisualSearchNetwork(0);
MoveToPoint(592,56); 
MoveToPoint(592,56);
#pause;
GetImage(5311);
SetScaleFactor(1.000000);
ForwardVisualSearchNetwork(0);
MoveToPoint(557,36); 
MoveToPoint(557,36);
#pause;
GetImage(5312);
SetScaleFactor(0.957503);
ForwardVisualSearchNetwork(0);
MoveToPoint(549,37); 
MoveToPoint(549,37);
#pause;
GetImage(5313);
SetScaleFactor(0.916631);
ForwardVisualSearchNetwork(0);
MoveToPoint(542,37); 
MoveToPoint(542,37);
#pause;
GetImage(5314);
SetScaleFactor(0.916631);
ForwardVisualSearchNetwork(0);
MoveToPoint(580,32); 
MoveToPoint(580,32);
#pause;
GetImage(5315);
SetScaleFactor(0.879190);
ForwardVisualSearchNetwork(0);
MoveToPoint(596,31); 
MoveToPoint(596,31);
#pause;
GetImage(5316);
SetScaleFactor(0.879190);
ForwardVisualSearchNetwork(0);
MoveToPoint(585,33); 
MoveToPoint(585,33);
#pause;
GetImage(5317);
SetScaleFactor(0.842034);
ForwardVisualSearchNetwork(0);
MoveToPoint(602,31); 
MoveToPoint(602,31);
#pause;
GetImage(5318);
SetScaleFactor(0.879975);
ForwardVisualSearchNetwork(0);
MoveToPoint(592,31); 
MoveToPoint(592,31);
#pause;
GetImage(5319);
SetScaleFactor(0.843319);
ForwardVisualSearchNetwork(0);
MoveToPoint(606,26); 
MoveToPoint(606,26);
#pause;
GetImage(5320);
SetScaleFactor(0.881411);
ForwardVisualSearchNetwork(0);
MoveToPoint(597,26); 
MoveToPoint(597,26);
#pause;
GetImage(5321);
SetScaleFactor(0.881411);
ForwardVisualSearchNetwork(0);
MoveToPoint(588,21); 
MoveToPoint(588,21);
#pause;
GetImage(5322);
SetScaleFactor(0.881411);
ForwardVisualSearchNetwork(0);
MoveToPoint(603,14); 
MoveToPoint(603,14);
#pause;
GetImage(5323);
SetScaleFactor(0.918371);
ForwardVisualSearchNetwork(0);
MoveToPoint(594,9); 
MoveToPoint(594,9);
#pause;
GetImage(5324);
SetScaleFactor(0.879443);
ForwardVisualSearchNetwork(0);
MoveToPoint(583,9); 
MoveToPoint(583,9);
#pause;
GetImage(5345);
SetScaleFactor(1.000000);
ForwardVisualSearchNetwork(0);
MoveToPoint(561,39); 
MoveToPoint(561,39);
#pause;
GetImage(5346);
SetScaleFactor(1.000000);
ForwardVisualSearchNetwork(0);
MoveToPoint(560,39); 
MoveToPoint(560,39);
#pause;
GetImage(5347);
SetScaleFactor(1.000000);
ForwardVisualSearchNetwork(0);
MoveToPoint(571,35); 
MoveToPoint(571,35);
#pause;
GetImage(5348);
SetScaleFactor(0.960557);
ForwardVisualSearchNetwork(0);
MoveToPoint(562,35); 
MoveToPoint(562,35);
#pause;
GetImage(5349);
SetScaleFactor(0.918622);
ForwardVisualSearchNetwork(0);
MoveToPoint(576,29); 
MoveToPoint(576,29);
#pause;
GetImage(5350);
SetScaleFactor(0.895527);
ForwardVisualSearchNetwork(0);
MoveToPoint(564,29); 
MoveToPoint(564,29);
#pause;
GetImage(5351);
SetScaleFactor(0.895527);
ForwardVisualSearchNetwork(0);
MoveToPoint(566,29); 
MoveToPoint(566,29);
#pause;
GetImage(5352);
SetScaleFactor(0.895527);
ForwardVisualSearchNetwork(0);
MoveToPoint(585,25); 
MoveToPoint(585,25);
#pause;
GetImage(5353);
SetScaleFactor(0.889171);
ForwardVisualSearchNetwork(0);
MoveToPoint(576,30); 
MoveToPoint(576,30);
#pause;
GetImage(5354);
SetScaleFactor(0.857316);
ForwardVisualSearchNetwork(0);
MoveToPoint(591,28); 
MoveToPoint(591,28);
#pause;
GetImage(5355);
SetScaleFactor(0.857316);
ForwardVisualSearchNetwork(0);
MoveToPoint(582,31); 
MoveToPoint(582,31);
#pause;
GetImage(5356);
SetScaleFactor(0.832412);
ForwardVisualSearchNetwork(0);
MoveToPoint(601,26); 
MoveToPoint(601,26);
#pause;
GetImage(5357);
SetScaleFactor(0.797239);
ForwardVisualSearchNetwork(0);
MoveToPoint(615,24); 
MoveToPoint(615,24);
#pause;
GetImage(5358);
SetScaleFactor(0.797239);
ForwardVisualSearchNetwork(0);
MoveToPoint(606,24); 
MoveToPoint(606,24);
#pause;
GetImage(5359);
SetScaleFactor(0.836947);
ForwardVisualSearchNetwork(0);
MoveToPoint(592,28); 
MoveToPoint(592,28);
#pause;
GetImage(5360);
SetScaleFactor(0.797092);
ForwardVisualSearchNetwork(0);
MoveToPoint(608,24); 
MoveToPoint(608,24);
#pause;
GetImage(5361);
SetScaleFactor(0.759135);
ForwardVisualSearchNetwork(0);
MoveToPoint(598,24); 
MoveToPoint(598,24);
#pause;
GetImage(5362);
SetScaleFactor(0.777551);
ForwardVisualSearchNetwork(0);
MoveToPoint(603,24); 
MoveToPoint(603,24);
#pause;
GetImage(5363);
SetScaleFactor(0.777551);
ForwardVisualSearchNetwork(0);
MoveToPoint(606,21); 
MoveToPoint(606,21);
#pause;
GetImage(5364);
SetScaleFactor(0.777551);
ForwardVisualSearchNetwork(0);
MoveToPoint(606,24); 
MoveToPoint(606,24);
#pause;
GetImage(5365);
SetScaleFactor(0.800405);
ForwardVisualSearchNetwork(0);
MoveToPoint(601,24); 
MoveToPoint(601,24);
#pause;
GetImage(5366);
SetScaleFactor(0.828823);
ForwardVisualSearchNetwork(0);
MoveToPoint(595,28); 
MoveToPoint(595,28);
#pause;
GetImage(5367);
SetScaleFactor(0.858003);
ForwardVisualSearchNetwork(0);
MoveToPoint(606,26); 
MoveToPoint(606,26);
#pause;
GetImage(5368);
SetScaleFactor(0.828919);
ForwardVisualSearchNetwork(0);
MoveToPoint(604,26); 
MoveToPoint(604,26);
#pause;
GetImage(5369);
SetScaleFactor(0.828919);
ForwardVisualSearchNetwork(0);
MoveToPoint(612,24); 
MoveToPoint(612,24);
#pause;
GetImage(5370);
SetScaleFactor(0.800169);
ForwardVisualSearchNetwork(0);
MoveToPoint(596,30); 
MoveToPoint(596,30);
#pause;
GetImage(5371);
SetScaleFactor(0.790403);
ForwardVisualSearchNetwork(0);
MoveToPoint(594,35); 
MoveToPoint(594,35);
#pause;
GetImage(5372);
SetScaleFactor(0.790403);
ForwardVisualSearchNetwork(0);
MoveToPoint(610,38); 
MoveToPoint(610,38);
#pause;
GetImage(5373);
SetScaleFactor(0.790403);
ForwardVisualSearchNetwork(0);
MoveToPoint(605,41); 
MoveToPoint(605,41);
#pause;
GetImage(5374);
SetScaleFactor(0.790403);
ForwardVisualSearchNetwork(0);
MoveToPoint(614,38); 
MoveToPoint(614,38);
#pause;
GetImage(5375);
SetScaleFactor(0.790403);
ForwardVisualSearchNetwork(0);
MoveToPoint(604,38); 
MoveToPoint(604,38);
#pause;
GetImage(5376);
SetScaleFactor(0.759615);
ForwardVisualSearchNetwork(0);
MoveToPoint(618,35); 
MoveToPoint(618,35);
#pause;
GetImage(5377);
SetScaleFactor(0.759615);
ForwardVisualSearchNetwork(0);
MoveToPoint(607,35); 
MoveToPoint(607,35);
#pause;
GetImage(5378);
SetScaleFactor(0.793746);
ForwardVisualSearchNetwork(0);
MoveToPoint(624,32); 
MoveToPoint(624,32);
#pause;
GetImage(5379);
SetScaleFactor(0.779108);
ForwardVisualSearchNetwork(0);
MoveToPoint(612,35); 
MoveToPoint(612,35);
#pause;
GetImage(5380);
SetScaleFactor(0.752092);
ForwardVisualSearchNetwork(0);
MoveToPoint(629,35); 
MoveToPoint(629,35);
#pause;
GetImage(5381);
SetScaleFactor(0.781938);
ForwardVisualSearchNetwork(0);
MoveToPoint(618,41); 
MoveToPoint(618,41);
#pause;
GetImage(5382);
SetScaleFactor(0.781938);
ForwardVisualSearchNetwork(0);
MoveToPoint(610,44); 
MoveToPoint(610,44);
#pause;
GetImage(5383);
SetScaleFactor(0.756153);
ForwardVisualSearchNetwork(0);
MoveToPoint(627,41); 
MoveToPoint(627,41);
#pause;
GetImage(5384);
SetScaleFactor(0.788934);
ForwardVisualSearchNetwork(0);
MoveToPoint(615,44); 
MoveToPoint(615,44);
#pause;
GetImage(5385);
SetScaleFactor(0.772298);
ForwardVisualSearchNetwork(0);
MoveToPoint(605,47); 
MoveToPoint(605,47);
#pause;
GetImage(5386);
SetScaleFactor(0.772298);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,42); 
MoveToPoint(621,42);
#pause;
GetImage(5387);
SetScaleFactor(0.772298);
ForwardVisualSearchNetwork(0);
MoveToPoint(614,42); 
MoveToPoint(614,42);
#pause;
GetImage(5388);
SetScaleFactor(0.802632);
ForwardVisualSearchNetwork(0);
MoveToPoint(631,37); 
MoveToPoint(631,37);
#pause;
GetImage(5389);
SetScaleFactor(0.802632);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,37); 
MoveToPoint(621,37);
#pause;
GetImage(5390);
SetScaleFactor(0.820428);
ForwardVisualSearchNetwork(0);
MoveToPoint(622,37); 
MoveToPoint(622,37);
#pause;
GetImage(5391);
SetScaleFactor(0.832926);
ForwardVisualSearchNetwork(0);
MoveToPoint(627,37); 
MoveToPoint(627,37);
#pause;
GetImage(5392);
SetScaleFactor(0.832926);
ForwardVisualSearchNetwork(0);
MoveToPoint(620,39); 
MoveToPoint(620,39);
#pause;
GetImage(5393);
SetScaleFactor(0.832926);
ForwardVisualSearchNetwork(0);
MoveToPoint(613,39); 
MoveToPoint(613,39);
#pause;
GetImage(5394);
SetScaleFactor(0.842991);
ForwardVisualSearchNetwork(0);
MoveToPoint(629,37); 
MoveToPoint(629,37);
#pause;
GetImage(5395);
SetScaleFactor(0.802849);
ForwardVisualSearchNetwork(0);
MoveToPoint(617,39); 
MoveToPoint(617,39);
#pause;
GetImage(5396);
SetScaleFactor(0.825623);
ForwardVisualSearchNetwork(0);
MoveToPoint(633,34); 
MoveToPoint(633,34);
#pause;
GetImage(5397);
SetScaleFactor(0.825623);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,38); 
MoveToPoint(621,38);
#pause;
GetImage(5398);
SetScaleFactor(0.806959);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,38); 
MoveToPoint(621,38);
#pause;
GetImage(5399);
SetScaleFactor(0.806959);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,43); 
MoveToPoint(621,43);
#pause;
GetImage(5400);
SetScaleFactor(0.768532);
ForwardVisualSearchNetwork(0);
MoveToPoint(611,47); 
MoveToPoint(611,47);
#pause;
GetImage(5401);
SetScaleFactor(0.780831);
ForwardVisualSearchNetwork(0);
MoveToPoint(631,44); 
MoveToPoint(631,44);
#pause;
GetImage(5402);
SetScaleFactor(0.803036);
ForwardVisualSearchNetwork(0);
MoveToPoint(635,42); 
MoveToPoint(635,42);
#pause;
GetImage(5403);
SetScaleFactor(0.803036);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,44); 
MoveToPoint(621,44);
#pause;
GetImage(5404);
SetScaleFactor(0.829090);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,44); 
MoveToPoint(621,44);
#pause;
GetImage(5405);
SetScaleFactor(0.805970);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,44); 
MoveToPoint(621,44);
#pause;
GetImage(5406);
SetScaleFactor(0.818279);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,46); 
MoveToPoint(621,46);
#pause;
GetImage(5407);
SetScaleFactor(0.799216);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,48); 
MoveToPoint(621,48);
#pause;
GetImage(5408);
SetScaleFactor(0.813181);
ForwardVisualSearchNetwork(0);
MoveToPoint(630,48); 
MoveToPoint(630,48);
#pause;
GetImage(5409);
SetScaleFactor(0.820937);
ForwardVisualSearchNetwork(0);
MoveToPoint(620,50); 
MoveToPoint(620,50);
#pause;
GetImage(5410);
SetScaleFactor(0.804726);
ForwardVisualSearchNetwork(0);
MoveToPoint(620,48); 
MoveToPoint(620,48);
#pause;
GetImage(5411);
SetScaleFactor(0.785654);
ForwardVisualSearchNetwork(0);
MoveToPoint(620,48); 
MoveToPoint(620,48);
#pause;
GetImage(5412);
SetScaleFactor(0.753795);
ForwardVisualSearchNetwork(0);
MoveToPoint(620,48); 
MoveToPoint(620,48);
#pause;
GetImage(5413);
SetScaleFactor(0.721628);
ForwardVisualSearchNetwork(0);
MoveToPoint(617,51); 
MoveToPoint(617,51);
#pause;
GetImage(5414);
SetScaleFactor(0.758981);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,46); 
MoveToPoint(621,46);
#pause;
GetImage(5415);
SetScaleFactor(0.758981);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,59); 
MoveToPoint(621,59);
#pause;
GetImage(5416);
SetScaleFactor(0.798928);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,62); 
MoveToPoint(621,62);
#pause;
GetImage(5417);
SetScaleFactor(0.787742);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,67); 
MoveToPoint(621,67);
#pause;
GetImage(5418);
SetScaleFactor(0.787742);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,67); 
MoveToPoint(621,67);
#pause;
GetImage(5419);
SetScaleFactor(0.787742);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,67); 
MoveToPoint(621,67);
#pause;
GetImage(5420);
SetScaleFactor(0.787742);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,67); 
MoveToPoint(621,67);
#pause;
GetImage(5421);
SetScaleFactor(0.787742);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,64); 
MoveToPoint(621,64);
#pause;
GetImage(5422);
SetScaleFactor(0.812531);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,61); 
MoveToPoint(621,61);
#pause;
GetImage(5423);
SetScaleFactor(0.812531);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,56); 
MoveToPoint(621,56);
#pause;
GetImage(5424);
SetScaleFactor(0.800039);
ForwardVisualSearchNetwork(0);
MoveToPoint(619,54); 
MoveToPoint(619,54);
#pause;
GetImage(5425);
SetScaleFactor(0.800039);
ForwardVisualSearchNetwork(0);
MoveToPoint(633,52); 
MoveToPoint(633,52);
#pause;
GetImage(5426);
SetScaleFactor(0.800039);
ForwardVisualSearchNetwork(0);
MoveToPoint(631,52); 
MoveToPoint(631,52);
#pause;
GetImage(5427);
SetScaleFactor(0.768149);
ForwardVisualSearchNetwork(0);
MoveToPoint(631,52); 
MoveToPoint(631,52);
#pause;
GetImage(5428);
SetScaleFactor(0.785046);
ForwardVisualSearchNetwork(0);
MoveToPoint(628,49); 
MoveToPoint(628,49);
#pause;
GetImage(5429);
SetScaleFactor(0.785046);
ForwardVisualSearchNetwork(0);
MoveToPoint(623,52); 
MoveToPoint(623,52);
#pause;
GetImage(5430);
SetScaleFactor(0.785046);
ForwardVisualSearchNetwork(0);
MoveToPoint(620,55); 
MoveToPoint(620,55);
#pause;
GetImage(5431);
SetScaleFactor(0.782837);
ForwardVisualSearchNetwork(0);
MoveToPoint(617,55); 
MoveToPoint(617,55);
#pause;
GetImage(5432);
SetScaleFactor(0.782837);
ForwardVisualSearchNetwork(0);
MoveToPoint(634,50); 
MoveToPoint(634,50);
#pause;
GetImage(5433);
SetScaleFactor(0.804748);
ForwardVisualSearchNetwork(0);
MoveToPoint(623,51); 
MoveToPoint(623,51);
#pause;
GetImage(5434);
SetScaleFactor(0.774107);
ForwardVisualSearchNetwork(0);
MoveToPoint(632,49); 
MoveToPoint(632,49);
#pause;
GetImage(5435);
SetScaleFactor(0.804631);
ForwardVisualSearchNetwork(0);
MoveToPoint(632,49); 
MoveToPoint(632,49);
#pause;
GetImage(5436);
SetScaleFactor(0.804631);
ForwardVisualSearchNetwork(0);
MoveToPoint(622,50); 
MoveToPoint(622,50);
#pause;
GetImage(5437);
SetScaleFactor(0.804631);
ForwardVisualSearchNetwork(0);
MoveToPoint(622,50); 
MoveToPoint(622,50);
#pause;
GetImage(5438);
SetScaleFactor(0.774470);
ForwardVisualSearchNetwork(0);
MoveToPoint(622,48); 
MoveToPoint(622,48);
#pause;
GetImage(5439);
SetScaleFactor(0.748152);
ForwardVisualSearchNetwork(0);
MoveToPoint(622,48); 
MoveToPoint(622,48);
#pause;
GetImage(5440);
SetScaleFactor(0.720000);
ForwardVisualSearchNetwork(0);
MoveToPoint(619,51); 
MoveToPoint(619,51);
#pause;
GetImage(5441);
SetScaleFactor(0.747167);
ForwardVisualSearchNetwork(0);
MoveToPoint(619,51); 
MoveToPoint(619,51);
#pause;
GetImage(5442);
SetScaleFactor(0.773926);
ForwardVisualSearchNetwork(0);
MoveToPoint(619,51); 
MoveToPoint(619,51);
#pause;
GetImage(5443);
SetScaleFactor(0.805583);
ForwardVisualSearchNetwork(0);
MoveToPoint(619,52); 
MoveToPoint(619,52);
#pause;
GetImage(5444);
SetScaleFactor(0.844327);
ForwardVisualSearchNetwork(0);
MoveToPoint(607,57); 
MoveToPoint(607,57);
#pause;
GetImage(5445);
SetScaleFactor(0.877589);
ForwardVisualSearchNetwork(0);
MoveToPoint(622,55); 
MoveToPoint(622,55);
#pause;
GetImage(5446);
SetScaleFactor(0.839248);
ForwardVisualSearchNetwork(0);
MoveToPoint(622,55); 
MoveToPoint(622,55);
#pause;
GetImage(5447);
SetScaleFactor(0.875884);
ForwardVisualSearchNetwork(0);
MoveToPoint(624,52); 
MoveToPoint(624,52);
#pause;
GetImage(5448);
SetScaleFactor(0.915335);
ForwardVisualSearchNetwork(0);
MoveToPoint(625,51); 
MoveToPoint(625,51);
#pause;
GetImage(5449);
SetScaleFactor(0.961231);
ForwardVisualSearchNetwork(0);
MoveToPoint(624,19); 
MoveToPoint(624,19);
#pause;
GetImage(5450);
SetScaleFactor(1.010624);
ForwardVisualSearchNetwork(0);
MoveToPoint(598,47); 
MoveToPoint(598,47);
#pause;
GetImage(5458);
SetScaleFactor(1.000000);
ForwardVisualSearchNetwork(0);
MoveToPoint(582,43); 
MoveToPoint(582,43);
#pause;
GetImage(5459);
SetScaleFactor(0.986759);
ForwardVisualSearchNetwork(0);
MoveToPoint(581,41); 
MoveToPoint(581,41);
#pause;
GetImage(5460);
SetScaleFactor(0.965472);
ForwardVisualSearchNetwork(0);
MoveToPoint(590,37); 
MoveToPoint(590,37);
#pause;
GetImage(5461);
SetScaleFactor(0.927645);
ForwardVisualSearchNetwork(0);
MoveToPoint(606,35); 
MoveToPoint(606,35);
#pause;
GetImage(5462);
SetScaleFactor(0.901116);
ForwardVisualSearchNetwork(0);
MoveToPoint(622,31); 
MoveToPoint(622,31);
#pause;
GetImage(5463);
SetScaleFactor(0.871630);
ForwardVisualSearchNetwork(0);
MoveToPoint(617,31); 
MoveToPoint(617,31);
#pause;
GetImage(5464);
SetScaleFactor(0.840266);
ForwardVisualSearchNetwork(0);
MoveToPoint(634,29); 
MoveToPoint(634,29);
#pause;
GetImage(5465);
SetScaleFactor(0.863477);
ForwardVisualSearchNetwork(0);
MoveToPoint(624,29); 
MoveToPoint(624,29);
#pause;
GetImage(5466);
SetScaleFactor(0.863477);
ForwardVisualSearchNetwork(0);
MoveToPoint(619,32); 
MoveToPoint(619,32);
#pause;
GetImage(5467);
SetScaleFactor(0.886669);
ForwardVisualSearchNetwork(0);
MoveToPoint(624,34); 
MoveToPoint(624,34);
#pause;
GetImage(5468);
SetScaleFactor(0.896597);
ForwardVisualSearchNetwork(0);
MoveToPoint(624,36); 
MoveToPoint(624,36);
#pause;
GetImage(5469);
SetScaleFactor(0.872678);
ForwardVisualSearchNetwork(0);
MoveToPoint(624,36); 
MoveToPoint(624,36);
#pause;
GetImage(5470);
SetScaleFactor(0.898969);
ForwardVisualSearchNetwork(0);
MoveToPoint(634,32); 
MoveToPoint(634,32);
#pause;
GetImage(5471);
SetScaleFactor(0.898969);
ForwardVisualSearchNetwork(0);
MoveToPoint(624,36); 
MoveToPoint(624,36);
#pause;
GetImage(5472);
SetScaleFactor(0.898969);
ForwardVisualSearchNetwork(0);
MoveToPoint(624,38); 
MoveToPoint(624,38);
#pause;
GetImage(5473);
SetScaleFactor(0.898969);
ForwardVisualSearchNetwork(0);
MoveToPoint(624,40); 
MoveToPoint(624,40);
#pause;
GetImage(5474);
SetScaleFactor(0.888034);
ForwardVisualSearchNetwork(0);
MoveToPoint(616,42); 
MoveToPoint(616,42);
#pause;
GetImage(5475);
SetScaleFactor(0.908937);
ForwardVisualSearchNetwork(0);
MoveToPoint(625,42); 
MoveToPoint(625,42);
#pause;
GetImage(5476);
SetScaleFactor(0.905071);
ForwardVisualSearchNetwork(0);
MoveToPoint(618,42); 
MoveToPoint(618,42);
#pause;
GetImage(5477);
SetScaleFactor(0.885140);
ForwardVisualSearchNetwork(0);
MoveToPoint(610,40); 
MoveToPoint(610,40);
#pause;
GetImage(5478);
SetScaleFactor(0.885140);
ForwardVisualSearchNetwork(0);
MoveToPoint(622,38); 
MoveToPoint(622,38);
#pause;
GetImage(5479);
SetScaleFactor(0.856431);
ForwardVisualSearchNetwork(0);
MoveToPoint(615,38); 
MoveToPoint(615,38);
#pause;
GetImage(5480);
SetScaleFactor(0.828883);
ForwardVisualSearchNetwork(0);
MoveToPoint(628,36); 
MoveToPoint(628,36);
#pause;
GetImage(5481);
SetScaleFactor(0.828883);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,36); 
MoveToPoint(621,36);
#pause;
GetImage(5482);
SetScaleFactor(0.855177);
ForwardVisualSearchNetwork(0);
MoveToPoint(613,38); 
MoveToPoint(613,38);
#pause;
GetImage(5483);
SetScaleFactor(0.883324);
ForwardVisualSearchNetwork(0);
MoveToPoint(623,36); 
MoveToPoint(623,36);
#pause;
GetImage(5484);
SetScaleFactor(0.866228);
ForwardVisualSearchNetwork(0);
MoveToPoint(625,38); 
MoveToPoint(625,38);
#pause;
GetImage(5485);
SetScaleFactor(0.830845);
ForwardVisualSearchNetwork(0);
MoveToPoint(631,40); 
MoveToPoint(631,40);
#pause;
GetImage(5486);
SetScaleFactor(0.852178);
ForwardVisualSearchNetwork(0);
MoveToPoint(621,42); 
MoveToPoint(621,42);
#pause;
GetImage(5487);
SetScaleFactor(0.875931);
ForwardVisualSearchNetwork(0);
MoveToPoint(623,42); 
MoveToPoint(623,42);
#pause;
GetImage(5488);
SetScaleFactor(0.875931);
ForwardVisualSearchNetwork(0);
MoveToPoint(623,42); 
MoveToPoint(623,42);
#pause;
GetImage(5489);
SetScaleFactor(0.850224);
ForwardVisualSearchNetwork(0);
MoveToPoint(623,42); 
MoveToPoint(623,42);
#pause;
GetImage(5490);
SetScaleFactor(0.865439);
ForwardVisualSearchNetwork(0);
MoveToPoint(623,42); 
MoveToPoint(623,42);
#pause;
GetImage(5491);
SetScaleFactor(0.857224);
ForwardVisualSearchNetwork(0);
MoveToPoint(618,42); 
MoveToPoint(618,42);
#pause;
GetImage(5492);
SetScaleFactor(0.892908);
ForwardVisualSearchNetwork(0);
MoveToPoint(625,40); 
MoveToPoint(625,40);
#pause;
GetImage(5493);
SetScaleFactor(0.892908);
ForwardVisualSearchNetwork(0);
MoveToPoint(625,40); 
MoveToPoint(625,40);
#pause;
GetImage(5494);
SetScaleFactor(0.878693);
ForwardVisualSearchNetwork(0);
MoveToPoint(615,38); 
MoveToPoint(615,38);
#pause;
GetImage(5495);
SetScaleFactor(0.896487);
ForwardVisualSearchNetwork(0);
MoveToPoint(628,36); 
MoveToPoint(628,36);
#pause;
GetImage(5496);
SetScaleFactor(0.885469);
ForwardVisualSearchNetwork(0);
MoveToPoint(624,34); 
MoveToPoint(624,34);
#pause;
GetImage(5497);
SetScaleFactor(0.921625);
ForwardVisualSearchNetwork(0);
MoveToPoint(624,34); 
MoveToPoint(624,34);
#pause;
GetImage(5498);
SetScaleFactor(0.963071);
ForwardVisualSearchNetwork(0);
MoveToPoint(624,32); 
MoveToPoint(624,32);
#pause;
GetImage(5499);
SetScaleFactor(1.012227);
ForwardVisualSearchNetwork(0);
MoveToPoint(620,39); 
MoveToPoint(620,39);
#pause;
GetImage(5500);
SetScaleFactor(1.012227);
ForwardVisualSearchNetwork(0);
MoveToPoint(625,48); 
MoveToPoint(625,48);
###
DisposeMemory(0);
quit;
