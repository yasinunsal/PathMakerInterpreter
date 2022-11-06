#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <ctype.h>

#if defined (_WIN32)
#include <io.h>
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <dirent.h>
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

//Üstteki kısım current directoryi alan fonksiyonun farklı işletim sistemlerinde çalışabilmesi için var.
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning (disable : 4996)

FILE* inputFile;

bool ifBool = false, ifNotBool = false, makeBool = false, goBool = false, firstAlpha = false, ifBlockStarted = false;
int asteriskCounter = 0;
int lineCounter = 1;
bool directories(char* directoryPath) {
	asteriskCounter = 0;
	int directoryCounter = 1;
	//Directory isminin başında veya sonunda '/' karakterinin olup olmadığını kontrol ediyor.
	if (directoryPath[0] == '/' || directoryPath[(int)strlen(directoryPath) - 1] == '/') {
		printf("Directories can not start or end with '/'.(Line : %d)\n",lineCounter);
		return false;
	}
	else {
		//Bu kısım arka arkaya iki tane '/' karakterinin olup olmadığını kontrol ediyor.
		bool twoSlashAdjacent = false;
		for (int k = 0; k < (int)strlen(directoryPath); k++) {
			if (directoryPath[k] == '/') {
				if (twoSlashAdjacent) {
					printf("Directories can not have adjacent slashes. (Line : %d)\n",lineCounter);
					return false;
				}
				else {
					twoSlashAdjacent = true;
					directoryCounter++;
				}
			}
			else {
				twoSlashAdjacent = false;
			}
		}
		bool asteriskFound = false;
		bool asteriskFoundBefore = true;
		bool firstAlphaDirectories = false;
		bool spaceFound = false;
		bool emptyDirectory = true;
		//Girilen pathin syntaxa uygun olup olmadığını kontrol ediyor.
		for (int i = 0; i < (int)strlen(directoryPath); i++) {
			if (directoryPath[i] == '*') {
				//Eğer dosya adı '*' içeriyorsa.
				if (firstAlpha) {
					printf("Directory names can not contain '*' operator. (Line : %d)\n",lineCounter);
					return false;
				}
				//'*' bulunmadıysa.
				if (!asteriskFound) {
					if (asteriskFoundBefore) {
						emptyDirectory = false;
						asteriskFound = true;
						asteriskCounter++;
					}
					else {
						printf("'*' operator is misplaced. (Line : %d)\n",lineCounter);
						return false;
					}
				}
				else {
					printf("There can only be one '*' operator. (Line : %d)\n",lineCounter);//Birden fazla asterisk varsa.
					return false;
				}
			}
			else if (isalpha(directoryPath[i])) {
				if (!firstAlphaDirectories)
					emptyDirectory = false;
				firstAlphaDirectories = true;
				if (spaceFound) {
					printf("Directory names can not contain spaces. (Line : %d)\n",lineCounter); //Klasör adında boşluk varsa.
					return false;
				}
				if (asteriskFound) {
					printf("Directory names can not contain '*' operator. (Line : %d)\n",lineCounter); //Klasör adı '*' içeriyorsa.
					return false;
				}
			}
			else if (isdigit(directoryPath[i]) || directoryPath[i] == '_') {
				if (!firstAlphaDirectories) {
					printf("Directory names have to start with an alphabetic character. (Line : %d)\n",lineCounter); //Dosya adı alfabetik karakterle başlamazsa.
					return false;
				}
				else if (asteriskFound) {
					printf("Directory names can not contain '*' operator. (Line : %d)\n",lineCounter); // Klasör adı '*' içeriyorsa.
					return false;
				}
				else {
					if (spaceFound) {
						printf("Directory names can not contain spaces. (Line : %d)\n",lineCounter); //Klasör adında boşluk varsa.
						return false;
					}
				}
			}
			else if (directoryPath[i] == ' ') {
				if (!spaceFound && firstAlphaDirectories) { //Dosya adı başladıktan sonra boşluk gelirse true olur. Bu dosyanın adında boşluk olup olmadığını kontrol etmemizi sağlar.
					spaceFound = true;
				}
			}
			else if (directoryPath[i] == '/') {
				if (!firstAlphaDirectories && !asteriskFound) {
					printf("Invalid directory name. (Line : %d)\n",lineCounter); // Sadece boşluksa
					return false;
				}
				if (asteriskFound) {
					asteriskFoundBefore = true;
				}
				else {
					asteriskFoundBefore = false;
				}
				emptyDirectory = true;
				asteriskFound = false;
				spaceFound = false;
				firstAlphaDirectories = false;
			}
			else {
				printf("Directory names can not contain this character '%c'. ", directoryPath[i]); // Dosya adında geçerli olmayan bir karakter varsa.
				printf("(Line : %d)\n",lineCounter);
				return false;
			}
		}
		if (emptyDirectory) {
			printf("Invalid directory name. (Line : %d)\n",lineCounter); //Klasör adı boşsa.
			return false;
		}
		return true;
	}
}
bool directoryExists(char* path) {
#if defined(_WIN32)
	if (_access(path, 0) != 0)
		return false;
	return true;
#else
	DIR* dir = opendir(path);
	if (dir) {
		closedir(dir);
		return true;
	}
	return false;
#endif
}
bool asteriskControl(char* currentPathParameter) {//'*' operatörünü bulunduğumuz klasör içinde kullanıp kullanamayacağımızı kontrol ediyor. Eğer geri gidebileceğimiz dosya sayısından fazla '*' operatörü varsa hata bastırır.
	int directory;
#if defined(_WIN32)
	directory = 0;
#else
	directory = -1;
#endif
	for (int i = 0; currentPathParameter[i] != '\0'; ++i) {
		if (currentPathParameter[i] == '/')
			directory++;
	}
	if (asteriskCounter > directory) {
		printf("Can not use '*' operator for the folder that the program is currently in. (Line : %d)\n",lineCounter);
		return false;
	}
	return true;
}
char* asteriskOperator(char* pathParameter) {//'*' operatörünün yapacağı işlemleri gerçekleştirir ve yeni bulunduğu klasörü pointer olarak döndürür.
	int slashCounter = 0;
	int cutIndex = 0;
	for (int i = (int)strlen(pathParameter) - 1; i > -1; i--) {
		if (pathParameter[i] == '/') { // '/' sayısı '*' eşit olduğunda girilen pathin kaçıncı indeksinden sonrasına ihtiyacımız olduğunu belirler.
			slashCounter++;
			if (slashCounter == asteriskCounter) {
				cutIndex = i;
				break;
			}
		}
	}
	char* temp = (char*)calloc(cutIndex + 1, sizeof(char));
	for (int j = 0; j < cutIndex + 1; j++) {//Belirlediğimiz indekse göre de kalan kısmı bir pointera atar ve döndürür.
		if (j == cutIndex) {
			temp[j] = '\0';
		}
		else {
			temp[j] = pathParameter[j];
		}
	}
	return temp;
}
bool ifControlFunction(char* directoryPathArrayParameter, char* currentPathParameter) {
	//Eğer '*' operatörü varsa.
	if (asteriskCounter > 0) {
		char* tempPath = asteriskOperator(currentPathParameter); //Geçici bir current path oluşturup pathin asterisk operatörlü halini döndürür.
		bool asteriskError = asteriskControl(currentPathParameter);
		if(!asteriskError){
            return false;
		}
		int slashCounter = 0;
		int cutIndex = 0;
		int len = 0;
		for (int j = 0; directoryPathArrayParameter[j] != '\0'; j++) { //Girilen path adını operatöre göre düzenleyeceğimiz indeksi belirler.
			len++;
			if (directoryPathArrayParameter[j] == '/') {
				slashCounter++;
				if (slashCounter == asteriskCounter) {
					cutIndex = j;
				}
			}
		}
		char* temp;
		if (slashCounter < asteriskCounter) {//Eğer slash sayısı operatör sayısından az kalıyorsa sonuncu indeksi seçer ve boş bir pointer oluşturmamızı sağlar.
			cutIndex = len - 1;
		}
		temp = (char*)calloc(len - cutIndex, sizeof(char));
		for (int q = 0; q < len - cutIndex - 1; q++) {//Directory pathi geçici pointera atar.
			temp[q] = directoryPathArrayParameter[cutIndex + 1 + q];
		}
		int length = (int)strlen(tempPath) + (len - cutIndex) + 1;
		char* path = (char*)calloc(length, sizeof(char));
		int forCounter = (int)strlen(tempPath);

		//Geçici bir pointera current path ve directory pathin asterisk operatörlü hallerini birleştirerek atar.
		int index = 0;
		for (int q = 0; q < forCounter; q++) {//Current pathin operatörlü halini atar.
			//Boşlukları görmezden geliyor.
			if (tempPath[q] != ' ') {
				path[index] = tempPath[q];
				index++;
			}
		}
		//Directory pathin operatörlü hali boş değilse current path ile arasına '/' karakterini ekler.
		if ((int)strlen(temp) > 0) {
			path[index] = '/';
			index++;
		}
		else {//Eğer üstteki koşul sağlanmazsa uzunluğu azaltır.
			length--;
		}
		for (int z = 0; z < (int)strlen(temp); z++) {//Directory pathin operatörlü halini atar.
			//Boşlukları görmezden geliyor.
			if (temp[z] != ' ') {
				path[index] = temp[z];
				index++;
			}
		}
		//Gerekli hafıza boşaltma işlemleri yapılır.
		if (len - cutIndex != 0)
			free(temp);
		free(tempPath);
		//Pathin olup olmadığını kontrol eder.
		if (directoryExists(path)) {
			free(path);
			return true;
		}
		else {
			free(path);
			return false;
		}
	}
	else {//Eğer asterisk operatörü yoksa.
		int length;
		int forCounter;
		length = (int)strlen(currentPathParameter) + (int)strlen(directoryPathArrayParameter) + 2;
		forCounter = (int)strlen(currentPathParameter);
		char* path = (char*)calloc(length, sizeof(char));
		//Current path ile directory pathi bir pointerda birleştirir.
		int index = 0;
		for (int q = 0; q < forCounter; q++) {//Current pathi atıyor.
			//Boşlukları görmezden gelmek için.
			if (currentPathParameter[q] != ' ') {
				path[index] = currentPathParameter[q];
				index++;
			}
		}
		path[index] = '/';
		index++;
		//Directory pathi atıyor.
		for (int z = 0; z < (int)strlen(directoryPathArrayParameter); z++) {
			//Boşlukları görmezden gelmek için.
			if (directoryPathArrayParameter[z] != ' ') {
				path[index] = directoryPathArrayParameter[z];
				index++;
			}
		}
		//Path var mı diye kontrol ediyor.
		if (directoryExists(path)) {
			free(path);
			return true;
		}
		else {
			free(path);
			return false;
		}
	}
}
//If veya ifnot fonksiyonları çağırıldığında ikisi içinde geçerli olan bir fonksiyon çağırılır. Bu fonksiyonun çıktısı ifnotta tersine çevirilir.
bool ifFunction(char* directoryPathArrayParameter, char* currentPathParameter) {
	bool ifControl = ifControlFunction(directoryPathArrayParameter, currentPathParameter);
	if (ifControl) {
		return true;
	}
	else { return false; }
}
bool ifNotFunction(char* directoryPathArrayParameter, char* currentPathParameter) {
	bool ifNotControl = ifControlFunction(directoryPathArrayParameter, currentPathParameter);
	if (ifNotControl) {
		return false;
	}
	else { return true; }
}
void createDirectory(char* path)//Klasör yaratmak için kullanılıyor. Farklı işletim sistemlerinde çalışabilmesi için bu şekilde kullanılıyor.
{
#if defined(_WIN32)
	mkdir(path);
#else
	mkdir(path, 0700);
#endif
}
void makeFunction(char* directoryPathArrayParameter, char* currentPathParameter) {
	//Eğer '*' varsa.
	if (asteriskCounter > 0) {
		char* tempPath = asteriskOperator(currentPathParameter);//Geçici bir current path oluşturup pathin asterisk operatörlü halini döndürür.
		bool asteriskError = asteriskControl(currentPathParameter);
		if(!asteriskError){
            return;
		}
		int slashCounter = 0;
		int cutIndex = 0;
		int len = 0;
		for (int j = 0; directoryPathArrayParameter[j] != '\0'; j++) {//Girilen path adını operatöre göre düzenleyeceğimiz indeksi belirler.
			len++;
			if (directoryPathArrayParameter[j] == '/') {
				slashCounter++;
				if (slashCounter == asteriskCounter) {
					cutIndex = j;
				}
			}
		}
		if (slashCounter < asteriskCounter) {//Eğer slash sayısı operatör sayısından az kalıyorsa.
			printf("There is no directory to create. (Line : %d)\n",lineCounter);
		}
		else {
			char* temp = (char*)calloc(len - cutIndex, sizeof(char));
			int tempSlashCounter = 0;
			for (int q = 0; q < len - cutIndex - 1; q++) {//Directory pathi geçici pointera atar.
				temp[q] = directoryPathArrayParameter[cutIndex + 1 + q];
			}
			int length = (int)strlen(tempPath) + (len - cutIndex) + 1;
			char* path1 = (char*)calloc(length, sizeof(char));
			int forCounter = (int)strlen(tempPath);

			//Geçici bir pointera current path ve directory pathin asterisk operatörlü hallerini birleştirerek atar.
			for (int q = 0; q < forCounter; q++) {//Current pathin operatörlü halini atar.
				path1[q] = tempPath[q];
			}
			//Current path ile directory pathin arasına '/' karakterini ekler.
			path1[forCounter] = '/';
			forCounter++;
			int a = 0;
			for (int z = forCounter; z < length - 1; z++) {//Directory pathin operatörlü halini atar.
				path1[z] = temp[a];
				a++;
			}
			for (int u = 0; u < length - 1; u++) {//Programın klasörleri oluştururken döngünün kaç kere çalışacağını tutar.
				if (path1[u] == '/') {
					tempSlashCounter++;
				}
			}
			if (directoryExists(path1))
				printf("The directory that you are trying to create already exists. (Line : %d)\n",lineCounter);//Yaratılmaya çalışılan dosya zaten varsa.
			else {
				//Verilen yoldaki her bir klasörü teker teker oluşturur.
				for (int i = 0; i < tempSlashCounter + 1; i++) {
					int counter = -1;
					for (int q = 0; q < length; q++) {
						//Bir sonraki dosya adına geçilirse.
						if (path1[q] == '/') {
							counter++;
							//Geçici bir pointera '/' karakteri yakalanana kadar olan kısmı atar ve dosyayı oluşturur.
							if (counter == i) {
								int index = 0;
								char* path2 = (char*)calloc(q + 1, sizeof(char));
								for (int y = 0; y < q; y++) {
									//Boşlukları görmezden gelmek için.
									if (path1[y] != ' ') {
										path2[index] = path1[y];
										index++;
									}
								}
								createDirectory(path2);
								free(path2);
							}
						}
						//Sonuncu dosya adıysa.
						else if (q + 1 == length) {
							//Geçici bir pointera pathin tamamını atar ve dosyayı oluşturur.
							int index = 0;
							char* path2 = (char*)calloc(q + 2, sizeof(char));
							for (int y = 0; y < q + 1; y++) {
								//Boşlukları görmezden gelmek için.
								if (path1[y] != ' ') {
									path2[index] = path1[y];
									index++;
								}
							}
							createDirectory(path2);
							free(path2);
						}
					}
				}
			}
			free(path1);
			free(temp);
		}
		free(tempPath);
	}
	else {//Eğer asterisk operatörü yoksa.
		int length;
		int forCounter;
		length = (int)strlen(currentPathParameter) + (int)strlen(directoryPathArrayParameter) + 2;
		forCounter = (int)strlen(currentPathParameter);
		char* path1 = (char*)calloc(length, sizeof(char));
		//Current path ile directory pathi bir pointerda birleştirir.
		for (int q = 0; q < forCounter; q++) {//Current pathi atıyor.
			path1[q] = currentPathParameter[q];
		}
		path1[forCounter] = '/';
		forCounter++;
		int a = 0;
		for (int z = forCounter; z < length - 1; z++) {//Directory pathi atıyor.
			path1[z] = directoryPathArrayParameter[a];
			a++;
		}
		int tempSlashCounter = 0;
		for (int h = 0; h < length - 1; h++) {//Programın klasörleri oluştururken döngünün kaç kere çalışacağını tutar.
			if (path1[h] == '/')
				tempSlashCounter++;
		}
		if (directoryExists(path1))
			printf("The directory that you are trying to create already exists. (Line : %d)\n",lineCounter);//Yaratılmaya çalışılan dosya zaten varsa.
		else {
			//Verilen yoldaki her bir klasörü teker teker oluşturur.
			for (int i = 0; i < tempSlashCounter + 1; i++) {
				int counter = -1;
				for (int q = 0; q < length - 1; q++) {
					//Bir sonraki dosya adına geçilirse.
					if (path1[q] == '/') {
						counter++;
						//Geçici bir pointera '/' karakteri yakalanana kadar olan kısmı atar ve dosyayı oluşturur.
						if (counter == i) {
							int index = 0;
							char* path2 = (char*)calloc(q + 1, sizeof(char));
							for (int y = 0; y < q; y++) {
								//Boşlukları görmezden gelmek için.
								if (path1[y] != ' ') {
									path2[index] = path1[y];
									index++;
								}
							}
							createDirectory(path2);
							free(path2);
						}
					}
					//Sonuncu dosya adıysa.
					else if (path1[q + 1] == '\0') {
						//Geçici bir pointera pathin tamamını atar ve dosyayı oluşturur.
						int index = 0;
						char* path2 = (char*)calloc(q + 2, sizeof(char));
						for (int y = 0; y < q + 1; y++) {
							//Boşlukları görmezden gelmek için.
							if (path1[y] != ' ') {
								path2[index] = path1[y];
								index++;
							}
						}
						createDirectory(path2);
						free(path2);
					}
				}
			}
		}
		free(path1);
	}
}
char* goFunction(char* directoryPathArrayParameter, char* currentPathParameter) {
	int index = 0;
	if (asteriskCounter > 0) {
		char* failSafe = strdup(currentPathParameter);//Eğer hatalı bir pathe gidilmeye çalışılırsa diye şu an bulunduğumuz pathi saklar ve hata durumunda bu pathe döndürür.
		bool asteriskError = asteriskControl(currentPathParameter);
		if(!asteriskError){
            return failSafe;
		}
		char* tempPath = asteriskOperator(currentPathParameter);//Geçici bir current path oluşturup pathin asterisk operatörlü halini döndürür.
		int slashCounter = 0;
		int cutIndex = 0;
		int len = 0;
		for (int j = 0; directoryPathArrayParameter[j] != '\0'; j++) {//Girilen path adını operatöre göre düzenleyeceğimiz indeksi belirler.
			len++;
			if (directoryPathArrayParameter[j] == '/') {
				slashCounter++;
				if (slashCounter == asteriskCounter) {
					cutIndex = j;
				}
			}
		}
		char* temp;
		if (slashCounter < asteriskCounter) {//Eğer slash sayısı operatör sayısından az kalıyorsa sonuncu indeksi seçer ve boş bir pointer oluşturmamızı sağlar.
			cutIndex = len - 1;
		}
		temp = (char*)calloc(len - cutIndex, sizeof(char));
		for (int q = 0; q < len - cutIndex - 1; q++) {//Directory pathi atar geçici pointera atar.
			temp[q] = directoryPathArrayParameter[cutIndex + 1 + q];
		}
		int length = (int)strlen(tempPath) + (len - cutIndex) + 1;
		char* path = (char*)calloc(length, sizeof(char));
		int forCounter = (int)strlen(tempPath);
		//Geçici bir pointera current path ve directory pathin asterisk operatörlü hallerini birleştirerek atar.
		for (int q = 0; q < forCounter; q++) {//Current pathin operatörlü halini atar.
			//Boşlukları görmezden geliyor.
			if (tempPath[q] != ' ') {
				path[index] = tempPath[q];
				index++;
			}
		}
		//Directory pathin operatörlü hali boş değilse current path ile arasına '/' karakterini ekler.
		if ((int)strlen(temp) > 0) {
			path[index] = '/';
			index++;
		}
		else {//Eğer üstteki koşul sağlanmazsa uzunluğu azaltır.
			length--;
		}
		for (int z = 0; z < (int)strlen(temp); z++) {//Directory pathin operatörlü halini atar.
			//Boşlukları görmezden geliyor.
			if (temp[z] != ' ') {
				path[index] = temp[z];
				index++;
			}
		}
		//Gerekli hafıza boşaltma işlemleri yapılır.
		if (len - cutIndex != 0)
			free(temp);
		free(tempPath);
		//Dosyanın olup olmadığını kontrol eder.
		if (directoryExists(path)) {
			free(failSafe);
			return path;
		}
		else {
			printf("The directory that you tried to go does not exist. (Line : %d)\n",lineCounter);//Dosya yoksa bir önceki current pathi döndürür ve hata bastırır.
			free(path);
			return failSafe;
		}
	}
	else {
		index = 0;
		char* failSafe = strdup(currentPathParameter);//Eğer hatalı bir pathe gidilmeye çalışılırsa diye şu an bulunduğumuz pathi saklar ve hata durumunda bu pathe döndürür.
		int length = (int)strlen(currentPathParameter) + (int)strlen(directoryPathArrayParameter) + 2;
		char* path = (char*)calloc(length, sizeof(char));
		int forCounter = (int)strlen(currentPathParameter);
		for (int q = 0; q < forCounter; q++) {
			//Boşlukları görmezden geliyor.
			if (currentPathParameter[q] != ' ') {
				path[index] = currentPathParameter[q];
				index++;
			}
		}
		path[index] = '/';
		index++;
		for (int z = 0; z < (int)strlen(directoryPathArrayParameter); z++) {
			//Boşlukları görmezden geliyor.
			if (directoryPathArrayParameter[z] != ' ') {
				path[index] = directoryPathArrayParameter[z];
				index++;
			}
		}
		//Dosyanın olup olmadığını kontrol ediyor.
		if (directoryExists(path)) {
			free(failSafe);
			return path;
		}
		else {//Dosya yoksa bir önceki current pathi döndürür ve hata bastırır.
			printf("The directory that you tried to go does not exist. (Line : %d)\n",lineCounter);
			free(path);
			return failSafe;
		}
	}
}
//Fonksiyon isminin doğru olup olmadığını kontrol eder.
void control(char* word) {
	bool found = false;
	if (strcmp(word, "if") == 0) {
		found = true;
		ifBool = true;
	}
	else if (strcmp(word, "ifnot") == 0) {
		found = true;
		ifNotBool = true;
	}
	else if (strcmp(word, "make") == 0) {
		found = true;
		makeBool = true;
	}
	else if (strcmp(word, "go") == 0) {
		found = true;
		goBool = true;
	}
	else{
        printf("Syntax error. Invalid command. (Line: %d)\n",lineCounter);
	}
}
int main() {
	char fileName[100];
	char* directoryPathArray = NULL;
	bool directoryPathBool = false;
	int firstIndex = 0;
	char* currentPath = GetCurrentDir(NULL, 0); //Başlangıçta projenin açıldığı klasörü "current path" olarak alır.
	if (currentPath == NULL) {
		printf("Failed to get current directory. (Line : %d)\n",lineCounter);
	}
	else {//GetCurrentDir fonksiyonundaki pathteki geçişleri "\\" yerine "/" ile göstermesini sağlar.
		for (int i = 0; i < (int)strlen(currentPath); i++) {
			if (currentPath[i] == '\\') {
				currentPath[i] = '/';
			}
		}
	}
	bool ifFunctionControl = false, ifNotFunctionControl = false, calledIfFunction = false, calledIfNotFunction = false, ifFirstAlpha = false; // If fonksiyonlarının çalışması için gerekli olan booleanlar
	int leftBracketCounter = 0;
	int ifIndexesRunTime[100][3]; // Program çalışırken iflerin başlangıç ve sonları tutan dizi. Bu dizi program çalışırken sürekli güncelleniyor.
	int ifIndexesPreRunTime[100][2]; // Program çalışmadan önce iflerin başlangıç ve sonları tutan dizi. Bu dizi ise sabit kalıyor.

	//Dizinin tamamına 0 değerini atar.
	for (int h = 0; h < 100; h++) {
		for (int c = 0; c < 3; c++) {
			ifIndexesRunTime[h][c] = 0;
			if(c<2){
                ifIndexesPreRunTime[h][c] = 0;
			}
		}
	}
	int ifCounter = -1; // Program çalışırken aktif olan iflerin sayısını tutar.
	printf("Enter the file name without its extension: ");
	scanf("%s", fileName);
	strcat(fileName, ".pmk.txt");
	if ((inputFile = fopen(fileName, "rb")) == NULL) {
		printf("The file is not accessable.");
	}
	else
	{
		int charCounter = 0;
		int tempChar;
		char* fileAddress = (char*)malloc(2 * sizeof(char)); // " " olan bir string kadar alan ayırır. Sonrasında realloc ile güncellenir.
		int elementsCounter = 0;

		//Bu döngü anlık olarak dosyadaki harfleri okuyarak fileAddress isimli pointera yerleştirir. Realloc ile her seferinde gerekli olan alanı günceller.
		while ((tempChar = fgetc(inputFile)) != EOF) {
			charCounter++;
			if (charCounter == 1) {
				fileAddress[elementsCounter] = (char)tempChar;
			}
			else {
				if (fileAddress != NULL) {
					char* tempFileAddress = (char*)realloc(fileAddress, (charCounter + 1) * sizeof(char));
					if (tempFileAddress == NULL) {
						free(fileAddress);
					}
					else {
						fileAddress = tempFileAddress;
						fileAddress[elementsCounter] = (char)tempChar;
					}
				}
			}
			elementsCounter++;
		}
		fileAddress[elementsCounter] = '\0';//Terminator değeri

		// Bu kısım dosyadan alınmış tüm yazıyı alarak süslü parantezlerin sayısını tutar ve indekslerini diziye atar.
		int tempLeftBracketCounter = 0, tempRightBracketCounter = 0;
		int tempIfIndexesPreRunTime[2][100];
		for (int j = 0; j < charCounter; j++) {
			if (fileAddress[j] == '{') {
				tempIfIndexesPreRunTime[0][tempLeftBracketCounter] = j;
				tempLeftBracketCounter++;
			}
			else if (fileAddress[j] == '}') {
				tempIfIndexesPreRunTime[1][tempRightBracketCounter] = j;
				tempRightBracketCounter++;
			}
		}
		// Diziye atılmış süslü parantezlerin indekslerini birbirleriyle eşler.
		int tempIfCounter = 0;
		int tempIndex1 = 0;
		for (int q = 0; q < tempRightBracketCounter; q++) {
			for (int j = 0; j < tempLeftBracketCounter; j++) {
				//Eğer sol süslü parantezin indeksi sağ süslü parantezin indeksinden büyükse, bir önceki sol süslü parantezi şu anki sağ süslü parantez ile eşler.
				if (tempIfIndexesPreRunTime[0][j] > tempIfIndexesPreRunTime[1][q]) {
					ifIndexesPreRunTime[tempIfCounter][0] = tempIfIndexesPreRunTime[0][tempIndex1];
					ifIndexesPreRunTime[tempIfCounter][1] = tempIfIndexesPreRunTime[1][q];
					tempIfIndexesPreRunTime[0][j - 1] = 0;
					tempIfCounter++;
					break;
				}
				//Eğer üst kısımdaki kontrolde herhangi bir eşleme yapılamazsa bu kısımdaki kontroller başlar.
				else if (j + 1 == tempLeftBracketCounter) {
					// Orijinal dizideki değerleri kaybetmemek için geçici bir dizi kullanılır ve bu dizide eşlenmiş olan süslü parantezlerin indeksi 0'a değiştirilir.
					if (tempIfIndexesPreRunTime[0][j] != 0) { // Değeri 0 değilse sol süslü parantezi ana diziye atar.
						ifIndexesPreRunTime[tempIfCounter][0] = tempIfIndexesPreRunTime[0][j];
						tempIfIndexesPreRunTime[0][j] = 0;
					}
					else {
						ifIndexesPreRunTime[tempIfCounter][0] = tempIfIndexesPreRunTime[0][tempIndex1]; // Değeri 0'sa Tempindex1'deki değeri 0 olmayan sol süslü parantezi ana diziye atar.
						tempIfIndexesPreRunTime[0][tempIndex1] = 0;
					}
					// Sol süslü parantezin eşleniği olan sağ süslü parantezi diziye atar.
					ifIndexesPreRunTime[tempIfCounter][1] = tempIfIndexesPreRunTime[1][q];
					tempIfCounter++;
					break;
				}
				//Eğer ki index değeri 0 değilse tempIndex1'i günceller.
				if (tempIfIndexesPreRunTime[0][j] != 0)
					tempIndex1 = j;
			}
		}
		// Program çalışırken if bloğu kapandığında anlık olarak aktif olan if sayısını azaltır.
		int leftBracket = 0;
		int rightBracket = 0;
		for(int j = 0; j<charCounter; j++){
            if(fileAddress[j] == '{'){
                leftBracket++;
            }
            else if (fileAddress[j] == '}'){
                rightBracket++;
            }
		}
		if(rightBracket != leftBracket){
            printf("Right curly bracket count is not equal to left curly bracket count.");
            return 0;
		}
		for (int i = 0; i < charCounter; i++) {
			if (ifIndexesRunTime[ifCounter][1] == i && i != 0) {
				ifCounter--;
				continue;
			}
			//'<' karakteri bulunduktan sonra directoryPathBool aktif olur.
			else if (directoryPathBool) {
				if (fileAddress[i] == '>') {
					ifFirstAlpha = false;
					directoryPathBool = false;
					firstIndex++;
					bool noErrors = false;
					if (i == firstIndex) { //İçi boş mu diye kontrol eder.
						printf("Directory name does not contain any character. (Line : %d)\n",lineCounter);
						noErrors = false;
					}
					//Directory path olarak girilen değeri bir pointer'a atar.
					else {
						directoryPathArray = (char*)calloc(i - 1 - firstIndex + 2, sizeof(char));
						int tempIndex = 0;
						for (int j = firstIndex; j <= i - 1; j++) {
							directoryPathArray[tempIndex++] = fileAddress[j];
							if (j == i - 1)
								directoryPathArray[tempIndex] = '\0';
						}
						// Bu satırda da girilen isimde hata var mı kontrolünün yapılacağı fonksiyon çağırılır.
						noErrors = directories(directoryPathArray);
					}
					bool noCommand = true;
					//Anlık olan indeksin aktif olan if bloğunun içinde olup olmadığını kontrol eder.
					if (ifIndexesRunTime[ifCounter][0] < i && i < ifIndexesRunTime[ifCounter][1] && ifCounter != -1) {
						//Eğer ki indeks ifin içindeyse ifin koşulunun sağlanıp sağlanılmadığını kontrol eder. Eğer ki koşul sağlanılmamış ise girilen fonksiyonların çağırılmasını sağlayan boolean değerlerini false yapar.
						if (ifIndexesRunTime[ifCounter][2] == 0) {
							noCommand = false;
						}
					}
					//If fonksiyonunu çağırır.
					if (ifBool && noErrors && noCommand ) {
						ifFunctionControl = ifFunction(directoryPathArray, currentPath);
						calledIfFunction = true;
						ifBool = false;

					}
					//Ifnot fonksiyonunu çağırır.
					else if (ifNotBool && noErrors && noCommand) {
						ifNotFunctionControl = ifNotFunction(directoryPathArray, currentPath);
						calledIfNotFunction = true;
						ifNotBool = false;

					}
					//Make fonksiyonunu çağırır.
					else if (makeBool && noErrors && noCommand ) {
						//Fonksiyon çağırıldıktan sonra ';' karakterinin koyulup koyulmadığını kontrol eder. Eğer yoksa fonksiyon çağırılmaz.
						if (i + 1 == charCounter) {
							printf("Function is missing ';' (Line : %d)\n",lineCounter);
						}
						else if (fileAddress[i + 1] != ';') {
							printf("Function is missing ';' (Line : %d)\n",lineCounter);
						}
						else {
							makeFunction(directoryPathArray, currentPath);
						}
						makeBool = false; //Fonksiyon çağırıldıktan sonra değeri sıfırlar.
					}
					else if (goBool && noErrors && noCommand ) {
						//Fonksiyon çağırıldıktan sonra ';' karakterinin koyulup koyulmadığını kontrol eder. Eğer yoksa fonksiyon çağırılmaz.
						if (i + 1 == charCounter) {
							printf("Function is missing ';' (Line : %d)\n",lineCounter);
						}
						else if (fileAddress[i + 1] != ';') {
							printf("Function is missing ';' (Line : %d)\n",lineCounter);
						}
						else {
							//Current pathin tuttuğu pathi fonksiyon çağırıldıktan sonra değiştirir.
							char* tempCurrentPath = goFunction(directoryPathArray, currentPath);
							if (currentPath != NULL)
								free(currentPath);
							currentPath = strdup(tempCurrentPath);
							if (tempCurrentPath != NULL)
								free(tempCurrentPath);

						}
						goBool = false; // Fonksiyon çağırıldıktan sonra değeri sıfırlar.
					}
					else if(!ifBool && !ifNotBool && !makeBool && !goBool ) {
						printf("Syntax error. There is no command to execute. (Line : %d)\n",lineCounter); // Bu hatayı önünde herhangi bir komut olmadan bir dizin çağırıldığında bastırır.
					}
					else{
                        ifBool = false;
                        ifNotBool = false;
                        goBool = false;
                        makeBool = false;
					}
					free(directoryPathArray);
				}
			}
			else {
				//Eğer if veya ifnot fonksiyonu çağırılmışsa bu kısım çalışır.
				if (calledIfFunction || calledIfNotFunction) {
					//Eğer ki fonksiyon süslü parantezli ise bu kısım çalışır.
					if (fileAddress[i] == '{') {
						ifCounter++;
						//Program çalıştırmadan önce eşlenen parantezleri diziye atar.
						for (int g = 0; g < tempLeftBracketCounter; g++) {
							if (ifIndexesPreRunTime[g][0] == i) {
								ifIndexesRunTime[ifCounter][0] = i;
								ifIndexesRunTime[ifCounter][1] = ifIndexesPreRunTime[g][1];
								ifIndexesRunTime[ifCounter][2] = 0;
								break;
							}
						}
						//Dizideki indeksler arasındaki fonksiyonların çalışıp çalışmayacağını belirler. 0 değeri verilen if fonksiyonun içi çalışmayacaktır.
						if (calledIfFunction) {
							if (ifFunctionControl) {
								ifIndexesRunTime[ifCounter][2] = 1;
							}
						}
						else if (calledIfNotFunction) {
							if (ifNotFunctionControl) {
								ifIndexesRunTime[ifCounter][2] = 1;
							}
						}
						//Fonksiyonların booleanları sıfırlandı.
						calledIfFunction = false;
						calledIfNotFunction = false;
					}
					// Eğer fonksiyonda süslü parantez yoksa sadece tek bir satır fonksiyonda olur.
					else if (fileAddress[i] != ' ') {
						//Eğer bulunan ilk karakter boşluk değilse indeksi kaydeder ve ';' karakterini bulduğunda if veya ifnot bloğunun geçerli olduğu kısmı belirlemiş olur. Süslü parantezlerin indeksini tutuyormuş gibi indeksler tutulur.
						if (!ifFirstAlpha) {
							//İlk bulunan karakter ';' ise indeks tutmaz.
							if (fileAddress[i] == ';') {
								//Fonksiyonların booleanları sıfırlandı.
								ifFirstAlpha = true;
								calledIfFunction = false;
								calledIfNotFunction = false;
								continue;
							}
							else {
								ifCounter++;
								ifIndexesRunTime[ifCounter][0] = i - 1;
								for (int y = i + 1; y < charCounter; y++) {
									if (fileAddress[y] == ';') {
										ifIndexesRunTime[ifCounter][1] = y;
										ifIndexesRunTime[ifCounter][2] = 0;
										break;
									}
								}
								ifFirstAlpha = true; // İlk karakter bulununca bir daha bu kısım çalışmaması için true yapılır.
								if (calledIfFunction) {
									if (ifFunctionControl) {
										ifIndexesRunTime[ifCounter][2] = 1;
									}
								}
								else if (calledIfNotFunction) {
									if (ifNotFunctionControl) {
										ifIndexesRunTime[ifCounter][2] = 1;
									}
								}
								calledIfFunction = false;
								calledIfNotFunction = false;
							}
						}
					}
				}
				//Directory path girdisi başlarsa eğer directoryPathBool değerini true yapar ve girdinin başladığı indeksi tutar.
				if (fileAddress[i] == '<') {
					directoryPathBool = true;
					firstIndex = i;
				}
				//Eğer girilen karakter alfabetikse
				else if (isalpha(fileAddress[i])) {
					//Eğer ilk alfabetik karakterse indeksi tutulur ve gerekli boolean true yapılır.
					if (!firstAlpha) {
						firstIndex = i;
						firstAlpha = true;
					}
					//Eğer dosyanın son karakteri değilse
					if (i + 1 < charCounter) {
						//Bir sonraki karakter de alfabetik mi diye kontrol eder. Eğer ki alfabetikse uzunluğu tüm fonksiyonların isimlerinden uzunsa bir hata bastırır.
						if (isalpha(fileAddress[i + 1])) {
                            continue;
						}
						//Eğer alfabetik değilse
						else {
							//İlk alfabetik karakterden son alfabetik karaktere kadar olan aralığı bir pointer'a atar.
							firstAlpha = false;
							char* tempCommand = (char*)calloc(i - firstIndex + 2, sizeof(char));
							int tempIndex = 0;
							for (int j = firstIndex; j <= i; j++) {
								tempCommand[tempIndex++] = fileAddress[j];
								if (j == i)
									tempCommand[tempIndex] = '\0';
							}
							control(tempCommand);//Girilen kelimenin fonksiyon isimleriyle uyuşup uyuşmadığını kontrol eder.
							free(tempCommand);
						}
					}
				}
				else if (fileAddress[i] == '\n') {
                    lineCounter++;
                    continue;
				}
				else if(fileAddress[i] == '\t' || fileAddress[i] == '\r' || fileAddress[i] == ';' || fileAddress[i] == ' ' || fileAddress[i] == '{' || fileAddress[i] == '}' ){
                    continue;
				}
				else{
                    printf("Syntax error. Invalid character. (%c) ",fileAddress[i]);
                    printf("(Line: %d)\n",lineCounter);
				}
			}
		}
		free(fileAddress);
	}
	return 0;
}
