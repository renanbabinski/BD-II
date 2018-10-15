#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct Header {
  int memFree;
  int next;
  int qtdItems;
  int toast;
} header;

typedef struct Item {
  int offset;
  int totalLen;
  int writed;
} item;

typedef struct Attribute {
  char name[15];
  int size;
  char type;
} attribute;

void getTableName(char *sql, char *name);

void buildHeader(char *sql, char *tableName, int qtdPages);

void getAllAtributes(char *sql, char *attributes);

void leArquivo(char *tableName);

void getOp(char *sql, char *operation){
  char sqlCopy[1000], *token;
  char key[2] = " ";

  memset(sqlCopy, '\0', sizeof(sqlCopy)); //preenche sqlCopy com \0

  strcpy(sqlCopy, sql);    //copia dados de sql para sqlCopy

  token = strtok(sqlCopy, key); //strtok define a leitura da string até o delimitador key que é o espaço em branco

  strcpy(operation, token); //copia a primeira operação SQL para a variavel operation
}

void createPage(char *tableName, int numPage){
  header head;
  head.memFree = 8180;
  head.next = 8191;
  head.qtdItems = 0;

  struct stat stateDir = {0};

  char pageName[600];

  //printf("tableName: %s\n\n", tableName);
  //printf("NumPage: %d\n\n", numPage);

  snprintf(pageName, sizeof(pageName), "%s/page%d.dat", tableName, numPage);

  FILE *page = fopen(pageName, "wb");
  fwrite(&head.memFree, sizeof(int), 1, page);  // free
  fwrite(&head.next, sizeof(int), 1, page);     // onde inserir o proximo elemento
  fwrite(&head.qtdItems, sizeof(int), 1, page);// n elementos
  fclose(page);
  if(page == NULL){
    printf("Failed to create page\n");
    return;
  }
}

/*
Cria diretório como nome da tabela se o mesmo não existir
Abre um novo arquivo binário de leitura e escrita onde serão gravadas as informações da tabela
como variaveis e seus respectivos dados
*/
void createTable(char *sql){
  header head;    //cria uma variavel estruturada head
  head.memFree = 8180;  //quantidade de memória livre
  head.next = 8191;     //posição da próxima pagina
  head.qtdItems = 0;    //itens na página

  char special = '0';

  struct stat stateDir = {0}; 

  char tableName[500];  //nome da tabela
  char pageName[600];   //nome pagina
  memset(tableName, '\0', sizeof(tableName)); //seta todos os campos da string 'tableName' para \0
  getTableName(sql, tableName);  //seta tableName com seu respectivo nome

  if(stat(tableName, &stateDir) == -1){ //verifica se a tabela ja existe, se não existir prossiga
    if(mkdir(tableName, 0777) == 0)   // cria um diretório com o nome da tabela e retorna 0 se foi criado com sucesso
      printf("Created table %s\n", tableName);
      snprintf(pageName, sizeof(pageName), "%s/header.dat", tableName);  //verifica se a string resultante não ultrapassa o tamanho da string "pageName"
      FILE *headerPage = fopen(pageName, "wb"); //cria um arquivo binário com o nome da tabela
      fclose(headerPage);  //fecha arquivo
      snprintf(pageName, sizeof(pageName), "%s/page1.dat", tableName);  //verifica se a string resultante não ultrapassa o tamanho da string "pageName"
      FILE *page = fopen(pageName, "wb"); //cria um arquivo binário com o nome da tabela
      fwrite(&head.memFree, sizeof(int), 1, page);  // free
      fwrite(&head.next, sizeof(int), 1, page);     // onde inserir o proximo elemento
      fwrite(&head.qtdItems, sizeof(int), 1, page);// n elementos

      fseek(page, 8191, SEEK_SET); //seta o ponteiro seek para a posição 8191 a partir do inicio da pagina
      fwrite(&special, 1, 1, page); //escreve o caractere especial no arquivo
      fclose(page); //fecha arquivo
      if(page == NULL){     //se page não existir retorna erro
        printf("Failed to create page\n");
        return;
      }
      buildHeader(sql, tableName, 1); //
  }else{ // se a tabela existir, retorna
    printf("Table %s already created\n", tableName);
  }
}

