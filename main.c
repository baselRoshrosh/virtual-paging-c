#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
//#include <sys/random.h>

// unit8_t = 8 bits = 1 byte
uint8_t hd_mem[4194304];  //2^22
uint8_t ra_mem[65536];    //2^16
int counter = 0;  //[0-15] ich benutze das für replacementstratiegie FIFO (first in first out)

struct seitentabellen_zeile {
	uint8_t present_bit;
	uint8_t dirty_bit;
	int8_t page_frame;
}seitentabelle[1024]; // 4194304 >> 12 = 1024



uint16_t get_seiten_nr(uint32_t virt_address) {
	return virt_address >> 12;
}


//tanslate virutal address to physical address, size of 16 bit
uint16_t virt_2_ram_address(uint32_t virt_address) {
	/**
	 * Wandelt eine virtuelle Adresse in eine physikalische Adresse um.
	 * Der Rückgabewert ist die physikalische 16 Bit Adresse.
	 */

    uint16_t offset = (uint16_t)virt_address << 4;        //change virt_address to 16 bit of size (remove 4 of high order bits)
	offset = offset >> 4;                                 //re-arrange the bits into the lower order bits of the offset.
    uint16_t pa = seitentabelle[get_seiten_nr(virt_address)].page_frame;   //get the page frame number
    pa = pa << 12;                            //shift the page frame number by 12 bits to get the page frame number as the high order bits.

    return pa+offset;
}

int8_t check_present(uint32_t virt_address) {    //checks the present bit
	/**
	 * Wenn eine Seite im Arbeitsspeicher ist, gibt die Funktion "check_present" 1 zurück, sonst 0
	 */
    return seitentabelle[get_seiten_nr(virt_address)].present_bit;
}
int8_t check_dirty(uint32_t virt_address) {     //checks the dirty bit
    return seitentabelle[get_seiten_nr(virt_address)].dirty_bit;
}

int8_t is_mem_full() {
	/**
	 * Wenn der Speicher voll ist, gibt die Funktion 1 zurück;
	 */
    int counter2 = 0;
    for (int i = 0; i < 1024; ++i) {
        if(seitentabelle[i].present_bit == 1)
            counter2++;
    }
    if(counter2 == 16)  //there is 16 page in Ram , this is why we check with counter2 == 16
        return 1;
    return 0;
}

int8_t write_page_to_hd(uint32_t seitennummer, uint32_t virt_address) {
	/**
	 * Schreibt eine Seite zurück auf die HD
	 */

    //hard disc[virt address * 4kb] = ram[page number * 4kb]
    for (int i = 0; i < 4096; i++){
        hd_mem[get_seiten_nr(virt_address) * 4096 + i] = ra_mem[seitennummer * 4096 + i];
    }
    return 1;
}

uint16_t swap_page(uint32_t virt_address) {
	/**
	 * Das ist die Funktion zur Auslagerung einer Seite.
	 * Wenn das "Dirty_Bit" der Seite in der Seitentabelle gesetzt ist,
	 * muss die Seite zurück in den hd_mem geschrieben werden.
	 * Welche Rückschreibstrategie Sie implementieren möchten, ist Ihnen überlassen.
	 */
    if(check_present(virt_address) == 0 )
        return 0;
    if(check_dirty(virt_address) == 0 )
        return 0;
    uint32_t ram_page_index = virt_2_ram_address(virt_address) >> 12; //wir brauchen 20 bits um zu wissen welche Seitennummer 
    seitentabelle[get_seiten_nr(virt_address)].dirty_bit = 0;   
    return write_page_to_hd(ram_page_index, virt_address); 
}

