
#include <stdio.h>										//____ZPP2 - Projekt2 - Akad. rok 2020/2021____
#include <stdlib.h>												//Jan Agh, FIIT STUBA
#include <string.h>									//Copyright(c) 2021 Jan Agh, all rights reserved

#define HOTEL "hotel.txt"					//!!!!!!!DOLEZITE!!!!!!! <<<<< SEM ZADAT MENO SUBORU

typedef struct Guest {
	char name[50];
	char addr[100];
	int begin;
	int end;
	struct Guest* next_g;
} guest;

typedef struct Room {
	int room_id;
	int room_size;
	float price;
	guest* people;
	struct Room* next_r;
} room;

void n(FILE**, room**, int*);				//zoznam hlavnych funkcii
void v(room*);
void r(FILE**, room**);
void z(FILE**, room**);
void h(room*);
void a(FILE**, room**);

void expandRoomsList(room**, room**, int, int, float);			//zoznam pomocnych funkcii
void expandGuestsList(room**, char*, char*, int, int);
void insertStart(room**, room**);
void insertMiddle(room**, room**);
void deallocList(room**);
void deallocPeople(guest**);
void deallocOneRoom(room**);
void updateFile(FILE**, room**);
int openFile(FILE**, char*);

int main() {

	room* base = NULL;
	FILE* input = NULL;

	char command;
	int control = 1;

	do {
		printf("|||||===[ ZADAJTE PRIKAZ ]===> ");
		scanf(" %c", &command);

		switch (command) {
			case 'n': n(&input, &base, &control); break;
			case 'v': v(base); break;
			case 'r': r(&input, &base); break;
			case 'z': z(&input, &base); break;
			case 'h': h(base); break;
			case 'a': a(&input, &base); break;
		}

	} while (command != 'k' && control);

	deallocList(&base);					//implementacia funkcie 'k' na dealokaciu zoznamov

	return 0;
}

void n(FILE** input, room** base, int* control) {

	char buffer[50] = "\0", name[50] = "\0", addr[100] = "\0";
	int room_id, room_size, begin, end, numOfEntries = 0;
	float price;
	room* current = NULL;

	if (openFile(input, "r+")) {
		deallocList(base);								//ak zoznam existuje, dealokujeme ho
		while (fscanf(*input, "%[^\n]\n", buffer) > 0) {
			if (!strcmp(buffer, "---")) {		//ak nacitame "---", znamena to, ze nasleduju udaje o izbe
				fscanf(*input, "%d\n%d\n%f\n", &room_id, &room_size, &price);
				expandRoomsList(base, &current, room_id, room_size, price);			//vlozenie izby do zoznamu
				numOfEntries++;
			}
			else if (!strcmp(buffer, "#")) {	//ak nacitame "#", znamena to, ze nasleduju udaje o hostovi
				fscanf(*input, "%50[^\n]\n%100[^\n]\n%d\n%d\n", name, addr, &begin, &end);
				expandGuestsList(&current, name, addr, begin, end);					//vlozenie hosta do zoznamu
			}
		}
		printf("Nacitalo sa %d zaznamov.\n", numOfEntries);
		fclose(*input);
	}
	else {
		printf("Zaznamy neboli nacitane.\n");
		(*control)--;
	}
}

void v(room* base) {
	
	room* helper = base;
	guest* helper2 = (helper == NULL ? NULL : helper->people);

	while (helper != NULL) {
		printf("Izba cislo: %d\nPocet lozok: %d\nCena: %.2f\nZoznam hosti:\n", helper->room_id, helper->room_size, helper->price);
		while (helper2 != NULL) {
			printf("Meno: %s\nAdresa: %s\nZaciatok rezervacie: %d\nKoniec rezervacie: %d\n", helper2->name, helper2->addr,
				helper2->begin, helper2->end);
			if (helper2->next_g != NULL) {
				printf("############################\n");
			}
			helper2 = helper2->next_g;
		}
		if (helper->next_r != NULL) {
			printf("--------------------------\n--------------------------\n");
			helper2 = helper->next_r->people;
		}
		helper = helper->next_r;
	}
}

