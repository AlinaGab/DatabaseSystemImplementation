#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"
#include "Record.h"
#include "HP.h"

#include <sys/stat.h>


char *surname[58]={"mirnov","Ivanov","Kuznetsov","Popov","Sokolov","Lebedev",
	"Kozlov","Novikov","Morozov","Petrov","Volkov","Solovyov","Vasilyev","Rose",
	"Zaytsev","Pavlov","Semyonov","Golubev","Vinogradov","Bogdanov","Vorobyov",
	"vanovic","Petrovic","Milosevic","Markovic","Pavlovic","Knezevic","Davies",
	"Todorovic","Nikolic","Stankovic","Vukovic","Savic","Maric","Brown","Smith",
	"Patel","Jones","Williams","Johnson","Taylor","Thomas","Roberts","Khan",
	"Lewis","Jackson","Clarke","James","Phillips","Wilson","Ali","Mason","Cox",
	"Mitchell","Davis","Rodríguez","Alexander",NULL };

char *city[112]={"Athens","Thessaloniki","Piraeus","Patras","Heraklion","Argos",
	"Peristeri","Larissa","Kallithea","Nikaia","Kalamaria","Glyfada","Acharnes",
	"Volos","Ilio","Keratsini","Ilioupoli","Chalandri","Marousi","Zografou",
	"Egaleo","Agios","Korydallos","Evosmos","Palaio","Ioannina","Agia","Sparta",
	"Vyronas","Kavala","Galatsi","Rhodes","Serres","Alexandroupoli","Chania",
	"Chalcis","Petroupoli","Katerini","Kalamata","Trikala","Xanthi","Lamia",
	"Irakleio","Komotini","Kifissia","Sykies","Veria","Chaidari","Drama","Arta",
	"Agrinio","Stavroupoli","Alimos","Polichni","Ampelokipoi","Kozani","Agios",
	"Aspropyrgos","Argyroupoli","Karditsa","Chios","Vrilissia","Agia","Corinth",
	"Cholargos","Voula","Rethymno","Ptolemaida","Metamorfosi","Ano","Mytilene",
	"Giannitsa","Eleusina","Salamis","Tripoli","Perama","Corfu","Kaisariani",
	"Kamatero","Eleftherio","Megara","Melissia","Pylaia","Moschato","Thermi",
	"Dafni","Artemis","Thebes","Pyrgos","Kilkis","Livadeia","Pefki","Aigio",
	"Amaliada","Kos","Naousa","Edessa","Ellinikon","Gerakas","Koropi","Kalyvia",
	"Preveza","Panorama","Peraia","Oraiokastro","Orestiada","Pallini","Florina",
	"Menemeni","Paiania",NULL};

char *name[293]={"August","Augusta","Avrele","Agatha","Agnes","Ada","Adam",
	"Alexander","Alexandria","Alvina","Ambrose","Amelia","Amilian","Anastasia",
	"Anatole","Angeline","Andrew","Andrian","Andriana","Anina","Ann","Anthony",
	"Antonina","Apollinary","Apollinaria","Apollonia","Arkadiy","August","Emil",
	"Augusta","Barbara","Basil","Bartholomew","Bohdan","Bohdanna","Bohuslav",
	"Bohuslava","Boniface","Boris","Boryslav","Boryslava","Benedict","Candid",
	"Candida","Cecelia","Celestine","Charita","Christian","Christine","Emily",
	"Christopher","Clara","Clem","Clementine","Conrad","Constantine","Eugene",
	"Constance","Cyprian","Cyril","Cyrus","Daniel","Daria","David","Demetrius",
	"Dennis","Denise","Diodor","Dorothea","Edward","Elias","Elizabeth","Ella",
	"Eugenia","Eunice","Eustace","Eva","Evdokia","Fedir","Felix","Fialka","Les",
	"Flavia","Flora","Florent","Florence","Gabriel","George","Georgina","Ihor",
	"Gervais","Gregory","Hannah","Helen","Herman","Hilary","Hnat","Ignatius",
	"Ilaria","Ilarion","Ilary","Irene","Isabel","Isadore","Isidora","Jeremiah",
	"Jervis","Joachim","John","Joseph","Josephine","Judith","Julian","Julianna",
	"June","Justin","Justina","Kalyna","Katherine","Krystentia","Ksenia","Ivan",
	"Jacob","Kuzma","Larissa","Laura","Lawrence","Leon","Leonard","Leonid",
	"Lesia","Lev","Louise","Lubomyr","Lubomyra","Lubov","Lucian","Ludmilla",
	"Luke","Lukia","Lusia","Lydia","Magdalyna","Maksym","Marcel","Margaret",
	"Maria","Marian","Marianna","Markian","Marko","Martha","Martin","Maryna",
	"Matthew","Maura","Maya","Maximillian","Mecheslav","Mecheslava","Melania",
	"Methodius","Michael","Michaelina","Mina","Monica","Mstyslav","Mstyslava",
	"Mykyta","Myron","Myroslav","Myroslava","Nadia","Natalia","Nathaniel",
	"Nestor","Nicholas","Nina","Nona","Nykon","Oksana","Oleh","Oleksa","Oles",
	"Olesia","Olha","Onopriy","Oryna","Ostap","Palahna","Palladiy","Paul",
	"Pauline","Peter","Petronella","Philip","Phillipa","Philemon","Philemona",
	"Pollyanna","Potap","Priscilla","Prokip","Rachel","Radoslav","Radomyr",
	"Raisa","Raphael","Rebecca","Regina","Roman","Romanna","Rosalia","Sandra",
	"Rostyslav","Rostyslava","Roxoliana","Ruth","Salome","Samuel","Sarah",
	"Savina","Sebastian","Seraphim","Seraphina","Serhiy","Severyn","Simon",
	"Solomon","Sophia","Stephan","Stephania","Susanna","Svitlana","Svyatoslav",
	"Svyatoslava","Syla","Sylvan","Sylvester","Tamara","Taras","Thekla",
	"Theodore","Theodora","Theodosey","Theodosia","Theodot","Teofan","Teofil",
	"Teofila","Teon","Teresa","Tetiana","Thomas","Timothy","Toma","Tryfon",
	"Tymon","Ulas","Ulianna","Una","Valentine","Valentina","Valerian","Valerie",
	"Vasylyna","Veronica","Victor","Victoria","Vincent","Violet","Vitaliy",
	"Vitalia","Vladyslav","Vladyslava","Volodymyr","Volodymyra","Volodyslav",
	"Vyacheslav","Yarodar","Yaromyr","Yaropolk","Yaroslav","Yaroslava","Yvonna",
	"Yukhym","Zachary","Zenon","Zenonia","Zinaida","Zoia",NULL};