int8_t get_page_from_hd(uint32_t virt_address) {
	/**
	 * Lädt eine Seite von der Festplatte und speichert diese Daten im ra_mem (Arbeitsspeicher).
	 * Erstellt einen Seitentabelleneintrag.
	 * Wenn der Arbeitsspeicher voll ist, muss eine Seite ausgetauscht werden.
	 */
	for (int i = 0; i < 1024; i++) {
        if (seitentabelle[i].present_bit == 1) {
            if(seitentabelle[i].page_frame == counter) {//we check with counter because we use FIFO method
                swap_page(i * 4096);   //we make sure that when we remove a page , we write the new data in hd (check dirty...)
                seitentabelle[i].present_bit = 0;
            }
        }
    }
    for (int j = 0; j < 4096; j++) {
        ra_mem[counter * 4096 + j] = hd_mem[get_seiten_nr(virt_address) * 4096 + j]; //we copy the data
    }
    seitentabelle[get_seiten_nr(virt_address)].page_frame = counter; //make the new page located the the number of counter in ram (between 0 and 15)
    seitentabelle[get_seiten_nr(virt_address)].present_bit = 1;
    seitentabelle[get_seiten_nr(virt_address)].dirty_bit = 0;
    if(counter == 15)
        counter = 0;
    else
        counter++;
    return 1;


}

uint8_t get_data(uint32_t virt_address) {
	/**
	 * Gibt ein Byte aus dem Arbeitsspeicher zurück.
	 * Wenn die Seite nicht in dem Arbeitsspeicher vorhanden ist,
	 * muss erst "get_page_from_hd(virt_address)" aufgerufen werden. Ein direkter Zugriff auf hd_mem[virt_address] ist VERBOTEN!
	 * Die definition dieser Funktion darf nicht geaendert werden. Namen, Parameter und Rückgabewert muss beibehalten werden!
	 */
	if(check_present(virt_address)) //if it's in ram then just bring it
        return ra_mem[virt_2_ram_address(virt_address)];
    else{
        get_page_from_hd(virt_address); //wenn nicht  dann benutzen wir get_page_from_hd und so ist es in ram 
        return ra_mem[virt_2_ram_address(virt_address)];
    }
}

void set_data(uint32_t virt_address, uint8_t value) {
	/**
	 * Schreibt ein Byte in den Arbeitsspeicher zurück.
	 * Wenn die Seite nicht in dem Arbeitsspeicher vorhanden ist,
	 * muss erst "get_page_from_hd(virt_address)" aufgerufen werden. Ein direkter Zugriff auf hd_mem[virt_address] ist VERBOTEN!
	 */
    if(check_present(virt_address)) { //if it's in ram , we can access and write it directly
        ra_mem[virt_2_ram_address(virt_address)] = value; 
        seitentabelle[get_seiten_nr(virt_address)].dirty_bit = 1; //change of data in ram --> dirty bit
    }
    else{
        get_page_from_hd(virt_address); //not in ram , then we write it in ram and then access it
        ra_mem[virt_2_ram_address(virt_address)] = value;
        seitentabelle[get_seiten_nr(virt_address)].dirty_bit = 1; //change of data in ram --> dirty bit
    }
}