void r(FILE** input, room** base) {

	int id, size, numOfGuests, begin, end;
	float price;
	char name[50] = "\0", addr[100] = "\0";
	room* current = NULL;

	if (openFile(input, "a") && (*base) != NULL) {
		scanf("%d", &id);					//nacitanie udajov o izbe a ich ulozenie do zoznamu aj suboru
		scanf("%d", &size);
		scanf("%f", &price);
		expandRoomsList(base, &current, id, size, price);
		fprintf(*input, "\n---\n%d\n%d\n%f", id, size, price);
		scanf("%d", &numOfGuests);
											//ak je pocet hosti vacsi ako zadany pocet lozok, nacitaj iba tolko ludi,
		for (int x = 0; x < (numOfGuests <= size ? numOfGuests : size); x++) {			//kolko lozok je v izbe
			scanf(" %50[^\n]\n", name);
			scanf("%100[^\n]\n", addr);		//nacitanie udajov o hostovi a ich ulozenie do zoznamu aj suboru
			scanf("%d", &begin);
			scanf("%d", &end);
			expandGuestsList(&current, name, addr, begin, end);
			fprintf(*input, "\n#\n%s\n%s\n%d\n%d", name, addr, begin, end);
		}
		fclose(*input);
	}
}

void z(FILE** input, room** base) {

	int id;
	room* helper = (*base), *helper2 = (*base);

	scanf("%d", &id);

	if (helper != NULL) {
		while (helper != NULL) {
			if (helper->room_id == id) {
				if (helper == (*base)) {		//toto sa vykona, ak chceme zrusit rezervaciu prvej izby
					(*base) = helper->next_r;
				}
				else if (helper->next_r == NULL) {		//toto sa vykona, ak chceme zrusit rezervaciu poslednej izby
					helper2->next_r = NULL;
				}
				else {		//toto sa vykona, ak chceme zrusit rezervaciu niektorej zo strednych izieb
					helper2->next_r = helper->next_r;
				}
				deallocOneRoom(&helper);			//odstranenie izby aj s hostami zo zaznamu (plus dealokacia)
				updateFile(input, base);			//aktualizacia suboru
				printf("Rezervacia izby cislo %d bola zrusena.\n", id); break;
			}
			helper2 = helper, helper = helper->next_r;
		}
	}
	else {
		printf("Situacia bez presneho zadania: Error 404, zoznam sa nenasiel.\n");
	}
}

void h(room* base) {

	int date, counter, ct = 0;
	room* helper = base;
	guest* helper2 = (helper == NULL ? NULL : helper->people);
		
	scanf("%d", &date);					//DOLEZITE: zoznam bol vzostupne usporiadany uz pri nacitani

	if (helper != NULL) {
		while (helper != NULL) {
			counter = 0;
			while (helper2 != NULL) {		//zistenie, ci su nejaki ludia ubytovani v izbe v zadanom case
				ct += counter += (date >= helper2->begin && date <= helper2->end ? 1 : 0);
				helper2 = helper2->next_g;
			}
			if (counter) {			//ak su ubytovani, vypis id izby
				printf("%d\n", helper->room_id);
			}
			helper2 = (helper->next_r == NULL ? NULL : helper->next_r->people), helper = helper->next_r;
		}
		if (!ct) {
			printf("K datumu %d neevidujeme rezervaciu.\n", date);
		}
	}
	else {
		printf("Situacia bez presneho zadania: Error 404, zoznam sa nenasiel.\n");
	}
}

void a(FILE** input, room** base) {

	int id, size;
	room* helper = (*base);

	scanf("%d", &id);
	scanf("%d", &size);

	if (helper != NULL) {
		while (helper != NULL) {
			if (helper->room_id == id) {			//najdenie izby podla zadaneho id
				helper->room_size = size;
				printf("Izba cislo %d ma lozkovu kapacitu %d.\n", id, size);
				updateFile(input, base); break;		//aktualizacia suboru
			}
			helper = helper->next_r;
		}
	}
	else {
		printf("Situacia bez presneho zadania: Error 404, zoznam sa nenasiel.\n");
	}
}

//**********************************************POMOCNE FUNKCIE******************************************************