/*
A função abaixo define os nomes e tamanhos das variaveis definidas na criação da tabela em SQL
Essas informações são gravadas na pagina header(cabeça)
*/

void buildHeader(char *sql, char *tableName, int qtdPages){
  char fieldName[15], fieldType;
  int fieldSize, i = 0;

  char sqlCopy[1000], attribute[50], pageName[600];;
  char *token, *tokenAttribute;

  memset(fieldName, '\0', sizeof(fieldName)); //seta fieldName com 0\

  strcpy(sqlCopy, sql); //copia dados de sql para sqlCopy

  snprintf(pageName, sizeof(pageName), "%s/header.dat", tableName); //verifica se a string resultante não ultrapassa o tamanho da string "pageName"
  FILE *headerPage = fopen(pageName, "rb+"); //abre headerPage para leitura/escrita

  fwrite(&i, sizeof(int), 1, headerPage); //escreve i na pagina

  token = strtok(sqlCopy, "(");
  token = strtok(NULL, "()[], "); //quebra a string sqlCopy em substrings para operações abaixo
  

  while(token != NULL){
    if(strcmp(token, "char") == 0){  //define nome e tamanho para variaveis "char"
      fieldType = 'C';
      token = strtok(NULL, "()[], ");
      fieldSize = atoi(token);
      token = strtok(NULL, "()[], ");
      strcpy(fieldName, token);
      fwrite(fieldName, 15, 1, headerPage);
      fwrite(&fieldSize, sizeof(int), 1, headerPage);
      fwrite(&fieldType, 1, 1, headerPage);
      i++;
    } else if(strcmp(token, "int") == 0){  //define nome e tamanho para variaveis "int"
      fieldType = 'I';
      token = strtok(NULL, "()[], ");
      fieldSize = sizeof(int);
      strcpy(fieldName, token);
      fwrite(fieldName, 15, 1, headerPage);
      fwrite(&fieldSize, sizeof(int), 1, headerPage);
      fwrite(&fieldType, 1, 1, headerPage);
      i++;
    } else if(strcmp(token, "varchar") == 0){  //define nome e tamanho para variaveis "varchar"
      fieldType = 'V';
      token = strtok(NULL, "()[], ");
      printf("\n1-%s\n2-%s",token,sqlCopy);
      getchar();
      fieldSize = atoi(token);
      printf("%d",fieldSize);
      token = strtok(NULL, "()[], ");
      strcpy(fieldName, token);
      fwrite(fieldName, 15, 1, headerPage);
      fwrite(&fieldSize, sizeof(int), 1, headerPage);
      fwrite(&fieldType, 1, 1, headerPage);
      i++;
    }
    token = strtok(NULL, "()[], ");
  }

  fwrite(&qtdPages, sizeof(int), 1, headerPage);  //grava endereço de qtdPages no arquivo
  fseek(headerPage, 0, SEEK_SET);  //seta ponteriro da headerPage para o inicio

  fwrite(&i, sizeof(int), 1, headerPage);  //grava i no arquivo

  fclose(headerPage); //fecha a pagina

  leArquivo(tableName);  //mostra informações da página
}