int main(void) {
		puts("test driver_");
	static uint8_t hd_mem_expected[4194304];
	srand(1);
	fflush(stdout);
	for(int i = 0; i < 4194304; i++) {
		//printf("%d\n",i);
		uint8_t val = (uint8_t)rand();
		hd_mem[i] = val;
		hd_mem_expected[i] = val;
	}

	for (uint32_t i = 0; i < 1024;i++) {
//		printf("%d\n",i);
		seitentabelle[i].dirty_bit = 0;
		seitentabelle[i].page_frame = -1;
		seitentabelle[i].present_bit = 0;
	}


	uint32_t zufallsadresse = 4192425;
	uint8_t value = get_data(zufallsadresse);
//	printf("value: %d\n", value);

	if(hd_mem[zufallsadresse] != value) {
		printf("ERROR_ at Address %d, Value %d =! %d!\n",zufallsadresse, hd_mem[zufallsadresse], value);
	}

	value = get_data(zufallsadresse);

	if(hd_mem[zufallsadresse] != value) {
			printf("ERROR_ at Address %d, Value %d =! %d!\n",zufallsadresse, hd_mem[zufallsadresse], value);

	}

//		printf("Address %d, Value %d =! %d!\n",zufallsadresse, hd_mem[zufallsadresse], value);


	srand(3);

	for (uint32_t i = 0; i <= 1000;i++) {
		uint32_t zufallsadresse = rand() % 4194304;//i * 4095 + 1;//rand() % 4194303
		uint8_t value = get_data(zufallsadresse);
		if(hd_mem[zufallsadresse] != value) {
			printf("ERROR_ at Address %d, Value %d =! %d!\n",zufallsadresse, hd_mem[zufallsadresse], value);
			for (uint32_t i = 0; i <= 1023;i++) {
				//printf("%d,%d-",i,seitentabelle[i].present_bit);
				if(seitentabelle[i].present_bit) {
					printf("i: %d, seitentabelle[i].page_frame %d\n", i, seitentabelle[i].page_frame);
				    fflush(stdout);
				}
			}
			exit(1);
		}
//		printf("i: %d data @ %u: %d hd value: %d\n",i,zufallsadresse, value, hd_mem[zufallsadresse]);
		fflush(stdout);
	}


	srand(3);

	for (uint32_t i = 0; i <= 100;i++) {
		uint32_t zufallsadresse = rand() % 4095 *7;
		uint8_t value = (uint8_t)zufallsadresse >> 1;
        if(zufallsadresse == 5775){
        }
		set_data(zufallsadresse, value);
		hd_mem_expected[zufallsadresse] = value;

//		printf("i : %d set_data address: %d - %d value at ram: %d\n",i,zufallsadresse,(uint8_t)value, ra_mem[virt_2_ram_address(zufallsadresse)]);
	}



	srand(4);
	for (uint32_t i = 0; i <= 16;i++) {
		uint32_t zufallsadresse = rand() % 4194304;//i * 4095 + 1;//rand() % 4194303
		uint8_t value = get_data(zufallsadresse);
		if(hd_mem_expected[zufallsadresse] != get_data(zufallsadresse)) {
//			printf("ERROR_ at Address %d, Value %d =! %d!\n",zufallsadresse, hd_mem[zufallsadresse], value);
			for (uint32_t i = 0; i <= 1023;i++) {
				//printf("%d,%d-",i,seitentabelle[i].present_bit);
				if(seitentabelle[i].present_bit) {
					printf("i: %d, seitentabelle[i].page_frame %d\n", i, seitentabelle[i].page_frame);
				    fflush(stdout);
				}
			}

			exit(2);
		}
//		printf("i: %d data @ %u: %d hd value: %d\n",i,zufallsadresse, value, hd_mem[zufallsadresse]);
		fflush(stdout);
	}

	srand(3);
	for (uint32_t i = 0; i <= 2500;i++) {
		uint32_t zufallsadresse = rand() % (4095 *5);//i * 4095 + 1;//rand() % 4194303
		uint8_t value = get_data(zufallsadresse);
		if(hd_mem_expected[zufallsadresse] != get_data(zufallsadresse) ) {
			printf("ERROR_ at Address %d, Value %d =! %d!\n",zufallsadresse, hd_mem_expected[zufallsadresse], value);
			for (uint32_t i = 0; i <= 1023;i++) {
				//printf("%d,%d-",i,seitentabelle[i].present_bit);
				if(seitentabelle[i].present_bit) {
					printf("i: %d, seitentabelle[i].page_frame %d\n", i, seitentabelle[i].page_frame);
				    fflush(stdout);
				}
			}
			exit(3);
		}
//		printf("i: %d data @ %u: %d hd value: %d\n",i,zufallsadresse, value, hd_mem_expected[zufallsadresse]);
		fflush(stdout);
	}
	puts("test end");
	fflush(stdout);
	return EXIT_SUCCESS;
}