Record GetRandom(void){
	Record random;
	strncpy(random.name,name[rand()%290],15);
	random.id = rand() % 1000;
	strncpy(random.surname,surname[rand()%55],20);
	strncpy(random.city,city[rand()%110],10);
	return random;
}

void rprint(Record record){
	printf("{ %-8d | %-15s | %-20s | %15s }\n",record.id,record.name,record.surname,record.city);
}


int main(int argc, char** argv) {
	int fd,i;
	Record r;
	struct stat buf;
	char *file="../Alina";
	
	BF_Init();
	srand(time(NULL));

	if(stat(file,&buf)){
		printf("Creating file %s\n",file);
		HP_CreateFile(file);
	}
	
	fd=HP_OpenFile(file);
	
#if 0	
	 for(i=0;i<8;++i){
		r=GetRandom();
		printf("Inserting: ");
		rprint(r);
		HP_InsertEntry(fd,r);
	}
#endif
	 
#if 0	
	r.id=222;
	char *name1="Alina";
	strcpy(r.name,name1);
	char *surname1="Gabaraev";
	strcpy(r.surname,surname1);
	char *city1="Moscow";
	strcpy(r.city,city1);
	HP_InsertEntry(fd,r);
	printf("Inserting: ");
	rprint(r);
#endif

#if 1	
	r.id=174;
	char *name1="Ria";
	strcpy(r.name,name1);
	char *surname1="Chavelia";
	strcpy(r.surname,surname1);
	char *city1="Kavala";
	strcpy(r.city,city1);
	HP_InsertEntry(fd,r);
	printf("Inserting: ");
	rprint(r);
#endif
	
#if 0	
	r.id=1;
	char *name1="Christina";
	strcpy(r.name,name1);
	char *surname1="Gabaraev";
	strcpy(r.surname,surname1);
	char *city1="Koping";
	strcpy(r.city,city1);
	HP_InsertEntry(fd,r);
	printf("Inserting: ");
	rprint(r);
#endif
	
#if 0	
	r.id=109;
	char *name1="Evangelia";
	strcpy(r.name,name1);
	char *surname1="Tsapos";
	strcpy(r.surname,surname1);
	char *city1="DC";
	strcpy(r.city,city1);
	HP_InsertEntry(fd,r);
	printf("Inserting: ");
	rprint(r);
#endif
	
#if 0	
	r.id=199;
	char *name1="Ek";
	strcpy(r.name,name1);
	char *surname1="Vr";
	strcpy(r.surname,surname1);
	char *city1="Maroussi";
	strcpy(r.city,city1);
	HP_InsertEntry(fd,r);
	printf("Inserting: ");
	rprint(r);
#endif
	
#if 0
	char *victim="Tsapos";
	HP_DeleteEntry(fd,"surname",victim);
#endif	

#if 0
	HP_GetAllEntries(fd,"id",NULL);
#endif
	
	BF_CloseFile(fd);
	return 0;
}