void expandRoomsList(room** base, room** current, int room_id, int room_size, float price) {

	room* newRoom = (room*)malloc(sizeof(room)), *helper;		//funkcia na pridanie dalsej izby do zoznamu
	newRoom->room_id = room_id;
	newRoom->room_size = room_size;
	newRoom->price = price;
	newRoom->people = NULL;
	newRoom->next_r = NULL;
	helper = (*base);

	if ((*base) == NULL) {			//ak bol zoznam doteraz prazdny, pridaj izbu na zaciatok
		(*base) = newRoom;
	}
	else {				//!!!DOLEZITE: vzostupne usporiadanie sa uskutocnuje uz tu (pocas nacitania), nie vo funkcii 'h'!!!
		if (helper->room_id > newRoom->room_id) {	//ak je ID novej izby mensie, ako ID zaciatocnej izby, pridaj
			insertStart(base, &newRoom);											//novu izbu na zaciatok
		}
		else {
			while (helper->next_r != NULL) {
				if (helper->next_r->room_id > newRoom->room_id) {		//pridanie novej izby niekde do stredu zoznamu
					insertMiddle(&helper, &newRoom); break;
				}
				helper = helper->next_r;
			}
			if (helper->next_r == NULL) {		//pridanie novej izby na koniec zoznamu
				helper->next_r = newRoom;
			}
		}
	}
	(*current) = newRoom;
}

void expandGuestsList(room** current, char* name, char* addr, int begin, int end) {

	guest* person = (guest*)malloc(sizeof(guest)), *helper = (*current)->people;
	strcpy(person->name, name);
	strcpy(person->addr, addr);				//funkcia na pridanie dalsieho hosta do zoznamu
	person->begin = begin;
	person->end = end;
	person->next_g = NULL;

	if ((*current)->people == NULL) {		//ak doteraz ziadny host v izbe nebol, pridaj hosta na zaciatok
		(*current)->people = person;
	}
	else {
		while (helper->next_g != NULL) {		//pridaj hosta na koniec zoznamu hosti
			helper = helper->next_g;
		}
		helper->next_g = person;
	}
}

void insertStart(room** base, room** newRoom) {		//funkcia na pridanie novej izby na zaciatok zoznamu izieb
	(*newRoom)->next_r = (*base);
	(*base) = (*newRoom);
}

void insertMiddle(room** helper, room** newRoom) {	//funkcia na pridanie novej izby niekde do stredu zoznamu izieb
	(*newRoom)->next_r = (*helper)->next_r;
	(*helper)->next_r = (*newRoom);
}

void deallocList(room** base) {				//funkcia na dealokovanie celeho zoznamu (izby + hostia v nich)

	room* helper = (*base), *helper2 = NULL;

	while (helper != NULL) {
		helper2 = helper->next_r;
		deallocOneRoom(&helper);
		helper = helper2;
	}
	(*base) = NULL;
}

void deallocPeople(guest** person) {			//funkcia na dealokaciu hosti v jednej izbe

	guest* helper = (*person), *helper2 = NULL;

	while (helper != NULL) {
		helper2 = helper->next_g;
		free(helper);
		helper = helper2;
	}
}

void deallocOneRoom(room** room) {			//funkcia na dealokaciu jednej izby spolu s hostami v nich
	if ((*room)->people != NULL) {
		deallocPeople(&((*room)->people));
	}
	free(*room);
}

void updateFile(FILE** input, room** base) {			//funkcia na aktualizaciu udajov v subore

	room* helper = (*base);
	guest* helper2 = (*base)->people;

	if (openFile(input, "w")) {
		while (helper != NULL) {
			fprintf(*input, "---\n%d\n%d\n%f\n", helper->room_id, helper->room_size, helper->price);
			while (helper2 != NULL) {
				fprintf(*input, "#\n%s\n%s\n%d\n%d\n", helper2->name, helper2->addr, helper2->begin, helper2->end);
				helper2 = helper2->next_g;
			}
			helper = helper->next_r, helper2 = (helper != NULL ? helper->people : NULL);
		}
		fclose(*input);
	}
}

int openFile(FILE** input, char* mode) {
	return (((*input) = fopen(HOTEL, mode)) == NULL ? 0 : 1);
}