void insertInto(char *sql, int numPage){
  char sqlCopy[1000], *token, tableName[500], pageName[600], attrSql[1000], attrSqlCopy[1000];
  char endVarchar = '$', special = ' ', endChar = '\0';
  int insertSize = 0, qtdFields, nextItem, intVar, nextPage, qtdEndChar = 0;
  int i = 0, countVarchar = 0;

  attribute attributes[64];

  header head;

  memset(sqlCopy, '\0', sizeof(sqlCopy));

  strcpy(sqlCopy, sql);

  token = strtok(sqlCopy, " () ,");

  while(token != NULL){
    // Second parameter table name
    if(i == 2)
      strcpy(tableName, token);
    token = strtok(NULL, " () ,");
    i++;
  }

  snprintf(pageName, sizeof(pageName), "%s/header.dat", tableName);
  FILE *headerPage = fopen(pageName, "rb+");

  fread(&qtdFields, sizeof(int), 1, headerPage);

  for(int i = 0; i < qtdFields; i++){
    fread(attributes[i].name, 15, 1, headerPage);
    fread(&attributes[i].size, sizeof(int), 1, headerPage);
    fread(&attributes[i].type, 1, 1, headerPage);
  }

  fread(&nextPage, sizeof(int), 1, headerPage);

  fclose(headerPage);

  strcpy(sqlCopy, sql);
  token = strtok(sqlCopy, "()");
  token = strtok(NULL, "()");

  memset(attrSql, '\0', sizeof(attrSql));
  memset(attrSqlCopy, '\0', sizeof(attrSqlCopy));

  strcpy(attrSql, token);

  strcpy(attrSqlCopy, attrSql);

  token = strtok(attrSqlCopy, ",");

  for(int i = 0; i < qtdFields; i++){
    if(attributes[i].type == 'C'){
      insertSize += attributes[i].size;
    } else if(attributes[i].type == 'I'){
      insertSize += attributes[i].size;
    } else if(attributes[i].type == 'V'){
      insertSize += strlen(token) + 1; // + 1 for special char of varchar '$'
      //countVarchar
    }
    token = strtok(NULL, ",");
  }

  strcpy(attrSqlCopy, attrSql);

  snprintf(pageName, sizeof(pageName), "%s/page%d.dat", tableName, numPage);
  FILE * page = fopen(pageName, "rb+");

  fread(&head.memFree, sizeof(int), 1, page);
  fread(&head.next, sizeof(int), 1, page);
  fread(&head.qtdItems, sizeof(int), 1, page);

  if(head.memFree > insertSize){
    token = strtok(attrSqlCopy, ",");
    item newItem;
    newItem.offset = head.next - insertSize;
    newItem.totalLen = insertSize;
    newItem.writed = 1;

    nextItem = 12 + 12 * head.qtdItems;

    printf("Inicio da gravacao: %d\n", newItem.offset);

    fseek(page, nextItem, SEEK_SET);

    fwrite(&newItem.offset, sizeof(int), 1, page);
    fwrite(&newItem.totalLen, sizeof(int), 1, page);
    fwrite(&newItem.writed, sizeof(int), 1, page);

    fseek(page, newItem.offset, SEEK_SET);

    printf("OFFSET NEW ITEM: %d\n", newItem.offset);

    for(int i = 0; i < qtdFields; i++){
      if(attributes[i].type == 'C'){
        fwrite(token, strlen(token), 1, page);
        qtdEndChar = attributes[i].size - strlen(token);
        fwrite(&endChar, 1, qtdEndChar, page);
      } else if(attributes[i].type == 'I'){
        intVar = atoi(token);
        fwrite(&intVar, attributes[i].size, 1, page);
      } else if(attributes[i].type == 'V'){
        fwrite(token, strlen(token), 1, page);
        fwrite(&endVarchar, 1, 1, page);
      }
      token = strtok(NULL, ",");
    }

    head.memFree = head.memFree - insertSize;
    head.next = newItem.offset;
    head.qtdItems += 1;

    fseek(page, 0, SEEK_SET);

    fwrite(&head.memFree, sizeof(int), 1, page);
    fwrite(&head.next, sizeof(int), 1, page);
    fwrite(&head.qtdItems, sizeof(int), 1, page);

    fclose(page);

    printf("New item inserted\n");
  }else {
    fseek(page, 8191, SEEK_SET);
    fread(&special, 1, 1, page);
    printf("Special: %c\n", special);
    if(special == '0'){
      printf("Entrou special\n");
      nextPage++;
      printf("Nome Pagina: %s\n", tableName);
      printf("Numero pagina: %d\n", nextPage);
      printf("SQL - %s\n", sql);
      createPage(tableName, nextPage);
      special = '1';
      fseek(page, 8191, SEEK_SET);
      fwrite(&special, 1, 1, page);
      fclose(page);
      insertInto(sql, nextPage);
    }else{
      fclose(page);
      insertInto(sql, nextPage);
    }
  }
}

/*
A função abaixo separa a string inicial "sql" em substrings (tokens) para que seja 
possivel definir o nome da tabela
*/

void getTableName(char *sql, char *name){
  char sqlCopy[1000];
  char *token;
  char keyParen[2] = "(";
  char keySpace[2] = " ";

  memset(sqlCopy, '\0', sizeof(sqlCopy));  //seta campos da string sqlCopy para \0

  strcpy(sqlCopy, sql); //copia dados de sql para sqlCopy

  token = strtok(sqlCopy, keyParen); //le sqlCopy até o primeiro parenteses e joga os dados para token

  strcpy(sqlCopy, token); //copia o valor do token para sqlCopy

  token = strtok(sqlCopy, keySpace); //le sqlCopy até o primeiro espaço e joga dados para token

  while(token != NULL){     // nome da tabela é copiado para name
    strcpy(name, token);
    token = strtok(NULL, keySpace);
  }
}

/*
A função abaixo retorna informações como espaço livre, posição da proxima pagina e quantos itens 
existem na pagina após a inserção de uma nova tabela
*/

void leArquivo(char *tableName){
  char pageName[600];
  char pageCoisa[600];

  snprintf(pageCoisa, sizeof(pageCoisa), "%s/page1.dat", tableName); //verifica se a string resultante não ultrapassa o tamanho da string "pageCoisa"

  FILE *page = fopen(pageCoisa, "rb+"); //abre arquivo binário page para leitura/escrita

  header head;

  fread(&head.memFree, sizeof(int), 1, page); //faz a leitura de memória livre	
  fread(&head.next, sizeof(int), 1, page);  // faz a leitura do endereço da proxima pagina
  fread(&head.qtdItems, sizeof(int), 1, page); //faz a leitura de quantos itens tem na pagina
  printf("MemFree - %d; Next - %d - QtdItems - %d\n", head.memFree, head.next, head.qtdItems);

  fclose(page); //fecha arquivo
}

void selectFrom(char *sql, int numPage){
  char tableName[50], sqlCopy[1000], pageName[600], *token, charInFile, special = ' ';
  int i = 0, qtdFields = 0, moveItem = 0, moveTupla = 0, intInFile, stopChar;

  item readItem;
  attribute attributes[64];

  memset(sqlCopy, '\0', sizeof(sqlCopy));

  strcpy(sqlCopy, sql);

  token = strtok(sqlCopy, " ");

  while(token != NULL){
    // Second parameter table name
    if(i == 3){
      token = strtok(token, "\n");
      strcpy(tableName, token);
    } else
      token = strtok(NULL, " ");
    i++;
  }

  //printf("\nTable selected - %s\n", tableName);

  snprintf(pageName, sizeof(pageName), "%s/header.dat", tableName);

  FILE *headerPage = fopen(pageName, "rb+");
  if(!headerPage){
    printf("Can not read %s/header.dat\n", tableName);
  }

  fread(&qtdFields, sizeof(int), 1, headerPage);

  //printf("Quantidade de campos: %d\n", qtdFields);

  for(int i = 0; i < qtdFields; i++){
    fread(attributes[i].name, 15, 1, headerPage);
    printf("%s\t", attributes[i].name);
    fread(&attributes[i].size, sizeof(int), 1, headerPage);
    fread(&attributes[i].type, 1, 1, headerPage);
  }
  printf("\n");

  fclose(headerPage);

  snprintf(pageName, sizeof(pageName), "%s/page%d.dat", tableName, numPage);
  FILE *page = fopen(pageName, "rb+");

  header head;

  item items;
  fread(&head.memFree, sizeof(int), 1, page);
  fread(&head.next, sizeof(int), 1, page);
  fread(&head.qtdItems, sizeof(int), 1, page);

  //printf("memFree: %d - next: %d - qtdItems: %d\n", head.memFree, head.next, head.qtdItems);

  for(int i = 0; i < head.qtdItems; i++){
    fseek(page, 0, SEEK_SET);
    moveItem = sizeof(item) * (i + 1);
    fseek(page, moveItem, SEEK_SET);

    fread(&readItem.offset, sizeof(int), 1, page);
    fread(&readItem.totalLen, sizeof(int), 1, page);
    fread(&readItem.writed, sizeof(int), 1, page);

    //printf("offset: %d - totalLen: %d - writed: %d\n", readItem.offset, readItem.totalLen, readItem.writed);

    if(readItem.writed == 0)
      continue;

    fseek(page, readItem.offset, SEEK_SET);

    for(int j = 0; j < qtdFields; j++){
      if(attributes[j].type == 'C'){
        charInFile = ' ';
        stopChar = 0;
        for(int k = 0; k < attributes[j].size; k++){
          fread(&charInFile, 1, 1, page);
          if(charInFile == '\0')
            stopChar = 1;
          if(!stopChar)
            printf("%c", charInFile);
        }
        printf("\t");
      } else if(attributes[j].type == 'I'){
        fread(&intInFile, sizeof(int), 1, page);
        printf("%d\t", intInFile);
      } else if(attributes[j].type == 'V'){
        charInFile = ' ';
        do{
            fread(&charInFile, 1, 1, page);
            if(charInFile != '$')
              printf("%c", charInFile);
        }while(charInFile != '$');
        printf("\t");
      }
    }
    printf("\n");
  }
  fseek(page, 8191, SEEK_SET);
  fread(&special, 1, 1, page);
  if(special == '1'){
    numPage++;
    selectFrom(sql, numPage);
  }else
    fclose(page);
}
void deleteFrom(char *sql, int numPage){
  char tableName[50], sqlCopy[1000], pageName[600], *token, charInFile, attributeName[20], intInput[20], special = ' ';
  int i = 0, k = 0, qtdFields = 0, moveItem = 0, moveTupla = 0, intInFile, stopChar, delete = 0, inputDelete, count = 0;

  item readItem;  //cria varivel estruturada "readItem"
  attribute attributes[64]; //cria vetor estruturado "attributes"

  memset(sqlCopy, '\0', sizeof(sqlCopy)); //preenche sqlCopy com \0

  strcpy(sqlCopy, sql);  //copia sql para sqlCopy

  token = strtok(sqlCopy, " "); //quebra a string em substrings delimitadas por espaço

  //abaixo o loop le o atributo e de qual tabela será deletado
  while(token != NULL){
    // Second parameter table name delete from table where name = anderson
    if(i == 2){
      strcpy(tableName, token);
    } else if(i == 5){
      strcpy(attributeName, token);
    } else if(i == 8){
      break;
    } else
      token = strtok(NULL, " ");
    i++;
  }
  token = strtok(token, "\n");

  //printf("\nTable selected - %s\n", tableName);
  //printf("\nAttribute selected - %s\n", attributeName);
  //printf("\nAttribute value - %s\n", token);


  snprintf(pageName, sizeof(pageName), "%s/header.dat", tableName); //verifica se a string resultante não ultrapassa o tamanho da string "pageName"

  FILE *headerPage = fopen(pageName, "rb+");  //abre o arquivo binario para leitura/escrita
  if(!headerPage){  //se o arquivo headerPage não existir retorna erro
    printf("Can not read %s/header.dat\n", tableName);
  }

  fread(&qtdFields, sizeof(int), 1, headerPage);  //le quantidade de campos

  for(int i = 0; i < qtdFields; i++){  //le os dados do atributo para a estrutura attributes
    fread(attributes[i].name, 15, 1, headerPage);
    fread(&attributes[i].size, sizeof(int), 1, headerPage);
    fread(&attributes[i].type, 1, 1, headerPage);
  }
  printf("\n");

  fclose(headerPage); //fecha arquivo

  snprintf(pageName, sizeof(pageName), "%s/page%d.dat", tableName, numPage); //verifica se a string resultante não ultrapassa o tamanho da string "pageCoisa"
  FILE *page = fopen(pageName, "rb+"); //abre arquivo binario para leitura e gravação

  header head; //variavel estruturada head

  item items;  //variavel estruturada items
  //abaixo copia as informações da pagina pra head
  fread(&head.memFree, sizeof(int), 1, page);
  fread(&head.next, sizeof(int), 1, page);
  fread(&head.qtdItems, sizeof(int), 1, page);

  for(int i = 0; i < head.qtdItems; i++){
    fseek(page, 0, SEEK_SET); //ponteiro no inicio do arquivo
    moveItem = sizeof(item) * (i + 1);
    fseek(page, moveItem, SEEK_SET); 

    fread(&readItem.offset, sizeof(int), 1, page);
    fread(&readItem.totalLen, sizeof(int), 1, page);
    fread(&readItem.writed, sizeof(int), 1, page);

    if(readItem.writed == 0)
      continue;

    fseek(page, readItem.offset, SEEK_SET);

    for(int j = 0; j < qtdFields; j++){
      delete = 1;
      if(attributes[j].type == 'C') {
        charInFile = ' ';
        stopChar = 0;
        if(j > 0)
          fread(&charInFile, 1, 1, page); // Read '\0' varchar
        for(int l = 0; l < attributes[j].size; l++){
          fread(&charInFile, 1, 1, page);
          if(delete == 1 && charInFile == '\0')
            stopChar = 1;
            //printf("%c = %c\n", charInFile, token[l]);
          if(charInFile != token[l] && !stopChar)
            delete = 0;
        }
        if(delete && !(strcmp(attributeName, attributes[j].name))){
          fseek(page, moveItem, SEEK_SET);
          readItem.writed = 0;
          fwrite(&readItem.offset, sizeof(int), 1, page);
          fwrite(&readItem.totalLen, sizeof(int), 1, page);
          fwrite(&readItem.writed, sizeof(int), 1, page);
          printf("Attribute deleted\n");
          count++;
          break;
        }else if(j > 0)
          fseek(page, -1, SEEK_CUR); // Read '\0' varchar
      } else if(attributes[j].type == 'I'){
        if(j > 0)
          fread(&charInFile, 1, 1, page); // Read '\0' varchar
        fread(&intInFile, sizeof(int), 1, page);
        //printf("From file: %d\n", intInFile);
        inputDelete = atoi(token);
        if(intInFile != inputDelete)
          delete = 0;
        if(delete && !(strcmp(attributeName, attributes[j].name))){
          fseek(page, moveItem, SEEK_SET);
          readItem.writed = 0;
          fwrite(&readItem.offset, sizeof(int), 1, page);
          fwrite(&readItem.totalLen, sizeof(int), 1, page);
          fwrite(&readItem.writed, sizeof(int), 1, page);
          printf("Attribute deleted\n");
          count++;
          break;
        }else if(j > 0)
          fseek(page, -1, SEEK_CUR); // Read '\0' varchar
      } else if(attributes[j].type == 'V'){
        k = 0;
        if(j > 0)
          fread(&charInFile, 1, 1, page); // Read '\0' varchar
        while(charInFile != '$'){
            fread(&charInFile, 1, 1, page);
            //printf("%c = %c\n", charInFile, token[k]);
            if(delete == 1 && charInFile == '$')
              break;
            if(charInFile != token[k])
              delete = 0;
            k++;
        }
        if(delete && !(strcmp(attributeName, attributes[j].name))){
          fseek(page, moveItem, SEEK_SET);
          readItem.writed = 0;
          fwrite(&readItem.offset, sizeof(int), 1, page);
          fwrite(&readItem.totalLen, sizeof(int), 1, page);
          fwrite(&readItem.writed, sizeof(int), 1, page);
          printf("Attribute deleted\n");
          count++;
          break;
        }else if(j > 0)
          fseek(page, -1, SEEK_CUR); // Read '\0' varchar
      }
    }
    fseek(page, 0, SEEK_SET);
  }
  printf("Attributes deleted = %d\n", count);
  fseek(page, 8191, SEEK_SET);
  fread(&special, 1, 1, page);
  if(special == '1'){
    numPage++;
    fclose(page);
    deleteFrom(sql, numPage);
  }else
    fclose(page);
  //fclose(page);
}

/*A função main apresenta vários erros em relação a operação desejada.
 A função "quit" deve estar acompanhada de espaço no final para funcionar e a mensagem de erro "Something went wrong"
 aparece toda vez que "quit" é solicitada
*/

int main(){
  char sql[1000], operation[10], attributes[500];

  do{
    printf(">> ");
    fgets(sql, 1000, stdin); // le operação
    getOp(sql, operation); // função que define a operação a ser realizada

    if(strcmp(operation, "create") == 0){  
      createTable(sql);  //função que cria tabela
    }else if(strcmp(operation, "delete") == 0){
      deleteFrom(sql, 1); //função que deleta uma tupla
    }else if(strcmp(operation, "insert") == 0){
      insertInto(sql, 1);
    }else if(strcmp(operation, "select") == 0){
      selectFrom(sql, 1);
    }else{
      printf("Something went wrong");
    }
  }while(strcmp(operation, "quit") != 0);

  return 0;
}
